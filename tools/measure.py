#!/usr/bin/env python3
"""
measure.py - снимает высоту цветных полос со скриншота и переводит её в такты.

Полосы рисует измеритель из src/main.c: перед серией вызовов примитива цвет
фона COLBK переключается, после - возвращается. Высота полосы в строках
растра равна затраченному времени, потому что строка растра PAL - это ровно
114 тактов процессора.

Цвет читается в колонке бордюра (левее игрового поля), где кроме COLBK
ничего нет.

    tools/measure.py <кадр.png> [повторов]
"""

import sys
from collections import Counter

CYCLES_PER_LINE = 114           # PAL: 312 строк * 114 = 35568 тактов в кадре
LINES_PER_FRAME = 312           # строк растра в кадре PAL
TICK_FRAMES = 5                 # игровой такт = каждый 5-й кадр = 10 Гц
BORDER_X = 2                    # колонка бордюра

# ВАЖНО про единицы: высота полосы - это РЕАЛЬНОЕ ВРЕМЯ в строках растра.
# Умножение на 114 даёт полное число машинных тактов, включая те, что ANTIC
# забрал под DMA (в режиме E это 40 тактов из 114 на каждой активной строке).
# Это НЕ число тактов, доступных процессору. Для планирования бюджета важно
# именно реальное время, поэтому основная единица здесь - строки растра.


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    path = sys.argv[1]
    repeats = int(sys.argv[2]) if len(sys.argv) > 2 else 4

    from PIL import Image
    im = Image.open(path).convert('RGB')
    w, h = im.size
    px = im.load()

    col = [px[BORDER_X, y] for y in range(h)]

    runs = []
    cur, start = col[0], 0
    for y in range(1, h):
        if col[y] != cur:
            runs.append((start, y - 1, cur))
            cur, start = col[y], y
    runs.append((start, h - 1, cur))

    # Фон - это цвет НИЖНЕЙ строки: все полосы к этому моменту уже кончились.
    # Брать самый частый цвет нельзя: длинная полоса легко оказывается частой.
    black = col[h - 1]
    print(f'кадр {w}x{h}, цвет фона {black}')
    print(f'повторов на полосу: {repeats}, строка растра = {CYCLES_PER_LINE} тактов')
    print()

    tick_lines = LINES_PER_FRAME * TICK_FRAMES
    total = 0
    for a, b, c in runs:
        if c == black:
            continue
        lines = b - a + 1
        per = lines / repeats
        total += lines
        print(f'  строки {a:3d}..{b:3d}  высота {lines:3d}  цвет {c}')
        print(f'      на вызов: {per:6.1f} строк растра'
              f'  = {per * CYCLES_PER_LINE:7.0f} машинных тактов'
              f'  = {100.0 * per / tick_lines:5.2f}% игрового такта')

    if total:
        print()
        print(f'  всего измерено: {total} строк '
              f'({100.0 * total / LINES_PER_FRAME:.1f}% кадра PAL, '
              f'{100.0 * total / tick_lines:.1f}% такта 10 Гц)')
        print(f'  бюджет такта: {tick_lines} строк растра '
              f'({TICK_FRAMES} кадра PAL)')
    return 0


if __name__ == '__main__':
    sys.exit(main())
