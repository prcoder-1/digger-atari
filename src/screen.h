/*
 * screen.h - кадровый буфер ANTIC F, дисплей-лист и таблицы адресов строк.
 */
#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include "hw.h"

/* Резервируются в screen.s по фиксированным адресам */
extern uint8_t screen_buf[];    /* $9000, 9248 байт */
extern uint8_t dlist[];         /* $B800, 256 байт  */

/*
 * Адреса начала каждой строки растра. Буфер не непрерывен (16 байт дырки
 * перед третьим прогоном), поэтому адрес считается ТОЛЬКО через эти таблицы,
 * а не как база + y * SCR_STRIDE.
 */
extern uint8_t screen_rowlo[SCR_LINES];
extern uint8_t screen_rowhi[SCR_LINES];

/* Указатель на начало строки y растра */
#define SCREEN_ROW(y)  ((uint8_t *)((unsigned)screen_rowlo[y] | \
                                   ((unsigned)screen_rowhi[y] << 8)))

/* Строит таблицы строк и дисплей-лист, чистит буфер, ставит палитру
 * и включает ANTIC. Вызывать один раз при старте. */
void screen_init(void);

/* Байт режима строки растра в дисплей-листе: туда ставится бит DL_DLI.
 * Раскладка со сдвигами на LMS расписана в screen.c. */
uint8_t *screen_dlist_line(uint8_t line);

/* Заливает весь буфер байтом value (value=0 - чёрный фон). */
void screen_clear(uint8_t value);

/* Восстанавливает экран ОС - вызывать перед выходом в DOS. */
void screen_done(void);

/* Тряска поля: короткий отскок вниз и обратно, шаг по кадрам.
 * Ведёт кадровое прерывание, см. src/vbi.s. */
void screen_shake(void);

#endif /* SCREEN_H */
