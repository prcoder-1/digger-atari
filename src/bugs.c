/*
 * bugs.c - враги.
 */
#include "bugs.h"
#include "field.h"
#include "blit.h"
#include "game.h"

/*
 * Ноббин и Хоббин - разные существа и внешне. В исходниках оригинала
 * спрайты подписаны так: m - "существо обычного типа" (это Ноббин),
 * c - "существо, преследующее вас, вправо", a - то же влево (это Хоббин,
 * который грызёт землю). Игрок должен видеть разницу: Хоббина нельзя
 * переждать за стенкой, он придёт сквозь землю.
 */
extern const uint8_t spr_m0_p0[], spr_m0_p1[];
extern const uint8_t spr_m1_p0[], spr_m1_p1[];
extern const uint8_t spr_m2_p0[], spr_m2_p1[];
extern const uint8_t spr_c0_p0[], spr_c0_p1[];
extern const uint8_t spr_c1_p0[], spr_c1_p1[];
extern const uint8_t spr_c2_p0[], spr_c2_p1[];
extern const uint8_t spr_a0_p0[], spr_a0_p1[];
extern const uint8_t spr_a1_p0[], spr_a1_p1[];
extern const uint8_t spr_a2_p0[], spr_a2_p1[];

bug_t bugs[MAX_BUGS];
uint8_t bugs_active;
uint8_t bugs_fleeing;

static uint8_t nest_x, nest_y;          /* клетка гнезда */
static uint8_t spawn_delay;             /* тактов между появлениями */
static uint8_t spawn_count;
static uint8_t bugs_total;              /* сколько ещё родится за уровень */
static uint8_t bugs_diff;               /* сложность уровня */

/* [облик: 0 Ноббин, 1 Хоббин вправо, 2 Хоббин влево][фаза][сдвиг] */
static const uint8_t *const bug_spr[3][3][2] = {
    { { spr_m0_p0, spr_m0_p1 }, { spr_m1_p0, spr_m1_p1 },
      { spr_m2_p0, spr_m2_p1 } },
    { { spr_c0_p0, spr_c0_p1 }, { spr_c1_p0, spr_c1_p1 },
      { spr_c2_p0, spr_c2_p1 } },
    { { spr_a0_p0, spr_a0_p1 }, { spr_a1_p0, spr_a1_p1 },
      { spr_a2_p0, spr_a2_p1 } },
};

static void bug_draw(const bug_t *b)
{
    uint8_t px = (uint8_t)(b->x * SUBSTEP_X_PX);
    uint8_t look;

    /* У Хоббина своя графика на каждую сторону; при движении по вертикали
     * сохраняется та, что была по горизонтали в последний раз. */
    if (b->kind != BUG_HOBBIN)
        look = 0;
    else
        look = (uint8_t)((b->dir == DIR_LEFT) ? 2u : 1u);

    bl_x = (uint8_t)(px >> 2);
    bl_y = b->y;
    bl_h = SPRITE_H;
    if (px & 2u) {
        bl_w = 3;
        bl_src = bug_spr[look][b->frame % 3u][1];
    } else {
        bl_w = 2;
        bl_src = bug_spr[look][b->frame % 3u][0];
    }
    blit_mask();
}

static void bug_erase(const bug_t *b)
{
    cp_x = (uint8_t)(b->x * SUBSTEP_X_PX);
    cp_y = b->y;
    cp_w = SPRITE_W_PX;
    cp_h = SPRITE_H;
    clear_px();
}

void bugs_init(uint8_t screen, uint8_t difficulty)
{
    uint8_t x, y, i;

    for (i = 0; i < MAX_BUGS; ++i)
        bugs[i].state = BUG_NONE;
    bugs_active = 0;
    bugs_fleeing = 0;

    /* Гнездо - клетка, где сходятся оба прохода (символ S карты) */
    nest_x = W_MAX - 1u;
    nest_y = 0;
    for (y = 0; y < H_MAX; ++y)
        for (x = 0; x < W_MAX; ++x)
            if (level_sym(screen, x, y) == LEV_BOTH) {
                nest_x = x;
                nest_y = y;
            }

    /* Чем выше сложность, тем чаще рождаются враги - формула порта на БК */
    spawn_delay = (uint8_t)(45u - 2u * difficulty);
    spawn_count = spawn_delay;
    bugs_total = (uint8_t)(difficulty + 5u);
    bugs_diff = difficulty;
}

