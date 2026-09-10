#!/usr/bin/env python3
"""
ca65_to_mos.py - перевод наших ассемблерных файлов с ca65 на llvm-mos.

    tools/ca65_to_mos.py <вход.s> <выход.S>

Зачем переводчик, а не второй комплект файлов: источник правды должен остаться
один. Ассемблер порта - это блиттер с самомодификацией, прерывания и звуковой
движок, и держать их в двух редакциях значит однажды поправить только одну.

Словарь директив у нас крошечный (.byte .word .res .export .import .importzp
.segment .macro .endmacro .ifndef .endif), поэтому перевод получается полным, а
не приблизительным. Всё, чего переводчик не знает, он не пропускает молча, а
роняет сборку - незамеченный пропуск здесь дороже остановки.

Правила перевода, проверенные разведкой (doc/llvm-mos.md):

    .segment "ZEROPAGE"  -> .section .zp.bss + директива .zeropage на имена
    .segment "CODE"      -> .section .text
    .export / .import    -> .globl / ничего (внешнее имя и так внешнее)
    .importzp имя        -> .zeropage имя
    .res N               -> .zero N
    .macro/.endmacro     -> .macro/.endm, параметры внутри тела через \\имя
    _имя                 -> имя (у cc65 символы из Си с подчёркиванием)
"""
import re
import sys

SEGMENTS = {
    'CODE':       '.section .text,"ax",@progbits',
    'RODATA':     '.section .rodata,"a",@progbits',
    'DATA':       '.section .data,"aw",@progbits',
    'LEVELS':     '.section .rodata.levels,"a",@progbits',
    # Спрайты уезжают под ПЗУ: адрес исполнения там, адрес загрузки в обычном
    # ОЗУ, перенос делает mos/init.S.
    'SHADOW_RAM':  '.section .romdata,"a",@progbits',
    'SHADOW_RAM2': '.section .romdata2,"a",@progbits',
    # Области, которые только резервируют адреса и ничем не заполняются.
    'ZEROPAGE':   '.section .zp.bss,"aw",@nobits',
    # Отдельного BSS_HI у llvm-mos нет: места хватает, всё идёт в обычную .bss,
    # которую стартовый код чистит сам.
    'BSS_HI':     '.section .bss,"aw",@nobits',
    'SCRBUF':     '.section .scrbuf,"aw",@nobits',
    'DLIST':      '.section .dlist,"aw",@nobits',
    'ROWTAB':     '.section .rowtab,"aw",@nobits',
    'PMGBUF':     '.section .pmgbuf,"aw",@nobits',
}

# Имена, которых у llvm-mos нет: подчёркивание снимается со всех символов из
# Си, но эти пришли из потрохов cc65 и подставляются поимённо.
# Постоянные, которые в сборке llvm-mos значат другое. XITVBV - это выход из
# кадрового прерывания в ПЗУ ОС; ПЗУ у нас выключено, а по $E462 лежат спрайты,
# поэтому выход берётся из своей заглушки (mos/nmi.S).
CONSTS = {
    'XITVBV': 'mos_xitvbv',
}

RENAME = {
    'ptr1': 'mos_ptr',                  # рабочий указатель, свой (см. mos/init.S)
    '__BSS_HI_RUN__':  '__bss_hi_start',
    '__BSS_HI_SIZE__': '__bss_hi_size',
}

KNOWN = {'.byte', '.word', '.addr', '.res', '.export', '.import', '.importzp',
         '.segment', '.macro', '.endmacro', '.ifndef', '.ifdef', '.endif',
         '.else', '.align', '.zeropage'}


def anon_labels(text):
    """Безымянные метки cc65 (":" как определение, ":+" как ссылка вперёд) в
    llvm-mos не поддерживаются. Каждому определению даётся своё имя вида
    .Lanon<N>, а ссылка ":+" указывает на ближайшее следующее.

    Обратная ссылка ":-" у нас не встречается; если появится, перевод падает,
    а не выдаёт что-то похожее."""
    lines = text.splitlines()
    defs = [i for i, l in enumerate(lines)
            if re.match(r'^:(\s|$)', l.partition(';')[0])]
    for i, l in enumerate(lines):
        code, sep, comment = l.partition(';')
        if ':-' in code:
            raise SystemExit('строка %d: ссылка назад ":-" не поддерживается' % (i + 1))
        if ':+' in code:
            nxt = [d for d in defs if d > i]
            if not nxt:
                raise SystemExit('строка %d: ":+" не на что сослаться' % (i + 1))
            code = code.replace(':+', '.Lanon%d' % nxt[0])
        if i in defs:
            code = re.sub(r'^:', '.Lanon%d:' % i, code)
        lines[i] = code + sep + comment
    return '\n'.join(lines)


