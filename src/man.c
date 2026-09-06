/*
 * man.c - движение Диггера и прокопка.
 *
 * Модель взята из оригинала для PC: положение - левый верхний угол клетки,
 * шаг по X равен подшагу (2 px), по Y - трём строкам растра. Повернуть на
 * вертикаль можно, стоя ровно в столбце, на горизонталь - стоя ровно в ряду;
 * по одной оси, а не по обеим сразу (см. can_turn).
 *
 * Прокопка идёт по ходу движения: гасится узкая полоса перед спрайтом, а за
 * спрайтом гасится такая же полоса следа. Ширина хода равна ширине спрайта
 * (8 px из 10) и высота - высоте спрайта (15 строк из 18), поэтому справа
 * и снизу остаётся земля - это разделители клеток, как в оригинале.
 */
#include "game.h"
#include "field.h"
#include "blit.h"
#include "hud.h"
#include "bags.h"
#include "sfx.h"
#include "pmg.h"
#include "missile.h"

/*
 * Спрайты Диггера: по ШЕСТЬ кадров на каждое из четырёх направлений. Первые
 * три - с башенкой, вторые три (r4..r6 и такие же у остальных направлений) -
 * без неё.
 *
 * Башенка есть, только когда выстрел готов: пока снаряд летит или идёт
 * перезарядка, Диггер едет с приплюснутым верхом. Это единственный признак
 * готовности оружия на экране, и он же есть в оригинале - в порте на БК то
 * же самое называется cab и считается как !mis.flying && !mis.wait.
 */
extern const uint8_t spr_r0_p0[], spr_r0_p1[];
extern const uint8_t spr_r1_p0[], spr_r1_p1[];
extern const uint8_t spr_r2_p0[], spr_r2_p1[];
extern const uint8_t spr_l0_p0[], spr_l0_p1[];
extern const uint8_t spr_l1_p0[], spr_l1_p1[];
extern const uint8_t spr_l2_p0[], spr_l2_p1[];
extern const uint8_t spr_u0_p0[], spr_u0_p1[];
extern const uint8_t spr_u1_p0[], spr_u1_p1[];
extern const uint8_t spr_u2_p0[], spr_u2_p1[];
extern const uint8_t spr_d0_p0[], spr_d0_p1[];
extern const uint8_t spr_d1_p0[], spr_d1_p1[];
extern const uint8_t spr_d2_p0[], spr_d2_p1[];
extern const uint8_t spr_r4_p0[], spr_r4_p1[];
extern const uint8_t spr_r5_p0[], spr_r5_p1[];
extern const uint8_t spr_r6_p0[], spr_r6_p1[];
extern const uint8_t spr_l4_p0[], spr_l4_p1[];
extern const uint8_t spr_l5_p0[], spr_l5_p1[];
extern const uint8_t spr_l6_p0[], spr_l6_p1[];
extern const uint8_t spr_u4_p0[], spr_u4_p1[];
extern const uint8_t spr_u5_p0[], spr_u5_p1[];
extern const uint8_t spr_u6_p0[], spr_u6_p1[];
extern const uint8_t spr_d4_p0[], spr_d4_p1[];
extern const uint8_t spr_d5_p0[], spr_d5_p1[];
extern const uint8_t spr_d6_p0[], spr_d6_p1[];

/* Заливка для аппаратных спрайтов: два плана на кадр (корпус, клюв). Колёса
 * плана лишились - два игрока ушли врагам, см. src/pmg.s. Предсдвинутых фаз
 * здесь нет: позиция игрока задаётся регистром HPOS с точностью до такта
 * цвета, то есть вдвое мельче нашего подшага. */
extern const uint8_t pm_r0_0[], pm_r0_1[];
extern const uint8_t pm_r1_0[], pm_r1_1[];
extern const uint8_t pm_r2_0[], pm_r2_1[];
extern const uint8_t pm_l0_0[], pm_l0_1[];
extern const uint8_t pm_l1_0[], pm_l1_1[];
extern const uint8_t pm_l2_0[], pm_l2_1[];
extern const uint8_t pm_d0_0[], pm_d0_1[];
extern const uint8_t pm_d1_0[], pm_d1_1[];
extern const uint8_t pm_d2_0[], pm_d2_1[];
extern const uint8_t pm_u0_0[], pm_u0_1[];
extern const uint8_t pm_u1_0[], pm_u1_1[];
extern const uint8_t pm_u2_0[], pm_u2_1[];

