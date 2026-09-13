#!/usr/bin/env python3
"""
png2antic.py - картинка -> данные экрана ANTIC E (160x192, четыре цвета).

Источник - обложка заставки с БК 0010 (Digger_title.png). Там экран 512x256 в
цветном режиме, то есть пара бит на точку: цветных точек 256 по горизонтали, и
вся картинка нарисована штриховкой из четырёх цветов - чёрного, красного,
зелёного и синего.

На ATARI ближайший режим - ANTIC E: 160 точек в строке, две бита на точку,
четыре цвета. Значение точки выбирает регистр: 0 - COLBK (фон), 1 - COLPF0,
2 - COLPF1, 3 - COLPF2. Ровно те же четыре цвета, что и на БК, поэтому палитра
переносится один в один, а работа сводится к уменьшению до 160x192.

Уменьшать надо С УСРЕДНЕНИЕМ, а не выборкой каждой N-й точки: штриховка на БК
кодирует полутона (красное через точку по чёрному - это тёмно-красный), и
выборка превратила бы её в кашу из чистых цветов. Поэтому сначала усреднение в
RGB, потом обратное разложение по четырём цветам с рассеиванием ошибки.

    tools/png2antic.py вход.png выход.s [--label _cover] [--segment COVER]
"""

import argparse
import sys

try:
    from PIL import Image
except ImportError:
    sys.exit("нужен Pillow: pip install pillow")

W, H = 160, 192                 # экран ANTIC E
BYTES_PER_ROW = W // 4          # четыре точки в байте

# Порядок задаёт номер цвета в кадре: 0 - фон, дальше PF0, PF1, PF2.
PALETTE = [
    (0, 0, 0),                  # 0 - COLBK,  чёрный
    (255, 0, 0),                # 1 - COLPF0, красный
    (0, 255, 0),                # 2 - COLPF1, зелёный
    (0, 0, 255),                # 3 - COLPF2, синий
]


def convert(src):
    im = Image.open(src).convert("RGB")
    im = im.resize((W, H), Image.LANCZOS)

    pal_img = Image.new("P", (1, 1))
    flat = [c for rgb in PALETTE for c in rgb]
    pal_img.putpalette(flat + [0] * (768 - len(flat)))

    # Рассеивание ошибки: сохраняет полутона штриховки оригинала
    idx = im.quantize(palette=pal_img, dither=Image.Dither.FLOYDSTEINBERG)
    return idx


def pack(idx):
    """Индексы точек -> байты ANTIC E: четыре точки в байте, слева старшие."""
    px = idx.load()
    out = bytearray()
    for y in range(H):
        for bx in range(BYTES_PER_ROW):
            b = 0
            for k in range(4):
                b = (b << 2) | (px[bx * 4 + k, y] & 3)
            out.append(b)
    return bytes(out)


def emit(data, label, segment, src):
    lines = [
        "; %s - СГЕНЕРИРОВАНО tools/png2antic.py, не править вручную." % label,
        "; Источник: %s" % src,
        ";",
        "; Экран ANTIC E: 160 точек в строке, 192 строки, две бита на точку.",
        "; Значение точки - номер краски: 0 COLBK, 1 COLPF0, 2 COLPF1, 3 COLPF2.",
        "",
        "        .export %s" % label,
        '        .segment "%s"' % segment,
        "",
        "%s:" % label,
    ]
    for i in range(0, len(data), 16):
        chunk = data[i:i + 16]
        lines.append("        .byte " + ",".join("$%02X" % b for b in chunk))
    lines.append("")
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("src")
    ap.add_argument("dst")
    ap.add_argument("--label", default="_cover")
    ap.add_argument("--segment", default="COVER")
    ap.add_argument("--preview", help="куда положить PNG с тем, что выйдет на экране")
    a = ap.parse_args()

    idx = convert(a.src)
    data = pack(idx)
    if len(data) != W * H // 4:
        sys.exit("размер данных %d, ожидалось %d" % (len(data), W * H // 4))

    with open(a.dst, "w", encoding="utf-8") as f:
        f.write(emit(data, a.label, a.segment, a.src))

    if a.preview:
        # Предпросмотр в пропорциях экрана: точка ANTIC E вдвое шире строки
        idx.convert("RGB").resize((W * 2, H), Image.NEAREST).save(a.preview)

    print("%s: %d байт" % (a.dst, len(data)))


if __name__ == "__main__":
    main()
