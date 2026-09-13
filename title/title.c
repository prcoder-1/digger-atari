/*
 * title.c - заставка. Порт title.c из версии для БК 0010.
 *
 * Заставка идёт ДО игры и памяти у неё не отнимает: её кусок файла лежит
 * первым, запускается вектором INITAD, а по возвращении загрузчик DOS кладёт
 * игру поверх. Разбор устройства - в cfg/title.cfg и title/crt.s.
 *
 * Стадии те же, что на БК:
 *
 *      1. ОБЛОЖКА - картинка в четырёх цветах, ждёт нажатия;
 *      2. ДЕМО    - таблица: слева панель кредитов и клавиш, справа по
 *                   очереди выходят Ноббин, Хоббин и Диггер, за ними
 *                   появляются призы. Нажатие запускает игру.
 *
 * Таймлайн демо устроен как в оригинале: у каждого действующего лица своё
 * ОКНО времени, а внутри окна всё считается от его начала. Так добавить или
 * убрать участника - это правка одной таблицы, а не разбор switch по каждому.
 */
#include "tscreen.h"
#include "ttext.h"

#define CH      (*(volatile unsigned char *)0x02FC)   /* код клавиши      */
#define STRIG0  (*(volatile unsigned char *)0x0284)   /* кнопка джойстика */
#define RTCLOK2 (*(volatile unsigned char *)0x0014)   /* счётчик кадров   */
#define KEY_NONE 0xFFu

/* Блиттер и гашение прямоугольника - из игры, как есть */
extern unsigned char bl_x, bl_y, bl_w, bl_h;
extern const unsigned char *bl_src;
extern unsigned char cp_x, cp_y, cp_w, cp_h;
#pragma zpsym ("bl_x")
#pragma zpsym ("bl_y")
#pragma zpsym ("bl_w")
#pragma zpsym ("bl_h")
#pragma zpsym ("bl_src")
#pragma zpsym ("cp_x")
#pragma zpsym ("cp_y")
#pragma zpsym ("cp_w")
#pragma zpsym ("cp_h")
void blit_mask(void);
void clear_px(void);

/* Спрайты игры. Подписи из исходников оригинала: m - Ноббин, c/a - Хоббин
 * вправо/влево, r/l - Диггер вправо/влево, cn - камень, z1 - вишенка. */
extern const unsigned char spr_m0_p0[], spr_m0_p1[];
extern const unsigned char spr_m1_p0[], spr_m1_p1[];
extern const unsigned char spr_m2_p0[], spr_m2_p1[];
extern const unsigned char spr_a0_p0[], spr_a0_p1[];
extern const unsigned char spr_a1_p0[], spr_a1_p1[];
extern const unsigned char spr_a2_p0[], spr_a2_p1[];
extern const unsigned char spr_c0_p0[], spr_c0_p1[];
extern const unsigned char spr_l0_p0[], spr_l0_p1[];
extern const unsigned char spr_l1_p0[], spr_l1_p1[];
extern const unsigned char spr_l2_p0[], spr_l2_p1[];
extern const unsigned char spr_r0_p0[], spr_r0_p1[];
extern const unsigned char spr_1_p0[],  spr_1_p1[];
extern const unsigned char spr_cn_p0[], spr_cn_p1[];
extern const unsigned char spr_z1_p0[], spr_z1_p1[];

#define SPRITE_H      15u
#define SPRITE_W_SUB  4u        /* подшаг - 4 точки, полбайта */

/* --- геометрия таблицы демо ---------------------------------------------- */

#define TITLE_Y       2u
#define RULE_Y        18u
#define PANEL_X       0u        /* мелкий шрифт: 15 знаков по 8 точек */
#define PANEL_W       15u
#define PANEL_Y       26u
#define PANEL_ROWS    19u
#define PANEL_STEP    8u

#define DEMO_X        16u       /* байт: правая половина под демо */
#define ONE_PLAYER_Y  24u

