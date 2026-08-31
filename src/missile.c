/*
 * missile.c - файрбол.
 */
#include "missile.h"
#include "game.h"
#include "field.h"
#include "bugs.h"
#include "blit.h"
#include "hud.h"
#include "sfx.h"

extern const uint8_t spr_f1_p0[], spr_f1_p1[];
extern const uint8_t spr_f2_p0[], spr_f2_p1[];
extern const uint8_t spr_f3_p0[], spr_f3_p1[];
extern const uint8_t spr_e1_p0[], spr_e1_p1[];
extern const uint8_t spr_e2_p0[], spr_e2_p1[];
extern const uint8_t spr_e3_p0[], spr_e3_p1[];

void __fastcall__ score_add(unsigned int bcd);

#define MIS_H         7u        /* снаряд ниже существ: 8x7 пикселей */
#define MIS_STEP_X    2u        /* подшага за такт - вдвое быстрее ходьбы */
#define MIS_STEP_Y    (2u * SUBSTEP_Y_LINES)
#define EXPLODE_TICKS 3u
#define BUG_SCORE     0x0250u   /* 250 очков за врага */

/*
 * Накопительные счётчики для демо-проверки. Мгновенное состояние снаряда о
 * работе почти ничего не говорит: снимок ловит одну точку из сотен тактов,
 * и ноль одинаково означает "перезаряжается" и "ни разу не стрелял".
 * Счётчики отвечают на вопрос с первого прогона.
 */
uint8_t mis_n_fired;
uint8_t mis_n_blocked;
uint8_t mis_n_offscreen;

static struct {
    uint8_t state;
    uint8_t x, y;
    uint8_t dir;
    uint8_t count;              /* фаза взрыва       */
    uint8_t reload;             /* тактов до готовности */
    uint8_t reload_max;
} mis;

static const uint8_t *const fly_spr[3][2] = {
    { spr_f1_p0, spr_f1_p1 },
    { spr_f2_p0, spr_f2_p1 },
    { spr_f3_p0, spr_f3_p1 },
};
static const uint8_t *const boom_spr[3][2] = {
    { spr_e1_p0, spr_e1_p1 },
    { spr_e2_p0, spr_e2_p1 },
    { spr_e3_p0, spr_e3_p1 },
};

void missile_init(uint8_t difficulty)
{
    mis.state = MIS_NONE;
    mis.reload = 0;
    mis_n_fired = mis_n_blocked = mis_n_offscreen = 0;
#ifdef DEMO_MISSILE
    /* Испытательная сборка: перезарядка укорочена, чтобы за один прогон
     * прошло много выстрелов и ветка попадания успела сработать. Сама
     * проверка попадания при этом не меняется. В игру это не входит. */
    (void)difficulty;
    mis.reload_max = 3u;
#else
    /* Перезарядка растёт со сложностью - формула порта на БК */
    mis.reload_max = (uint8_t)(81u + 4u * difficulty);
#endif
}

uint8_t missile_state(void)
{
    return mis.state;
}

static void mis_draw(const uint8_t *const spr[3][2], uint8_t phase)
{
    uint8_t px = (uint8_t)(mis.x * SUBSTEP_X_PX);

    bl_x = (uint8_t)(px >> 2);
    bl_y = mis.y;
    bl_h = MIS_H;
    if (px & 2u) {
        bl_w = 3;
        bl_src = spr[phase][1];
    } else {
        bl_w = 2;
        bl_src = spr[phase][0];
    }
    blit_mask();
}

static void mis_erase(void)
{
    cp_x = (uint8_t)(mis.x * SUBSTEP_X_PX);
    cp_y = mis.y;
    cp_w = SPRITE_W_PX;
    cp_h = MIS_H;
    clear_px();
}

void missile_fire(void)
{
    if (mis.state != MIS_NONE || mis.reload)
        return;
    ++mis_n_fired;
    sfx_play(12u, SFX_NOISE, 8u, 3u);   /* свист выстрела - шумовой тембр */
    mis.state = MIS_FLYING;
    mis.dir = man.dir;
    mis.x = man.x;
    /* Снаряд летит на середине высоты Диггера */
    mis.y = (uint8_t)(man.y + (SPRITE_H - MIS_H) / 2u);
    mis.count = 0;
}

/* Не упёрся ли снаряд в целую землю */
static uint8_t mis_blocked(void)
{
    uint8_t xl = x_log_tbl[mis.x];
    uint8_t yl = y_log_tbl[mis.y];
    uint16_t bg = background[yl][xl];

    return (uint8_t)((bg & BG_HORIZ_ALL) != 0u && (bg & BG_VERT_ALL) != 0u);
}

/* Попал ли снаряд во врага; при попадании враг снимается с поля */
static uint8_t mis_hit_bug(void)
{
    uint8_t i;
    for (i = 0; i < MAX_BUGS; ++i) {
        bug_t *b = &bugs[i];
        if (b->state != BUG_ALIVE)
            continue;
        if ((uint8_t)(b->x - mis.x + 3u) < 7u &&
            (uint16_t)(b->y - mis.y + 10u) < 21u) {
            b->state = BUG_NONE;
            if (bugs_active)
                --bugs_active;
            score_add(BUG_SCORE);
            sfx_play(75u, SFX_ROUGH, 12u, 3u);  /* взрыв */
            hud_score();
            return 1;
        }
    }
    return 0;
}

void missile_tick(void)
{
    if (mis.reload)
        --mis.reload;

    switch (mis.state) {
    case MIS_NONE:
        return;

    case MIS_FLYING:
        mis_erase();
        switch (mis.dir) {
        case DIR_RIGHT:
            if (mis.x + MIS_STEP_X > MAN_X_MAX) { ++mis_n_offscreen; goto fizzle; }
            mis.x = (uint8_t)(mis.x + MIS_STEP_X);
            break;
        case DIR_LEFT:
            if (mis.x < MAN_X_MIN + MIS_STEP_X) { ++mis_n_offscreen; goto fizzle; }
            mis.x = (uint8_t)(mis.x - MIS_STEP_X);
            break;
        case DIR_DOWN:
            if (mis.y + MIS_STEP_Y > MAN_Y_MAX) { ++mis_n_offscreen; goto fizzle; }
            mis.y = (uint8_t)(mis.y + MIS_STEP_Y);
            break;
        case DIR_UP:
            if (mis.y < MAN_Y_MIN + MIS_STEP_Y) { ++mis_n_offscreen; goto fizzle; }
            mis.y = (uint8_t)(mis.y - MIS_STEP_Y);
            break;
        default:
            goto fizzle;
        }

        if (mis_hit_bug()) {
            mis.state = MIS_EXPLODING;
            mis.count = EXPLODE_TICKS;
            mis_draw(boom_spr, 0);
            return;
        }
        if (mis_blocked()) {
            ++mis_n_blocked;
            goto fizzle;
        }

        ++mis.count;
        mis_draw(fly_spr, (uint8_t)(mis.count % 3u));
        return;

    case MIS_EXPLODING:
        mis_draw(boom_spr, (uint8_t)(mis.count % 3u));
        if (--mis.count == 0) {
            mis_erase();
            mis.state = MIS_NONE;
            mis.reload = mis.reload_max;
        }
        return;

    default:
        return;
    }

fizzle:
    mis_erase();
    mis.state = MIS_NONE;
    mis.reload = mis.reload_max;
}