man_t man;

/* Счёт в упакованном BCD, см. score.s */
extern uint8_t score[3];
void __fastcall__ score_add(unsigned int bcd);

/*
 * Счётчик камней, взятых подряд. В оригинале за восемь подряд даётся ещё
 * 250 очков; цепочка рвётся, если между камнями прошло слишком много тактов.
 */
static uint8_t coin_run;
static uint8_t coin_timer;

#define COIN_RUN_TICKS  12u     /* столько тактов цепочка ещё жива */
#define COIN_SCORE      0x0025u /* 25 очков, сразу в BCD           */
#define COIN_RUN_BONUS  0x0250u /* 250 очков за восемь подряд      */

/* [направление][кадр 0..5: три с башенкой, три без][фаза сдвига] */
static const uint8_t *const man_spr[4][6][2] = {
    { { spr_r0_p0, spr_r0_p1 }, { spr_r1_p0, spr_r1_p1 },
      { spr_r2_p0, spr_r2_p1 },
      { spr_r4_p0, spr_r4_p1 }, { spr_r5_p0, spr_r5_p1 },
      { spr_r6_p0, spr_r6_p1 } },
    { { spr_l0_p0, spr_l0_p1 }, { spr_l1_p0, spr_l1_p1 },
      { spr_l2_p0, spr_l2_p1 },
      { spr_l4_p0, spr_l4_p1 }, { spr_l5_p0, spr_l5_p1 },
      { spr_l6_p0, spr_l6_p1 } },
    { { spr_d0_p0, spr_d0_p1 }, { spr_d1_p0, spr_d1_p1 },
      { spr_d2_p0, spr_d2_p1 },
      { spr_d4_p0, spr_d4_p1 }, { spr_d5_p0, spr_d5_p1 },
      { spr_d6_p0, spr_d6_p1 } },
    { { spr_u0_p0, spr_u0_p1 }, { spr_u1_p0, spr_u1_p1 },
      { spr_u2_p0, spr_u2_p1 },
      { spr_u4_p0, spr_u4_p1 }, { spr_u5_p0, spr_u5_p1 },
      { spr_u6_p0, spr_u6_p1 } },
};

/*
 * Заливка аппаратных спрайтов - только для трёх кадров: у кадров без башенки
 * она та же самая. Проверено по данным оригинала: краски корпуса и клюва у
 * пары r0/r4 совпадают точка в точку, отличается только башенка, а она
 * жёлтая - того же цвета, что колёса, и своего плана не имеет.
 *
 * [направление][кадр][план] - порядок направлений тот же, что выше
 */
static const uint8_t *const man_pm[4][3][2] = {
    { { pm_r0_0, pm_r0_1 }, { pm_r1_0, pm_r1_1 },
      { pm_r2_0, pm_r2_1 } },
    { { pm_l0_0, pm_l0_1 }, { pm_l1_0, pm_l1_1 },
      { pm_l2_0, pm_l2_1 } },
    { { pm_d0_0, pm_d0_1 }, { pm_d1_0, pm_d1_1 },
      { pm_d2_0, pm_d2_1 } },
    { { pm_u0_0, pm_u0_1 }, { pm_u1_0, pm_u1_1 },
      { pm_u2_0, pm_u2_1 } },
};

void man_init(void)
{
    man.x = (uint8_t)(FIELD_X_OFFSET + MAN_START_X * POS_X_STEP);
    man.y = (uint8_t)(FIELD_Y_OFFSET + MAN_START_Y * CELL_H_PX);
    man.dir = DIR_LEFT;
    man.want = DIR_STOP;
    man.frame = 0;
    man.frame_inc = 1;
    coin_run = 0;
    coin_timer = 0;
}

