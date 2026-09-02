/*
 * main.c - M4: Диггер ходит и копает.
 *
 * Игровой такт - каждый пятый кадр PAL, то есть 10 Гц. Это на 9% медленнее
 * такта БК-порта (11 кадров в секунду), поэтому все счётчики, выраженные
 * в кадрах, переносятся из него без пересчёта.
 *
 * Ввод берётся из защёлки, которую заполняет отложенный VBI: к моменту его
 * работы теневые ячейки STICK0/STRIG0 уже обновлены операционной системой.
 */
#include <stdint.h>
#include "hw.h"
#include "screen.h"
#include "field.h"
#include "game.h"
#include "hud.h"
#include "bags.h"
#include "bugs.h"
#include "text.h"
#include "missile.h"
#include "sfx.h"
#include "menu.h"
#include "blit.h"
#include "pmg.h"
#include "pmgbug.h"

extern const uint8_t spr_z1_p0[];
extern uint8_t score[3];

/* Надгробие: в исходниках оригинала f1 - самый большой камень, f5 - самый
 * маленький, поэтому кадры идут от f5 к f1 и камень как бы вырастает. */
extern const uint8_t spr_f5_p0[], spr_f5_p1[];
extern const uint8_t spr_f4_p0[], spr_f4_p1[];
extern const uint8_t spr_f3_p0[], spr_f3_p1[];
extern const uint8_t spr_f2_p0[], spr_f2_p1[];
extern const uint8_t spr_f1_p0[], spr_f1_p1[];

static const uint8_t *const rip_spr[5][2] = {
    { spr_f5_p0, spr_f5_p1 }, { spr_f4_p0, spr_f4_p1 },
    { spr_f3_p0, spr_f3_p1 }, { spr_f2_p0, spr_f2_p1 },
    { spr_f1_p0, spr_f1_p1 },
};

/*
 * Траурный марш: одиннадцать нот, периоды и длительности перенесены из
 * порта на БК. Играется на голосе эффектов, а фоновая музыка на время
 * гибели умолкает - как в оригинале.
 */
/*
 * Делители канала мелодии, посчитанные из периодов БК той же формулой,
 * что и остальная музыка: AUDF = 63337/(2f) - 1, где f = 59664/период.
 * Через голос эффектов марш звучал бы октавой выше - там связанная пара
 * каналов и другая тактовая частота.
 */
static const uint8_t march_audf[11] = {
    120u, 120u, 120u, 120u, 100u, 107u, 107u, 120u, 120u, 127u, 120u
};
/*
 * Длительности в игровых тактах. В БК-порте они заданы в полупериодах и
 * дают ряд 3, 2, 1, 3, 2, 1, 2, 1, 2, 1, 6 в единицах восьмой ноты.
 * Восьмая там выходит около 213 мс (128 полупериодов x период 228 x 7.3 мкс
 * на такт цикла ожидания) - это близко к 181.6 мс восьмой основной мелодии.
 * При такте 10 Гц восьмая занимает ДВА такта, а не один, поэтому весь ряд
 * удвоен. С одинарным маршем звучал вдвое быстрее и не помещался в сцену.
 */
static const uint8_t march_len[11] = {
    6u, 4u, 2u, 6u, 4u, 2u, 4u, 2u, 4u, 2u, 12u
};

#define MARCH_TICKS   48u       /* сумма длительностей выше */
void __fastcall__ score_add(unsigned int bcd);
#ifdef DEMO_PROFILE
/* Профильная сборка: цвет фона переключается вокруг игрового такта, и его
 * длительность видна на снимке как цветная полоса. Единица - строка растра,
 * то есть реальное время; такт при 10 Гц это 5 кадров = 1560 строк. */
void __fastcall__ set_colbk(uint8_t c);
/* Такт начинается сразу после кадрового прерывания, то есть в гасящем
 * импульсе. Короткий такт целиком в него укладывается, полоса не попадает
 * на снимок, и отсутствие полосы неотличимо от неработающего измерителя.
 * Поэтому в профильной сборке начало такта сдвигается в видимую область. */
void __fastcall__ prof_mark(uint8_t on);
#endif
#ifdef DIGGER_DEMO
#include "demo.h"
#endif

void wait_frame(void);
void vbi_install(void);
void vbi_remove(void);

extern uint8_t vbi_frame;
extern uint8_t vbi_stick;
extern uint8_t vbi_trig;

#ifdef DEMO_PROFILE
/* В профильной сборке такт идёт каждый кадр: иначе полоса видна лишь в
 * одном кадре из пяти, снимок ловит произвольный, и её отсутствие ничего
 * не доказывает. Нагрузка самого такта от частоты вызова не зависит. */
