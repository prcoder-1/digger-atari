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
#include "screen.h"

/*
 * Статические данные модуля уезжают в ОЗУ под ПЗУ: MAIN забит под завязку.
 * Обнуляет этот сегмент game_clear_hi_bss() первым делом в main - стартовый
 * код cc65 сюда не дотягивается, он чистит только сегмент BSS.
 */
#pragma bss-name (push, "BSS_HI")


extern const uint8_t spr_1_p0[],  spr_1_p1[];    /* стоит прямо           */
extern const uint8_t spr_b4_p0[], spr_b4_p1[];   /* наклон влево          */
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
/*
 * Сколько золото лежит, пока его не съели. В оригинале (init_tb, Work/TB.C)
 * broke_max = 150 - сложность*10 тактов, и отсчёт там начинается с первого
 * кадра разбивания, поэтому у нас из него вычитается BREAK_TICKS.
 *
 * Если под золотом ПУСТО, оно уходит быстро: оригинал в этом случае
 * перескакивает счётчик на последние десять тактов. Смысл в том, что куча,
 * лежащая на краю шахты, вот-вот провалится, и ждать её незачем.
 */
#define GOLD_QUICK    10u
#define BAG_BUG_SCORE 0x0250u                   /* 250 очков за придавленного врага */

/*
 * Спрайты подобраны ПО КАДРУ АНИМАЦИИ, а не по состоянию: и у раскачки, и у
 * разбивания кадров несколько. Раньше таблица шла по состоянию, и раскачка
 * навсегда застывала в одном наклоне.
 *
 * Соответствие оригиналу взято из draw_bag() в Source/IMAGE.C: BAG_0 - это
 * спрайт "1" (мешок прямо), BAG_4 и BAG_5 - наклоны, BAG_6 - падение,
 * BAG_7..BAG_9 - разбивание и золото.
 */
#define BAG_F_UPRIGHT   0u
#define BAG_F_LEFT      1u
#define BAG_F_RIGHT     2u
#define BAG_F_FALL      3u
#define BAG_F_BREAK1    4u
#define BAG_F_BREAK2    5u
#define BAG_F_GOLD      6u

static const uint8_t *const bag_spr[7][2] = {
    { spr_1_p0,  spr_1_p1  },   /* прямо           */
    { spr_b4_p0, spr_b4_p1 },   /* наклон влево    */
    { spr_b5_p0, spr_b5_p1 },   /* наклон вправо   */
    { spr_b6_p0, spr_b6_p1 },   /* падает          */
    { spr_b7_p0, spr_b7_p1 },   /* разбивается, 1  */
    { spr_b8_p0, spr_b8_p1 },   /* разбивается, 2  */
    { spr_b9_p0, spr_b9_p1 },   /* золото          */
};

/*
 * Раскачка: четыре кадра по кругу - влево, прямо, вправо, прямо. Кадр держится
 * два такта, поэтому индекс берётся как (count % 8) / 2. Ровно так это в
 * оригинале (Work/TB.C): там рисуют по остатку от восьми на значениях 0, 2, 4
 * и 6, а на нечётных не перерисовывают вовсе.
 *
 * Счётчик УБЫВАЕТ, как и в оригинале, поэтому и порядок кадров тот же.
 */
static const uint8_t bag_wobble[4] = {
    BAG_F_LEFT, BAG_F_UPRIGHT, BAG_F_RIGHT, BAG_F_UPRIGHT
};

static uint8_t bag_frame(const bag_t *b)
{
    switch (b->state) {
    case BAG_LOOSE:     return bag_wobble[(b->count & 7u) >> 1];
    case BAG_FALLING:   return BAG_F_FALL;
    /* Два кадра разбивания вместо одного: в оригинале за b7 идёт b8, и
     * только потом золото. Счётчик идёт 2, 1 - по кадру на такт. */
    case BAG_BREAKING:  return (b->count > 1u) ? BAG_F_BREAK1 : BAG_F_BREAK2;
    case BAG_GOLD:      return BAG_F_GOLD;
    default:            return BAG_F_UPRIGHT;
    }
}

