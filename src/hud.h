/*
 * hud.h - верхняя панель.
 */
#ifndef HUD_H
#define HUD_H

#include <stdint.h>

void hud_init(void);
void hud_score(void);
void hud_lives(uint8_t n);

#endif /* HUD_H */
