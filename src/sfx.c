/*
 * sfx.c - звуковые эффекты.
 */
#include "sfx.h"

extern uint16_t sfx_f;
extern uint8_t  sfx_c;
#pragma zpsym ("sfx_f")
#pragma zpsym ("sfx_c")

void pokey_init(void);
void pokey_silence(void);
void sfx_apply(void);

uint8_t sfx_started;

static uint8_t left;            /* тактов до гашения текущего эффекта */

void sfx_init(void)
{
    pokey_init();
    left = 0;
    sfx_started = 0;
}

void sfx_play(uint16_t bk_period, uint8_t timbre, uint8_t vol, uint8_t ticks)
{
    uint16_t n;

    if (bk_period == 0)
        return;

    /* N = 14.8615 * P - 7, посчитано сдвигом: (P * 238) >> 4 даёт 14.875 */
    n = (uint16_t)(((uint32_t)bk_period * 238u) >> 4);
    if (n < 8u)
        n = 0;
    else
        n -= 7u;

    sfx_f = n;
    sfx_c = (uint8_t)(timbre | (vol & 0x0Fu));
    sfx_apply();

    left = ticks;
    ++sfx_started;
}

void sfx_tick(void)
{
    if (left == 0)
        return;
    if (--left == 0)
        pokey_silence();
}

/*
 * Выбор мелодии. Длительность восьмой хранится в формате 8.8 кадров:
 * у основной мелодии это 181.6 мс = 9.08 кадра ($0915), у бонусной вдвое
 * быстрее - 90.8 мс = 4.54 кадра ($048A). Целое число кадров дало бы
 * заметный уход темпа, поэтому дробная часть обязательна.
 */
extern uint8_t mus_base[2];
extern uint8_t mus_egt[2];
#pragma zpsym ("mus_base")
#pragma zpsym ("mus_egt")

void music_start(void);
void music_stop(void);

extern const uint8_t music_popcorn[];
extern const uint8_t music_bonus[];

void music_select(uint8_t bonus)
{
    const uint8_t *p = bonus ? music_bonus : music_popcorn;
    uint16_t eighth = bonus ? 0x048Au : 0x0915u;

    mus_base[0] = (uint8_t)(uint16_t)p;
    mus_base[1] = (uint8_t)(((uint16_t)p) >> 8);
    mus_egt[0]  = (uint8_t)eighth;
    mus_egt[1]  = (uint8_t)(eighth >> 8);
    music_start();
}
