/*
 * bugs.c - враги.
 */
#include "bugs.h"
#include "field.h"
#include "blit.h"
#include "game.h"
#include "pmgbug.h"

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

/*
 * Выводит врага. Заливка аппаратного спрайта кладётся ЗДЕСЬ ЖЕ, вплотную к
 * рисунку: кадровый буфер не двойной, вывод идёт посреди кадра, и растр видит
 * любое расхождение между рисунком и заливкой (см. src/pmgbug.c).
 */
static void bug_draw(uint8_t idx)
{
    const bug_t *b = &bugs[idx];
    uint8_t look;

    /* У Хоббина своя графика на каждую сторону; при движении по вертикали
     * сохраняется та, что была по горизонтали в последний раз. */
    if (b->kind != BUG_HOBBIN)
        look = 0;
    else
        look = (uint8_t)((b->dir == DIR_LEFT) ? 2u : 1u);

    bl_x = (uint8_t)(b->x >> 1);
    bl_y = b->y;
    bl_h = SPRITE_H;
    if (b->x & 1u) {
        bl_w = 3;
        bl_src = bug_spr[look][b->frame % 3u][1];
    } else {
        bl_w = 2;
        bl_src = bug_spr[look][b->frame % 3u][0];
    }
    blit_mask();
    pmg_bug_show(idx);
}

/*
 * Касается ли враг i другого живого врага. Нужно для смены облика: Ноббин
 * становится Хоббином не по таймеру, а когда упёрся - в том числе когда
 * толкается с соседом. Габарит и приём сравнения те же, что в bugs_hit_man.
 */
static uint8_t bug_touches_other(uint8_t i)
{
    uint8_t t;
    const bug_t *b = &bugs[i];

    for (t = 0; t < MAX_BUGS; ++t) {
        const bug_t *o = &bugs[t];
        if (t == i || o->state != BUG_ALIVE)
            continue;
        if ((uint8_t)(o->x - b->x + 3u) < 7u &&
            (uint16_t)(o->y - b->y + 14u) < 29u)
            return 1;
    }
    return 0;
}

/*
 * Стирает врага. Заливка аппаратного спрайта снимается ПЕРВОЙ, до рисунка.
 *
 * Порядок не косметика. Растр идёт по картинке всё время, и в зазор между
 * двумя действиями он успевает пройти по врагу. Если сначала стереть рисунок,
 * в этот зазор видна заливка БЕЗ фигуры - зелёный силуэт-призрак. В обратном
 * порядке в зазоре видна фигура без цвета, чего почти не заметно. По той же
 * причине в bug_draw заливка кладётся ПОСЛЕДНЕЙ, после рисунка.
 */
static void bug_erase(uint8_t idx)
{
    const bug_t *b = &bugs[idx];

    pmg_bug_hide(idx);
    cp_x = b->x;
    cp_y = b->y;
    cp_w = SPRITE_W_SUB;
    cp_h = SPRITE_H;
    clear_px();
}

void bugs_init(uint8_t screen, uint8_t difficulty)
{
    uint8_t x, y, i;

    /* Заливку аппаратных спрайтов снять ЯВНО: смена уровня перерисовывает
     * кадровый буфер целиком, а буфер игроков - нет, и от прежних врагов
     * остались бы зелёные пятна на новом поле. */
    for (i = 0; i < MAX_BUGS; ++i) {
        pmg_bug_hide(i);
        bugs[i].state = BUG_NONE;
    }
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

    /*
     * Разворот назад ПЕРЕНОСИТСЯ в конец списка, а остальные сдвигаются на
     * его место. Так это сделано и в оригинале, и в порте на БК.
     *
     * Раньше вызывающий просто пропускал разворот, если тот стоял не
     * последним, - и направление терялось совсем. Существо в тупике
     * объявлялось запертым, хотя назад пройти могло. Хоббин, свернувший под
     * прямым углом и ставший Ноббином, оказывался именно в такой яме:
     * впереди и по бокам земля, сзади собственный ход, а списка до него не
     * хватало. Он замирал до обратного превращения - это и выглядело как
     * застревание Хоббинов на поворотах.
     */
    {
        uint8_t back = (uint8_t)(b->dir ^ 1u);
        uint8_t j = 0;

        while (j < 4u && out[j] != back)
            ++j;
        for (; j < 3u; ++j)
            out[j] = out[j + 1u];
        if (j == 3u)
            out[3] = back;
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
            bug_erase(i);
        bugs[i].state = BUG_NONE;
    }
    bugs_active = 0;
    bugs_fleeing = 0;
    spawn_count = spawn_delay;
    /*
     * Запас врагов на уровень ВОССТАНАВЛИВАЕТСЯ. Так в оригинале: после
     * гибели зовётся restart_level, а он - init_ta, который заново ставит
     * bugs_created = 0 и bugs_total = 5 + сложность (Work/TA.C, Work/TZ.C).
     *
     * Без этого гибель сама закрывала уровень: она сметает всех живых врагов,
     * и если запас к тому моменту исчерпан, то условие "все враги кончились"
     * срабатывает на первом же такте после возрождения - игра прыгала на
     * следующий уровень сама.
     */
    bugs_total = (uint8_t)(bugs_diff + 5u);
}

uint8_t bugs_left_to_spawn(void)
{
    return bugs_total;
}

