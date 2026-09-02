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

/* Области игроков внутри буфера, в страницах от его начала */
#define PMG_PL_NOBBIN   6u
#define PMG_PL_HOBBIN   7u

extern uint8_t pmg_hpos;        /* Диггер: позиция в тактах цвета */
extern uint8_t pmg_y;           /* Диггер: строка растра          */
extern const uint8_t *pmg_s0;   /* план 0: корпус                 */
extern const uint8_t *pmg_s1;   /* план 1: клюв                   */

extern uint8_t pmg_by;          /* враг: строка растра            */
extern const uint8_t *pmg_bs;   /* враг: данные плана             */
extern uint8_t pmg_bpl;         /* враг: область игрока, 6 или 7  */

void pmg_init(void);
void pmg_show(void);
void pmg_hide(void);

/* Гасит области игроков врагов целиком */
void pmg_bugs_clear(void);

/* Кладёт план врага по pmg_by в область pmg_bpl */
void pmg_bug_put(void);

#endif /* PMG_H */
