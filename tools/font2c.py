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


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--src', default='/home/prcoder/БК0010-develop/'
                    'digger-bk0010/digger_short_font.c')
    ap.add_argument('--asm', default='assets/font.s')
    a = ap.parse_args()

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
        '        .segment "RODATA"',
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

    with open(a.asm, 'w') as f:
        f.write('\n'.join(lines) + '\n')

    print('цифр: 10, по 12 строк, 2 байта в строке = %d байт'
          % (10 * 12 * 2))
    print('asm:', a.asm)
    return 0


if __name__ == '__main__':
    sys.exit(main())