static void bag_draw(const bag_t *b)
{
    uint8_t f = bag_frame(b);

    bl_x = (uint8_t)(b->x >> 1);
    bl_y = b->y;
    bl_h = SPRITE_H;
    if (b->x & 1u) {
        bl_w = 3;
        bl_src = bag_spr[f][1];
    } else {
        bl_w = 2;
        bl_src = bag_spr[f][0];
    }
    blit_mask();
}

/*
 * Убирает мешок ПО ЕГО ФОРМЕ: земля вокруг остаётся нетронутой.
 *
 * Нужно на первом шаге падения. Вокруг стоящего мешка земля ещё целая, и
 * прямоугольное стирание срезало бы её ровным краем - верх шахты выходил
 * рубленым, вместо скруглённого прогрыза оригинала. В порте на БК это тот же
 * приём: на первом этаже мешок стирается маской, дальше прямоугольником.
 *
 * Маска берётся у спрайта ПАДЕНИЯ: на экране к этому моменту нарисован уже
 * он - состояние сменилось на BAG_FALLING тактом раньше, и bag_draw успел
 * вывести именно этот кадр.
 */
static void bag_erase_shaped(const bag_t *b)
{
    bl_x = (uint8_t)(b->x >> 1);
    bl_y = b->y;
    bl_h = SPRITE_H;
    if (b->x & 1u) {
        bl_w = 3;
        bl_src = bag_spr[BAG_F_FALL][1];
    } else {
        bl_w = 2;
        bl_src = bag_spr[BAG_F_FALL][0];
    }
    blit_erase();
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

/* Сколько тактов живёт куча золота на этом уровне; см. GOLD_QUICK */
static uint8_t gold_wait;

void bags_init(uint8_t screen, uint8_t difficulty)
{
    uint8_t x, y, n = 0;

    gold_wait = (uint8_t)(150u - 10u * difficulty - BREAK_TICKS);

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

/*
 * Держит ли Диггер мешок собой. Перенос get_man_block (Work/TC.C): подкопав
 * мешок снизу, Диггер упирается в него и не даёт раскачиваться, пока лезет по
 * вертикали в том же столбце - в клетке мешка или прямо под ней.
 *
 * Условие "лезет" - именно ШАГ по вертикали, а не взгляд: в оригинале сюда
 * приходит m_dir, а он становится STOP, едва отпущен джойстик. Стоит Диггеру
 * замереть под мешком - и тот пойдёт качаться.
 */
static uint8_t man_blocks(uint8_t xl, uint8_t yl)
{
    uint8_t my;

    if (man.move != DIR_UP && man.move != DIR_DOWN)
        return 0;
    if (x_log_tbl[man.x] != xl)
        return 0;
    my = y_log_tbl[man.y];
    return (uint8_t)(my == yl || (uint8_t)(my + 1u) == yl);
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
    return (uint8_t)(near8(b->x, x, 3u) && near8(b->y, y, 14u) &&
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

/*
 * Стоит ли на пути мешок. Вбок его толкают (bags_push), а сверху и снизу
 * сдвинуть нельзя - он просто не пускает. В оригинале это ветка move_bag для
 * UP/DOWN, в порте на БК - collision_flag при man.dir вверх или вниз.
 */
uint8_t bags_blocked(uint8_t x_sub, uint8_t y)
{
    uint8_t i;

    for (i = 0; i < MAX_BAGS; ++i) {
        const bag_t *b = &bags[i];
        if (b->state != BAG_STANDING && b->state != BAG_LOOSE)
            continue;
        if (near8(b->x, x_sub, 3u) && near8(b->y, y, 14u))
            return 1;
    }
    return 0;
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

/*
 * Конец жизни: мешки, уже пришедшие в движение, с поля убираются.
 *
 * Правило одинаковое в оригинале (end_tb в Work/TB.C) и в порте на БК
 * (init_level_state): остаётся только тот мешок, что стоял смирно и никуда
 * не двигался. Качающийся, падающий, разбитый и тот, которого толкали,
 * исчезают вместе с Диггером - иначе после возрождения они продолжили бы
 * начатое падение, а Диггер к этому моменту уже в другом конце поля.
 */
void bags_respawn(void)
{
    uint8_t i;

    for (i = 0; i < MAX_BAGS; ++i) {
        bag_t *b = &bags[i];
        if (b->state == BAG_NONE)
            continue;
        if (b->state == BAG_STANDING && b->dir == DIR_STOP)
            continue;                   /* стоял смирно - остаётся */
        bag_erase(b);
        b->state = BAG_NONE;
    }
}

/*
 * Вернуть на экран уцелевшие мешки и золото.
 *
 * Стоящий мешок и золото сами себя не перерисовывают: их рисуют один раз и
 * дальше не трогают. Стирание надгробия или врага накрывает соседнюю клетку и
 * может их задеть, поэтому после уборки поля их нужно нарисовать заново -
 * иначе мешок пропал бы с картинки, оставшись в модели. В порте на БК тем же
 * занят redraw_bags.
 */
void bags_redraw_all(void)
{
    uint8_t i;

    for (i = 0; i < MAX_BAGS; ++i)
        if (bags[i].state != BAG_NONE)
            bag_draw(&bags[i]);
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
                if (!supported(b) &&
                    !man_blocks(x_log_tbl[b->x],
                                (uint8_t)(y_log_tbl[b->y] + 1u))) {
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
            /*
             * ПОТОЛОК шахты закругляется обычным прогрызом ВВЕРХ - тем же,
             * которым Диггер закругляет потолок своего хода. В порте на БК
             * это gnaw(DIR_UP, x, y + 9) на каждом шаге падения.
             */
            field_gnaw(DIR_UP, b->x, (uint8_t)(b->y + 8u));
            /*
             * На первом шаге мешок убирается ПО ФОРМЕ: вокруг него земля ещё
             * целая, и прямоугольник срезал бы её ровным краем. Дальше мешок
             * уже внутри своей шахты, там прямоугольник ничего не портит и
             * стоит вдвое дешевле.
             */
            if (b->fell == 0u)
                bag_erase_shaped(b);
            else
                bag_erase(b);
            bag_carve(b);               /* до сдвига: отмечается клетка снизу */
            /* Дно и бока шахты - свои стиратели мешка, см. field_gnaw_bag */
            field_gnaw_bag(b->x, b->y, (uint8_t)(b->fell == 0u));
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
                if (b->fell) {
                    sfx_play(750u, SFX_THUD, 12u, 2u);   /* глухой удар */
                    screen_shake();                      /* и поле подпрыгивает */
                }
                b->count = BREAK_TICKS;
            }
            if (b->y >= (FIELD_Y_OFFSET + (H_MAX - 1u) * CELL_H_PX)) {
                b->y = (uint8_t)(FIELD_Y_OFFSET + (H_MAX - 1u) * CELL_H_PX);
                b->state = BAG_BREAKING;
                b->count = BREAK_TICKS;
                sfx_play(750u, SFX_THUD, 12u, 2u);
                screen_shake();         /* удар о дно поля тоже трясёт */
            }
            break;

        case BAG_BREAKING:
            if (--b->count == 0) {
                b->state = BAG_GOLD;
                b->count = gold_wait;
            }
            break;

        case BAG_GOLD:
            /*
             * Куча не лежит вечно: не съели - истлела. Перенос отсчёта
             * BAG_BROKEN до broke_max из task_b (Work/TB.C).
             */
            if (b->count > GOLD_QUICK) {
                uint8_t xl = x_log_tbl[b->x];
                uint8_t yl = (uint8_t)(y_log_tbl[b->y] + 1u);

                if (yl < H_MAX && (background[yl][xl] & BG_INTACT) == 0u)
                    b->count = GOLD_QUICK;      /* под кучей пусто - уйдёт скоро */
            }
            if (--b->count == 0) {
                bag_erase(b);
                b->state = BAG_NONE;
                continue;                       /* рисовать уже нечего */
            }
            break;

        default:
            break;
        }

        bag_draw(b);
    }
}

/*
 * Золото съел ВРАГ. В оригинале это та же grab_money (Work/TB.C): куча
 * исчезает, но очков и звука не даётся - они в ней только для Диггера
 * (colision & 0x01), врагу достаётся лишь сброс задержки.
 */
void bags_bug_gold(uint8_t x_log, uint8_t y_log)
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
        return;
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
        sfx_gold();                             /* съедание золота */
        return 1;
    }
    return 0;
}

#pragma bss-name (pop)
