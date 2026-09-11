/*
 * sfx.c - звуковые эффекты.
 */
#include "sfx.h"

/*
 * Статические данные модуля уезжают в ОЗУ под ПЗУ: MAIN забит под завязку.
 * Обнуляет этот сегмент clear_hi_bss() первым делом в main - стартовый код
 * cc65 сюда не дотягивается, он чистит только сегмент BSS.
 */
#ifdef __CC65__
#pragma bss-name (push, "BSS_HI")
#endif


extern uint16_t sfx_f;
extern uint8_t  sfx_c;
#ifdef __CC65__
#pragma zpsym ("sfx_f")
#endif
#ifdef __CC65__
#pragma zpsym ("sfx_c")
#endif

void pokey_init(void);
void pokey_silence(void);
void sfx_apply(void);

/* Состояние звука съедаемого золота, его ведёт кадровое прерывание */
extern uint8_t  sfx_money;
extern uint16_t sfx_m1;
extern uint16_t sfx_m2;
extern uint8_t  sfx_mvol;
extern uint8_t  sfx_env_vol;
extern uint8_t  sfx_pulse, sfx_pph, sfx_pvol;
extern uint8_t  sfx_bugcnt, sfx_bugset, sfx_bugvol;
extern uint8_t  sfx_bugf[2];
extern uint8_t  sfx_pf[2], sfx_pclick[2];

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
    sfx_pulse = 0;              /* и звук камня тоже */
    sfx_bugset = 0;             /* и съеденного врага */
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

/*
 * Звук съедаемого камня: щелчок и пульсирующая нота, см. src/pokey.s.
 * step - номер камня в цепочке, от него растёт высота: C5, D5 ... C6.
 * Периоды взяты из порта на БК.
 */

void sfx_coin(uint8_t step)
{
    static const uint16_t note[8] = { 114u, 101u, 90u, 85u, 76u, 67u, 60u, 57u };
    uint16_t f = bk_to_div(note[step & 7u]);
    /* Щелчок: у оригинала t_period 1000 счётчика PIT, в единицах БК это 50 */
    uint16_t c = bk_to_div(50u);

    sfx_pf[0] = (uint8_t)f;      sfx_pf[1] = (uint8_t)(f >> 8);
    sfx_pclick[0] = (uint8_t)c;  sfx_pclick[1] = (uint8_t)(c >> 8);
    sfx_pvol = (uint8_t)(SFX_TONE | 10u);
    sfx_env_vol = 0;            /* огибающая на это время молчит */
    sfx_money = 0;
    left = 0;                   /* канал ведёт последовательность */
    sfx_pph = 0;
    sfx_pulse = 7u;             /* семь импульсов, как coin_2_count */
    ++sfx_started;
}

/*
 * Звук съеденного в бонус-режиме врага: три восходящих свипа, см. src/pokey.s.
 * Начальный делитель ставит сам обработчик, здесь только запуск.
 */
void sfx_bug(void)
{
    sfx_env_vol = 0;            /* огибающая на это время молчит */
    sfx_money = 0;
    sfx_pulse = 0;
    left = 0;                   /* канал ведёт последовательность */
    sfx_bugvol = (uint8_t)(SFX_TONE | 12u);
    sfx_bugcnt = 0;             /* нулевой счётчик заведёт первый свип */
    sfx_bugset = 4u;            /* три свипа: первый расходуется на завод */
    ++sfx_started;
}

void sfx_tick(void)
{
    if (left != 0u && --left == 0u)
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
#ifdef __CC65__
#pragma zpsym ("mus_base")
#endif
#ifdef __CC65__
#pragma zpsym ("mus_egt")
#endif

void music_start(void);
void music_stop(void);

extern const uint8_t music_popcorn[];
extern const uint8_t music_bonus[];

/*
 * Форма ноты. В оригинале мелодия идёт широтно-импульсным источником, и у
 * каждой ноты своя огибающая: ширина импульса нарастает до attack_level,
 * спадает к hold_level, а через hold_count тиков падает в ноль (chk_music и
 * init_music в Work/TH.C). Ровных нот там нет.
 *
 * На POKEY та же форма ложится на громкость. Числа оригинала переведены
 * дважды: по величине (ширина 1..50 -> громкость 0..15, множитель 0.3) и по
 * времени (там тик звука около 33 Гц, у нас кадр 50 Гц, множитель 0.66).
 * Итого скорости умножены на 0.2, уровни - на 0.3, и всё это в формате 8.8.
 *
 *                 атака  удержание  спад   уровень удержания
 *   обычная         50       15       1            8
 *   бонусная        20       10       4           20
 */
extern uint8_t mus_atk[2], mus_hlvl[2], mus_hrate[2], mus_drate[2];
extern uint8_t mus_hcnt;

static void music_shape(uint16_t atk, uint16_t hlvl,
                        uint16_t hrate, uint16_t drate, uint8_t hcnt)
{
    mus_atk[0]   = (uint8_t)atk;   mus_atk[1]   = (uint8_t)(atk >> 8);
    mus_hlvl[0]  = (uint8_t)hlvl;  mus_hlvl[1]  = (uint8_t)(hlvl >> 8);
    mus_hrate[0] = (uint8_t)hrate; mus_hrate[1] = (uint8_t)(hrate >> 8);
    mus_drate[0] = (uint8_t)drate; mus_drate[1] = (uint8_t)(drate >> 8);
    mus_hcnt     = hcnt;
}

void music_select(uint8_t bonus)
{
    const uint8_t *p = bonus ? music_bonus : music_popcorn;
    uint16_t eighth = bonus ? 0x048Au : 0x0915u;

    /*
     * Уровни удержания опущены в той же пропорции, что и потолок громкости в
     * music.s (15 -> 10, то есть две трети): иначе музыка стала бы тише только
     * на атаке, а тянущаяся часть ноты осталась бы прежней громкости.
     *
     *      бонус    6.00 -> 4.00
     *      обычная  2.40 -> 1.60
     */
    if (bonus)
        music_shape(0x03F6u, 0x0400u, 0x01FBu, 0x00CBu, 8u);
    else
        music_shape(0x09E7u, 0x0199u, 0x02F8u, 0x0033u, 18u);

    mus_base[0] = (uint8_t)(uint16_t)p;
    mus_base[1] = (uint8_t)(((uint16_t)p) >> 8);
    mus_egt[0]  = (uint8_t)eighth;
    mus_egt[1]  = (uint8_t)(eighth >> 8);
    music_start();
}

#ifdef __CC65__
#pragma bss-name (pop)
#endif
