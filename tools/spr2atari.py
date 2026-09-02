#!/usr/bin/env python3
"""
spr2atari.py - спрайты оригинала Digger для PC -> данные для ANTIC F.

Что делает:
  1. Разбирает Sprites/*.SPR (см. sprlib.py).
  2. Переводит четыре краски CGA в одну: 320 точек оригинала ложатся в
     320 точек ANTIC F один к одному, сжимать ничего не нужно.
  3. Строит две предсдвинутые фазы (сдвиг 0 и 4 точки), потому что подшаг
     по X равен 4 точкам, то есть половине байта.
  4. Раскладывает маску и изображение вперемежку по строке (msk0, img0,
     msk1, img1, ...), чтобы блиттер шёл по обоим одним индексным регистром.
     Порядок именно "маска, потом изображение": тогда внутренний цикл -
     это lda (src),y / and экран,x / ora (src),y / sta экран,x, то есть
     25 тактов на байт без временных ячеек.
  5. По ключу --pmg дополнительно выдаёт кадры Диггера для аппаратных
     спрайтов - там своя упаковка, см. emit_pmg().
  6. Выдаёт .s для ca65 и контрольный лист PNG для сверки глазами.

Модель вывода (как в оригинале и в порте на БК):
    экран = (экран И маска) ИЛИ изображение
Маска содержит единицы там, где виден фон, и нули там, где спрайт или его
ореол. Точки с изображением 0 и маской 0 - это ореол: он гасится в чёрный.
Ореол важен вдвойне при одной краске: фигура одного цвета с землёй, и
отделяет её от фона именно чёрная кайма.

Правило перевода в одну краску разное для фигур и для земли, см. monochrome()
и threshold_tile() - там же объяснено, почему одного правила не хватило.
"""

import argparse
import glob
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sprlib

SUBSTEP_PX = 4          # подшаг по X в точках = половина байта
PX_PER_BYTE = 8         # один бит на точку

# Ёмкость двух кусков ОЗУ под ПЗУ. Разделение считается точно: в первый
# сегмент набирается столько спрайтов, сколько влезает целиком, остальное
# уходит во второй. Порог "на глаз" тут не работает - зазор всего десятки байт.
HIDDEN_RAM_BYTES  = 0xFFFA - 0xE400     # 7162
HIDDEN_RAM2_BYTES = 0x0800              # 2048

# Значок жизни второго игрока и пустой значок: режим на двух игроков в порт
# не входит. Экономия нужна буквально - данные не помещались под ПЗУ.
SKIP = ('s1', 's2')
PHASES = 2              # фазы сдвига: 0 и 4 точки

# Кадры Диггера: четыре направления по три кадра анимации.
MAN_FRAMES = ('r0', 'r1', 'r2', 'l0', 'l1', 'l2',
              'd0', 'd1', 'd2', 'u0', 'u1', 'u2')

# Кадры врагов: Ноббин (m) один облик на все стороны, Хоббин вправо (c) и
# влево (a). Подписи взяты из исходников оригинала, см. шапку src/bugs.c.
BUG_FRAMES = ('m0', 'm1', 'm2', 'c0', 'c1', 'c2', 'a0', 'a1', 'a2')


def pack(rows):
    """Матрица нулей и единиц -> байты ANTIC F, старший бит слева."""
    out = bytearray()
    for row in rows:
        assert len(row) % PX_PER_BYTE == 0, 'ширина должна быть кратна 8'
        for bx in range(0, len(row), PX_PER_BYTE):
            b = 0
            for p in range(PX_PER_BYTE):
                if row[bx + p]:
                    b |= 1 << (PX_PER_BYTE - 1 - p)
            out.append(b)
    return bytes(out)