#define WALK_FROM_SUB 76u       /* появляется у правого края экрана */
#define WALK_TO_SUB   40u       /* доходит до места и встаёт        */
#define WALK_ROW0     44u
#define WALK_STEP     22u       /* шаг строк между участниками      */
#define NAME_DX       4u        /* байт от фигуры до подписи        */

#define ITEM_ROW0     (WALK_ROW0 + 3u * WALK_STEP)

/* --- таймлайн (в кадрах, кадр 1/50 с) ------------------------------------ */

#define MOVE_EVERY    4u        /* кадров на один подшаг            */
#define WALK_MOVE     ((WALK_FROM_SUB - WALK_TO_SUB) * MOVE_EVERY)
#define WALK_TURN     (WALK_MOVE + 1u)
#define WALK_NAME     (WALK_TURN + 16u)
#define WALK_SPAN     (WALK_NAME + 40u)
#define WALKERS_START 50u

#define ITEM_NAME     16u
#define ITEM_SPAN     120u
#define ITEMS_START   (WALKERS_START + 3u * WALK_SPAN)

#define DEMO_END      (ITEMS_START + 3u * ITEM_SPAN + 150u)

/* --- действующие лица ----------------------------------------------------- */

/*
 * Ходячий участник: три кадра анимации на ходу (каждый в двух фазах сдвига,
 * как у всех спрайтов игры) и отдельный кадр "стоит", повёрнутый к зрителю.
 * У Ноббина поворота нет - у него в оригинале нет зеркального спрайта.
 */
typedef struct {
    const unsigned char *walk[3][2];
    const unsigned char *stand[2];
    const char *name;
} walker_t;

static const walker_t walkers[3] = {
    { { { spr_m0_p0, spr_m0_p1 }, { spr_m1_p0, spr_m1_p1 },
        { spr_m2_p0, spr_m2_p1 } },
      { spr_m0_p0, spr_m0_p1 }, "NOBBIN" },
    { { { spr_a0_p0, spr_a0_p1 }, { spr_a1_p0, spr_a1_p1 },
        { spr_a2_p0, spr_a2_p1 } },
      { spr_c0_p0, spr_c0_p1 }, "HOBBIN" },
    { { { spr_l0_p0, spr_l0_p1 }, { spr_l1_p0, spr_l1_p1 },
        { spr_l2_p0, spr_l2_p1 } },
      { spr_r0_p0, spr_r0_p1 }, "DIGGER" },
};

/* Призы. Подписи те же, что на БК */
typedef struct {
    const unsigned char *img[2];
    const char *name;
} item_t;

static const item_t items[3] = {
    { { spr_1_p0,  spr_1_p1  }, "GOLD" },
    { { spr_cn_p0, spr_cn_p1 }, "EMERALD" },
    { { spr_z1_p0, spr_z1_p1 }, "BONUS" },
};

/* --- панели --------------------------------------------------------------- */

/*
 * Текст панелей перенесён с БК (digger_credits.c). Цветовые коды оттуда
 * убраны: в ANTIC F оттенок один на строку растра, и раскрасить слова внутри
 * строки нечем. Названия клавиш - НАШИ: на ATARI движение идёт на +*-= или
 * джойстиком, выстрел - кнопкой.
 */
static const char *const credits[PANEL_ROWS] = {
    "THE CANADIAN",
    "STUDIO",
    "WINDMILL",
    "SOFTWARE",
    "LEAD PROGRAMMER",
    "ROB STEALTH AND",
    "HIS TEAMMATES",
    "RAY EWAN AND",
    "BILL MONTGOMERY",
    "RELEASED",
    "DIGGER IN 1983",
    "FOR IBM PC.",
    "",
    "IN 2026 DIGGER",
    "WAS CONVERTED",
    "TO RUN ON THE",
    "ATARI 65XE",
    "HOME COMPUTER.",
    "",
};

