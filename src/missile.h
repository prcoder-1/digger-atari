/*
 * missile.h - файрбол.
 *
 * Летит по прямой в том направлении, куда смотрел Диггер в момент выстрела,
 * вдвое быстрее ходьбы. Гаснет, упёршись в землю или в край поля, и взрывается
 * на враге. Пока снаряд в полёте, выстрелить снова нельзя; после взрыва идёт
 * задержка перезарядки, растущая со сложностью - как в оригинале.
 */
#ifndef MISSILE_H
#define MISSILE_H

#include <stdint.h>

#define MIS_NONE      0u
#define MIS_FLYING    1u
#define MIS_EXPLODING 2u

void missile_init(uint8_t difficulty);
void missile_fire(void);        /* выстрел, если снаряд готов */
void missile_tick(void);
/* Убрать снаряд с экрана при гибели Диггера */
void missile_clear(void);

uint8_t missile_state(void);

/* Готов ли выстрел: снаряда нет на поле и перезарядка кончилась. По этому же
 * признаку у Диггера рисуется башенка - см. src/man.c. */
uint8_t missile_ready(void);

#endif /* MISSILE_H */
