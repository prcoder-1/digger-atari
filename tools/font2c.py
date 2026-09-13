#!/usr/bin/env python3
"""
font2c.py - шрифт счёта из порта на БК -> данные для ANTIC F.

Источник: digger_short_font.c. Там цифры хранятся сжато: массив уникальных
строк digit_rows и массив индексов digit_indices, по двенадцать строк на
цифру. Такой шрифт занимает 192 байта вместо 360 и, в отличие от полного
шрифта заставки, взят не из ПЗУ БК, а из оригинала.

Цифра оригинала - 12 точек в ширину, а байт экрана в ANTIC F покрывает 8, и на
границу байта 12 не ложится. Поэтому цифра кладётся в ЯЧЕЙКУ шириной 16 точек
(2 байта) с полем в 2 точки слева и справа. Счёт из шести цифр займёт 96 точек
из 320 против 72 у оригинала - разница незаметна, зато вывод остаётся
байтовыровненным и непрозрачным, без предсдвинутых фаз и без маски.

ВАЖНО про порядок точек: у БК младшая точка в байте левая, у ANTIC - старшая.
Здесь источник читается по паре бит на точку слева направо, а результат
собирается старшим битом вперёд.
"""

import argparse
import os
import re
import sys

GLYPH_PX = 12           # ширина цифры в источнике
CELL_PX = 16            # ячейка на экране, 2 байта
LEFT_MARGIN = (CELL_PX - GLYPH_PX) // 2


def parse_2d(text, name, inner):
    """Читает массив вида name[N][inner] в список списков."""
    m = re.search(r'\b' + re.escape(name) + r'\s*\[[^\]]*\]\s*\[[^\]]*\]\s*=\s*\{(.*?)\}\s*;',
                  text, re.S)
    if not m:
        return None
    # Комментарии обязательно вырезать: в исходнике рядом со строками стоят
    # пояснения с числами, и без этого они попадают в данные как значения.
    body = re.sub(r'/\*.*?\*/', ' ', m.group(1), flags=re.S)
    body = re.sub(r'//[^\n]*', ' ', body)
    vals = [int(v, 0) for v in re.findall(r'0[xX][0-9a-fA-F]+|\b\d+\b', body)]
    return [vals[i:i + inner] for i in range(0, len(vals), inner)]


