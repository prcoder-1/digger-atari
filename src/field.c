/*
 * field.c - состояние поля и его отрисовка.
 */
#include <string.h>
#include "field.h"
#include "screen.h"
#include "blit.h"

/* Данные из assets/, сгенерированные конвертерами */
extern const uint8_t level_data[];      /* levels.s: 80 байт на уровень   */
extern const uint8_t earth_tbl[];       /* earth.s:  8 указателей на блок */
extern const uint8_t spr_cn_p0[];       /* камень, фаза 0 */
extern const uint8_t spr_cn_p1[];       /* камень, фаза 1 */
extern const uint8_t spr_b4_p0[];       /* мешок, фаза 0  */
extern const uint8_t spr_b4_p1[];       /* мешок, фаза 1  */

#define LEVEL_BYTES     80u             /* (15/2 + 1) * 10 */
#define ROW_BYTES       8u              /* байт на строку карты */

uint16_t background[H_MAX][W_MAX];
uint16_t coins[H_MAX];

uint8_t x_log_tbl[80];
uint8_t x_rem_tbl[80];
uint8_t y_log_tbl[SCR_LINES];
uint8_t y_rem_tbl[SCR_LINES];

/*
 * Ход прокапывается ровно под спрайт, а не по всей клетке. В оригинале
 * положение существа - это ЛЕВЫЙ ВЕРХНИЙ угол клетки (man_x = x*20 + 12),
 * то есть спрайт 16x15 прижат к левому верхнему углу клетки 20x18, а
 * незанятые 4 px справа и 3 строки снизу и есть разделители земли.
 * У нас всё вдвое уже по горизонтали: спрайт 8x15 в клетке 10x18,
 * остаются 2 px справа и 3 строки снизу.
 */
#define CORRIDOR_W      SPRITE_W_PX     /* 8 px из 10  */
#define CORRIDOR_H      SPRITE_H        /* 15 строк из 18 */

void field_build_tables(void)
{
    uint8_t i, cell, rem;

    /* X: подшаг 2 px, 5 подшагов в клетке */
    cell = 0;
    rem = 0;
    for (i = 0; i < 80; ++i) {
        if (i < FIELD_X_OFFSET) {
            x_log_tbl[i] = 0;
            x_rem_tbl[i] = 0;
            continue;
        }
        x_log_tbl[i] = cell;
        x_rem_tbl[i] = rem;
        if (++rem == POS_X_STEP) {
            rem = 0;
            ++cell;
        }
    }

    /* Y: подшаг 3 строки, 6 подшагов в клетке */
    for (i = 0; i < SCR_LINES; ++i) {
        if (i < FIELD_Y_OFFSET) {
            y_log_tbl[i] = 0;
            y_rem_tbl[i] = 0;
        } else {
            uint8_t d = (uint8_t)(i - FIELD_Y_OFFSET);
            y_log_tbl[i] = (uint8_t)(d / CELL_H_PX);
            y_rem_tbl[i] = (uint8_t)((d % CELL_H_PX) / SUBSTEP_Y_LINES);
        }
    }
}

uint8_t level_sym(uint8_t screen, uint8_t x, uint8_t y)
{
    const uint8_t *p = level_data + (unsigned)screen * LEVEL_BYTES
                                  + (unsigned)y * ROW_BYTES;
    uint8_t b;

    if (x == W_MAX - 1)                 /* последняя клетка занимает байт */
        return p[ROW_BYTES - 1] & 0x0Fu;
    b = p[x >> 1];
    return (x & 1u) ? (uint8_t)(b >> 4) : (uint8_t)(b & 0x0Fu);
}

/*
 * Обёртка над ассемблерным примитивом гашения прямоугольника в пикселях.
 * Сама процедура лежит в blit.s: её версия на Си неверно собиралась
 * с оптимизацией (см. комментарий там).
 */
void clear_px_at(uint8_t x_px, uint8_t y, uint8_t w_px, uint8_t h)
{
    cp_x = x_px;
    cp_y = y;
    cp_w = w_px;
    cp_h = h;
    clear_px();
}

