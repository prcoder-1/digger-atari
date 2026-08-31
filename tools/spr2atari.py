#!/usr/bin/env python3
"""
spr2atari.py - спрайты оригинала Digger для PC -> данные для ANTIC E.

Что делает:
  1. Разбирает Sprites/*.SPR (см. sprlib.py).
  2. Сжимает вдвое по горизонтали (320 -> 160), по вертикали один к одному.
  3. Строит две предсдвинутые фазы (сдвиг 0 и 2 пикселя), потому что подшаг
     по X равен 2 пикселям, то есть половине байта.
  4. Раскладывает маску и изображение вперемежку по строке (msk0, img0,
     msk1, img1, ...), чтобы блиттер шёл по обоим одним индексным регистром.
     Порядок именно "маска, потом изображение": тогда внутренний цикл -
     это lda (src),y / and экран,x / ora (src),y / sta экран,x, то есть
     25 тактов на байт без временных ячеек.
  5. Выдаёт .s для ca65 и контрольный лист PNG для сверки глазами.

Модель вывода (как в оригинале и в порте на БК):
    экран = (экран И маска) ИЛИ изображение
Маска содержит 3 (все биты) там, где виден фон, и 0 там, где спрайт или его
ореол. Пиксели с изображением 0 и маской 0 - это ореол: он гасится в чёрный.

Правило сжатия: два соседних пикселя оригинала дают один. Для изображения
берётся первый ненулевой (силуэт важнее оттенка), для покрытия - объединение.
Инвариант "изображение != 0 => маска == 0" при этом сохраняется.
"""

import argparse
import glob
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sprlib

SUBSTEP_PX = 2          # подшаг по X в пикселях = половина байта

# Порог разделения на два сегмента: столько байт уходит в первый.
# Под ПЗУ доступно $E400-$FFF9 (7162 байта) и $D800-$DFFF (2048).
# Ёмкость двух кусков ОЗУ под ПЗУ. Разделение считается точно: в первый
# сегмент набирается столько спрайтов, сколько влезает целиком, остальное
# уходит во второй. Порог "на глаз" тут не работает - зазор всего десятки байт.
HIDDEN_RAM_BYTES  = 0xFFFA - 0xE400     # 7162
HIDDEN_RAM2_BYTES = 0x0800              # 2048

# Значок жизни второго игрока и пустой значок: режим на двух игроков в порт
# не входит. Экономия нужна буквально - данные не помещались под ПЗУ.
SKIP = ('s1', 's2')
PHASES = 2              # фазы сдвига: 0 и 2 пикселя


def halve(image, outln):
    """Сжимает спрайт вдвое по горизонтали, возвращает (image, outln)."""
    out_img, out_msk = [], []
    for y in range(len(image)):
        ri, rm = [], []
        src_i, src_m = image[y], outln[y]
        for x in range(0, len(src_i), 2):
            a, b = src_i[x], src_i[x + 1]
            # покрытие = объединение (маска 0 = закрыто спрайтом или ореолом)
            covered = (src_m[x] == 0) or (src_m[x + 1] == 0)
            ri.append(a if a else b)
            rm.append(0 if covered else 3)
        out_img.append(ri)
        out_msk.append(rm)
    return out_img, out_msk


def shift_rows(rows, shift_px, out_w_px, fill):
    """Сдвигает строки вправо на shift_px, дополняя до out_w_px."""
    out = []
    for row in rows:
        r = [fill] * shift_px + list(row)
        r += [fill] * (out_w_px - len(r))
        out.append(r[:out_w_px])
    return out