/* Подбирает камень в клетке, где стоит Диггер */
static void take_coin(void)
{
    if (coin_timer)
        --coin_timer;
    else
        coin_run = 0;

    if (bags_take_gold(x_log_tbl[man.x], y_log_tbl[man.y]))
        hud_score();

    if (!field_take_coin(x_log_tbl[man.x], y_log_tbl[man.y]))
        return;

    score_add(COIN_SCORE);
    /* Звук камня из двух частей, как в оригинале: щелчок и пульсирующая
     * нота, высота которой растёт с номером камня в цепочке. */
    sfx_coin(coin_run);
    coin_timer = COIN_RUN_TICKS;
    if (++coin_run == 8u) {
        coin_run = 0;
        score_add(COIN_RUN_BONUS);
        /* Восемь камней подряд: отдельный сигнал поверх ноты камня */
        sfx_play(57u, SFX_TONE, 14u, 3u);
    }
    hud_score();
}

/*
 * Выводит Диггера в текущей позиции.
 *
 * Рисунок остаётся в кадровом буфере и при аппаратных спрайтах - те лишь
 * подкладывают под него цвет, поэтому выводится и то, и другое. Заливка идёт
 * первой: она грубее рисунка, и линии по краю должны лечь поверх неё, а не
 * наоборот.
 */
static void man_draw(void)
{
    /* Башенка есть, только пока выстрел готов; иначе берётся вторая тройка
     * кадров - та же поза, но с приплюснутым верхом. */
    uint8_t f = (uint8_t)(man.frame + (missile_ready() ? 0u : 3u));

    pmg_hpos = PMG_HPOS(man.x);
    pmg_y = man.y;
    pmg_s0 = man_pm[man.dir][man.frame][0];
    pmg_s1 = man_pm[man.dir][man.frame][1];
    pmg_show();
    bl_x = (uint8_t)(man.x >> 1);
    bl_y = man.y;
    bl_h = SPRITE_H;
    /* Спрайт стоит на границе байта при чётном подшаге; при нечётном берётся
     * предсдвинутая на полбайта фаза, которая на байт шире. */
    if (man.x & 1u) {
        bl_w = 3;
        bl_src = man_spr[man.dir][f][1];
    } else {
        bl_w = 2;
        bl_src = man_spr[man.dir][f][0];
    }
    blit_mask();
}

/*
 * Гасит всё место, где Диггер был на прошлом такте.
 *
 * Одной полоски следа шириной в подшаг недостаточно: вывод идёт с маской,
 * и там, где новый кадр анимации прозрачен, а старый был непрозрачен,
 * от старого кадра оставался мусор. При смене фазы или направления формы
 * кадров не совпадают, и след виден. У мешков и врагов старое место
 * гасится целиком - Диггер должен вести себя так же.
 */
static void man_erase(uint8_t sx, uint8_t y)
{
    cp_x = sx;
    cp_y = y;
    cp_w = SPRITE_W_SUB;
    cp_h = SPRITE_H;
    clear_px();
}

/* Гасит полосу шириной w подшагов и высотой h строк в позиции (sx, y) */
static void carve(uint8_t sx, uint8_t y, uint8_t w, uint8_t h)
{
    cp_x = sx;
    cp_y = y;
    cp_w = w;
    cp_h = h;
    clear_px();
}

/*
 * Отмечает выеденный "укус" в матрице фона - перенос era_background()
 * из оригинала. Там это пять и шесть веток switch подряд, здесь то же
 * правило записано формулой: гасится бит (1 << x_rem) для горизонтали и
 * (1 << (6 + y_rem)) для вертикали. При движении вперёд укус приходится на
 * СЛЕДУЮЩУЮ клетку, назад - на текущую с заёмом в предыдущую.
 * Когда сняты все биты своей оси, снимается и бит 13 "клетка цела".
 */