/* Заливает поле текстурой земли: блок 5 байт x 4 строки, 8 раз по ширине */
static void draw_earth(uint8_t screen)
{
    const uint8_t *tile = (const uint8_t *)
        ((unsigned)earth_tbl[screen * 2] |
         ((unsigned)earth_tbl[screen * 2 + 1] << 8));
    uint8_t row, i, b;

    for (row = 0; row < FIELD_H_LINES; ++row) {
        uint8_t *p = SCREEN_ROW(FIELD_Y_OFFSET + row);
        const uint8_t *src = tile + (row & 3u) * 5u;
        for (i = 0; i < SCR_STRIDE; i += 5u)
            for (b = 0; b < 5u; ++b)
                p[i + b] = src[b];
    }
}

/* Выводит спрайт по координате в подшагах: фаза выбирается по младшему биту */
static void draw_at(uint8_t x_graph, uint8_t y, uint8_t h,
                    const uint8_t *p0, const uint8_t *p1)
{
    bl_x = (uint8_t)(x_graph >> 1);
    bl_y = y;
    bl_h = h;
    if (x_graph & 1u) {
        bl_w = 3;
        bl_src = p1;
    } else {
        bl_w = 2;
        bl_src = p0;
    }
    blit_mask();
}

uint8_t field_take_coin(uint8_t x, uint8_t y)
{
    uint16_t bit = (uint16_t)(1u << x);
    if (y >= H_MAX || x >= W_MAX || !(coins[y] & bit))
        return 0;
    coins[y] &= (uint16_t)~bit;
    return 1;
}

uint8_t field_coins_left(void)
{
    uint8_t y;
    for (y = 0; y < H_MAX; ++y)
        if (coins[y])
            return 1;
    return 0;
}

/*
 * Рисует поле по ТЕКУЩЕМУ состоянию: земля, прокопанные ходы по битам фона,
 * оставшиеся камни по битовой маске. Уровень при этом не перестраивается,
 * поэтому после гибели Диггера собранные камни не возвращаются, а
 * прокопанные ходы остаются - как в оригинале.
 */
void field_redraw(uint8_t screen)
{
    uint8_t x, y;

    draw_earth(screen);

    for (y = 0; y < H_MAX; ++y) {
        uint8_t cy = (uint8_t)(FIELD_Y_OFFSET + y * CELL_H_PX);
        for (x = 0; x < W_MAX; ++x) {
            uint8_t cx = (uint8_t)(FIELD_X0_PX + x * CELL_W_PX);
            if ((background[y][x] & BG_VERT_ALL) != BG_VERT_ALL)
                clear_px_at(cx, cy, CORRIDOR_W, CELL_H_PX);
            if ((background[y][x] & BG_HORIZ_ALL) != BG_HORIZ_ALL)
                clear_px_at(cx, cy, CELL_W_PX, CORRIDOR_H);
        }
    }

    for (y = 0; y < H_MAX; ++y) {
        uint8_t cy = (uint8_t)(FIELD_Y_OFFSET + y * CELL_H_PX);
        for (x = 0; x < W_MAX; ++x) {
            if (coins[y] & (uint16_t)(1u << x))
                draw_at((uint8_t)(FIELD_X_OFFSET + x * POS_X_STEP),
                        (uint8_t)(cy + 3u), 10, spr_cn_p0, spr_cn_p1);
        }
    }
}

/* Полная подготовка уровня: состояние фона и камней, затем отрисовка. */
void field_init(uint8_t screen)
{
    uint8_t x, y, sym;

    for (y = 0; y < H_MAX; ++y) {
        coins[y] = 0;
        for (x = 0; x < W_MAX; ++x) {
            uint16_t bg = 0xFFFFu;
            sym = level_sym(screen, x, y);
            if (sym == LEV_VERT || sym == LEV_BOTH)
                bg &= BG_DIG_VERT;
            if (sym == LEV_HORIZ || sym == LEV_BOTH)
                bg &= BG_DIG_HORIZ;
            background[y][x] = bg;
            if (sym == LEV_COIN)
                coins[y] |= (uint16_t)(1u << x);
        }
    }

    field_redraw(screen);
}
