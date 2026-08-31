/*
 * text.c - вывод цифр и букв в кадровый буфер ANTIC E.
 *
 * Знакогенератор берётся из ПЗУ операционной системы по адресу $E000: он
 * однобитный, 8x8 на символ, и его достаточно расширить до двух бит на
 * пиксель. Собственный шрифт пришлось бы рисовать и хранить, а этот уже
 * лежит в ПЗУ и на цели "atari" всегда доступен.
 *
 * Расширение идёт по таблице: полубайт исходного глифа (4 пикселя) даёт
 * ровно один байт экрана (4 пикселя по 2 бита). Таблица строится в начале
 * работы под нужный цвет.
 */
#include "text.h"
#include "screen.h"
#include "blit.h"

#define CHARSET_ROM  ((const uint8_t *)0xE000u)

/* Внутренние коды Atari: цифры начинаются с $10, заглавные буквы с $21 */
#define INT_DIGIT0   0x10u
#define INT_LETTER_A 0x21u

static uint8_t expand[16];      /* полубайт глифа -> байт экрана */

void text_init(uint8_t color)
{
    uint8_t n, i, v;

    for (n = 0; n < 16; ++n) {
        v = 0;
        for (i = 0; i < 4; ++i)
            if (n & (8u >> i))
                v |= (uint8_t)(color << (6u - (i << 1)));
        expand[n] = v;
    }
}

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
 * Цифры счёта берутся из шрифта ОРИГИНАЛА, а не из знакогенератора ПЗУ:
 * 12x12 пикселей, три байта на строку, сплошные данные без маски.
 * Знакогенератор ПЗУ остаётся для букв - в счёте их нет, а для надписей
 * заставки нужен отдельный набор.
 */
extern const uint8_t font_digits[];

#define FONT_W      3u          /* байт на строку  */
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
    /* Цифра шрифта оригинала шире символа ПЗУ: три байта вместо двух */
    for (i = 0; i < 3u; ++i) {
        text_digit(x_byte, y, (uint8_t)(bcd[i] >> 4));
        text_digit((uint8_t)(x_byte + FONT_W), y, (uint8_t)(bcd[i] & 0x0Fu));
        x_byte += (uint8_t)(2u * FONT_W);
    }
}
