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

extern const uint8_t spr_z1_p0[];
extern uint8_t score[3];
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

#define DYING_TICKS  12u
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
    dir_t want, kd;

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

    menu_init();
#ifndef DIGGER_DEMO
    /* В демо-сборке заставку показывать нельзя: она ждёт кнопку, а её
     * там некому нажать, и автопроверка зависла бы. */
    menu_show();
#endif

    /*
     * Уровень готовится ПОСЛЕ заставки: иначе поле успевало нарисоваться
     * и тут же затиралось экраном заставки - выглядело так, будто игра
     * стартовала и сразу выбросила на таблицу рекордов.
     */
    /* Экран чистится целиком: field_init заливает землёй только полосу
     * поля, и надписи заставки оставались бы в зоне HUD. */
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
#ifdef DIGGER_DEMO
            if (want == DIR_STOP)
                missile_fire();
#else
            if (vbi_trig == 0)
                missile_fire();
#endif
            man_tick(want);
            bags_tick();
            bugs_tick();
            missile_tick();
            sfx_tick();

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
                bugs_init(screen, game_level > 10u ? 10u : game_level);
                bonus_state = BONUS_OFF;
                missile_init(game_level > 10u ? 10u : game_level);
                hud_score();
                continue;
            }

            /* Вишенка появляется, когда рождать больше некого */
            if (bonus_state == BONUS_OFF && bugs_left_to_spawn() == 0u) {
                bonus_state = BONUS_READY;
                bl_x = (uint8_t)((FIELD_X0_PX + (W_MAX - 1u) * CELL_W_PX) >> 2);
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

            hit = bugs_hit_man();
            if (hit != BUG_NO_HIT) {
                if (bonus_state == BONUS_ON) {
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
                    bugs_remove(hit);
#else
                    state = ST_DYING;
                    dying = DYING_TICKS;
                    sfx_play(950u, SFX_ROUGH, 14u, DYING_TICKS);  /* гибель */
#endif
                }
            }
        } else if (state == ST_DYING) {
            /* Пауза на гибель: поле замирает, но такт продолжает идти */
            if (--dying == 0) {
                if (game_lives > 1u) {
                    --game_lives;
                    hud_lives(game_lives);
                    man_init();
                    bugs_respawn();     /* запас врагов на уровень сохраняется */
                    field_redraw(0);    /* без сброса камней и ходов */
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
