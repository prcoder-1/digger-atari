/*
 * dli.h - прерывание по строке: своя палитра у панели счёта и у поля.
 *
 * Реализация в src/dli.s. Краски поля кладутся в dli_pf1/dli_pf2 перед
 * dli_install(); краски панели живут в теневых ячейках ОС (OS_COLOR1/COLOR2).
 */
#ifndef DLI_H
#define DLI_H

#include <stdint.h>

extern uint8_t dli_pf1;         /* яркость рисунка поля -> COLPF1 */
extern uint8_t dli_pf2;         /* фон и оттенок поля   -> COLPF2 */

/* Ставит вектор и разрешает строчное прерывание. Вызывать после того, как в
 * дисплей-листе выставлен бит DLI, и до включения ANTIC. */
void dli_install(void);

/* Возвращает прежний вектор и гасит строчное прерывание. */
void dli_remove(void);

#endif /* DLI_H */
