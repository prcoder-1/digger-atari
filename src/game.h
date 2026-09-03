/*
 * game.h - состояние игры.
 *
 * Перечисления оригинала и порта на БК записаны через typedef + #define:
 * cc65 не понимает "enum ... : uint8_t" из C23, а без указания базового типа
 * поле перечисления заняло бы два байта, что ломает размеры структур.
 */
#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "hw.h"

/* Направления. Порядок важен: dir ^ 1 даёт противоположное. */
typedef uint8_t dir_t;
#define DIR_RIGHT   0u
#define DIR_LEFT    1u
#define DIR_DOWN    2u
#define DIR_UP      3u
#define DIR_STOP    4u

/*
 * Координаты:
 *   x - в подшагах по 2 пикселя, 5 подшагов на клетку
 *   y - в строках растра,        6 подшагов по 3 строки на клетку
 * Положение существа - это ЛЕВЫЙ ВЕРХНИЙ угол его клетки, как в оригинале.
 */
typedef struct {
    uint8_t x;
    uint8_t y;
    dir_t   dir;            /* текущее направление  */
    dir_t   want;           /* желаемое направление */
    /*
     * Направление ШАГА в этом такте - DIR_STOP, если Диггер стоит. Это m_dir
     * оригинала (Work/TC.C), и от dir оно отличается: dir - то, куда Диггер
     * смотрит, и оно сохраняется на месте. По move решается, держит ли он
     * собой мешок, см. get_man_block и man_blocks в src/bags.c.
     */
    dir_t   move;
    uint8_t frame;          /* фаза анимации 0..2   */
    int8_t  frame_inc;      /* направление перебора фаз */
} man_t;

extern man_t man;

/* Границы поля в тех же единицах */
#define MAN_X_MIN   FIELD_X_OFFSET
#define MAN_X_MAX   (FIELD_X_OFFSET + (W_MAX - 1) * POS_X_STEP)
#define MAN_Y_MIN   FIELD_Y_OFFSET
#define MAN_Y_MAX   (FIELD_Y_OFFSET + (H_MAX - 1) * CELL_H_PX)

/* Стартовая клетка Диггера - как в оригинале */
#define MAN_START_X 7u
#define MAN_START_Y 9u

void man_init(void);
void man_tick(dir_t want);      /* один игровой такт */

#endif /* GAME_H */
