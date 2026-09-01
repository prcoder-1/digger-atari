/*
 * bags.c - мешки с деньгами.
 */
#include "bags.h"
#include "field.h"
#include "blit.h"
#include "game.h"
#include "sfx.h"
#include "bugs.h"
#include "hud.h"

extern const uint8_t spr_b4_p0[], spr_b4_p1[];   /* стоит / наклон влево  */
extern const uint8_t spr_b5_p0[], spr_b5_p1[];   /* наклон вправо         */
extern const uint8_t spr_b6_p0[], spr_b6_p1[];   /* падает                */
extern const uint8_t spr_b7_p0[], spr_b7_p1[];   /* разбивается, кадр 1   */
extern const uint8_t spr_b8_p0[], spr_b8_p1[];   /* разбивается, кадр 2   */
extern const uint8_t spr_b9_p0[], spr_b9_p1[];   /* золото                */

void __fastcall__ score_add(unsigned int bcd);

bag_t bags[MAX_BAGS];

#define BAG_FALL_STEP (2u * SUBSTEP_Y_LINES)    /* падает вдвое быстрее ходьбы */
#define BREAK_TICKS   3u
#define GOLD_SCORE    0x0500u                   /* 500 очков за золото */
#define BAG_BUG_SCORE 0x0250u                   /* 250 очков за придавленного врага */

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
    bl_x = (uint8_t)(b->x >> 1);
    bl_y = b->y;
    bl_h = SPRITE_H;
    if (b->x & 1u) {
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
    cp_x = b->x;
    cp_y = b->y;
    cp_w = SPRITE_W_SUB;
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
            bags[n].dir = DIR_STOP;
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

/*
 * Падающий мешок прогрызает себе шахту: помечает клетку ПОД собой выеденной
 * и уничтожает лежащий там камень. Перенос era_background(...,DOWN) и
 * dest_coin() из оригинала (Work/TB.C).
 *
 * На то, когда мешок остановится, это не влияет - остановка считается по
 * клетке снизу ДО отметки, как и в оригинале. Влияет на другое: без отметки
 * пробитая шахта остаётся дырой только на картинке, а в модели поля там
 * по-прежнему целая земля. Ноббины такой ход не видят и в него не идут, и
 * опора соседних мешков считается по нетронутому грунту.
 */
static void bag_carve(const bag_t *b)
{
    uint8_t xl = x_log_tbl[b->x];
    uint8_t yl = (uint8_t)(y_log_tbl[b->y] + 1u);
    uint8_t yr = y_rem_tbl[b->y];

    if (yl >= H_MAX)
        return;
    background[yl][xl] &= (uint16_t)~(1u << (yr + 6u));
    if ((background[yl][xl] & BG_VERT_ALL) == 0u)
        background[yl][xl] &= (uint16_t)~BG_INTACT;
    field_take_coin(xl, yl);            /* камень в шахте уничтожается */
}

/*
 * Накрывает ли падающий мешок точку (x, y). Габарит тот же, что у проверки
 * столкновения с врагом. Условие y >= b->y из оригинала: мешок бьёт ВНИЗ,
 * тех, кто оказался не выше его, - подойти к падающему мешку снизу смертельно,
 * сверху нет.
 */
static uint8_t bag_covers(const bag_t *b, uint8_t x, uint8_t y)
{
    return (uint8_t)((uint8_t)(x - b->x + 3u) < 7u &&
                     (uint16_t)(y - b->y + 14u) < 29u &&
                     y >= b->y);
}

/* Есть ли падающий мешок, придавивший Диггера */
uint8_t bags_hit_man(void)
{
    uint8_t i;
    for (i = 0; i < MAX_BAGS; ++i)
        if (bags[i].state == BAG_FALLING && bag_covers(&bags[i], man.x, man.y))
            return 1;
    return 0;
}

/* Придавленные падающим мешком враги гибнут - как от выстрела */
static void bag_crush_bugs(const bag_t *b)
{
    uint8_t j;
    for (j = 0; j < MAX_BUGS; ++j) {
        if (bugs[j].state != BUG_ALIVE)
            continue;
        if (!bag_covers(b, bugs[j].x, bugs[j].y))
            continue;
        bugs_remove(j);
        score_add(BAG_BUG_SCORE);
        hud_score();
        sfx_play(120u, SFX_THUD, 12u, 2u);
    }
}

/* Ищет стоящий или качающийся мешок в клетке; 0xFF - мешка нет */
static uint8_t bag_at(uint8_t x_log, uint8_t y_log)
{
    uint8_t i;
    for (i = 0; i < MAX_BAGS; ++i) {
        const bag_t *b = &bags[i];
        if (b->state != BAG_STANDING && b->state != BAG_LOOSE)
            continue;
        if (x_log_tbl[b->x] == x_log && y_log_tbl[b->y] == y_log)
            return i;
    }
    return 0xFFu;
}

/*
 * Толкание вбок с учётом ЦЕПОЧКИ мешков: если за мешком стоит другой,
 * двигаются оба, и так далее, пока цепочка не упрётся в край поля.
 * Раньше двигался только первый, а стоящий за ним просто блокировал ход -
 * в оригинале Диггер расталкивает подряд стоящие мешки.
 *
 * Цепочка проходится списком, а не рекурсией: в оригинале это рекурсивная
 * процедура, но здесь глубина ограничена семью мешками, а итерация
 * не зависит от того, как собран модуль (рекурсию в cc65 ломает
 * ключ -Cl, и об этом легко забыть).
 */
uint8_t bags_push(uint8_t x_log, uint8_t y_log, uint8_t dir)
{
    uint8_t chain[MAX_BAGS];
    uint8_t n = 0;
    uint8_t cx = x_log;
    uint8_t i, k;

    if (dir != DIR_RIGHT && dir != DIR_LEFT)
        return 0;                       /* сверху и снизу мешок не сдвинуть */

    /* Собрать подряд стоящие мешки в направлении толчка */
    for (;;) {
        i = bag_at(cx, y_log);
        if (i == 0xFFu)
            break;
        if (n >= MAX_BAGS)
            return 0;
        chain[n++] = i;
        if (dir == DIR_RIGHT) {
            if (cx + 1u >= W_MAX)
                return 0;               /* цепочка упёрлась в край */
            ++cx;
        } else {
            if (cx == 0u)
                return 0;
            --cx;
        }
    }

    if (n == 0)
        return 1;                       /* мешков на пути нет */

    /* Двигать с дальнего конца, иначе мешки наехали бы друг на друга */
    for (k = n; k > 0; --k) {
        bag_t *b = &bags[chain[k - 1u]];
        bag_erase(b);
        if (dir == DIR_RIGHT)
            ++b->x;
        else
            --b->x;
        b->count = 0;
        b->dir = dir;                   /* мешок катится, а не стоит */
        b->state = BAG_STANDING;
        bag_draw(b);
    }
    return 1;
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
            /*
             * Проверка только на границе клетки по X: мешок, застигнутый
             * посреди толчка, падать не должен.
             *
             * Дальше решает направление. Мешок, который ТОЛКАЮТ вбок и
             * закатили на пустое место, проваливается сразу - раскачка тут
             * ни к чему, его столкнули намеренно. Раскачивается только тот,
             * что стоял неподвижно и под кем подкопали: это и есть
             * предупреждение, что сейчас упадёт.
             */
            if (x_rem_tbl[b->x] == 0) {
                if (!supported(b)) {
                    if (b->dir == DIR_LEFT || b->dir == DIR_RIGHT) {
                        b->state = BAG_FALLING;
                        b->fell = 0;
                    } else {
                        b->state = BAG_LOOSE;
                        b->count = LOOSE_WAIT;
                    }
                }
                if (b->state != BAG_FALLING)
                    b->dir = DIR_STOP;
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
            bag_carve(b);               /* до сдвига: отмечается клетка снизу */
            b->y = (uint8_t)(b->y + BAG_FALL_STEP);
            bag_crush_bugs(b);
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
