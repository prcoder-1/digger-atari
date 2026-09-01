#!/usr/bin/env python3
"""
anres.py - определение режима экрана и происхождения цветов по снимку кадра.

    tools/anres.py <кадр.png> [ещё кадры...] [--rows] [--spans] [--bands]

Кадр обязательно снимать так, чтобы одна точка режима 320 приходилась ровно на
один пиксель изображения:

    tools/shot.sh игра.xex out.png 8 -win-width 384 -win-height 240 \
                  -horiz-area full

Три независимых измерения, каждое отвечает на свой вопрос.

1. РАЗРЕШЕНИЕ - по длинам серий одинакового цвета вдоль строки.
   В режимах 160 точек любая серия занимает чётное число пикселей кадра,
   поэтому доля нечётных серий равна нулю. Единичные серии и доля нечётных
   около половины означают 320 точек. Промежуточные значения - смешанный
   дисплей-лист: считаем строки по отдельности (--rows).

2. ЦВЕТА ПОЛЯ ПРОТИВ СПРАЙТОВ - по разбросу цвета внутри строки (--spans).
   Игрок в обычном размере шириной восемь тактов цвета, то есть восемь точек
   режима 160. Если цвет во всех строках укладывается в такое окно, он пришёл
   из регистра PMG; если растягивается на десятки точек - это поле.
   Считать надо именно разброс (от левого вхождения до правого), а не длину
   непрерывной серии: тонкие штрихи шрифта тоже дают короткие серии.

3. ПОЛОСЫ ПАЛИТРЫ - по смене набора цветов сверху вниз (--bands).
   Если в кадре цветов больше, чем даёт режим, но в каждой строке не больше
   штатного числа, значит регистры переписываются прерыванием по строке.
"""
import sys
from collections import Counter

from PIL import Image

PLAYER_W = 8            # ширина игрока в точках режима 160
LEFT_MARGIN = 32        # первый пиксель видимого поля при -horiz-area full


def load(path):
    im = Image.open(path).convert('RGB')
    w, h = im.size
    px = im.load()
    return px, w, h


def hx(c):
    return '#%02x%02x%02x' % c


def runs(row):
    out = []
    n = 1
    for i in range(1, len(row)):
        if row[i] == row[i - 1]:
            n += 1
        else:
            out.append(n)
            n = 1
    out.append(n)
    return out


def resolution(px, w, h, per_row):
    lens = Counter()
    colors = set()
    maxrow = hires = plain = 0
    detail = []
    for y in range(h):
        row = [px[x, y] for x in range(w)]
        body = runs(row)[1:-1]           # крайние серии - рамка эмулятора
        if not body:
            body = [w]
        odd = sum(1 for v in body if v % 2)
        frac = odd / len(body)
        lens.update(body)
        cs = set(row[8:-8])
        colors |= cs
        maxrow = max(maxrow, len(cs))
        if frac > 0.30 and len(body) > 8:
            hires += 1
        else:
            plain += 1
        detail.append((y, len(cs), round(frac * 100)))
    tot = max(1, sum(lens.values()))
    odd = sum(c for v, c in lens.items() if v % 2)
    print('  цветов %d, максимум в строке %d' % (len(colors), maxrow))
    print('  нечётных серий %d%%  ->  %s'
          % (round(100 * odd / tot),
             '160 точек' if odd * 20 < tot else 'есть 320 точек'))
    print('  строк с высоким разрешением: %d из %d' % (hires, hires + plain))
    print('  длины серий:', sorted(lens.items())[:6])
    if per_row:
        for y, nc, f in detail:
            print('    %3d  цветов %d  нечётных %3d%%' % (y, nc, f))


def spans(px, w, h):
    """Максимальный разброс каждого цвета внутри строки, в точках 160."""
    best = {}
    for y in range(h):
        row = [px[LEFT_MARGIN + 2 * X, y] for X in range(160)]
        pos = {}
        for X, c in enumerate(row):
            pos.setdefault(c, [X, X])[1] = X
        for c, (a, b) in pos.items():
            d = b - a + 1
            if d > best.get(c, 0):
                best[c] = d
    print('  цвет      разброс   происхождение')
    for c, n in sorted(best.items(), key=lambda kv: -kv[1]):
        print('    %s  %4d   %s'
              % (hx(c), n, 'поле' if n > PLAYER_W else 'PMG'))


def bands(px, w, h):
    """Границы полос: печатаем строку, где набор цветов изменился."""
    prev = None
    for y in range(h):
        cs = frozenset(px[x, y] for x in range(12, w - 12))
        if cs != prev:
            print('    y=%3d  цветов %d  %s'
                  % (y, len(cs), ' '.join(hx(c) for c in sorted(cs)[:8])))
            prev = cs


def main():
    files = [a for a in sys.argv[1:] if not a.startswith('--')]
    if not files:
        print(__doc__)
        return 1
    for p in files:
        px, w, h = load(p)
        print('==', p)
        resolution(px, w, h, '--rows' in sys.argv)
        if '--spans' in sys.argv:
            spans(px, w, h)
        if '--bands' in sys.argv:
            bands(px, w, h)
    return 0


if __name__ == '__main__':
    sys.exit(main())
