/*
 * blit.h - вывод спрайтов (реализация в blit.s).
 *
 * Параметры передаются через глобальные ячейки нулевой страницы, а не
 * аргументами: в cc65 это заметно дешевле. Порядок вызова:
 *
 *      bl_x = байт экрана;  bl_y = строка растра;
 *      bl_w = байт в строке; bl_h = строк;
 *      bl_src = данные спрайта;
 *      blit_mask();
 *
 * Данные спрайта - пары (маска, изображение) подряд по строке,
 * см. tools/spr2atari.py. Вывод: экран = (экран И маска) ИЛИ изображение.
 */
#ifndef BLIT_H
#define BLIT_H

#include <stdint.h>

extern uint8_t bl_x;
extern uint8_t bl_y;
extern uint8_t bl_w;
extern uint8_t bl_h;
extern const uint8_t *bl_src;

#ifdef __CC65__
#pragma zpsym ("bl_x")
#endif
#ifdef __CC65__
#pragma zpsym ("bl_y")
#endif
#ifdef __CC65__
#pragma zpsym ("bl_w")
#endif
#ifdef __CC65__
#pragma zpsym ("bl_h")
#endif
#ifdef __CC65__
#pragma zpsym ("bl_src")
#endif

void blit_mask(void);   /* с маской   */
/* Только вырез по форме спрайта: экран = экран И маска, рисунок не выводится.
 * Нужен там, где фигуру надо убрать, не срезая землю вокруг прямоугольником. */
void blit_erase(void);
void blit_plain(void);  /* непрозрачно, источник сплошной (шрифт счёта) */

/* Гашение прямоугольника, заданного В ПИКСЕЛЯХ - основной примитив прокопки.
 * Края, не попавшие на границу байта, гасятся маской. */
extern uint8_t cp_x;
extern uint8_t cp_y;
extern uint8_t cp_w;
extern uint8_t cp_h;

#ifdef __CC65__
#pragma zpsym ("cp_x")
#endif
#ifdef __CC65__
#pragma zpsym ("cp_y")
#endif
#ifdef __CC65__
#pragma zpsym ("cp_w")
#endif
#ifdef __CC65__
#pragma zpsym ("cp_h")
#endif

void clear_px(void);

#endif /* BLIT_H */
