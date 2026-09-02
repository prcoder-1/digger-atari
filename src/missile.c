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

/*
 * Спрайты летящего снаряда: в исходниках оригинала они подписаны
 * "traveling missile" и называются 15, k1 и k2. Группа f1..f5 - это
 * надгробные камни от большего к меньшему, а f9 - перевёрнутый грузовик;
 * они понадобятся для сцены гибели, но не здесь.
 */
extern const uint8_t spr_15_p0[], spr_15_p1[];
extern const uint8_t spr_k1_p0[], spr_k1_p1[];
extern const uint8_t spr_k2_p0[], spr_k2_p1[];
extern const uint8_t spr_e1_p0[], spr_e1_p1[];
extern const uint8_t spr_e2_p0[], spr_e2_p1[];
extern const uint8_t spr_e3_p0[], spr_e3_p1[];

void __fastcall__ score_add(unsigned int bcd);

/*
 * Снаряд заметно мельче существ: 8 точек на 7 строк, то есть ОДИН байт в
 * строке, а не два. Раньше здесь выводилось два и три байта, как у спрайтов
 * 16 точек, и блиттер читал за концом данных - на экране вместо искрящегося
 * шара появлялось то, что лежит в памяти следом.
 */
#define MIS_H         7u        /* строк растра            */
#define MIS_W_BYTES   1u        /* байт в строке, фаза 0   */
#define MIS_W_SUB     2u        /* подшага в ширину (8 точек) */
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
    { spr_15_p0, spr_15_p1 },
    { spr_k1_p0, spr_k1_p1 },
    { spr_k2_p0, spr_k2_p1 },
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

uint8_t missile_ready(void)
{
    return (uint8_t)(mis.state == MIS_NONE && mis.reload == 0u);
}

static void mis_draw(const uint8_t *const spr[3][2], uint8_t phase)
{
    bl_x = (uint8_t)(mis.x >> 1);
    bl_y = mis.y;
    bl_h = MIS_H;
    if (mis.x & 1u) {
        bl_w = MIS_W_BYTES + 1u;        /* сдвинутая фаза на байт шире */
        bl_src = spr[phase][1];
    } else {
        bl_w = MIS_W_BYTES;
        bl_src = spr[phase][0];
    }
    blit_mask();
}

static void mis_erase(void)
{
    /* Гасится ровно то, что было выведено: байт - это два подшага, и у
     * сдвинутой фазы байтов на один больше. Прежняя ширина в целый спрайт
     * (16 точек) выскабливала лишнюю землю по бокам от снаряда. */
    cp_x = mis.x;
    cp_y = mis.y;
    cp_w = (uint8_t)(((mis.x & 1u) ? MIS_W_BYTES + 1u : MIS_W_BYTES)
                     * SUBSTEP_PER_BYTE);
    cp_h = MIS_H;
    clear_px();
}

/*
 * Убрать снаряд с экрана. При гибели Диггера снаряд может быть в полёте или
 * взрываться: без этого его спрайт остался бы висеть на поле, а стирать его
 * стало бы уже некому. В порте на БК то же делается в начале man_rip.
 */
void missile_clear(void)
{
    if (mis.state != MIS_NONE) {
        mis_erase();
        mis.state = MIS_NONE;
    }
    mis.reload = 0;
}

/*
 * Точка вылета своя на каждое направление - снаряд выходит из клюва, а не
 * из угла габарита. Смещения из оригинала (move_missile, Work/TC.C), точки
 * переведены в подшаги по четыре:
 *
 *      вправо  (x+8, y+4)   ->  подшаг +2, строка +4
 *      влево   (x,   y+4)   ->  подшаг  0, строка +4
 *      вверх   (x+4, y)     ->  подшаг +1, строка  0
 *      вниз    (x+4, y+8)   ->  подшаг +1, строка +8
 *
 * По горизонтали снаряд идёт серединой высоты Диггера, по вертикали -
 * серединой ширины: он вдвое уже (2 подшага против 4). Раньше смещение было
 * одно на все стороны, и при ходьбе вверх-вниз выстрел выходил от левого
 * края фигуры, а не из клюва.
 */
void missile_fire(void)
{
    static const uint8_t fire_dx[4] = { 2u, 0u, 1u, 1u };  /* RIGHT LEFT DOWN UP */
    static const uint8_t fire_dy[4] = { 4u, 4u, 8u, 0u };

    if (mis.state != MIS_NONE || mis.reload)
        return;
    if (man.dir > DIR_UP)
        return;
    ++mis_n_fired;
    sfx_play(12u, SFX_NOISE, 8u, 3u);   /* свист выстрела - шумовой тембр */
    mis.state = MIS_FLYING;
    mis.dir = man.dir;
    mis.x = (uint8_t)(man.x + fire_dx[man.dir]);
    mis.y = (uint8_t)(man.y + fire_dy[man.dir]);
    mis.count = 0;
}

/*
 * Пролетит ли снаряд через точку (подшаг, строка) при движении по dir.
 *
 * Двух правил мало по отдельности, поэтому проверяются оба:
 *
 *   - клетка прокопана НАСКВОЗЬ хотя бы по одной оси. Так ходят Ноббины
 *     (bug_can_go в bugs.c): на стыке коридоров ход идёт по другой оси, но
 *     пройти через него можно;
 *   - либо снят конкретный УКУС по оси движения. Без этого выстрел вдоль
 *     хода, который Диггер копает прямо сейчас, гас в первом же такте:
 *     клетка перед ним прогрызена лишь частично, "насквозь" ещё не считается,
 *     а лететь по ней уже можно - в оригинале там смотрят прямо на точки
 *     экрана (ppoint в move_missile, Work/TC.C).
 */
