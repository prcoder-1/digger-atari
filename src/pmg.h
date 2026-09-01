/*
 * pmg.h - Диггер в аппаратных спрайтах.
 *
 * Собирается только с -DDIGGER_PMG. Игроки не заменяют рисунок, а красят его
 * изнутри: подробности и измерения - в шапке src/pmg.s.
 *
 * Параметры передаются через нулевую страницу, как у блиттера: это снимает
 * всю цену передачи аргументов cc65.
 */
#ifndef PMG_H
#define PMG_H

#include <stdint.h>

extern uint8_t pmg_x;           /* подшаг по X          */
extern uint8_t pmg_y;           /* строка растра        */
extern const uint8_t *pmg_s0;   /* план 0: корпус  */
extern const uint8_t *pmg_s1;   /* план 1: клюв    */
extern const uint8_t *pmg_s2;   /* план 2: колёса  */

void pmg_init(void);
void pmg_show(void);
void pmg_hide(void);

#endif /* PMG_H */
