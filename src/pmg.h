/*
 * pmg.h - фигуры в аппаратных спрайтах.
 *
 * Игроки не заменяют рисунок, а красят его изнутри: подробности, раскладка
 * игроков и измерения - в шапке src/pmg.s.
 *
 * Параметры передаются через нулевую страницу, как у блиттера: это снимает
 * всю цену передачи аргументов cc65.
 */
#ifndef PMG_H
#define PMG_H

#include <stdint.h>

/*
 * Левый край нормального поля в тактах цвета. Точка режима 320 - это полтакта,
 * подшаг (4 точки) - ровно 2 такта, поэтому позиция считается без остатка.
 * Позиция считается ЗДЕСЬ, а не в ассемблере: у врагов её кладут в таблицу
 * прерывания, и вторая копия константы разъехалась бы с первой.
 */
#define PMG_HPOS_LEFT   48u
#define PMG_HPOS(x_sub) ((uint8_t)((x_sub) * 2u + PMG_HPOS_LEFT))

/*
 * Области игроков врагов внутри буфера, в страницах от его начала. Игрока два,
 * и делятся они НЕ по виду врага: краска у Ноббина и Хоббина в оригинале одна,
 * поэтому второй игрок нужен только чтобы развести двух врагов, стоящих на
 * одной высоте.
 */
#define PMG_PL_BUG0     6u
#define PMG_PL_BUG1     7u

extern uint8_t pmg_hpos;        /* Диггер: позиция в тактах цвета */
extern uint8_t pmg_y;           /* Диггер: строка растра          */
extern const uint8_t *pmg_s0;   /* план 0: корпус                 */
extern const uint8_t *pmg_s1;   /* план 1: клюв                   */

extern uint8_t pmg_by;          /* враг: строка растра            */
extern const uint8_t *pmg_bs;   /* враг: данные плана             */
extern uint8_t pmg_bpl;         /* враг: область игрока, 6 или 7  */
extern uint8_t pmg_bmask;       /* враг: маска видимой части      */
extern uint8_t pmg_blo;         /* враг: первая строка плана      */
extern uint8_t pmg_bhi;         /* враг: последняя строка плана   */

/*
 * Все они лежат в нулевой странице (сегмент ZEROPAGE в src/pmg.s), и об этом
 * надо сказать компилятору: без zpsym он обращается к ним как к обычным
 * ячейкам, а линковщик ругается несовпадением разрядности адреса. Заодно
 * пропадает лишний байт и такт на каждом обращении - как у блиттера,
 * см. src/blit.h.
 */
#ifdef __CC65__
#pragma zpsym ("pmg_hpos")
#endif
#ifdef __CC65__
#pragma zpsym ("pmg_y")
#endif
#ifdef __CC65__
#pragma zpsym ("pmg_s0")
#endif
#ifdef __CC65__
#pragma zpsym ("pmg_s1")
#endif
#ifdef __CC65__
#pragma zpsym ("pmg_by")
#endif
#ifdef __CC65__
#pragma zpsym ("pmg_bs")
#endif
#ifdef __CC65__
#pragma zpsym ("pmg_bpl")
#endif
#ifdef __CC65__
#pragma zpsym ("pmg_bmask")
#pragma zpsym ("pmg_blo")
#pragma zpsym ("pmg_bhi")
#endif

void pmg_init(void);

/* Снимает с экрана ВСЮ спрайтовую графику. Зовётся при смене уровня: заливка
 * Диггера иначе переезжает на новый уровень, см. src/pmg.s */
void pmg_clear(void);

void pmg_show(void);
void pmg_hide(void);

/* Кладёт план врага по pmg_by в область pmg_bpl. Стирание - это он же с
 * указателем на пустой блок: отдельной процедуры не нужно. */
void pmg_bug_put(void);

#endif /* PMG_H */
