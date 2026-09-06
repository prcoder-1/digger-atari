/*
 * Вторая разведка llvm-mos: переносится ли наш ассемблерный слой и держится ли
 * стык с Си через нулевую страницу.
 *
 * Взят самый тяжёлый файл порта - blit.s: самомодификация операндов, макрос,
 * параметры в именованных ячейках нулевой страницы. Проверка функциональная:
 * тот же вывод считается и перенесённой процедурой, и простой реализацией на
 * Си, и результаты сравниваются побайтно. Зелёная граница - совпало.
 *
 * volatile на буферах обязателен: их адреса уходят в ассемблер через таблицы
 * строк как ЧИСЛА, и оптимизатор LLVM не обязан считать, что внешняя процедура
 * их меняет. Без volatile сравнение свернулось бы в константу.
 */
#include <stdint.h>

#define COLBK (*(volatile uint8_t *)0xD01A)

extern uint8_t bl_x, bl_y, bl_w, bl_h;
extern const uint8_t *bl_src;
extern uint8_t cp_x, cp_y, cp_w, cp_h;
void blit_mask(void);
void clear_px(void);

#define ROWS   20u
#define BYTES  8u

uint8_t screen_rowlo[ROWS];
uint8_t screen_rowhi[ROWS];

static volatile uint8_t buf[ROWS * BYTES];
static volatile uint8_t ref[ROWS * BYTES];

/* Спрайт 2 байта на строку, 15 строк: пары (маска, изображение) */
static const uint8_t spr[15u * 2u * 2u] = {
    0x0F,0xF0, 0xF0,0x0F,  0x1E,0xE1, 0xE1,0x1E,  0x2D,0xD2, 0xD2,0x2D,
    0x3C,0xC3, 0xC3,0x3C,  0x4B,0xB4, 0xB4,0x4B,  0x5A,0xA5, 0xA5,0x5A,
    0x69,0x96, 0x96,0x69,  0x78,0x87, 0x87,0x78,  0x0F,0xF0, 0xF0,0x0F,
    0x1E,0xE1, 0xE1,0x1E,  0x2D,0xD2, 0xD2,0x2D,  0x3C,0xC3, 0xC3,0x3C,
    0x4B,0xB4, 0xB4,0x4B,  0x5A,0xA5, 0xA5,0x5A,  0x69,0x96, 0x96,0x69,
};

static void rows_point_at(volatile uint8_t *base)
{
    uint8_t r;
    for (r = 0; r < ROWS; ++r) {
        uint16_t a = (uint16_t)(base + r * BYTES);
        screen_rowlo[r] = (uint8_t)a;
        screen_rowhi[r] = (uint8_t)(a >> 8);
    }
}

/* Простая реализация того же самого на Си - эталон для сравнения */
static void ref_blit(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                     const uint8_t *s)
{
    uint8_t r, c;
    uint16_t i = 0;
    for (r = 0; r < h; ++r) {
        volatile uint8_t *row = ref + (uint16_t)(y + r) * BYTES;
        for (c = 0; c < w; ++c) {
            uint8_t m = s[i++];
            uint8_t im = s[i++];
            row[x + c] = (uint8_t)((row[x + c] & m) | im);
        }
    }
}

/* Эталон гашения прямоугольника, заданного в ПОДШАГАХ по X */
static void ref_clear(uint8_t xs, uint8_t y, uint8_t ws, uint8_t h)
{
    uint8_t r, s;
    for (r = 0; r < h; ++r) {
        volatile uint8_t *row = ref + (uint16_t)(y + r) * BYTES;
        for (s = 0; s < ws; ++s) {
            uint8_t sub = (uint8_t)(xs + s);
            uint8_t m = (uint8_t)((sub & 1u) ? 0x0Fu : 0xF0u);
            row[sub >> 1] = (uint8_t)(row[sub >> 1] & (uint8_t)~m);
        }
    }
}

int main(void)
{
    uint16_t i;
    uint8_t ok = 1;

    for (i = 0; i < ROWS * BYTES; ++i) {
        buf[i] = (uint8_t)(i * 7u + 3u);
        ref[i] = (uint8_t)(i * 7u + 3u);
    }

    /* вывод спрайта: перенесённая процедура против эталона */
    rows_point_at(buf);
    bl_x = 2; bl_y = 1; bl_w = 2; bl_h = 15; bl_src = spr;
    blit_mask();
    ref_blit(2, 1, 2, 15, spr);

    /* гашение: край на границе байта и край в середине байта */
    rows_point_at(buf);
    cp_x = 5; cp_y = 2; cp_w = 7; cp_h = 6;
    clear_px();
    ref_clear(5, 2, 7, 6);

    for (i = 0; i < ROWS * BYTES; ++i)
        if (buf[i] != ref[i])
            ok = 0;

    COLBK = ok ? 0xC6u : 0x36u;
    for (;;)
        ;
}
