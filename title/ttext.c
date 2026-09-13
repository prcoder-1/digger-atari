/*
 * ttext.c - надписи заставки.
 *
 * Шрифтов ДВА, и это не прихоть, а устройство оригинала на БК: крупные
 * надписи там выводятся спрайтовым шрифтом 12x12, а текст боковой панели -
 * знакогенератором ПЗУ, который вдвое уже. Иначе панель просто не помещается:
 * знак крупного шрифта занимает 16 точек, и в строку их влезает двадцать на
 * весь экран - а панели нужно пятнадцать при живом демо рядом.
 *
 * Крупный шрифт - тот же, что у счёта в игре, и взят он из ОРИГИНАЛА
 * (CLib/CHAR__*.CHR). В игре из него лежат только цифры и семь букв: MAIN
 * забит. Заставке доступен весь набор - она живёт в памяти, которую потом
 * занимает игра, и место там есть.
 *
 * Мелкий - знакогенератор ПЗУ ATARI по $E000. Заставка идёт ДО игры, ПЗУ ещё
 * включено, и шрифт достаётся даром.
 */
#include "ttext.h"

extern const unsigned char font_full[];

/* Блиттер игры, используется как есть */
extern unsigned char bl_x, bl_y, bl_w, bl_h;
extern const unsigned char *bl_src;
#pragma zpsym ("bl_x")
#pragma zpsym ("bl_y")
#pragma zpsym ("bl_w")
#pragma zpsym ("bl_h")
#pragma zpsym ("bl_src")
void blit_plain(void);

#define GLYPH_BYTES (FONT_W * FONT_H)

/*
 * Номер знака в font_full. Порядок задан в tools/font2c.py --full:
 * цифры 0..9, буквы A..Z, точка, пробел, подчёркивание.
 */
#define GL_DOT      36u
#define GL_SPACE    37u
#define GL_UNDER    38u

static unsigned char glyph_index(char c)
{
    if (c >= '0' && c <= '9')
        return (unsigned char)(c - '0');
    if (c >= 'A' && c <= 'Z')
        return (unsigned char)(c - 'A' + 10);
    if (c == '.')
        return GL_DOT;
    if (c == ' ')
        return GL_SPACE;
    return GL_UNDER;                    /* всё прочее - подчёркивание */
}

void text_big(unsigned char x_byte, unsigned char y, const char *s)
{
    while (*s) {
        bl_x = x_byte;
        bl_y = y;
        bl_w = FONT_W;
        bl_h = FONT_H;
        bl_src = font_full + (unsigned int)glyph_index(*s++) * GLYPH_BYTES;
        blit_plain();
        x_byte += FONT_W;
    }
}

/*
 * Мелкая строка знакогенератором ПЗУ.
 *
 * В ПЗУ знаки лежат не в порядке ASCII, а в порядке ATASCII-кодов экрана:
 * пробел и знаки препинания в начале, цифры, потом буквы. Перевод из ASCII в
 * этот порядок - ровно то, что делает ОС при выводе: коды $20..$5F идут со
 * сдвигом на $20 вниз.
 */
#define CHARGEN ((const unsigned char *)0xE000)

void text_small(unsigned char x_byte, unsigned char y, const char *s)
{
    while (*s) {
        unsigned char c = (unsigned char)*s++;
        const unsigned char *g;

        if (c >= 0x20u && c <= 0x5Fu)
            c = (unsigned char)(c - 0x20u);
        else
            c = 0;                      /* незнакомый знак - пробел */

        g = CHARGEN + (unsigned int)c * 8u;

        bl_x = x_byte;
        bl_y = y;
        bl_w = 1;
        bl_h = 8;
        bl_src = g;
        blit_plain();
        ++x_byte;
    }
}

/* Ширина строки крупным шрифтом в байтах - чтобы считать середину */
unsigned char text_big_width(const char *s)
{
    unsigned char n = 0;
    while (*s++)
        n += FONT_W;
    return n;
}