/*
 * Можно ли войти в соседнюю клетку в направлении dir. Ноббину нужен готовый
 * ход: клетка должна быть прокопана насквозь по нужной оси. Хоббин грызёт
 * землю и проходит всюду в пределах поля.
 */
static uint8_t bug_can_go(const bug_t *b, uint8_t dir)
{
    uint8_t xl = x_log_tbl[b->x];
    uint8_t yl = y_log_tbl[b->y];
    uint16_t bg;

    switch (dir) {
    case DIR_RIGHT: if (xl + 1u >= W_MAX) return 0; ++xl; break;
    case DIR_LEFT:  if (xl == 0u)         return 0; --xl; break;
    case DIR_DOWN:  if (yl + 1u >= H_MAX) return 0; ++yl; break;
    case DIR_UP:    if (yl == 0u)         return 0; --yl; break;
    default: return 0;
    }

    if (b->kind == BUG_HOBBIN)
        return 1;

    /*
     * Клетка проходима, если прокопана насквозь ХОТЯ БЫ ПО ОДНОЙ оси.
     * Требовать прокопки именно по оси движения нельзя: на стыке коридоров
     * клетка прокопана по другой оси и всё равно проходима. Именно на этом
     * враги застревали - Ноббин в клетке (10,8) не мог войти в (10,9),
     * потому что тот ход горизонтальный.
     */
    bg = background[yl][xl];
    return (uint8_t)((bg & BG_HORIZ_ALL) == 0u || (bg & BG_VERT_ALL) == 0u);
}

/*
 * Строит порядок направлений: сначала ось, по которой Диггер дальше.
 * В бонус-режиме порядок переворачивается - враги убегают.
 */
static void rank_dirs(const bug_t *b, uint8_t *out)
{
    uint8_t bx = x_log_tbl[b->x], by = y_log_tbl[b->y];
    uint8_t mx = x_log_tbl[man.x], my = y_log_tbl[man.y];
    uint8_t dx = (mx > bx) ? (uint8_t)(mx - bx) : (uint8_t)(bx - mx);
    uint8_t dy = (my > by) ? (uint8_t)(my - by) : (uint8_t)(by - my);
    uint8_t hx = (mx > bx) ? DIR_RIGHT : DIR_LEFT;
    uint8_t vy = (my > by) ? DIR_DOWN  : DIR_UP;

    if (dy > dx) {
        out[0] = vy;             out[1] = hx;
        out[2] = (uint8_t)(hx ^ 1u); out[3] = (uint8_t)(vy ^ 1u);
    } else {
        out[0] = hx;             out[1] = vy;
        out[2] = (uint8_t)(vy ^ 1u); out[3] = (uint8_t)(hx ^ 1u);
    }

    /* В бонус-режиме тот же список читается наоборот - враги убегают */
    if (bugs_fleeing) {
        uint8_t t;
        t = out[0]; out[0] = out[3]; out[3] = t;
        t = out[1]; out[1] = out[2]; out[2] = t;
    }
}

static void bug_spawn(void)
{
    uint8_t i;

    if (bugs_total == 0)
        return;
    for (i = 0; i < MAX_BUGS; ++i) {
        if (bugs[i].state != BUG_NONE)
            continue;
        bugs[i].state = BUG_ALIVE;
        bugs[i].kind = BUG_NOBBIN;
        bugs[i].x = (uint8_t)(FIELD_X_OFFSET + nest_x * POS_X_STEP);
        bugs[i].y = (uint8_t)(FIELD_Y_OFFSET + nest_y * CELL_H_PX);
        bugs[i].dir = DIR_LEFT;
        bugs[i].frame = 0;
        bugs[i].count = 0;
        ++bugs_active;
        --bugs_total;
        return;
    }
}

/*
 * Возрождение Диггера: живые враги убираются с поля, но запас врагов на
 * уровень НЕ восстанавливается. Иначе гибель пополняла бы их количество,
 * и уровень нельзя было бы закрыть отстрелом - в оригинале запас на уровень
 * один на всю попытку.
 */
