/*
 * pmgbug.c - враги в аппаратных спрайтах.
 *
 * Игроков четыре, живых врагов до пяти, а Диггер занимает двоих. Врагам
 * достаются игроки 2 (Ноббины) и 3 (Хоббины), и каждый держит ВСЕХ врагов
 * своего вида сразу: в буфере игрока байт с индексом N выводится на строку
 * растра N, так что фигуры на разных высотах друг другу не мешают.
 *
 * Мешает другое: горизонтальная позиция у игрока одна на весь кадр. Её
 * переставляет цепочка прерываний по строке (src/dli.s) - по слоту на каждого
 * врага, на той строке, с которой он начинается.
 *
 * Отсюда единственное ограничение раскладки: двух врагов ОДНОГО ВИДА на одной
 * высоте развести нечем, второй остаётся в цвете поля. Условие ниже - это оно
 * и есть.
 */
#include "pmgbug.h"
#include "bugs.h"
#include "pmg.h"
#include "dli.h"
#include "screen.h"
#include "game.h"
#include "hw.h"

/* Заливка врагов: по одному плану на кадр, см. tools/spr2atari.py */
extern const uint8_t pm_m0_0[], pm_m1_0[], pm_m2_0[];
extern const uint8_t pm_c0_0[], pm_c1_0[], pm_c2_0[];
extern const uint8_t pm_a0_0[], pm_a1_0[], pm_a2_0[];

/* [облик: 0 Ноббин, 1 Хоббин вправо, 2 Хоббин влево][фаза] - как bug_spr */
static const uint8_t *const bug_pm[3][3] = {
    { pm_m0_0, pm_m1_0, pm_m2_0 },
    { pm_c0_0, pm_c1_0, pm_c2_0 },
    { pm_a0_0, pm_a1_0, pm_a2_0 },
};

/*
 * Врага и предыдущего надо разнести на всю высоту спрайта плюс фору
 * прерывания: слот ставится на DLI_LEAD строк выше, и при меньшем зазоре он
 * переставил бы позицию посреди предыдущей фигуры.
 */
#define BUG_GAP     (SPRITE_H + DLI_LEAD)

/* Строки дисплей-листа, где в прошлый раз стоял бит DLI */
static uint8_t marked[DLI_SLOTS];
static uint8_t marked_n;

/* Отобранные враги, уже по возрастанию строки растра */
static uint8_t sel_y[MAX_BUGS];
static uint8_t sel_hpos[MAX_BUGS];
static uint8_t sel_pl[MAX_BUGS];
static uint8_t sel_n;

/*
 * Отбор врагов одного вида: по возрастанию y, каждый следующий не ближе
 * BUG_GAP к предыдущему отобранному. Вставка идёт сразу в общий список
 * sel_*, отсортированный по y, - врагов до пяти, и сортировка вставками
 * дешевле любой другой.
 */
static void select_kind(uint8_t kind, uint8_t player)
{
    uint8_t i, j, k, best, best_y, prev_y, have_prev;

    prev_y = 0;
    have_prev = 0;

    for (;;) {
        /* следующий по высоте живой враг нужного вида */
        best = MAX_BUGS;
        best_y = 0xFFu;
        for (i = 0; i < MAX_BUGS; ++i) {
            const bug_t *b = &bugs[i];
            if (b->state != BUG_ALIVE || b->kind != kind)
                continue;
            if (have_prev && b->y <= prev_y)
                continue;
            if (b->y < best_y) {
                best_y = b->y;
                best = i;
            }
        }
        if (best == MAX_BUGS)
            break;

        prev_y = best_y;
        have_prev = 1;
        if (sel_n >= MAX_BUGS)
            break;

        /* Слишком близко к уже отобранному соседу того же вида - пропустить */
        {
            uint8_t too_close = 0;
            for (j = 0; j < sel_n; ++j)
                if (sel_pl[j] == player &&
                    (uint8_t)(best_y - sel_y[j]) < BUG_GAP) {
                    too_close = 1;
                    break;
                }
            if (too_close)
                continue;
        }

        /* Вставка с сохранением порядка по y */
        for (j = 0; j < sel_n; ++j)
            if (sel_y[j] > best_y)
                break;
        for (k = sel_n; k > j; --k) {
            sel_y[k] = sel_y[k - 1u];
            sel_hpos[k] = sel_hpos[k - 1u];
            sel_pl[k] = sel_pl[k - 1u];
        }
        sel_y[j] = best_y;
        sel_hpos[j] = PMG_HPOS(bugs[best].x);
        sel_pl[j] = player;
        ++sel_n;

        /* Заливка ложится в буфер сразу: позиция по Y у неё своя */
        {
            const bug_t *b = &bugs[best];
            uint8_t look;

            if (b->kind != BUG_HOBBIN)
                look = 0;
            else
                look = (uint8_t)((b->dir == DIR_LEFT) ? 2u : 1u);

            /* Счётчик кадра растёт без границы, как и в bug_draw: остаток
             * от трёх берётся на месте, а не при увеличении. */
            pmg_by = b->y;
            pmg_bpl = player;
            pmg_bs = bug_pm[look][b->frame % 3u];
            pmg_bug_put();
        }
    }
}

void pmg_bugs_update(void)
{
    uint8_t i, h2, h3, n, line;

    /*
     * Дисплей-лист и таблицы переписываются по живому: прерывание, пришедшее
     * посреди правки, возьмёт позицию из полузаполненной таблицы, и один кадр
     * покажет врага не там. Поэтому значение в таблицу кладётся РАНЬШЕ, чем
     * бит DLI на его строку, - тогда лишним оказывается только порядок слотов,
     * а не содержимое. Двойной буферизации нет намеренно: если начнёт мигать,
     * искать надо здесь.
     */
    for (i = 0; i < marked_n; ++i)
        *screen_dlist_line(marked[i]) &= (uint8_t)~DL_DLI;
    marked_n = 0;

    pmg_bugs_clear();

    sel_n = 0;
    select_kind(BUG_NOBBIN, PMG_PL_NOBBIN);
    select_kind(BUG_HOBBIN, PMG_PL_HOBBIN);

    /*
     * Слот пишет ОБЕ позиции, поэтому в таблицу кладётся состояние на его
     * строку целиком, а не только изменившийся регистр. Два врага разных
     * видов, попавшие на одну строку дисплей-листа, дают один слот: бит DLI
     * у строки один.
     */
    h2 = 0;
    h3 = 0;
    n = 0;
    for (i = 0; i < sel_n; ++i) {
        if (sel_pl[i] == PMG_PL_NOBBIN)
            h2 = sel_hpos[i];
        else
            h3 = sel_hpos[i];

        line = (uint8_t)(sel_y[i] - DLI_LEAD);
        if (n != 0 && marked[n - 1u] == line) {
            dli_h2[n - 1u] = h2;        /* та же строка - правим слот */
            dli_h3[n - 1u] = h3;
            continue;
        }
        if (n >= DLI_SLOTS)
            break;

        dli_h2[n] = h2;                 /* сперва значение... */
        dli_h3[n] = h3;
        marked[n] = line;
        *screen_dlist_line(line) |= DL_DLI;     /* ...потом бит */
        ++n;
    }

    marked_n = n;
}