#define TICK_FRAMES  1u
#else
#define TICK_FRAMES  5u         /* 50 Гц / 5 = 10 Гц */
#endif

/*
 * Состояние партии. На БК гибель Диггера была блокирующей процедурой с
 * задержками внутри - здесь она обязана быть автоматом, иначе такт встанет
 * и вместе с ним замрут музыка и звук.
 */
#define ST_PLAY   0u
#define ST_DYING  1u
#define ST_OVER   2u

/* Столько тактов длится сцена гибели: хватает на одиннадцать нот марша
 * и на пять кадров вырастающего надгробия. */
/*
 * Разбивка сцены гибели, по образцу порта на БК: свип, пауза, марш.
 * Пять кадров надгробия по четыре такта каждый занимают ровно ту часть
 * сцены, что идёт после паузы.
 */
/*
 * Длительности взяты из БК-порта: свип там около 0.9 с (64 коротких тона),
 * затем пауза delay_ms(500), затем марш вместе с надгробием.
 * При такте 10 Гц это девять и пять тактов соответственно.
 */
#define DEATH_SWEEP   9u        /* тактов скрипучего звука     */
#define DEATH_PAUSE   5u        /* тактов тишины перед маршем  */
/*
 * Свип гибели. Период БК 950 даёт 62.8 Гц - на POKEY это ниже полезного
 * диапазона, и вместе с самым шумным тембром (два полинома) слышалось как
 * треск, а не как скрежет. На БК тот же период звучал выше: там высота была
 * завышена примерно на 15% из-за некалиброванного цикла ожидания, и звук был
 * прямоугольным, а не шумовым.
 *
 * Поэтому диапазон поднят в слышимую область, а тембр взят четырёхбитным
 * полиномом: он даёт шероховатость с различимой высотой - это и есть скрежет.
 * Период РАСТЁТ, то есть тон ползёт вниз - как при падении.
 */
#define DEATH_P0      200u      /* 298 Гц в начале             */
/* Прибавка к делителю за КАДР, а не за такт: за девять тактов свипа высота
 * уезжает от 298 Гц примерно к 115 Гц, но плавно, без слышимых ступеней. */
#define DEATH_SLIDE   120u
#define DYING_TICKS   (DEATH_SWEEP + DEATH_PAUSE + MARCH_TICKS)

/* Пять кадров надгробия равномерно по всей длине марша */
#define RIP_STAGE     (MARCH_TICKS / 5u)
#define START_LIVES   3u

/*
 * Уровень считается пройденным по любому из двух условий оригинала: собраны
 * все камни, либо запас врагов на уровень исчерпан и на поле никого не
 * осталось. Второе условие и делает отстрел врагов осмысленной тактикой.
 */
#define MAX_LEVEL     1000u

uint8_t game_level = 1;
uint8_t levels_done = 0;        /* накопительный счётчик для проверки */

/*
 * Бонус-режим. Вишенка появляется в правом верхнем углу, когда запас врагов
 * на уровень исчерпан; съев её, Диггер на время меняется с врагами ролями.
 * Первый съеденный враг даёт 200 очков, каждый следующий вдвое больше -
 * как в оригинале.
 */
#define BONUS_OFF    0u
#define BONUS_READY  1u
#define BONUS_ON     2u

uint8_t bonus_state = BONUS_OFF;
uint8_t bonus_eaten = 0;        /* накопительный счётчик съеденных врагов */
static uint16_t bonus_time;
static uint16_t bonus_value;    /* очки за следующего врага, в BCD */

/* Глобальная, чтобы её видела телеметрия демо-сборки */
uint8_t game_lives = START_LIVES;

/* Джойстик: биты активны нулём (1 = вверх, 2 = вниз, 4 = влево, 8 = вправо).
 * Таблица разбирает и диагонали: побеждает первое совпавшее направление. */
static dir_t stick_dir(uint8_t s)
{
    s = (uint8_t)(~s & 0x0Fu);
    if (s & 0x08u) return DIR_RIGHT;
    if (s & 0x04u) return DIR_LEFT;
    if (s & 0x02u) return DIR_DOWN;
    if (s & 0x01u) return DIR_UP;
    return DIR_STOP;
}

/* Клавиши-стрелки Atari: коды KBCODE без учёта модификаторов */
#define KEY_PLUS    0x06u       /* '+' влево  */
#define KEY_STAR    0x07u       /* '*' вправо */
#define KEY_MINUS   0x0Eu       /* '-' вверх  */
#define KEY_EQUAL   0x0Fu       /* '=' вниз   */

