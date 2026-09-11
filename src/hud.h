/*
 * hud.h - верхняя панель.
 */
#ifndef HUD_H
#define HUD_H

#include <stdint.h>

/* Больше значков в ряд не помещается; оригинал держит тот же предел
 * (get_lives(1)<5 в add_score, Work/TF.C). */
#define MAX_LIVES   6u

void hud_init(void);
void hud_score(void);
void hud_lives(uint8_t n);

#endif /* HUD_H */