static const char *const keys_help[PANEL_ROWS] = {
    "CONTROLS:",
    "",
    "+ * - =  OR",
    "JOYSTICK -",
    "MOVE DIGGER",
    "",
    "JOY BUTTON -",
    "FIREBALL",
    "",
    "COLLECT ALL",
    "EMERALDS OR",
    "KILL ALL BUGS",
    "TO FINISH",
    "THE LEVEL.",
    "",
    "PUSH BAGS TO",
    "DROP GOLD ON",
    "THE BUGS.",
    "",
};

/* --- вывод ---------------------------------------------------------------- */

/* Спрайт по положению в подшагах: фаза сдвига выбирается младшим битом,
 * ровно как в игре (см. man_draw в src/man.c) */
static void put_sprite(unsigned char x_sub, unsigned char y,
                       const unsigned char *const *phases)
{
    bl_x = (unsigned char)(x_sub >> 1);
    bl_y = y;
    bl_h = SPRITE_H;
    if (x_sub & 1u) {
        bl_w = 3;
        bl_src = phases[1];
    } else {
        bl_w = 2;
        bl_src = phases[0];
    }
    blit_mask();
}

static void erase_sprite(unsigned char x_sub, unsigned char y)
{
    cp_x = x_sub;
    cp_y = y;
    cp_w = SPRITE_W_SUB;
    cp_h = SPRITE_H;
    clear_px();
}

/* Горизонтальная черта во всю ширину экрана */
static void draw_rule(unsigned char y)
{
    unsigned int i;
    unsigned char *p = screen_buf + (unsigned int)y * SCR_BYTES;

    for (i = 0; i < SCR_BYTES; ++i)
        p[i] = 0xFF;
    for (i = 0; i < SCR_BYTES; ++i)
        p[SCR_BYTES + i] = 0xFF;
}

static void draw_panel(const char *const *rows)
{
    unsigned char i;
    unsigned char y = PANEL_Y;

    for (i = 0; i < PANEL_ROWS; ++i) {
        const char *s = rows[i];
        unsigned char n = 0;
        unsigned char j;

        /* Гасим строку целиком: панели чередуются, и от прежней остались бы
         * хвосты там, где новая строка короче. */
        cp_x = PANEL_X * 2u;
        cp_y = y;
        cp_w = PANEL_W * 2u;
        cp_h = PANEL_STEP;
        clear_px();

        while (s[n])
            ++n;
        j = (unsigned char)(PANEL_X + (PANEL_W - n) / 2u);
        text_small(j, y, s);
        y += PANEL_STEP;
    }
}

/* --- состояние демо ------------------------------------------------------- */

static unsigned int demo_time;
static unsigned char walker_x[3];       /* 0 - участника ещё нет на экране */
static unsigned char phase, phase_inc;
static unsigned char panel_keys;

