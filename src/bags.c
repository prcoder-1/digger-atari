/*
 * bags.c - мешки с деньгами.
 */
#include "bags.h"
#include "field.h"
#include "blit.h"
#include "game.h"
#include "sfx.h"

extern const uint8_t spr_b4_p0[], spr_b4_p1[];   /* стоит / наклон влево  */
extern const uint8_t spr_b5_p0[], spr_b5_p1[];   /* наклон вправо         */
extern const uint8_t spr_b6_p0[], spr_b6_p1[];   /* падает                */
extern const uint8_t spr_b7_p0[], spr_b7_p1[];   /* разбивается, кадр 1   */
extern const uint8_t spr_b8_p0[], spr_b8_p1[];   /* разбивается, кадр 2   */
extern const uint8_t spr_b9_p0[], spr_b9_p1[];   /* золото                */

void __fastcall__ score_add(unsigned int bcd);

bag_t bags[MAX_BAGS];

/* Все биты укусов: пять горизонтальных и шесть вертикальных.
 * Клетка нетронута, когда все они установлены (см. шапку bags.h). */
#define BG_ANY_BITE   (BG_HORIZ_ALL | BG_VERT_ALL)      /* 0x0FDF */

#define BAG_FALL_STEP (2u * SUBSTEP_Y_LINES)    /* падает вдвое быстрее ходьбы */
#define BREAK_TICKS   3u
#define GOLD_SCORE    0x0500u                   /* 500 очков за золото */

static const uint8_t *const bag_spr[6][2] = {
    { spr_b4_p0, spr_b4_p1 },   /* BAG_NONE не рисуется, место занято  */
    { spr_b4_p0, spr_b4_p1 },   /* стоит      */
    { spr_b5_p0, spr_b5_p1 },   /* качается   */
    { spr_b6_p0, spr_b6_p1 },   /* падает     */
    { spr_b7_p0, spr_b7_p1 },   /* разбивается */
    { spr_b9_p0, spr_b9_p1 },   /* золото     */
};

static void bag_draw(const bag_t *b)
{
    uint8_t px = (uint8_t)(b->x * SUBSTEP_X_PX);

    bl_x = (uint8_t)(px >> 2);
    bl_y = b->y;
    bl_h = SPRITE_H;
    if (px & 2u) {
        bl_w = 3;
        bl_src = bag_spr[b->state][1];
    } else {
        bl_w = 2;
        bl_src = bag_spr[b->state][0];
    }
    blit_mask();
}

/* Гасит место, где мешок был, чтобы за ним не тянулся след */
static void bag_erase(const bag_t *b)
{
    cp_x = (uint8_t)(b->x * SUBSTEP_X_PX);
    cp_y = b->y;
    cp_w = SPRITE_W_PX;
    cp_h = SPRITE_H;
    clear_px();
}

void bags_init(uint8_t screen)
{
    uint8_t x, y, n = 0;

    for (x = 0; x < MAX_BAGS; ++x)
        bags[x].state = BAG_NONE;

    for (y = 0; y < H_MAX; ++y) {
        for (x = 0; x < W_MAX; ++x) {
            if (level_sym(screen, x, y) != LEV_BAG)
                continue;
            if (n >= MAX_BAGS)
                return;
            bags[n].state = BAG_STANDING;
            bags[n].x = (uint8_t)(FIELD_X_OFFSET + x * POS_X_STEP);
            bags[n].y = (uint8_t)(FIELD_Y_OFFSET + y * CELL_H_PX);
            bags[n].count = 0;
            bags[n].fell = 0;
            ++n;
        }
    }
}

/* Есть ли опора под мешком: клетка снизу должна быть целиком нетронута */
static uint8_t supported(const bag_t *b)
{
    uint8_t xl = x_log_tbl[b->x];
    uint8_t yl = (uint8_t)(y_log_tbl[b->y] + 1u);

    if (yl >= H_MAX)
        return 1;                       /* дно поля держит */
    return (uint8_t)((background[yl][xl] & BG_ANY_BITE) == BG_ANY_BITE);
}