def threshold_tile(rows):
    """Текстура земли 20x4 в четыре краски -> одна краска с сохранением узора.

    Для фигур годится силуэт (любая ненулевая краска - точка), но для земли
    нет: в оригинале текстура набрана красками 1..3 и НИ ОДНОГО чёрного
    пикселя не содержит, поэтому силуэт заливает клетку сплошняком и узор
    пропадает совсем.

    Порог берётся по самой светлой краске, встречающейся в ЭТОЙ текстуре, а не
    общий на все восемь: каждая набрана своей парой красок (p1 - это 2 и 3,
    p2 - 1 и 2), и любой общий порог гасит часть текстур в ноль.
    """
    top = max(v for row in rows for v in row)
    return [[1 if v == top else 0 for v in row] for row in rows]


def monochrome(image, outln):
    """Четыре краски -> одна. Для фигур, в отличие от земли, годится силуэт.

    Проверено по всем 77 спрайтам: медиана заполнения 35%, то есть фигуры
    оригинала нарисованы штрихами по чёрному, а не залиты. Силуэт сохраняет
    рисунок; сплошными выходят только текстуры земли, у которых своё правило.
    """
    img = [[1 if v else 0 for v in row] for row in image]
    msk = [[0 if m == 0 else 1 for m in row] for row in outln]
    return img, msk


def shift_rows(rows, shift_px, out_w_px, fill):
    """Сдвигает строки вправо на shift_px, дополняя до out_w_px."""
    out = []
    for row in rows:
        r = [fill] * shift_px + list(row)
        r += [fill] * (out_w_px - len(r))
        out.append(r[:out_w_px])
    return out