void bugs_tick(void)
{
    uint8_t i, k;
    uint8_t old_x, old_y;       /* положение до шага - для прогрыза */
    uint8_t aligned;            /* стоит ровно в клетке */
    uint8_t order[4];

    /* Кому из врагов достанется аппаратный спрайт. Раздаётся раз в такт, до
     * всякого движения: сама заливка кладётся потом в bug_draw. */
    pmg_bugs_assign();

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

        aligned = (uint8_t)(x_rem_tbl[b->x] == 0 && y_rem_tbl[b->y] == 0);

        /*
         * Смена облика - ДО выбора направления, как в оригинале.
         *
         * count это счётчик ЗАСТРЕВАНИЯ, а не свободный таймер: у Хоббина он
         * растёт каждый такт (тот всегда в работе), у Ноббина - только когда
         * идти некуда или он толкается с соседом.
         *
         * Порядок важен. Когда проверка стояла ПОСЛЕ выбора направления,
         * Хоббин успевал выбрать путь по своим правилам (ему проходимо всё) и
         * только потом становился Ноббином - то есть оказывался посреди земли,
         * идя в сторону, куда Ноббину хода нет. Дальше он утыкался в грунт и
         * замирал навсегда, потому что из ветки "заперт" выполнение уходило
         * по continue мимо обратного превращения. Снаружи это выглядело как
         * застрявшие на поворотах Хоббины.
         */
        if (b->kind == BUG_HOBBIN) {
            if (b->count < 100u)
                ++b->count;
            /* Обратно в Ноббины - только ровно в клетке: иначе новый облик
             * достался бы существу между клетками, откуда Ноббину не выйти. */
            if (aligned && b->count > HOBBIN_TICKS(bugs_diff)) {
                b->count = 0;
                b->kind = BUG_NOBBIN;
            }
        } else {
            if (bug_touches_other(i) && b->count < 100u)
                ++b->count;
            if (b->count > NOBBIN_TICKS(bugs_diff)) {
                b->count = 0;
                b->kind = BUG_HOBBIN;
            }
        }

        /* Направление выбирается только ровно в клетке */
        if (aligned) {
            rank_dirs(b, order);
            if (b->kind == BUG_HOBBIN) {
                /*
                 * Хоббин прокапывает себе путь, поэтому проходимость не
                 * ищет вовсе - идёт лучшим направлением. Единственная
                 * проверка: не уйти за край поля, там он разворачивается.
                 * (В порте на БК это check_out_of_range.)
                 */
                b->dir = order[0];
                if (!bug_can_go(b, b->dir))
                    b->dir = (uint8_t)(b->dir ^ 1u);
                k = 0;
            } else {
                for (k = 0; k < 4u; ++k) {
                    if (bug_can_go(b, order[k])) {
                        b->dir = order[k];
                        break;
                    }
                }
            }
            if (k == 4u) {
                /* Заперт - но анимация идёт всё равно: существо должно
                 * шевелиться и стоя на месте, иначе выглядит замершим.
                 * Застревание засчитывается: на следующем такте счётчик
                 * доведёт Ноббина до превращения, и он прогрызёт себе выход. */
                if (b->kind == BUG_NOBBIN && b->count < 100u)
                    ++b->count;
                bug_erase(i);
                ++b->frame;
                continue;
            }
        }

        old_x = b->x;
        old_y = b->y;
        bug_erase(i);
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
            cp_x = b->x;
            cp_y = b->y;
            cp_w = SPRITE_W_SUB;
            cp_h = SPRITE_H;
            clear_px();
            if (b->dir == DIR_RIGHT || b->dir == DIR_LEFT)
                background[yl][xl] &= (uint16_t)~BG_HORIZ_ALL;
            else
                background[yl][xl] &= (uint16_t)~BG_VERT_ALL;
            if ((background[yl][xl] & (BG_HORIZ_ALL | BG_VERT_ALL)) == 0u)
                background[yl][xl] &= (uint16_t)~BG_INTACT;
            /* Торец прогрызенного хода закругляется так же, как у Диггера */
            field_gnaw(b->dir, old_x, old_y);
        }

        ++b->frame;
    }

}

/*
 * Вывод врагов вынесен из такта ОТДЕЛЬНЫМ проходом и зовётся последним, когда
 * отработали и снаряд, и столкновения.
 *
 * Иначе выведенного врага затирает прямоугольником тот, кто обрабатывается
 * позже: сосед по списку своим стиранием, снаряд - своим. Раньше это стоило
 * дырки в монохромной фигуре и почти не читалось, а с аппаратными спрайтами
 * стало бросаться в глаза - рисунок пропал, заливка игрока осталась, и на
 * экране сплошной зелёный блок без фигуры.
 */
void bugs_draw_all(void)
{
    uint8_t i;

    for (i = 0; i < MAX_BUGS; ++i)
        if (bugs[i].state == BUG_ALIVE)
            bug_draw(i);
}

void bugs_remove(uint8_t i)
{
    if (i < MAX_BUGS && bugs[i].state == BUG_ALIVE) {
        bug_erase(i);
        bugs[i].state = BUG_NONE;
        if (bugs_active)
            --bugs_active;
        /*
         * В бонус-режиме убитый враг возвращается в запас: "create another bug
         * if in bonus round" (era_bug, Work/TA.C). Иначе поле пустеет за
         * несколько секунд - Диггер их как раз и ест, - а запас к началу
         * бонуса уже исчерпан, и условие конца уровня ("рождать некого, живых
         * нет") закрывает уровень прямо посреди бонуса.
         *
         * bugs_fleeing поднят ровно на бонус-режим, так что он тут и служит
         * признаком get_bonus().
         */
        if (bugs_fleeing && bugs_total < 200u)
            ++bugs_total;
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