static uint8_t mis_can_pass(uint8_t sub, uint8_t y, uint8_t dir)
{
    uint8_t xl = x_log_tbl[sub];
    uint8_t yl = y_log_tbl[y];
    uint16_t bg;

    if (xl >= W_MAX || yl >= H_MAX)
        return 0;
    bg = background[yl][xl];
    if ((bg & BG_HORIZ_ALL) == 0u || (bg & BG_VERT_ALL) == 0u)
        return 1;
    if (dir == DIR_RIGHT || dir == DIR_LEFT)
        return (uint8_t)((bg & (uint16_t)(1u << x_rem_tbl[sub])) == 0u);
    return (uint8_t)((bg & (uint16_t)(0x40u << y_rem_tbl[y])) == 0u);
}

/*
 * Попал ли снаряд во врага; при попадании враг снимается с поля.
 *
 * Габарит считается честным пересечением прямоугольников: враг 4 подшага на
 * 15 строк, снаряд 2 подшага на 7. Прежние границы были взяты на глаз и
 * заодно засчитывали попадание во врага, стоящего на десяток строк НИЖЕ
 * снаряда, где никакого пересечения нет.
 *
 * Убирает врага bugs_remove: она не только меняет состояние, но и СТИРАЕТ
 * его с поля. Раньше здесь состояние правилось руками, и убитый враг
 * оставался нарисованным до конца уровня - со стороны это выглядело так,
 * будто выстрелы врагов не убивают вовсе.
 */
static uint8_t mis_hit_bug(void)
{
    uint8_t i;
    for (i = 0; i < MAX_BUGS; ++i) {
        const bug_t *b = &bugs[i];
        if (b->state != BUG_ALIVE)
            continue;
        if ((uint8_t)(b->x - mis.x + SPRITE_W_SUB - 1u)
                < (uint8_t)(SPRITE_W_SUB + MIS_W_SUB - 1u) &&
            (uint8_t)(b->y - mis.y + SPRITE_H - 1u)
                < (uint8_t)(SPRITE_H + MIS_H - 1u)) {
            bugs_remove(i);
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

    case MIS_FLYING: {
        uint8_t nx = mis.x, lx = mis.x;
        uint8_t ny = mis.y, ly = mis.y;
        uint8_t blocked;

        /*
         * Ведущая кромка (lx, ly) - та точка снаряда, которая первой войдёт
         * в стену: при движении вправо это его правый край, вниз - нижний,
         * а влево и вверх кромка совпадает с самим положением.
         */
        switch (mis.dir) {
        case DIR_RIGHT:
            if (mis.x + MIS_STEP_X > MAN_X_MAX) { ++mis_n_offscreen; goto fizzle; }
            nx = (uint8_t)(mis.x + MIS_STEP_X);
            lx = (uint8_t)(nx + MIS_W_SUB - 1u);
            break;
        case DIR_LEFT:
            if (mis.x < MAN_X_MIN + MIS_STEP_X) { ++mis_n_offscreen; goto fizzle; }
            nx = lx = (uint8_t)(mis.x - MIS_STEP_X);
            break;
        case DIR_DOWN:
            if (mis.y + MIS_STEP_Y > MAN_Y_MAX) { ++mis_n_offscreen; goto fizzle; }
            ny = (uint8_t)(mis.y + MIS_STEP_Y);
            ly = (uint8_t)(ny + MIS_H - 1u);
            break;
        case DIR_UP:
            if (mis.y < MAN_Y_MIN + MIS_STEP_Y) { ++mis_n_offscreen; goto fizzle; }
            ny = ly = (uint8_t)(mis.y - MIS_STEP_Y);
            break;
        default:
            goto fizzle;
        }
        blocked = (uint8_t)!mis_can_pass(lx, ly, mis.dir);

        /*
         * В стену снаряд НЕ ВХОДИТ: упёршись, он остаётся на месте и там же
         * взрывается. В оригинале то же самое сделано наоборот - шаг делается,
         * а потом снаряд возвращают назад (mis_x -= 8 в move_missile,
         * Work/TC.C). Результат один, а лишнего стирания земли нет: mis_erase
         * гасит прямоугольник, и шагнув в целый грунт снаряд прогрызал бы в
         * нём дырку.
         */
        mis_erase();
        if (!blocked) {
            mis.x = nx;
            mis.y = ny;
        }

        /*
         * Попадание проверяется ДО стены: враг может стоять в последней
         * клетке тоннеля, и тогда стена не должна его спасать. То же
         * замечание есть в порте на БК.
         */
        if (mis_hit_bug()) {
            mis.state = MIS_EXPLODING;
            mis.count = EXPLODE_TICKS;
            mis_draw(boom_spr, 0);
            return;
        }
        if (blocked) {
            ++mis_n_blocked;
            goto fizzle;
        }

        ++mis.count;
        mis_draw(fly_spr, (uint8_t)(mis.count % 3u));
        return;
    }

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