def build_phases(image, outln):
    """Строит список фаз: [(байт_в_строке, данные)] для сдвигов 0 и 4 точки."""
    w = len(image[0])
    phases = []
    for ph in range(PHASES):
        shift = ph * SUBSTEP_PX
        out_w_px = ((w + shift + PX_PER_BYTE - 1) // PX_PER_BYTE) * PX_PER_BYTE
        img = shift_rows(image, shift, out_w_px, 0)   # вне спрайта - чёрное
        msk = shift_rows(outln, shift, out_w_px, 1)   # вне спрайта - фон виден
        ib = pack(img)
        mb = pack(msk)
        bpr = out_w_px // PX_PER_BYTE
        # чередование msk/img по строке (маска первой - см. шапку файла)
        data = bytearray()
        for y in range(len(img)):
            for bx in range(bpr):
                data.append(mb[y * bpr + bx])
                data.append(ib[y * bpr + bx])
        phases.append((bpr, bytes(data)))
    return phases


def emit_earth(sprites, path):
    """Текстуры земли p1..p8 отдельным файлом, как повторяющийся блок.

    Клетка поля - 20 точек, в байте 8, поэтому наименьший байтовыровненный
    блок это НОК(20,8) = 40 точек = 5 байт = две клетки. Поле шириной 40 байт
    заполняется восемью такими блоками подряд.
    Маска здесь не нужна: земля выводится непрозрачно.
    """
    lines = [
        '; earth.s - СГЕНЕРИРОВАНО tools/spr2atari.py, не править вручную.',
        '; Текстуры земли (спрайты p1..p8 оригинала, 20x4 точки).',
        '; Каждая - блок 5 байт x 4 строки, повторяется по горизонтали 8 раз',
        '; и по вертикали на всю высоту поля.',
        ';',
        '; Лежит в RODATA, а не вместе со спрайтами: под ПЗУ операционной',
        '; системы места впритык, а текстуры земли мелкие.',
        '',
        '        .segment "RODATA"',
        '',
    ]
    names = []
    for s in sprites:
        if not (len(s['name']) == 2 and s['name'][0] == 'p'
                and s['name'][1].isdigit()):
            continue
        rows = threshold_tile(s['orig'])       # 20 точек x 4 строки
        doubled = [r + r for r in rows]        # две клетки = 5 байт
        data = pack(doubled)
        sym = '_earth_' + s['name']
        names.append(sym)
        lines.append(f'        .export {sym}')
        lines.append(f'{sym}:            ; 5 байт x {len(rows)} строк')
        for y in range(len(rows)):
            row = data[y * 5:(y + 1) * 5]
            lines.append('        .byte ' + ','.join(f'${b:02X}' for b in row))
        lines.append('')

    # таблица указателей, чтобы выбирать текстуру по номеру экрана
    lines.append('        .export _earth_tbl')
    lines.append('_earth_tbl:')
    for sym in sorted(names):
        lines.append(f'        .word {sym}')
    lines.append('')

    with open(path, 'w') as f:
        f.write('\n'.join(lines) + '\n')
    return len(names)


def erode(mask):
    """Убирает у области кайму в одну точку по четырём сторонам."""
    h, w = len(mask), len(mask[0])
    out = [[0] * w for _ in range(h)]
    for y in range(h):
        for x in range(w):
            if not mask[y][x]:
                continue
            if x == 0 or y == 0 or x == w - 1 or y == h - 1:
                continue
            if mask[y][x - 1] and mask[y][x + 1] \
                    and mask[y - 1][x] and mask[y + 1][x]:
                out[y][x] = 1
    return out


def downsample(mask, rule):
    """16 точек -> 8 бит: ячейка игрока накрывает ровно две точки."""
    out = []
    for row in mask:
        b = 0
        for i in range(8):
            a, c = row[2 * i], row[2 * i + 1]
            hit = (a and c) if rule != 'or' else (a or c)
            if hit:
                b |= 1 << (7 - i)
        out.append(b)
    return out


def emit_pmg(sprites, path, fill_rule):
    """Кадры Диггера и врагов для аппаратных спрайтов: планы заливки по 15 байт.

    Заливка ПОДКЛАДЫВАЕТСЯ ПОД рисунок, а не заменяет его. В кадровом буфере
    Диггер по-прежнему выводится целиком на 320 точках; игроки красят его
    изнутри, а по краю остаётся видна линия из кадрового буфера - в ней и
    держится чёткость.

    Почему именно так, а не игрок под полем: измерено, что поле высокого
    разрешения участвует в разборе приоритетов целиком как COLPF2. При
    приоритете поля над игроками ($04) зажжённые точки под игроком теряют
    яркость COLPF1 и становятся фоном, то есть рисунок стирается. Положить
    игрока под контур аппаратура не даёт.

    Ячейка игрока - две точки (такт цвета), отсюда три правила сжатия:
      erode - сначала у области снимается кайма в одну точку, потом пара точек
              даёт бит, только если обе внутри. Линия по краю гарантированно
              остаётся видна, но тонкие детали пропадают совсем.
      and   - бит, если обе точки внутри. Край выживает через раз, по чётности.
      or    - бит, если хоть одна. Цвет накрывает всю область вместе с краем.

    Правило у всех планов 'and'. Пробовали красить колёса по 'or', чтобы они
    тоже получили цвет: колёса нарисованы кольцами в одну точку толщиной, и
    заливка накрывает их целиком, превращая в глухие квадраты. Кольцо шириной
    четыре точки при ячейке игрока в две точки покрасить, не потеряв форму,
    нельзя вообще - ни заливкой, ни заливкой с эрозией.

    Игроков всего четыре, и они поделены между Диггером и врагами:

      Диггер  план 0 - корпус (краска 2), план 1 - клюв (краска 1)
      враги   план 0 - весь силуэт, один оттенок на фигуру

    Колёса Диггера (краска 3) свой план потеряли: два игрока ушли врагам, а
    из трёх красок колёса красились хуже всех - см. выше. Теперь они идут в
    цвете поля, зато со своей формой.

    У врагов план один на всю фигуру: делить их на краски бессмысленно, потому
    что при сжатии вдвое от мелких частей остаются считанные биты. Силуэт по
    правилу 'and' держится внутри чёрного ореола и на землю не вылезает.
    """
    MAN_PLANES = (((2,), 'and'), ((1,), 'and'))
    BUG_PLANES = (((1, 2, 3), 'and'),)
    GROUPS = ((MAN_FRAMES, MAN_PLANES, 'Диггера'),
              (BUG_FRAMES, BUG_PLANES, 'врагов'))
    lines = [
        '; pmspr.s - СГЕНЕРИРОВАНО tools/spr2atari.py, не править вручную.',
        '; Заливка для аппаратных спрайтов, правило сжатия: %s' % fill_rule,
        '; По байту на строку растра, 8 бит на 16 точек.',
        '; Диггер: план 0 - корпус (краска 2), план 1 - клюв (краска 1).',
        '; Враги:  план 0 - весь силуэт, один оттенок на фигуру.',
        '',
        '        .segment "RODATA"',
        '',
    ]
    n = 0
    by_name = {s['name']: s for s in sprites}
    for frames, planes, who in GROUPS:
        for name in frames:
            s = by_name.get(name)
            if s is None:
                print('нет кадра %s: %s' % (who, name), file=sys.stderr)
                continue
            for pl, (colors, rule) in enumerate(planes):
                if fill_rule is not None:
                    rule = fill_rule        # общий ключ перебивает умолчания
                area = [[1 if v in colors else 0 for v in row]
                        for row in s['orig']]
                if rule == 'erode':
                    area = erode(area)
                data = downsample(area, rule)
                sym = '_pm_%s_%d' % (name, pl)
                lines.append('        .export %s' % sym)
                lines.append('%s:            ; %d строк' % (sym, len(data)))
                for b in data:
                    lines.append('        .byte $%02X' % b)
                lines.append('')
                n += 1
    with open(path, 'w') as f:
        f.write('\n'.join(lines) + '\n')
    return n


def emit_asm(sprites, path):
    lines = [
        '; sprites.s - СГЕНЕРИРОВАНО tools/spr2atari.py, не править вручную.',
        '; Источник: оригинальные спрайты Digger для PC (Sprites/*.SPR),',
        '; переведённые в одну краску под ANTIC F. По горизонтали один к',
        '; одному: и там и здесь 320 точек в строке.',
        ';',
        '; Раскладка каждой фазы: по строке идут пары (маска, изображение),',
        '; всего <байт_в_строке> пар на строку, <строк> строк.',
        '; Вывод: экран = (экран И маска) ИЛИ изображение.',
        '',
        '; Данные разложены по трём сегментам. Под ПЗУ операционной системы',
        '; свободны два НЕСМЕЖНЫХ куска ОЗУ ($E400-$FFF9 и $D800-$DFFF), между',
        '; ними лежит знакогенератор, поэтому одним сегментом их не занять.',
        '; Имена SHADOW_RAM и SHADOW_RAM2 заданы библиотекой cc65: её процедура',
        '; копирования под ПЗУ ссылается на __SHADOW_RAM_LOAD__ и подобные.',
        '; Что не влезло под ПЗУ, уходит в RODATA, то есть в обычную память.',
        '',
        '        .segment "SHADOW_RAM"',
        '',
    ]
    # Куда попадёт каждый спрайт: набираем первый сегмент под завязку
    seg1, used = set(), 0
    for s in sprites:
        n = sum(len(d) for _, d in s['phases'])
        if used + n <= HIDDEN_RAM_BYTES:
            seg1.add(s['name'])
            used += n
    # Что не влезло в первый - во второй, а что не влезло и туда - в MAIN.
    seg2, used2 = set(), 0
    for s in sprites:
        if s['name'] in seg1:
            continue
        n = sum(len(d) for _, d in s['phases'])
        if used2 + n <= HIDDEN_RAM2_BYTES:
            seg2.add(s['name'])
            used2 += n

    def seg_of(name):
        if name in seg1:
            return 'SHADOW_RAM'
        if name in seg2:
            return 'SHADOW_RAM2'
        return 'RODATA'

    cur_seg = 'SHADOW_RAM'
    for s in sprites:
        name, h, phases = s['name'], s['h'], s['phases']
        want = seg_of(name)
        if want != cur_seg:
            cur_seg = want
            lines.append('        .segment "%s"' % want)
            lines.append('')
        lines.append(f'; --- {name}: {s["w"]} x {h} точек ---')
        for ph, (bpr, data) in enumerate(phases):
            sym = f'_spr_{name}_p{ph}'
            lines.append(f'        .export {sym}')
            lines.append(f'{sym}:            ; {bpr} байт/строку, {h} строк')
            for y in range(h):
                row = data[y * bpr * 2:(y + 1) * bpr * 2]
                vals = ','.join(f'${b:02X}' for b in row)
                lines.append(f'        .byte {vals}')
            lines.append('')
    with open(path, 'w') as f:
        f.write('\n'.join(lines) + '\n')


def contact_sheet(sprites, path, scale=4):
    """Контрольный лист: слева оригинал в красках CGA, справа результат."""
    try:
        from PIL import Image, ImageDraw
    except ImportError:
        print('PIL не установлен - контрольный лист пропущен', file=sys.stderr)
        return
    # Правая половина рисуется цветами ИГРЫ, иначе сверять по листу
    # бессмысленно: янтарный монохром из src/hw.h - COL_FIELD_PF2 (оттенок 1,
    # яркость 2) и COL_FIELD_PF1 (яркость 14). Значения сняты со снимка кадра
    # в atari800 PAL, а не пересчитаны из байтов цвета.
    pal_src = [(0, 0, 0), (60, 175, 60), (190, 70, 55), (205, 150, 60)]
    pal_dst = [(66, 16, 0), (255, 206, 148)]
    cols = 8
    cw, chh = 40, 30
    rows = (len(sprites) + cols - 1) // cols
    im = Image.new('RGB', (cols * cw * scale, rows * chh * scale), (24, 24, 24))
    d = ImageDraw.Draw(im)
    for i, s in enumerate(sprites):
        cx = (i % cols) * cw * scale
        cy = (i // cols) * chh * scale
        for y, row in enumerate(s['orig']):
            for x, v in enumerate(row):
                d.rectangle([cx + x * scale, cy + y * scale,
                             cx + (x + 1) * scale - 1, cy + (y + 1) * scale - 1],
                            fill=pal_src[v])
        # результат, правее
        ox = cx + (len(s['orig'][0]) + 2) * scale
        for y, row in enumerate(s['small']):
            for x, v in enumerate(row):
                d.rectangle([ox + x * scale, cy + y * scale,
                             ox + (x + 1) * scale - 1, cy + (y + 1) * scale - 1],
                            fill=pal_dst[v])
    im.save(path)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--src', default='/mnt/storage2/Games/Digger/src/Sprites')
    ap.add_argument('--asm', default='assets/sprites.s')
    ap.add_argument('--earth', default='assets/earth.s')
    ap.add_argument('--pmg', default='assets/pmspr.s')
    ap.add_argument('--fill', default=None,
                    choices=('erode', 'and', 'or'),
                    help='одно правило сжатия на все планы; по умолчанию у '
                         'каждого плана своё, см. emit_pmg()')
    ap.add_argument('--sheet', default='build/sprites_sheet.png')
    a = ap.parse_args()

    sprites = []
    for f in sorted(glob.glob(os.path.join(a.src, '*.SPR'))):
        s = sprlib.load(f)
        if s is None:
            print('пропущен (не разобран):', f, file=sys.stderr)
            continue
        if s.name in SKIP:
            continue
        img, msk = monochrome(s.image, s.outln)
        sprites.append({
            'name': s.name,
            'w': len(img[0]),
            'h': len(img),
            'orig': s.image,
            'small': img,
            'phases': build_phases(img, msk),
        })

    os.makedirs(os.path.dirname(a.asm), exist_ok=True)
    os.makedirs(os.path.dirname(a.sheet), exist_ok=True)
    emit_asm(sprites, a.asm)
    n_earth = emit_earth(sprites, a.earth)
    n_pmg = emit_pmg(sprites, a.pmg, a.fill)
    contact_sheet(sprites, a.sheet)

    total = sum(len(d) for s in sprites for _, d in s['phases'])
    print(f'спрайтов: {len(sprites)}')
    print(f'данных (обе фазы, изображение+маска): {total} байт')
    print(f'текстур земли: {n_earth}')
    print(f'планов PMG: {n_pmg} (заливка: {a.fill or "по умолчанию"})')
    print(f'asm:   {a.asm}')
    print(f'земля: {a.earth}')
    print(f'PMG:   {a.pmg}')
    print(f'лист:  {a.sheet}')


if __name__ == '__main__':
    main()