static void dig_background(dir_t dir)
{
    uint8_t xl = x_log_tbl[man.x], xr = x_rem_tbl[man.x];
    uint8_t yl = y_log_tbl[man.y], yr = y_rem_tbl[man.y];

    switch (dir) {
    case DIR_RIGHT:
        if (xl + 1u >= W_MAX) return;
        ++xl;
        break;
    case DIR_LEFT:
        if (xr == 0u) {
            if (xl == 0u) return;
            --xl;
            xr = POS_X_STEP - 1u;
        } else {
            --xr;
        }
        break;
    case DIR_DOWN:
        if (yl + 1u >= H_MAX) return;
        ++yl;
        break;
    case DIR_UP:
        if (yr == 0u) {
            if (yl == 0u) return;
            --yl;
            yr = POS_Y_STEP - 1u;
        } else {
            --yr;
        }
        break;
    default:
        return;
    }

    if (dir == DIR_RIGHT || dir == DIR_LEFT) {
        background[yl][xl] &= (uint16_t)~(1u << xr);
        if ((background[yl][xl] & BG_HORIZ_ALL) == 0u)
            background[yl][xl] &= (uint16_t)~BG_INTACT;
    } else {
        background[yl][xl] &= (uint16_t)~(1u << (yr + 6u));
        if ((background[yl][xl] & BG_VERT_ALL) == 0u)
            background[yl][xl] &= (uint16_t)~BG_INTACT;
    }
}

/*
 * Перебор фазы анимации 0-1-2-1-0-... Вызывается КАЖДЫЙ такт, а не только
 * при движении: в оригинале Диггер перебирает лапами и стоя на месте, и
 * привязка анимации к нажатию выглядела бы как замирание персонажа.
 */
static void next_frame(void)
{
    if (man.frame == 0)
        man.frame_inc = 1;
    else if (man.frame >= 2)
        man.frame_inc = -1;
    man.frame = (uint8_t)(man.frame + man.frame_inc);
}

/* Можно ли сдвинуться в направлении dir, не выйдя за поле */
static uint8_t can_move(dir_t dir)
{
    switch (dir) {
    case DIR_RIGHT: return man.x < MAN_X_MAX;
    case DIR_LEFT:  return man.x > MAN_X_MIN;
    case DIR_DOWN:  return man.y < MAN_Y_MAX;
    case DIR_UP:    return man.y > MAN_Y_MIN;
    default:        return 0;
    }
}

/*
 * Разрешён ли поворот в направление dir. Правило из оригинала (move_man,
 * Work/TC.C): повернуть на ВЕРТИКАЛЬ можно, когда Диггер ровно в столбце,
 * повернуть на ГОРИЗОНТАЛЬ - когда ровно в ряду. По одной оси, не по обеим.
 * Разворот назад получается сам собой: идя по столбцу, он уже в столбце.
 *
 * Раньше здесь требовалось попадание ровно в КЛЕТКУ, то есть по обеим осям
 * сразу. Упершись снизу в мешок, Диггер встаёт на подшаг выше границы клетки,
 * и уйти вбок оттуда было нельзя - он залипал под мешком намертво.
 */
static uint8_t can_turn(dir_t dir)
{
    return (dir >= DIR_DOWN) ? (uint8_t)(x_rem_tbl[man.x] == 0)
                             : (uint8_t)(y_rem_tbl[man.y] == 0);
}

/*
 * Такт, в котором шага не вышло: не пустил мешок. Диггер остаётся на месте, но
 * не замирает - перебирает лапами и подбирает камень под собой, как и стоя.
 *
 * Направление при этом РАЗВОРАЧИВАЕТСЯ. Это из оригинала: "reverse mans
 * direction to prevent him from becoming stuck". Без разворота Диггер,
 * упёршийся снизу в мешок, остаётся между границами клеток, где поворот вбок
 * запрещён, и выйти из-под мешка не может вовсе. С разворотом на следующем
 * такте он сдвигается на подшаг вниз, попадает ровно в ряд - и прижатое вбок
 * направление срабатывает.
 *
 * Пока джойстик держат в мешок, разворот наружу не виден: следующий такт
 * назначает то же направление заново, шага снова нет, и Диггер стоит
 * прижатым, удерживая мешок от падения.
 *
 * Рисуется он ДО разворота - лицом в мешок, как в оригинале.
 */
static void man_bumped(void)
{
    man_erase(man.x, man.y);
    next_frame();
    take_coin();
    man_draw();
    man.dir = (dir_t)(man.dir ^ 1u);
}