uint8_t bags_push(uint8_t x_log, uint8_t y_log, uint8_t dir)
{
    uint8_t i;

    for (i = 0; i < MAX_BAGS; ++i) {
        bag_t *b = &bags[i];
        if (b->state != BAG_STANDING && b->state != BAG_LOOSE)
            continue;
        if (x_log_tbl[b->x] != x_log || y_log_tbl[b->y] != y_log)
            continue;

        /* Толкать можно только вбок и только в пределах поля */
        if (dir == DIR_RIGHT) {
            if (b->x + 1u > (FIELD_X_OFFSET + (W_MAX - 1u) * POS_X_STEP))
                return 0;
            bag_erase(b);
            ++b->x;
        } else if (dir == DIR_LEFT) {
            if (b->x <= FIELD_X_OFFSET)
                return 0;
            bag_erase(b);
            --b->x;
        } else {
            return 0;                   /* сверху и снизу мешок не сдвинуть */
        }
        b->count = 0;
        b->state = BAG_STANDING;
        bag_draw(b);
        return 1;
    }
    return 1;                           /* мешка в этой клетке нет */
}

void bags_tick(void)
{
    uint8_t i;

    for (i = 0; i < MAX_BAGS; ++i) {
        bag_t *b = &bags[i];

        switch (b->state) {
        case BAG_NONE:
            continue;

        case BAG_STANDING:
            if (!supported(b)) {
                b->state = BAG_LOOSE;
                b->count = LOOSE_WAIT;
            }
            break;

        case BAG_LOOSE:
            /* Покачивание: четыре тона по кругу, звучит через такт */
            if ((b->count & 1u) == 0u) {
                static const uint16_t wobble[4] = { 125u, 150u, 125u, 100u };
                sfx_play(wobble[(b->count >> 1) & 3u], SFX_TONE, 6u, 1u);
            }
            if (supported(b)) {         /* опору вернули - мешок успокоился */
                b->state = BAG_STANDING;
            } else if (--b->count == 0) {
                b->state = BAG_FALLING;
                b->fell = 0;
            }
            break;

        case BAG_FALLING:
            bag_erase(b);
            b->y = (uint8_t)(b->y + BAG_FALL_STEP);
            /* Падение: тон тем ниже, чем дольше летит мешок */
            sfx_play((uint16_t)(32u + b->fell * 4u), SFX_TONE, 8u, 1u);
            if (b->fell < 40u)
                ++b->fell;
            /* Падение останавливается только на границе клетки и только
             * если под мешком снова целая земля - как в оригинале. */
            if (y_rem_tbl[b->y] == 0 && supported(b)) {
                b->state = b->fell ? BAG_BREAKING : BAG_STANDING;
                if (b->fell)
                    sfx_play(750u, SFX_THUD, 12u, 2u);   /* глухой удар */
                b->count = BREAK_TICKS;
            }
            if (b->y >= (FIELD_Y_OFFSET + (H_MAX - 1u) * CELL_H_PX)) {
                b->y = (uint8_t)(FIELD_Y_OFFSET + (H_MAX - 1u) * CELL_H_PX);
                b->state = BAG_BREAKING;
                b->count = BREAK_TICKS;
            }
            break;

        case BAG_BREAKING:
            if (--b->count == 0) {
                b->state = BAG_GOLD;
                b->count = 0;
            }
            break;

        case BAG_GOLD:
            break;

        default:
            break;
        }

        bag_draw(b);
    }
}

/* Забирает золото из клетки, если оно там лежит */
uint8_t bags_take_gold(uint8_t x_log, uint8_t y_log)
{
    uint8_t i;
    for (i = 0; i < MAX_BAGS; ++i) {
        bag_t *b = &bags[i];
        if (b->state != BAG_GOLD)
            continue;
        if (x_log_tbl[b->x] != x_log || y_log_tbl[b->y] != y_log)
            continue;
        bag_erase(b);
        b->state = BAG_NONE;
        score_add(GOLD_SCORE);
        sfx_play(25u, SFX_TONE, 12u, 3u);       /* съедание золота */
        return 1;
    }
    return 0;
}
