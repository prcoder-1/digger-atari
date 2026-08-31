/*
 * text.h - вывод цифр и строк знакогенератором ПЗУ.
 *
 * Собственный шрифт рисовать и хранить не нужно: в ПЗУ по адресу $E000 уже
 * лежит однобитный набор 8x8, его достаточно расширить до двух бит на
 * пиксель. Символ занимает ровно 2 байта экрана и 8 строк растра.
 *
 * Координата x задаётся В БАЙТАХ экрана (0..39), y - в строках растра.
 */
#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>

/* Строит таблицу расширения под заданный индекс цвета (1..3). */
void text_init(uint8_t color);

/* Символ по внутреннему коду Atari (не ATASCII) */
void text_char(uint8_t x_byte, uint8_t y, uint8_t internal_code);

/* Цифра 0..9 */
void text_digit(uint8_t x_byte, uint8_t y, uint8_t d);

/* Строка из цифр, заглавных букв и пробелов */
void text_string(uint8_t x_byte, uint8_t y, const char *s);

/* Шесть знаков счёта из трёх байт упакованного BCD */
void text_bcd6(uint8_t x_byte, uint8_t y, const uint8_t *bcd);

#endif /* TEXT_H */