static dir_t key_dir(void)
{
    uint8_t k = OS_CH;
    if (k == 0xFFu)
        return DIR_STOP;
    OS_CH = 0xFF;
    switch (k & 0x3Fu) {
    case KEY_STAR:  return DIR_RIGHT;
    case KEY_PLUS:  return DIR_LEFT;
    case KEY_MINUS: return DIR_UP;
    case KEY_EQUAL: return DIR_DOWN;
    default:        return DIR_STOP;
    }
}

int main(void)
{
    uint8_t next;
    uint8_t state = ST_PLAY;
    uint8_t dying = 0;
    uint8_t hit;
    uint8_t crushed;
    uint8_t march_note = 0, march_left = 0, rip_shown = 0xFF;
    dir_t want, kd;

    /*
     * Раньше screen_init: тот включает выборку игроков в DMACTL, и к этому
     * моменту PMBASE обязан указывать на уже очищенный буфер - иначе ANTIC
     * успеет вывести на экран мусор из непроинициализированной памяти.
     */
    pmg_init();
    OS_PCOLR0 = COL_PM_BODY;
    OS_PCOLR1 = COL_PM_BEAK;
    OS_PCOLR2 = COL_PM_BUG;
    OS_PCOLR3 = COL_PM_BUG;
    screen_init();
    field_build_tables();
    sfx_init();

    /*
     * Прерывание ставится ДО заставки. Заставка ждёт нажатия по ячейке
     * vbi_trig, а заполняет её только обработчик прерывания; при обратном
     * порядке ячейка навсегда оставалась бы в исходном состоянии
     * "кнопка не нажата", и заставка висла бы.
     */
    vbi_install();

    /* Таблица рекордов ведётся, но экран с ней не показывается: на его
     * месте будет анимированная заставка с выезжающими актёрами. Игра
     * стартует сразу в уровень. */
    menu_init();

    /* Экран чистится целиком: field_init заливает землёй только полосу
     * поля, и всё, что было выше, осталось бы в зоне HUD. */
    screen_clear(0);
    field_init(0);
    man_init();
    bags_init(0);
    bugs_init(0, 1);
    missile_init(1);
    music_select(0);
    hud_init();
    hud_score();
    hud_lives(START_LIVES);
#ifdef DIGGER_DEMO
    demo_init();
#endif

    next = (uint8_t)(vbi_frame + TICK_FRAMES);

    for (;;) {
        /* Ждём границу игрового такта. Если такт затянулся, досчитываем
         * до следующей границы, кратной TICK_FRAMES, а не пропускаем её. */
        while ((uint8_t)(next - vbi_frame) < 0x80u)
            wait_frame();
        next = (uint8_t)(next + TICK_FRAMES);

#ifdef DIGGER_DEMO
        /* Вместо ввода - зашитый маршрут; телеметрия рисуется каждый такт */
        want = demo_input();
        (void)kd;

        /*
         * Маршрут кончился - игра замирает целиком. Замирания одного счётчика
         * тактов мало: враги продолжали ходить, счётчики звука росли, и числа
         * зависели от того, на какой секунде снят кадр. Проверка при этом
         * сравнивала состояние по настенным часам, а не по такту, и падала
         * от одной только загрузки машины.
         */
        if (demo_finished()) {
            demo_telemetry();
            continue;
        }
#else
        want = stick_dir(vbi_stick);
        kd = key_dir();
        if (want == DIR_STOP)
            want = kd;
#endif

#ifdef DEMO_PROFILE
        prof_mark(1);                   /* начало такта */
#endif
        if (state == ST_PLAY) {
            /* Кнопка джойстика активна нулём; в демо-сборке стрельба
             * включается сама, когда Диггер стоит на месте. */
#if defined(DIGGER_DEMO) && !defined(DEMO_DEATH)
            /* В сборке под проверку гибели стрельба выключена: иначе Диггер
             * отстреливает подходящих врагов и не погибает. */
            if (want == DIR_STOP)
                missile_fire();
#elif !defined(DIGGER_DEMO)
            if (vbi_trig == 0)
                missile_fire();
#endif
            man_tick(want);
            bags_tick();
            bugs_tick();
            missile_tick();

            if (!field_coins_left() ||
                (bugs_left_to_spawn() == 0u && bugs_active == 0u)) {
                /* Экран уровня: после восьмого крутятся пятый..восьмой */
                uint8_t screen;
                ++levels_done;
                /* Дополнительная жизнь за каждые 20000 очков, как в оригинале */
                if (score[0] >= 0x02u && game_lives < 6u) {
                    ++game_lives;
                    hud_lives(game_lives);
                    sfx_play(38u, SFX_TONE, 14u, 4u);   /* звук новой жизни */
                }
                sfx_play(114u, SFX_TONE, 12u, 4u);   /* конец уровня */
                if (game_level < MAX_LEVEL)
                    ++game_level;
                screen = game_level;
                if (screen > 8u)
                    screen = (uint8_t)((game_level & 3u) + 5u);
                --screen;

                field_init(screen);
                man_init();
                /* Мешки перезаводятся вместе с полем: без этого на новый
                 * уровень переезжало их состояние с прошлого - падающие
                 * продолжали падать, а разбитые оставались золотом. */
                bags_init(screen);
                bugs_init(screen, game_level > 10u ? 10u : game_level);
                bonus_state = BONUS_OFF;
                missile_init(game_level > 10u ? 10u : game_level);
                hud_score();
                continue;
            }

            /* Вишенка появляется, когда рождать больше некого */
            if (bonus_state == BONUS_OFF && bugs_left_to_spawn() == 0u) {
                bonus_state = BONUS_READY;
                bl_x = (uint8_t)((FIELD_X_OFFSET + (W_MAX - 1u) * CELL_W_SUB) >> 1);
                bl_y = FIELD_Y_OFFSET;
                bl_w = 2;
                bl_h = SPRITE_H;
                bl_src = spr_z1_p0;
                blit_mask();
            }

            /* Диггер дошёл до вишенки */
            if (bonus_state == BONUS_READY &&
                x_log_tbl[man.x] == W_MAX - 1u && y_log_tbl[man.y] == 0u) {
                bonus_state = BONUS_ON;
                bonus_time = 250u;
                bonus_value = 0x0200u;
                bugs_fleeing = 1;
                music_select(1);        /* бонусная мелодия вдвое быстрее */
                score_add(0x1000u);             /* вишенка - 1000 очков */
                hud_score();
            }

            if (bonus_state == BONUS_ON) {
                /* Сирена: два тона попеременно, по одному на такт */
                sfx_play((bonus_time & 1u) ? 61u : 75u, SFX_TONE, 8u, 1u);
                if (--bonus_time == 0) {
                    bonus_state = BONUS_OFF;
                    bugs_fleeing = 0;
                    music_select(0);
                }
            }

            /*
             * Падающий мешок давит Диггера. Бонус-режим тут не помогает: он
             * меняет роли только с врагами, а мешок бьёт независимо от него.
             */
            crushed = bags_hit_man();
            hit = bugs_hit_man();
            if (crushed || hit != BUG_NO_HIT) {
                if (!crushed && bonus_state == BONUS_ON) {
                    /* Роли поменялись: враг съеден, очки удваиваются */
                    bugs_remove(hit);
                    ++bonus_eaten;
                    score_add(bonus_value);
                    if (bonus_value < 0x2000u)
                        bonus_value <<= 1;      /* BCD-удвоение сдвигом верно
                                                 * для 200, 400, 800 и 1600 */
                    hud_score();
                    sfx_play(93u, SFX_TONE, 12u, 2u);
                } else {
#ifdef DEMO_INVULNERABLE
                    /* Испытательная фикстура: бонус-режим не зависит от того,
                     * может ли Диггер погибнуть, а без неё проверка зависела
                     * бы от везения - до вишенки он не доживал.
                     * В игру не входит. */
                    if (!crushed)
                        bugs_remove(hit);
#else
                    state = ST_DYING;
                    /* Надгробие растёт в кадровом буфере, а не в спрайте:
                     * фигура Диггера должна уйти с экрана. Заодно снимается
                     * цвет с врага-убийцы: надгробие затрёт его фигуру, а
                     * перерисовывать врагов в этой сцене уже некому. */
                    pmg_hide();
                    pmg_bugs_hide_at(man.x, man.y);
                    dying = DYING_TICKS;
                    march_note = 0;
                    march_left = 0;
                    rip_shown = 0xFF;
                    /* Скрежет запускается ОДИН раз, дальше высота ползёт
                     * сама в кадровом прерывании: повторный запуск каждый
                     * такт давал новую атаку, и девять таких атак слышались
                     * как мелодия вместо скрежета. */
                    sfx_play(DEATH_P0, SFX_THUD, 14u, DEATH_SWEEP);
                    sfx_sweep = DEATH_SLIDE;
                    /* Popcorn умолкает на всю сцену. Раньше остановки здесь
                     * не было, и фоновая мелодия играла вперемешку с маршем
                     * по тому же каналу. */
                    music_stop();
#endif
                }
            }

            /*
             * Враги выводятся ПОСЛЕДНИМИ в такте, когда отработали и снаряд,
             * и столкновения. Иначе выведенного врага затирал прямоугольником
             * тот, кто обрабатывается позже; заливка аппаратного спрайта при
             * этом оставалась, и на экране получался зелёный блок без фигуры.
             */
            bugs_draw_all();
        }
        /* Гашение эффектов идёт и во время сцены гибели: иначе свип
         * оставался бы висеть на канале до конца сцены. */
        sfx_tick();

        if (state == ST_DYING) {
            /*
             * Сцена гибели построена как в порте на БК: сначала скрипучий
             * свип, затем короткая пауза, затем вырастающее надгробие под
             * траурный марш. Такт при этом продолжает идти - блокировать
             * его нельзя, иначе замрут и звук, и музыка.
             *
             * Popcorn молчит всю сцену и после возрождения начинается
             * заново, а не с места обрыва.
             */
            uint8_t elapsed = (uint8_t)(DYING_TICKS - dying);

            if (elapsed < DEATH_SWEEP) {
                /* Скрежет уже звучит и ползёт сам - здесь ничего не нужно */
            } else if (elapsed == DEATH_SWEEP) {
                sfx_sweep = 0;          /* скрежет кончился */
            } else if (elapsed < DEATH_SWEEP + DEATH_PAUSE) {
                /* Пауза перед надгробием - тишина, как в оригинале */
            } else if (march_left == 0 && march_note < 11u) {
                music_note(march_audf[march_note]);
                march_left = march_len[march_note];
                ++march_note;
            }
            if (march_left) {
                /*
                 * Последний такт ноты - тишина. Без этого разрыва четыре
                 * идущие подряд ноты одной высоты (начало марша) сливались
                 * в один затянутый тон, и начало марша пропадало на слух.
                 * В БК-порте ту же роль играет delay_ms(30) между нотами,
                 * а фоновый проигрыватель гасит последнюю восьмую ноты.
                 */
                if (march_left == 1u)
                    music_stop();
                --march_left;
            }

            /* Надгробие растёт только после паузы, вместе с маршем */
            if (elapsed >= DEATH_SWEEP + DEATH_PAUSE) {
                uint8_t stage = (uint8_t)((elapsed - DEATH_SWEEP - DEATH_PAUSE)
                                          / RIP_STAGE);
                if (stage < 5u && stage != rip_shown) {
                    rip_shown = stage;
                    cp_x = man.x; cp_y = man.y;
                    cp_w = SPRITE_W_SUB; cp_h = SPRITE_H;
                    clear_px();
                    bl_x = (uint8_t)(man.x >> 1);
                    bl_y = man.y;
                    bl_h = SPRITE_H;
                    if (man.x & 1u) { bl_w = 3; bl_src = rip_spr[stage][1]; }
                    else         { bl_w = 2; bl_src = rip_spr[stage][0]; }
                    blit_mask();
                }
            }

            if (--dying == 0) {
                music_stop();           /* оборвать последнюю ноту марша */
                rip_shown = 0xFF;
                if (game_lives > 1u) {
                    --game_lives;
                    hud_lives(game_lives);
                    /*
                     * Поле НЕ перерисовывается. Прокопанные ходы, камни и
                     * стоящие мешки остаются как были; убирается только то,
                     * что осталось от гибели: надгробие, снаряд в полёте,
                     * враги (в том числе убийца) и мешки, которые уже пришли
                     * в движение. Так устроено и в оригинале (end_tb), и в
                     * порте на БК (init_level_state).
                     *
                     * Стирать надо ДО man_init: он уводит Диггера на старт, а
                     * надгробие осталось там, где тот погиб.
                     */
                    cp_x = man.x;
                    cp_y = man.y;
                    cp_w = SPRITE_W_SUB;
                    cp_h = SPRITE_H;
                    clear_px();
                    missile_clear();
                    bugs_respawn();     /* запас врагов на уровень сохраняется */
                    bags_respawn();
                    bags_redraw_all();  /* вернуть задетых стиранием */
                    man_init();
                    music_select(0);    /* вернуть фоновую музыку */
                    hud_score();
                    state = ST_PLAY;
                } else {
                    game_lives = 0;
                    hud_lives(0);
                    menu_submit_score();
                    text_string(14, 100, "GAME OVER");
                    state = ST_OVER;
                }
            }
        }

#ifdef DEMO_PROFILE
        prof_mark(0);                   /* конец такта */
#endif
#ifdef DIGGER_DEMO
        demo_telemetry();
#endif
    }
}