def row_to_cell(row_bytes):
    """Строка 12 точек по два бита -> ячейка 16 точек по одному биту, 2 байта.

    Любая ненулевая краска даёт зажжённую точку: оттенков в ANTIC F нет.
    """
    bits = 0
    for k in range(GLYPH_PX):
        b = row_bytes[k // 4] & 0xFF
        px = (b >> (2 * (k % 4))) & 3          # у БК младшая точка левая
        if px:
            bits |= 1 << (CELL_PX - 1 - LEFT_MARGIN - k)
    return [(bits >> 8) & 0xFF, bits & 0xFF]


# Буквы берутся ПРЯМО ИЗ ОРИГИНАЛА, а не через порт на БК: у того полный шрифт
# взят из ПЗУ БК и к Digger отношения не имеет. В CLib/CHAR__X.CHR лежит тот же
# шрифт, что и у цифр - 12x12 точек, три байта на строку по два бита на точку.
#
# Проверено сверкой: цифра 0 из CHAR__0.CHR и наша font_digits[0] совпадают
# точка в точку, значит шрифты действительно одного набора.
CHR_DIR = '/mnt/storage2/Games/Digger/src/CLib'


def parse_chr(path):
    """CHAR__X.CHR -> двенадцать строк по 12 точек (список из 0/1)."""
    text = open(path, encoding='latin-1').read()
    body = text.split('{', 1)[1]
    nums = [int(v) for v in re.findall(r'-?\d+', body)]
    w, h = nums[0], nums[1]             # первые два числа - ширина в байтах и высота
    out = []
    for r in range(h):
        bits = []
        for b in nums[2 + r * w: 2 + (r + 1) * w]:
            for k in range(3, -1, -1):  # четыре точки в байте, по два бита
                bits.append(1 if (b >> (k * 2)) & 3 else 0)
        out.append(bits[:GLYPH_PX])
    return out


def bits_to_cell(bits):
    """12 точек -> два байта ячейки в 16 точек, старший бит слева."""
    v = 0
    for i, bit in enumerate(bits):
        if bit:
            v |= 1 << (CELL_PX - 1 - (LEFT_MARGIN + i))
    return [(v >> 8) & 0xFF, v & 0xFF]


# Полный набор знаков для ЗАСТАВКИ. В игре его нет и не надо: там MAIN забит,
# а надписей всего несколько. Заставка живёт в своей памяти, которую потом
# занимает игра, и место там есть.
#
# Порядок знаков задаёт индекс, по которому их ищет title/text.c:
#   0..9   цифры
#   10..35 буквы A..Z
#   36     точка, 37 пробел, 38 подчёркивание
FULL_ORDER = ([('CHAR__%d' % d) for d in range(10)] +
              [('CHAR__%s' % chr(ord('A') + i)) for i in range(26)] +
              ['CHAR__PE', 'CHAR__SP', 'CHAR__UL'])


def write_full(dst):
    lines = [
        '; font.s - СГЕНЕРИРОВАНО tools/font2c.py --full, не править вручную.',
        '; ПОЛНЫЙ шрифт оригинала для заставки: CLib/CHAR__*.CHR.',
        '; Знак 12 точек в ячейке 16 точек: 2 байта на строку, 12 строк.',
        '; Один бит на точку (ANTIC F), старший бит слева.',
        ';',
        '; Порядок: цифры 0..9, буквы A..Z, точка, пробел, подчёркивание.',
        '; По нему знаки и ищет title/text.c.',
        '',
        '        .export _font_full',
        '        .segment "RODATA"',
        '',
        '_font_full:',
    ]
    for name in FULL_ORDER:
        lines.append('; --- %s ---' % name)
        for bits in parse_chr(os.path.join(CHR_DIR, '%s.CHR' % name)):
            lines.append('        .byte ' +
                         ','.join('$%02X' % v for v in bits_to_cell(bits)))
    lines.append('')
    open(dst, 'w', encoding='utf-8').write('\n'.join(lines))
    print('%s: знаков %d, %d байт' % (dst, len(FULL_ORDER), len(FULL_ORDER) * 24))
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--src', default='/home/prcoder/БК0010-develop/'
                    'digger-bk0010/digger_short_font.c')
    ap.add_argument('--asm', default='assets/font.s')
    ap.add_argument('--full', metavar='ФАЙЛ',
                    help='вместо шрифта игры собрать ПОЛНЫЙ шрифт заставки')
    a = ap.parse_args()

    if a.full:
        return write_full(a.full)

    text = open(a.src, errors='replace').read()

    rows = parse_2d(text, 'digit_rows', 3)
    idx = parse_2d(text, 'digit_indices', 12)
    if not rows or not idx:
        print('не разобраны digit_rows/digit_indices', file=sys.stderr)
        return 1

    lines = [
        '; font.s - СГЕНЕРИРОВАНО tools/font2c.py, не править вручную.',
        '; Цифры счёта из оригинала (через порт на БК-0010).',
        '; Цифра 12 точек в ячейке 16 точек: 2 байта на строку, 12 строк.',
        '; Один бит на точку (ANTIC F), старший бит слева.',
        '',
        '; Шрифт уехал под ПЗУ вместе с картами уровней: MAIN упёрся в потолок.',
        '; Переносит их copy_levels (src/util.s), сегмент общий.',
        '        .segment "LEVELS"',
        '',
        '        .export _font_digits',
        '_font_digits:',
    ]
    for d in range(10):
        lines.append('; --- %d ---' % d)
        for r in idx[d]:
            row = row_to_cell(rows[r])
            lines.append('        .byte ' + ','.join('$%02X' % v for v in row))
    lines.append('')

    # Буквы для надписи GAME OVER. ВЕСЬ алфавит (26 букв, 624 байта) сейчас не
    # влезает: в MAIN свободно около полукилобайта. Понадобятся остальные -
    # добавить их в ALPHA, место придётся искать отдельно.
    ALPHA = 'AEGMORV'
    lines += [
        '',
        '; Буквы того же шрифта, прямо из оригинала (CLib/CHAR__X.CHR).',
        '; Порядок - как в строке ниже; смотри font_letter в src/text.c.',
        '        .export _font_alpha',
        '_font_alpha:',
    ]
    for ch in ALPHA:
        lines.append('; --- %s ---' % ch)
        for bits in parse_chr(os.path.join(CHR_DIR, 'CHAR__%s.CHR' % ch)):
            lines.append('        .byte ' +
                         ','.join('$%02X' % v for v in bits_to_cell(bits)))
    print('букв: %d, всего %d байт' % (len(ALPHA), len(ALPHA) * 12 * 2))
    lines.append('')

    with open(a.asm, 'w') as f:
        f.write('\n'.join(lines) + '\n')

    print('цифр: 10, по 12 строк, 2 байта в строке = %d байт'
          % (10 * 12 * 2))
    print('asm:', a.asm)
    return 0


if __name__ == '__main__':
    sys.exit(main())
