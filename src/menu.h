/*
 * menu.h - заставка и таблица рекордов.
 */
#ifndef MENU_H
#define MENU_H

#include <stdint.h>

void menu_init(void);

/* Показывает заставку с таблицей рекордов и ждёт нажатия кнопки */
void menu_show(void);

/* Заносит текущий счёт в таблицу. Возвращает занятое место (1 - лучшее)
 * или 0, если результат в таблицу не попал. */
uint8_t menu_submit_score(void);

#endif /* MENU_H */