void bugs_respawn(void)
{
    uint8_t i;
    for (i = 0; i < MAX_BUGS; ++i) {
        if (bugs[i].state == BUG_ALIVE)
            bug_erase(&bugs[i]);
        bugs[i].state = BUG_NONE;
    }
    bugs_active = 0;
    bugs_fleeing = 0;
    spawn_count = spawn_delay;
}

uint8_t bugs_left_to_spawn(void)
{
    return bugs_total;
}

void bugs_tick(void)
{
    uint8_t i, k;
    uint8_t order[4];

    if (spawn_count == 0) {
        bug_spawn();
        spawn_count = spawn_delay;
    } else {
        --spawn_count;
    }

    for (i = 0; i < MAX_BUGS; ++i) {
        bug_t *b = &bugs[i];
        if (b->state != BUG_ALIVE)
            continue;

        /* Направление выбирается только ровно в клетке */
        if (x_rem_tbl[b->x] == 0 && y_rem_tbl[b->y] == 0) {
            rank_dirs(b, order);
            for (k = 0; k < 4u; ++k) {
                /* назад разворачиваемся только если больше некуда */
                if (order[k] == (uint8_t)(b->dir ^ 1u) && k < 3u)
                    continue;
                if (bug_can_go(b, order[k])) {
                    b->dir = order[k];
                    break;
                }
            }
            if (k == 4u) {
                /* Заперт - но анимация идёт всё равно: существо должно
                 * шевелиться и стоя на месте, иначе выглядит замершим. */
                bug_erase(b);
                ++b->frame;
                bug_draw(b);
                continue;
            }
        }

        /* Смена облика по таймеру: Ноббин ходит по готовым ходам, Хоббин
         * грызёт землю. Перенос из порта на БК. */
        {
            uint8_t limit = (b->kind == BUG_HOBBIN)
                          ? HOBBIN_TICKS(bugs_diff) : NOBBIN_TICKS(bugs_diff);
            if (++b->count > limit) {
                b->count = 0;
                b->kind = (uint8_t)(b->kind ^ 1u);
            }
        }

        bug_erase(b);
        switch (b->dir) {
        case DIR_RIGHT: ++b->x; break;
        case DIR_LEFT:  --b->x; break;
        case DIR_DOWN:  b->y = (uint8_t)(b->y + SUBSTEP_Y_LINES); break;
        case DIR_UP:    b->y = (uint8_t)(b->y - SUBSTEP_Y_LINES); break;
        default: break;
        }
        /* Хоббин прогрызает ход: гасит землю под собой и помечает клетку
         * прокопанной, иначе картинка и модель поля разошлись бы, и Ноббины
         * не смогли бы потом пройти его тоннелем. */
        if (b->kind == BUG_HOBBIN) {
            uint8_t xl = x_log_tbl[b->x], yl = y_log_tbl[b->y];
            cp_x = (uint8_t)(b->x * SUBSTEP_X_PX);
            cp_y = b->y;
            cp_w = SPRITE_W_PX;
            cp_h = SPRITE_H;
            clear_px();
            if (b->dir == DIR_RIGHT || b->dir == DIR_LEFT)
                background[yl][xl] &= (uint16_t)~BG_HORIZ_ALL;
            else
                background[yl][xl] &= (uint16_t)~BG_VERT_ALL;
            if ((background[yl][xl] & (BG_HORIZ_ALL | BG_VERT_ALL)) == 0u)
                background[yl][xl] &= (uint16_t)~BG_INTACT;
        }

        ++b->frame;
        bug_draw(b);
    }
}

void bugs_remove(uint8_t i)
{
    if (i < MAX_BUGS && bugs[i].state == BUG_ALIVE) {
        bug_erase(&bugs[i]);
        bugs[i].state = BUG_NONE;
        if (bugs_active)
            --bugs_active;
    }
}

uint8_t bugs_hit_man(void)
{
    uint8_t i;
    for (i = 0; i < MAX_BUGS; ++i) {
        bug_t *b = &bugs[i];
        if (b->state != BUG_ALIVE)
            continue;
        /* Пересечение прямоугольников 8x15: обёрточное сравнение без знака,
         * как в порте на БК - int в cc65 шестнадцатибитный, приём работает. */
        if ((uint8_t)(b->x - man.x + 3u) < 7u &&
            (uint16_t)(b->y - man.y + 14u) < 29u)
            return i;
    }
    return BUG_NO_HIT;
}