void man_tick(dir_t want)
{
    uint8_t sx, old_sx, old_y;

    if (want != DIR_STOP) {
        if (can_turn(want))
            man.dir = want;
        man.want = want;
    }

    if (want == DIR_STOP || !can_move(man.dir)) {
        man.move = DIR_STOP;            /* шага нет: мешок он не держит */
        /* Стоя на месте Диггер всё равно перебирает фазы, поэтому старое
         * изображение надо погасить и здесь. */
        man_erase(man.x, man.y);
        next_frame();
        take_coin();
        man_draw();
        return;
    }

    old_sx = man.x;
    old_y = man.y;
    man.move = man.dir;

    if (man.dir == DIR_RIGHT || man.dir == DIR_LEFT) {
        uint8_t nx = (man.dir == DIR_RIGHT)
                   ? (uint8_t)(man.x + POS_X_STEP) : (uint8_t)(man.x - POS_X_STEP);
        if (!bags_push(x_log_tbl[nx], y_log_tbl[man.y], man.dir)) {
            man_bumped();               /* мешок упёрся - Диггер не идёт */
            return;
        }
    } else {
        /* Сверху и снизу мешок не сдвинуть: он не пускает. Проверка идёт по
         * БУДУЩЕМУ положению - иначе Диггер успел бы влезть в мешок и только
         * потом упереться. */
        uint8_t ny = (man.dir == DIR_DOWN)
                   ? (uint8_t)(man.y + SUBSTEP_Y_LINES)
                   : (uint8_t)(man.y - SUBSTEP_Y_LINES);
        if (bags_blocked(man.x, ny)) {
            man_bumped();
            return;
        }
    }

    /*
     * Укус отмечается ДО шага, по ПРЕЖНЕЙ позиции: "erase background in front
     * of man then move man" (Work/TC.C). Порядок тут не косметика. Укус всегда
     * приходится на следующую клетку по ходу, и его номер берётся из остатка
     * внутри текущей; если считать уже после шага, то остаток на единицу
     * меньше нужного, и у клетки, в которую Диггер ВХОДИТ, крайний укус со
     * стороны входа не снимается никогда.
     *
     * Клетка от этого остаётся "не прокопанной насквозь" - а Ноббин ходит
     * только по таким. Снаружи это выглядело так: враг доходит по коридору до
     * начала свежего хода и разворачивается обратно, будто там стена. Хода в
     * одну клетку это лишало целиком, длинного - только первой клетки после
     * поворота: у остальных недостающий укус снимается тем шагом, что встаёт
     * ровно на границу.
     */
    dig_background(man.dir);

    switch (man.dir) {
    case DIR_RIGHT: ++man.x; break;
    case DIR_LEFT:  --man.x; break;
    case DIR_DOWN:  man.y = (uint8_t)(man.y + SUBSTEP_Y_LINES); break;
    case DIR_UP:    man.y = (uint8_t)(man.y - SUBSTEP_Y_LINES); break;
    default: break;
    }

    sx = man.x;

    /* Прокопка перед собой и гашение следа за собой */
    switch (man.dir) {
    case DIR_RIGHT:
        carve((uint8_t)(sx + SPRITE_W_SUB - 1u), man.y, 1u, SPRITE_H);
        carve(old_sx, man.y, 1u, SPRITE_H);
        break;
    case DIR_LEFT:
        carve(sx, man.y, 1u, SPRITE_H);
        carve((uint8_t)(old_sx + SPRITE_W_SUB - 1u), man.y, 1u, SPRITE_H);
        break;
    case DIR_DOWN:
        carve(sx, (uint8_t)(man.y + SPRITE_H - SUBSTEP_Y_LINES),
              SPRITE_W_SUB, SUBSTEP_Y_LINES);
        carve(sx, old_y, SPRITE_W_SUB, SUBSTEP_Y_LINES);
        break;
    case DIR_UP:
        carve(sx, man.y, SPRITE_W_SUB, SUBSTEP_Y_LINES);
        carve(sx, (uint8_t)(old_y + SPRITE_H - SUBSTEP_Y_LINES),
              SPRITE_W_SUB, SUBSTEP_Y_LINES);
        break;
    default: break;
    }

    /* Закруглить торец хода. В оригинале era_right/left/up/down зовётся с
     * ПРЕЖНЕЙ позицией, до сдвига, - отсюда old_sx/old_y. */
    field_gnaw(man.dir, old_sx, old_y);

    man_erase(old_sx, old_y);
    next_frame();

    take_coin();
    man_draw();
}