static void demo_frame(void)
{
    unsigned int t;
    unsigned char i;

    if (demo_time == 0) {
        /* Начало круга: убрать прошлых участников и сменить панель */
        cp_x = DEMO_X * 2u;
        cp_y = WALK_ROW0;
        cp_w = (SCR_BYTES - DEMO_X) * 2u;
        cp_h = (unsigned char)(192u - WALK_ROW0);
        clear_px();

        for (i = 0; i < 3; ++i)
            walker_x[i] = 0;

        draw_panel(panel_keys ? keys_help : credits);
        panel_keys = (unsigned char)!panel_keys;
    }

    /* Ходячие: t - время внутри окна очередного участника */
    t = demo_time - WALKERS_START;
    for (i = 0; i < 3; ++i, t -= WALK_SPAN) {
        if (t >= WALK_SPAN)
            continue;                   /* не его окно (или ещё до начала) */

        if (t == 0) {
            walker_x[i] = WALK_FROM_SUB;
        } else if (t <= WALK_MOVE) {
            if ((demo_time & (MOVE_EVERY - 1u)) == 0) {
                erase_sprite(walker_x[i], (unsigned char)(WALK_ROW0 + i * WALK_STEP));
                --walker_x[i];
            }
        } else if (t == WALK_TURN) {
            /* Дошёл - поворачивается к зрителю */
            erase_sprite(walker_x[i], (unsigned char)(WALK_ROW0 + i * WALK_STEP));
            put_sprite(walker_x[i], (unsigned char)(WALK_ROW0 + i * WALK_STEP),
                       walkers[i].stand);
        } else if (t == WALK_NAME) {
            text_big((unsigned char)((walker_x[i] >> 1) + NAME_DX),
                     (unsigned char)(WALK_ROW0 + i * WALK_STEP + 2u),
                     walkers[i].name);
        }
        break;
    }

    /* Призы: появляются под Диггером, каждый строкой ниже */
    t = demo_time - ITEMS_START;
    for (i = 0; i < 3; ++i, t -= ITEM_SPAN) {
        if (t >= ITEM_SPAN)
            continue;
        if (t == 0) {
            put_sprite(WALK_TO_SUB,
                       (unsigned char)(ITEM_ROW0 + i * WALK_STEP),
                       items[i].img);
        } else if (t == ITEM_NAME) {
            text_big((unsigned char)((WALK_TO_SUB >> 1) + NAME_DX),
                     (unsigned char)(ITEM_ROW0 + i * WALK_STEP + 2u),
                     items[i].name);
        }
        break;
    }

    /* Отрисовка идущих: пока участник не встал, его рисуем кадром ходьбы */
    for (i = 0; i < 3; ++i) {
        unsigned int w = demo_time - WALKERS_START - (unsigned int)i * WALK_SPAN;
        if (walker_x[i] == 0 || w > WALK_MOVE)
            continue;
        put_sprite(walker_x[i], (unsigned char)(WALK_ROW0 + i * WALK_STEP),
                   walkers[i].walk[phase]);
    }

    if ((demo_time & (MOVE_EVERY - 1u)) == 0) {
        if (phase == 0)
            phase_inc = 1;
        else if (phase >= 2)
            phase_inc = 0xFFu;          /* -1 */
        phase = (unsigned char)(phase + phase_inc);
    }

    if (++demo_time > DEMO_END)
        demo_time = 0;
}

/* --- каркас --------------------------------------------------------------- */

static unsigned char any_key(void)
{
    if (CH != KEY_NONE) {
        CH = KEY_NONE;
        return 1;
    }
    return (unsigned char)(STRIG0 == 0);        /* кнопка активна нулём */
}

static void wait_frame(void)
{
    unsigned char t = RTCLOK2;
    while (RTCLOK2 == t)
        ;
}

static void demo_init(void)
{
    static const char digger_str[] = "D I G G E R";
    static const char one_player[] = "ONE PLAYER";

    screen_demo();

    text_big((unsigned char)((SCR_BYTES - text_big_width(digger_str)) / 2u),
             TITLE_Y, digger_str);
    draw_rule(RULE_Y);
    text_big((unsigned char)(DEMO_X +
                             (SCR_BYTES - DEMO_X - text_big_width(one_player)) / 2u),
             ONE_PLAYER_Y, one_player);

    demo_time = 0;
    phase = 0;
    phase_inc = 1;
    panel_keys = 0;                     /* первым кругом - кредиты */
}

void title_main(void)
{
    screen_save();
    CH = KEY_NONE;                      /* сбросить код, оставшийся от DOS */

    screen_cover();
    while (!any_key())
        wait_frame();

    demo_init();
    while (!any_key()) {
        demo_frame();
        wait_frame();
    }

    /*
     * Вернуть дисплей ОС. Пока загрузчик дочитывает игру, кадровое прерывание
     * ОС продолжает работать, а память под нашим дисплей-листом уже занимают
     * куски игры - без возврата на экране был бы мусор.
     */
    screen_restore();
}