def drop_aliases(text):
    """У cc65 символ, видимый из Си, пишется с подчёркиванием, и в ассемблере
    заводят псевдоним "_имя = имя". У llvm-mos имя одно на оба языка, и после
    снятия подчёркивания такая строка превращается в "имя = имя" - ассемблер
    справедливо ругается на переопределение. Псевдонимы просто убираются, а
    сама метка становится глобальной по .export."""
    out = []
    for l in text.splitlines():
        code = l.partition(';')[0]
        m = re.match(r'^\s*_(\w+)\s*=\s*(\w+)\s*$', code)
        if m and m.group(1) == m.group(2):
            out.append('; ' + l + '   <- псевдоним снят при переводе')
        else:
            out.append(l)
    return '\n'.join(out)


def translate(text):
    text = drop_aliases(text)
    text = anon_labels(text)
    out = []
    zp_names = []           # имена из ZEROPAGE - им нужна директива .zeropage
    in_zp = False
    macro_params = []
    pending_zp_import = []

    for lineno, raw in enumerate(text.splitlines(), 1):
        line = raw

        # Комментарий ca65 начинается с ';' - в llvm-mos он тоже ';'
        code, sep, comment = line.partition(';')

        # Метка может стоять на одной строке с директивой (bl_x: .res 1),
        # поэтому она отрезается, а остаток разбирается как обычная строка.
        lm = re.match(r'^([.\w]+:)\s*(.*)$', code)   # .Lanon... тоже метка
        label = ''
        if lm:
            label, code = lm.group(1), lm.group(2)
            if in_zp:
                zp_names.append(label[:-1])
            if code:
                code = ' ' + code       # чтобы отступ ниже не съел директиву

        m = re.match(r'^(\s*)\.(\w+)\s*(.*)$', code)
        if m:
            indent, d, args = m.group(1), '.' + m.group(2), m.group(3).rstrip()
            if d not in KNOWN:
                raise SystemExit('строка %d: неизвестная директива %s - '
                                 'переводчик её не знает, допиши правило' % (lineno, d))
            if d == '.segment':
                name = args.strip().strip('"')
                if name not in SEGMENTS:
                    raise SystemExit('строка %d: неизвестный сегмент %s' % (lineno, name))
                in_zp = (name == 'ZEROPAGE')
                code = indent + SEGMENTS[name]
            elif d == '.export':
                code = indent + '.globl ' + args
            elif d == '.import':
                code = indent + '; .import ' + args      # внешнее имя и так внешнее
            elif d == '.importzp':
                pending_zp_import += [a.strip() for a in args.split(',')]
                code = indent + '.zeropage ' + args
            elif d == '.res':
                code = indent + '.zero ' + args
            elif d == '.macro':
                parts = args.split(None, 1)
                macro_params = ([p.strip() for p in parts[1].split(',')]
                                if len(parts) > 1 else [])
                code = indent + '.macro ' + args
            elif d == '.endmacro':
                macro_params = []
                code = indent + '.endm'
            elif d == '.addr':
                code = indent + '.short ' + args

        # Внутри тела макроса параметры пишутся через обратную косую.
        # Сама строка .macro не в счёт: там имена объявляются, а не подставляются.
        if macro_params and not re.match(r'^\s*\.macro\b', code):
            for p in macro_params:
                code = re.sub(r'(?<![\w\\])%s\b' % re.escape(p), '\\\\' + p, code)

        # Ассемблер llvm-mos видит "/*" даже внутри строчного комментария и
        # начинает блочный. Пробел разводит их без потери смысла.
        comment = comment.replace('/*', '/ *').replace('*/', '* /')
        out.append(label + code + sep + comment)

    text = '\n'.join(out) + '\n'

    # Символы из Си: у cc65 с ведущим подчёркиванием, у llvm-mos без него.
    # Двойное подчёркивание не трогаем - это символы линковщика.
    text = re.sub(r'(?<![\w.])_(?=[A-Za-z])', '', text)

    # Постоянная переопределяется целиком: "XITVBV = $E462" -> ссылка на свою
    # заглушку. Значение в исходнике при этом не трогается - там cc65.
    for name, repl in CONSTS.items():
        text = re.sub(r'^(\s*)%s(\s*)=.*$' % re.escape(name),
                      r'\g<1>%s\g<2>= %s   ; переопределено переводчиком' % (name, repl),
                      text, flags=re.M)

    for a, b in RENAME.items():
        text = re.sub(r'(?<![\w.])%s\b' % re.escape(a), b, text)

    if zp_names:
        decl = '\n'.join('        .zeropage ' + n for n in zp_names)
        text = ('; Имена нулевой страницы объявлены явно: адрес известен только\n'
                '; линковщику, и без .zeropage ассемблер возьмёт двухбайтную\n'
                '; адресацию (см. doc/llvm-mos.md).\n' + decl + '\n\n' + text)
    return text


def main():
    if len(sys.argv) != 3:
        raise SystemExit(__doc__)
    src = open(sys.argv[1], encoding='utf-8').read()
    open(sys.argv[2], 'w', encoding='utf-8').write(
        '; СГЕНЕРИРОВАНО tools/ca65_to_mos.py из %s - не править вручную.\n'
        % sys.argv[1] + translate(src))


if __name__ == '__main__':
    main()