def build_phases(image, outln):
    """Строит список фаз: [(байт_в_строке, данные)] для сдвигов 0 и 2 px."""
    w = len(image[0])
    phases = []
    for ph in range(PHASES):
        shift = ph * SUBSTEP_PX
        # ширина в байтах с запасом на сдвиг, кратно 4 пикселям
        out_w_px = ((w + shift + 3) // 4) * 4
        img = shift_rows(image, shift, out_w_px, 0)   # вне спрайта - чёрное
        msk = shift_rows(outln, shift, out_w_px, 3)   # вне спрайта - фон виден
        ib = sprlib.pack_antice(img)
        mb = sprlib.pack_antice(msk)
        bpr = out_w_px // 4
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

    Клетка поля - 10 пикселей, в байте 4 пикселя, поэтому наименьший
    байтовыровненный блок это НОК(10,4) = 20 пикселей = 5 байт = две клетки.
    Поле шириной 40 байт заполняется восемью такими блоками подряд.
    Маска здесь не нужна: земля выводится непрозрачно.
    """
    lines = [
        '; earth.s - СГЕНЕРИРОВАНО tools/spr2atari.py, не править вручную.',
        '; Текстуры земли (спрайты p1..p8 оригинала, 20x4 px -> 10x4 px).',
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
        rows = s['small']                      # 10 px x 4 строки
        doubled = [r + r for r in rows]        # 20 px = 5 байт
        data = sprlib.pack_antice(doubled)
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


def emit_asm(sprites, path):
    lines = [
        '; sprites.s - СГЕНЕРИРОВАНО tools/spr2atari.py, не править вручную.',
        '; Источник: оригинальные спрайты Digger для PC (Sprites/*.SPR),',
        '; сжатые вдвое по горизонтали под ANTIC E.',
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
    # Ничего не выбрасывается: под ПЗУ всего 9210 байт, а спрайтов чуть
    # больше, и остаток спокойно живёт в обычной памяти.
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
        lines.append(f'; --- {name}: {s["w"]} x {h} px ---')
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
    """Контрольный лист: слева оригинал, справа результат ÷2."""
    try:
        from PIL import Image, ImageDraw
    except ImportError:
        print('PIL не установлен - контрольный лист пропущен', file=sys.stderr)
        return
    # Палитра листа обязана совпадать с игровой, иначе сверять спрайты по
    # нему бессмысленно. Значения - примерные RGB для цветов Atari,
    # заданных в src/hw.h: $CA зелёный, $38 красный, $1C жёлтый.
    pal = [(0, 0, 0), (76, 200, 76), (210, 80, 60), (232, 190, 90)]
    cols = 8
    cw, chh = 40, 30
    rows = (len(sprites) + cols - 1) // cols
    im = Image.new('RGB', (cols * cw * scale, rows * chh * scale), (24, 24, 24))
    d = ImageDraw.Draw(im)
    for i, s in enumerate(sprites):
        cx = (i % cols) * cw * scale
        cy = (i // cols) * chh * scale
        # оригинал
        for y, row in enumerate(s['orig']):
            for x, v in enumerate(row):
                d.rectangle([cx + x * scale, cy + y * scale,
                             cx + (x + 1) * scale - 1, cy + (y + 1) * scale - 1],
                            fill=pal[v])
        # результат, правее
        ox = cx + (len(s['orig'][0]) + 2) * scale
        for y, row in enumerate(s['small']):
            for x, v in enumerate(row):
                d.rectangle([ox + x * scale, cy + y * scale,
                             ox + (x + 1) * scale - 1, cy + (y + 1) * scale - 1],
                            fill=pal[v])
    im.save(path)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--src', default='/mnt/storage2/Games/Digger/src/Sprites')
    ap.add_argument('--asm', default='assets/sprites.s')
    ap.add_argument('--earth', default='assets/earth.s')
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
        img, msk = halve(s.image, s.outln)
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
    contact_sheet(sprites, a.sheet)

    total = sum(len(d) for s in sprites for _, d in s['phases'])
    print(f'спрайтов: {len(sprites)}')
    print(f'данных (обе фазы, изображение+маска): {total} байт')
    print(f'текстур земли: {n_earth}')
    print(f'asm:   {a.asm}')
    print(f'земля: {a.earth}')
    print(f'лист:  {a.sheet}')


if __name__ == '__main__':
    main()
