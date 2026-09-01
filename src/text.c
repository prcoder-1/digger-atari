/*
 * text.c - вывод цифр и букв в кадровый буфер ANTIC F.
 *
 * Буквы берутся из знакогенератора ПЗУ по адресу $E000: он однобитный, 8x8 на
 * символ, то есть в режиме 320 точек лёг бы ровно в один байт. Но тогда
 * надпись вышла бы вдвое мельче, чем была при 160 точках, поэтому каждая
 * точка удваивается: полубайт глифа (4 точки) даёт байт экрана (8 точек), а
 * символ - два байта.
 *
 * Таблица удвоения постоянна. Раньше она строилась в text_init() под нужный
 * индекс цвета, но при одном бите на точку цвета у надписи нет - оттенок
 * задаёт COLPF2 на всё поле сразу.
 */
#include "text.h"
#include "screen.h"
#include "blit.h"

#define CHARSET_ROM  ((const uint8_t *)0xE000u)

/* Внутренние коды Atari: цифры начинаются с $10, заглавные буквы с $21 */
#define INT_DIGIT0   0x10u
#define INT_LETTER_A 0x21u

/* Полубайт глифа -> байт экрана: каждый бит удвоен */
static const uint8_t expand[16] = {
    0x00, 0x03, 0x0C, 0x0F, 0x30, 0x33, 0x3C, 0x3F,
    0xC0, 0xC3, 0xCC, 0xCF, 0xF0, 0xF3, 0xFC, 0xFF
};

void text_char(uint8_t x_byte, uint8_t y, uint8_t internal_code)
{
    const uint8_t *g = CHARSET_ROM + ((unsigned)internal_code << 3);
    uint8_t row, b;

    for (row = 0; row < 8u; ++row) {
        uint8_t *p = SCREEN_ROW(y + row);
        b = g[row];
        p[x_byte]     = expand[b >> 4];
        p[x_byte + 1] = expand[b & 0x0Fu];
    }
}

/*
 * Цифры счёта берутся из шрифта ОРИГИНАЛА, а не из знакогенератора ПЗУ.
 * Цифра 12 точек не ложится на границу байта (байт - 8 точек), поэтому она
 * лежит в ячейке 16 точек = 2 байта с полем по 2 точки с каждой стороны.
 * Данные сплошные, без маски: соседние ячейки не перекрываются, и
 * непрозрачный вывод их не портит.
 */
extern const uint8_t font_digits[];

#define FONT_W      2u          /* байт на строку (ячейка 16 точек) */
#define FONT_H      12u         /* строк на цифру  */
#define FONT_BYTES  (FONT_W * FONT_H)

void text_digit(uint8_t x_byte, uint8_t y, uint8_t d)
{
    bl_x = x_byte;
    bl_y = y;
    bl_w = FONT_W;
    bl_h = FONT_H;
    bl_src = font_digits + (unsigned)(d & 0x0Fu) * FONT_BYTES;
    blit_plain();
}

void text_string(uint8_t x_byte, uint8_t y, const char *s)
{
    while (*s) {
        char c = *s++;
        uint8_t code;
        if (c >= '0' && c <= '9')
            code = (uint8_t)(INT_DIGIT0 + (c - '0'));
        else if (c >= 'A' && c <= 'Z')
            code = (uint8_t)(INT_LETTER_A + (c - 'A'));
        else
            code = 0;                       /* пробел */
        text_char(x_byte, y, code);
        x_byte += 2u;
    }
}

/*
 * Счёт хранится упакованным BCD в трёх байтах (шесть знаков), как и в
 * оригинале, где потолок 999999. Так сложение делается командой ADC в
 * десятичном режиме, а вывод не требует ни одного деления - в отличие от
 * 32-битного счёта БК-порта, где print_dec вычитал степени десятки.
 */
void text_bcd6(uint8_t x_byte, uint8_t y, const uint8_t *bcd)
{
    uint8_t i;
    for (i = 0; i < 3u; ++i) {
        text_digit(x_byte, y, (uint8_t)(bcd[i] >> 4));
        text_digit((uint8_t)(x_byte + FONT_W), y, (uint8_t)(bcd[i] & 0x0Fu));
        x_byte += (uint8_t)(2u * FONT_W);
    }
}
