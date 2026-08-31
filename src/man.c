/*
 * man.c - движение Диггера и прокопка.
 *
 * Модель взята из оригинала для PC: положение - левый верхний угол клетки,
 * шаг по X равен подшагу (2 px), по Y - трём строкам растра. Смена
 * направления поперёк движения разрешена только на границе клетки, разворот
 * назад - в любой момент.
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

/* Спрайты Диггера: по три фазы на каждое из четырёх направлений */
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

/* [направление][фаза][фаза сдвига] */
static const uint8_t *const man_spr[4][3][2] = {
    { { spr_r0_p0, spr_r0_p1 }, { spr_r1_p0, spr_r1_p1 },
      { spr_r2_p0, spr_r2_p1 } },
    { { spr_l0_p0, spr_l0_p1 }, { spr_l1_p0, spr_l1_p1 },
      { spr_l2_p0, spr_l2_p1 } },
    { { spr_d0_p0, spr_d0_p1 }, { spr_d1_p0, spr_d1_p1 },
      { spr_d2_p0, spr_d2_p1 } },
    { { spr_u0_p0, spr_u0_p1 }, { spr_u1_p0, spr_u1_p1 },
      { spr_u2_p0, spr_u2_p1 } },
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
    /* Восемь нот подряд вверх, как в оригинале: каждый следующий камень
     * в цепочке звучит выше предыдущего. Периоды взяты из порта на БК. */
    {
        static const uint16_t coin_note[8] = {
            114u, 101u, 90u, 85u, 76u, 67u, 60u, 57u
        };
        sfx_play(coin_note[coin_run & 7u], SFX_TONE, 10u, 2u);
    }
    coin_timer = COIN_RUN_TICKS;
    if (++coin_run == 8u) {
        coin_run = 0;
        score_add(COIN_RUN_BONUS);
        /* Восемь камней подряд: отдельный сигнал поверх ноты камня */
        sfx_play(57u, SFX_TONE, 14u, 3u);
    }
    hud_score();
}

/* Выводит Диггера в текущей позиции */
static void man_draw(void)
{
    uint8_t px = (uint8_t)(man.x * SUBSTEP_X_PX);

    bl_x = (uint8_t)(px >> 2);
    bl_y = man.y;
    bl_h = SPRITE_H;
    /* Спрайт стоит на границе байта, когда px кратен 4; иначе берётся
     * предсдвинутая на 2 пикселя фаза, которая на байт шире. */
    if (px & 2u) {
        bl_w = 3;
        bl_src = man_spr[man.dir][man.frame][1];
    } else {
        bl_w = 2;
        bl_src = man_spr[man.dir][man.frame][0];
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
static void man_erase(uint8_t px, uint8_t y)
{
    cp_x = px;
    cp_y = y;
    cp_w = SPRITE_W_PX;
    cp_h = SPRITE_H;
    clear_px();
}

/* Гасит полосу шириной w пикселей и высотой h в позиции (px, y) */
static void carve(uint8_t px, uint8_t y, uint8_t w, uint8_t h)
{
    cp_x = px;
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

/* Диггер стоит ровно в клетке - только тогда можно повернуть поперёк */
static uint8_t aligned(void)
{
    return (uint8_t)(x_rem_tbl[man.x] == 0 && y_rem_tbl[man.y] == 0);
}

void man_tick(dir_t want)
{
    uint8_t px, old_px, old_y;

    if (want != DIR_STOP) {
        /* Разворот назад разрешён всегда, поворот поперёк - только в клетке */
        if (want == (dir_t)(man.dir ^ 1u) || aligned())
            man.dir = want;
        man.want = want;
    }

    if (want == DIR_STOP || !can_move(man.dir)) {
        /* Стоя на месте Диггер всё равно перебирает фазы, поэтому старое
         * изображение надо погасить и здесь. */
        man_erase((uint8_t)(man.x * SUBSTEP_X_PX), man.y);
        take_coin();
        man_draw();
        return;
    }

    old_px = (uint8_t)(man.x * SUBSTEP_X_PX);
    old_y = man.y;

    if (man.dir == DIR_RIGHT || man.dir == DIR_LEFT) {
        uint8_t nx = (man.dir == DIR_RIGHT)
                   ? (uint8_t)(man.x + POS_X_STEP) : (uint8_t)(man.x - POS_X_STEP);
        if (!bags_push(x_log_tbl[nx], y_log_tbl[man.y], man.dir)) {
            man_draw();                 /* мешок упёрся - Диггер не идёт */
            return;
        }
    }

    switch (man.dir) {
    case DIR_RIGHT: ++man.x; break;
    case DIR_LEFT:  --man.x; break;
    case DIR_DOWN:  man.y = (uint8_t)(man.y + SUBSTEP_Y_LINES); break;
    case DIR_UP:    man.y = (uint8_t)(man.y - SUBSTEP_Y_LINES); break;
    default: break;
    }

    px = (uint8_t)(man.x * SUBSTEP_X_PX);

    dig_background(man.dir);

    /* Прокопка перед собой и гашение следа за собой */
    switch (man.dir) {
    case DIR_RIGHT:
        carve((uint8_t)(px + SPRITE_W_PX - SUBSTEP_X_PX), man.y,
              SUBSTEP_X_PX, SPRITE_H);
        carve(old_px, man.y, SUBSTEP_X_PX, SPRITE_H);
        break;
    case DIR_LEFT:
        carve(px, man.y, SUBSTEP_X_PX, SPRITE_H);
        carve((uint8_t)(old_px + SPRITE_W_PX - SUBSTEP_X_PX), man.y,
              SUBSTEP_X_PX, SPRITE_H);
        break;
    case DIR_DOWN:
        carve(px, (uint8_t)(man.y + SPRITE_H - SUBSTEP_Y_LINES),
              SPRITE_W_PX, SUBSTEP_Y_LINES);
        carve(px, old_y, SPRITE_W_PX, SUBSTEP_Y_LINES);
        break;
    case DIR_UP:
        carve(px, man.y, SPRITE_W_PX, SUBSTEP_Y_LINES);
        carve(px, (uint8_t)(old_y + SPRITE_H - SUBSTEP_Y_LINES),
              SPRITE_W_PX, SUBSTEP_Y_LINES);
        break;
    default: break;
    }

    man_erase(old_px, old_y);

    /* Фаза анимации перебирается 0-1-2-1-0-... */
    if (man.frame == 0)
        man.frame_inc = 1;
    else if (man.frame >= 2)
        man.frame_inc = -1;
    man.frame = (uint8_t)(man.frame + man.frame_inc);

    take_coin();
    man_draw();
}
