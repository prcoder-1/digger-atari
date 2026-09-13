/*
 * ttext.h - надписи заставки: крупный шрифт оригинала и мелкий из ПЗУ.
 */
#ifndef TTEXT_H
#define TTEXT_H

#define FONT_W  2u              /* байт на знак крупного шрифта */
#define FONT_H  12u             /* строк растра                 */

/* Крупный шрифт игры: знак 16 точек, двенадцать строк */
void text_big(unsigned char x_byte, unsigned char y, const char *s);
unsigned char text_big_width(const char *s);

/* Знакогенератор ПЗУ: знак 8 точек, восемь строк */
void text_small(unsigned char x_byte, unsigned char y, const char *s);

#endif /* TTEXT_H */
