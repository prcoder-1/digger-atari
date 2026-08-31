"""
sprlib.py - разбор спрайтов оригинала Digger для PC (файлы Sprites/*.SPR).

Формат файла:
  - блок комментария: сетка 32x32 значений пикселей (0..3), по строке на растр;
  - char image_XX[] = { ... };   упаковка CGA, 2 бита на пиксель, 4 пикселя
                                 в байте, старший пиксель слева;
  - char outln_XX[] = { ... };   маска (И), 1 в битах пикселя = фон виден;
  - char bkgnd_XX[] = { ... };   буфер сохранения фона, для порта не нужен;
  - #define X_WID_XX <байт в строке>
  - #define Y_WID_XX <строк>

Массивы завершаются лишним нулём, его надо отбрасывать.

Упаковка CGA совпадает с ANTIC E (2 бита на пиксель, 4 пикселя в байте,
старший пиксель слева), поэтому перестановка бит не нужна - только
уменьшение ширины вдвое.
"""

import re
from pathlib import Path

CGA_GRID_W = 32          # ширина сетки-комментария


def _parse_array(text, name):
    """Достаёт числа из объявления char <name>[] = { ... };"""
    m = re.search(r'\b' + re.escape(name) + r'\s*\[\s*\]\s*=\s*\{(.*?)\}',
                  text, re.S)
    if not m:
        return None
    body = m.group(1)
    return [int(v) for v in re.findall(r'-?\d+', body)]


def _parse_grid(text):
    """Читает сетку значений пикселей из блока комментария."""
    m = re.search(r'/\*(.*?)\*/', text, re.S)
    if not m:
        return None
    rows = []
    for line in m.group(1).splitlines():
        vals = re.findall(r'\d+', line)
        if len(vals) == CGA_GRID_W:
            rows.append([int(v) for v in vals])
    return rows or None


def unpack_cga(data, x_wid, y_wid):
    """Байты CGA -> матрица пикселей (y_wid строк по x_wid*4 пикселя)."""
    out = []
    for y in range(y_wid):
        row = []
        for bx in range(x_wid):
            b = data[y * x_wid + bx]
            for p in range(4):
                row.append((b >> (6 - 2 * p)) & 3)
        out.append(row)
    return out


def pack_antice(rows):
    """Матрица пикселей -> байты ANTIC E (та же упаковка, что у CGA)."""
    out = bytearray()
    for row in rows:
        assert len(row) % 4 == 0, 'ширина должна быть кратна 4 пикселям'
        for bx in range(0, len(row), 4):
            b = 0
            for p in range(4):
                b |= (row[bx + p] & 3) << (6 - 2 * p)
            out.append(b)
    return bytes(out)


class Sprite:
    def __init__(self, name, image, outln, x_wid, y_wid, grid):
        self.name = name
        self.x_wid = x_wid          # байт в строке (CGA)
        self.y_wid = y_wid          # строк
        self.image = image          # матрица пикселей
        self.outln = outln          # матрица маски (или None)
        self.grid = grid            # сетка из комментария (или None)

    @property
    def width_px(self):
        return self.x_wid * 4

    def grid_matches(self):
        """Сверяет декодированное изображение с сеткой-комментарием.

        Сетка 32x32 больше спрайта; спрайт лежит в её левом верхнем углу.
        Возвращает (совпало, всего_сравнено) - самопроверка формата.
        """
        if not self.grid:
            return (0, 0)
        ok = tot = 0
        for y in range(min(self.y_wid, len(self.grid))):
            for x in range(min(self.width_px, CGA_GRID_W)):
                tot += 1
                if self.grid[y][x] == self.image[y][x]:
                    ok += 1
        return (ok, tot)


def load(path):
    text = Path(path).read_text(errors='replace')
    m = re.search(r'\bimage_([A-Za-z0-9]+)\s*\[', text)
    if not m:
        return None
    name = m.group(1)

    x_wid = _parse_array(text, 'X_WID_' + name)
    mx = re.search(r'#define\s+X_WID_' + name + r'\s+(\d+)', text)
    my = re.search(r'#define\s+Y_WID_' + name + r'\s+(\d+)', text)
    if not (mx and my):
        return None
    x_wid, y_wid = int(mx.group(1)), int(my.group(1))

    need = x_wid * y_wid
    img = _parse_array(text, 'image_' + name)
    out = _parse_array(text, 'outln_' + name)
    if img is None or len(img) < need:
        return None
    img = [v & 0xFF for v in img[:need]]
    outm = None
    if out is not None and len(out) >= need:
        outm = unpack_cga([v & 0xFF for v in out[:need]], x_wid, y_wid)

    return Sprite(name, unpack_cga(img, x_wid, y_wid), outm,
                  x_wid, y_wid, _parse_grid(text))
