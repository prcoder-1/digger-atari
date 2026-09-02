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

/* Состояние звука съедаемого золота, его ведёт кадровое прерывание */
extern uint8_t  sfx_money;
extern uint16_t sfx_m1;
extern uint16_t sfx_m2;
extern uint8_t  sfx_mvol;
extern uint8_t  sfx_env_vol;

uint8_t sfx_started;

static uint8_t left;            /* тактов до гашения текущего эффекта */

/* Период БК -> 16-битный делитель POKEY, формула из шапки sfx.h */
static uint16_t bk_to_div(uint16_t bk_period)
{
    uint16_t n = (uint16_t)(((uint32_t)bk_period * 238u) >> 4);
    return (n < 8u) ? 0u : (uint16_t)(n - 7u);
}

void sfx_init(void)
{
    pokey_init();
    left = 0;
    sfx_started = 0;
}

void sfx_play(uint16_t bk_period, uint8_t timbre, uint8_t vol, uint8_t ticks)
{
    if (bk_period == 0)
        return;

    sfx_money = 0;              /* новый эффект вытесняет золото, как и всё */
    sfx_f = bk_to_div(bk_period);
    sfx_c = (uint8_t)(timbre | (vol & 0x0Fu));
    sfx_apply();

    left = ticks;
    ++sfx_started;
}

/*
 * Звук съедаемого золота. В оригинале (snd_money в Work/TH.C) это не один
 * тон, а тридцать коротких подряд: чётные одним делителем, нечётные другим,
 * и после каждого первый период растёт на одну шестнадцатую, а второй на
 * столько же убывает. Высоты идут навстречу, проходят друг сквозь друга и
 * расходятся - отсюда дребезжание вместо "диня".
 *
 * Периоды 500 и 4000 у оригинала заданы в счётчиках PIT; в единицах БК, из
 * которых считает наш движок, это 25 и 200. Сама последовательность идёт по
 * кадрам, её ведёт кадровое прерывание (см. src/pokey.s).
 */
void sfx_gold(void)
{
    sfx_m1 = bk_to_div(25u);
    sfx_m2 = bk_to_div(200u);
    sfx_mvol = (uint8_t)(SFX_TONE | 12u);
    sfx_env_vol = 0;            /* огибающая на это время молчит */
    left = 0;                   /* и таймер гашения тоже: канал ведёт золото */
    sfx_money = 30u;
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
