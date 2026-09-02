/*
 * pmgbug.c - враги в аппаратных спрайтах.
 *
 * Игроков четыре, живых врагов до пяти, а Диггер занимает двоих. Врагам
 * достаются игроки 2 и 3, и каждый держит НЕСКОЛЬКО врагов сразу: в буфере
 * игрока байт с индексом N выводится на строку растра N, так что фигуры на
 * разных высотах друг другу не мешают.
 *
 * Мешает другое: горизонтальная позиция у игрока одна на весь кадр. Её
 * переставляет цепочка прерываний по строке (src/dli.s) - по слоту на каждого
 * врага, на той строке, с которой он начинается.
 *
 * Игроки делятся НЕ по виду врага. В оригинале для PC и Ноббин, и Хоббин
 * зелёные - различаются формой, а не цветом, - поэтому краска у обоих игроков
 * одна, и второй нужен ровно затем, чтобы развести двух врагов, стоящих на
 * одной высоте. Каждый враг садится на того игрока, который к его строке уже
 * освободился; если заняты оба, враг остаётся в цвете поля.
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

static const uint8_t bug_player[2] = { PMG_PL_BUG0, PMG_PL_BUG1 };

/*
 * Врага и предыдущего на ТОМ ЖЕ игроке надо разнести на всю высоту спрайта
 * плюс фору прерывания: слот ставится на DLI_LEAD строк выше, и при меньшем
 * зазоре он переставил бы позицию посреди предыдущей фигуры.
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

/* Кладёт заливку врага в буфер выбранного игрока */
static void put_fill(const bug_t *b, uint8_t player)
{
    uint8_t look;

    /* Облик выбирается как в bug_draw: у Хоббина своя графика на сторону */
    if (b->kind != BUG_HOBBIN)
        look = 0;
    else
        look = (uint8_t)((b->dir == DIR_LEFT) ? 2u : 1u);

    /* Счётчик кадра растёт без границы, как и в bug_draw: остаток от трёх
     * берётся на месте, а не при увеличении. */
    pmg_by = b->y;
    pmg_bpl = player;
    pmg_bs = bug_pm[look][b->frame % 3u];
    pmg_bug_put();
}

/*
 * Один проход по всем живым врагам в порядке возрастания y. Порядок нужен
 * дважды: слоты прерывания идут сверху вниз, и занятость игрока проверяется
 * по последнему уже посаженному на него врагу.
 */
static void select_bugs(void)
{
    uint8_t i, p, best, best_y, prev_y, have_prev;
    uint8_t last_y[2], used[2];

    sel_n = 0;
    used[0] = 0;
    used[1] = 0;
    last_y[0] = 0;
    last_y[1] = 0;
    prev_y = 0;
    have_prev = 0;

    for (;;) {
        best = MAX_BUGS;
        best_y = 0xFFu;
        for (i = 0; i < MAX_BUGS; ++i) {
            const bug_t *b = &bugs[i];
            if (b->state != BUG_ALIVE)
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

        /* Свободен тот игрок, чей прошлый враг кончился достаточно высоко */
        for (p = 0; p < 2u; ++p)
            if (!used[p] || (uint8_t)(best_y - last_y[p]) >= BUG_GAP)
                break;
        if (p == 2u)
            continue;               /* оба заняты - этот враг без цвета */

        used[p] = 1;
        last_y[p] = best_y;

        /* Список уже растёт по возрастанию y, поэтому просто дописываем */
        sel_y[sel_n] = best_y;
        sel_hpos[sel_n] = PMG_HPOS(bugs[best].x);
        sel_pl[sel_n] = bug_player[p];
        ++sel_n;

        put_fill(&bugs[best], bug_player[p]);
    }
}

void pmg_bugs_update(void)
{
    uint8_t i, h2, h3, n, line;

    /*
     * Дисплей-лист и таблицы переписываются по живому: прерывание, пришедшее
     * посреди правки, возьмёт позицию из полузаполненной таблицы, и один кадр
     * покажет врага не там. Поэтому значение в таблицу кладётся РАНЬШЕ, чем
     * бит DLI на его строку, - тогда сбиться может только порядок слотов,
     * а не содержимое. Двойной буферизации нет намеренно: если начнёт мигать,
     * искать надо здесь.
     */
    for (i = 0; i < marked_n; ++i)
        *screen_dlist_line(marked[i]) &= (uint8_t)~DL_DLI;
    marked_n = 0;

    pmg_bugs_clear();
    select_bugs();

    /*
     * Слот пишет ОБЕ позиции, поэтому в таблицу кладётся состояние на его
     * строку целиком, а не только изменившийся регистр. Два врага, попавшие
     * на одну строку дисплей-листа, дают один слот: бит DLI у строки один.
     */
    h2 = 0;
    h3 = 0;
    n = 0;
    for (i = 0; i < sel_n; ++i) {
        if (sel_pl[i] == PMG_PL_BUG0)
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
