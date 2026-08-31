/*
 * hud.c - верхняя панель: счёт, жизни и отладочные числа демо-режима.
 *
 * Панель занимает строки 0..27, поле начинается со строки 28. Символ
 * знакогенератора ПЗУ - 8x8 пикселей, то есть 2 байта экрана и 8 строк.
 */
#include "hud.h"
#include "hw.h"
#include "text.h"
#include "blit.h"

extern const uint8_t spr_s0_p0[];       /* значок жизни */

extern uint8_t score[3];

#define SCORE_X     1u                  /* байт экрана */
#define SCORE_Y     2u
#define LIVES_X     28u
#define LIVES_Y     2u
#define MAX_LIVES   6u

static uint8_t lives_shown = 0xFF;

void hud_init(void)
{
    text_init(2);                       /* цвет надписей - COLPF1 */
    lives_shown = 0xFF;
    /* Подписи SCORE нет: в оригинале счёт стоит сам по себе */
}

void hud_score(void)
{
    text_bcd6(SCORE_X, SCORE_Y, score);
}

void hud_lives(uint8_t n)
{
    uint8_t i;

    if (n == lives_shown)
        return;
    lives_shown = n;

    if (n > MAX_LIVES)
        n = MAX_LIVES;

    /* Полоса значков перерисовывается целиком: она короткая, а следить за
     * тем, какой именно значок пропал, дороже, чем стереть и нарисовать. */
    cp_x = (uint8_t)(LIVES_X * 4u);
    cp_y = LIVES_Y;
    cp_w = (uint8_t)(MAX_LIVES * 8u);
    cp_h = 15u;
    clear_px();

    for (i = 0; i < n; ++i) {
        bl_x = (uint8_t)(LIVES_X + i * 2u);
        bl_y = LIVES_Y;
        bl_w = 2u;
        bl_h = 15u;
        bl_src = spr_s0_p0;
        blit_mask();
    }
}
