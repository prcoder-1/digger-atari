#!/usr/bin/env python3
"""
lvl2c.py - карты уровней оригинала Digger для PC -> данные для ca65.

Источник: Work/TX.C, восемь массивов mat_1..mat_8 вида char[H_MAX][W_MAX]
с символами:
    ' ' целая земля        'C' драгоценный камень   'B' мешок с деньгами
    'H' горизонтальный ход 'V' вертикальный ход     'S' оба (гнездо врагов)

Упаковка: по半 байта (нибблу) на клетку, две клетки в байте. Символов всего
шесть, они влезли бы и в 3 бита, но извлечение ниббла на 6502 - это одна
команда сдвига, а разбор 3-битных полей поперёк байтов заметно дороже.
Итог: 15*10/2 = 75 байт на уровень, 600 байт на все восемь.

Младший ниббл байта - клетка с чётным x, старший - с нечётным.
"""

import argparse
import re
import sys

SYMBOLS = {' ': 0, 'C': 1, 'B': 2, 'H': 3, 'V': 4, 'S': 5}
NAMES = {0: 'пусто', 1: 'камень', 2: 'мешок',
         3: 'гориз.ход', 4: 'верт.ход', 5: 'гнездо'}
W_MAX, H_MAX = 15, 10


def parse_levels(text):
    """Достаёт mat_1..mat_8 как списки строк символов."""
    levels = []
    for n in range(1, 9):
        m = re.search(r'\bmat_%d\s*\[[^\]]*\]\s*\[[^\]]*\]\s*=\s*\{(.*?)\}\s*;'
                      % n, text, re.S)
        if not m:
            raise SystemExit('не найден mat_%d в TX.C' % n)
        rows = []
        for rowtext in re.findall(r'\{([^{}]*)\}', m.group(1)):
            cells = re.findall(r"'(.)'", rowtext)
            if cells:
                rows.append(cells)
        if len(rows) != H_MAX or any(len(r) != W_MAX for r in rows):
            raise SystemExit('mat_%d: ожидалось %dx%d, получено %s'
                             % (n, H_MAX, W_MAX,
                                [len(r) for r in rows]))
        levels.append(rows)
    return levels


def pack(rows):
    out = bytearray()
    for y in range(H_MAX):
        for x in range(0, W_MAX - 1, 2):
            lo = SYMBOLS[rows[y][x]]
            hi = SYMBOLS[rows[y][x + 1]]
            out.append(lo | (hi << 4))
        out.append(SYMBOLS[rows[y][W_MAX - 1]])   # последняя клетка одна
    return bytes(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--src', default='/mnt/storage2/Games/Digger/src/Work/TX.C')
    ap.add_argument('--asm', default='assets/levels.s')
    ap.add_argument('--dump', action='store_true',
                    help='распечатать карты для сверки глазами')
    a = ap.parse_args()

    text = open(a.src, errors='replace').read()
    levels = parse_levels(text)

    if a.dump:
        for i, rows in enumerate(levels, 1):
            print('--- уровень %d ---' % i)
            for r in rows:
                print('  ' + ''.join(r))
        return 0

    bytes_per_level = (W_MAX // 2 + 1) * H_MAX
    lines = [
        '; levels.s - СГЕНЕРИРОВАНО tools/lvl2c.py, не править вручную.',
        '; Источник: карты уровней оригинала Digger для PC (Work/TX.C).',
        ';',
        '; По ниббл на клетку, две клетки в байте, младший ниббл - чётный x.',
        '; В строке %d клеток, поэтому байт на строку %d (последняя клетка'
        % (W_MAX, W_MAX // 2 + 1),
        '; занимает целый байт), строк %d, итого %d байт на уровень.'
        % (H_MAX, bytes_per_level),
        ';',
        '; Значения: ' + ', '.join('%d=%s' % (v, NAMES[v])
                                   for v in sorted(NAMES)),
        '',
        '        .export _level_data',
        '        .segment "LEVELS"',
        '',
        '_level_data:',
    ]
    for i, rows in enumerate(levels, 1):
        data = pack(rows)
        assert len(data) == bytes_per_level, len(data)
        lines.append('; --- уровень %d ---' % i)
        for y in range(H_MAX):
            row = data[y * (W_MAX // 2 + 1):(y + 1) * (W_MAX // 2 + 1)]
            lines.append('        .byte ' +
                         ','.join('$%02X' % b for b in row) +
                         '   ; ' + ''.join(rows[y]))
        lines.append('')

    with open(a.asm, 'w') as f:
        f.write('\n'.join(lines) + '\n')

    print('уровней: %d, байт на уровень: %d, всего: %d'
          % (len(levels), bytes_per_level, bytes_per_level * len(levels)))
    print('asm:', a.asm)
    return 0


if __name__ == '__main__':
    sys.exit(main())
