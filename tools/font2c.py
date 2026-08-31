#!/usr/bin/env python3
"""
font2c.py - шрифт счёта из порта на БК -> данные для ANTIC E.

Источник: digger_short_font.c. Там цифры хранятся сжато: массив уникальных
строк digit_rows и массив индексов digit_indices, по двенадцать строк на
цифру. Такой шрифт занимает 192 байта вместо 360 и, в отличие от полного
шрифта заставки, взят не из ПЗУ БК, а из оригинала.

Цифра - 12x12 пикселей, то есть 3 байта на строку.

ВАЖНО про порядок пикселей: у БК младший пиксель в байте левый, у ANTIC E -
старший. Для спрайтов оригинала перестановка не требовалась, потому что там
упаковка CGA совпадает с ANTIC E, а здесь источник другой и биты надо
переставлять.
"""

import argparse
import re
import sys


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


def swap_pixels(b):
    """Меняет порядок четырёх двухбитных пикселей в байте на обратный."""
    return (((b >> 0) & 3) << 6) | (((b >> 2) & 3) << 4) | \
           (((b >> 4) & 3) << 2) | (((b >> 6) & 3) << 0)


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
        '; Каждая цифра - 12x12 пикселей: 3 байта на строку, 12 строк.',
        '; Порядок пикселей в байте переставлен под ANTIC E.',
        '',
        '        .segment "RODATA"',
        '',
        '        .export _font_digits',
        '_font_digits:',
    ]
    for d in range(10):
        lines.append('; --- %d ---' % d)
        for r in idx[d]:
            row = [swap_pixels(v & 0xFF) for v in rows[r]]
            lines.append('        .byte ' + ','.join('$%02X' % v for v in row))
    lines.append('')

    with open(a.asm, 'w') as f:
        f.write('\n'.join(lines) + '\n')

    print('цифр: 10, по 12 строк, 3 байта в строке = %d байт'
          % (10 * 12 * 3))
    print('asm:', a.asm)
    return 0


if __name__ == '__main__':
    sys.exit(main())
