/*
 * menu.c - заставка, таблица рекордов и конец партии.
 *
 * Таблица живёт только в оперативной памяти: оригинал писал рекорды прямо
 * в сектор дискеты через прерывание BIOS, и на Atari прямого аналога этому
 * нет - понадобился бы файл через CIO, а с ним и обязательная поставка на
 * дискете. Для порта это несоразмерная цена, поэтому рекорды сбрасываются
 * при выключении.
 *
 * Счёт хранится упакованным BCD, поэтому сравнение рекордов - это обычное
 * побайтовое сравнение от старшего разряда к младшему, без всякого перевода
 * в двоичное число.
 */
#include <string.h>
#include "menu.h"
#include "hw.h"
#include "screen.h"
#include "text.h"

#define HS_COUNT  5u

extern uint8_t score[3];
void wait_frame(void);

extern uint8_t vbi_trig;

/* Рекорды, от лучшего к худшему; каждый - три байта упакованного BCD */
static uint8_t hiscore[HS_COUNT][3];

void menu_init(void)
{
    memset(hiscore, 0, sizeof(hiscore));
}

/* Сравнение двух счётов в BCD: 1, если a больше b */
static uint8_t bcd_greater(const uint8_t *a, const uint8_t *b)
{
    uint8_t i;
    for (i = 0; i < 3u; ++i) {
        if (a[i] != b[i])
            return (uint8_t)(a[i] > b[i]);
    }
    return 0;
}

uint8_t menu_submit_score(void)
{
    uint8_t i, j;

    for (i = 0; i < HS_COUNT; ++i) {
        if (!bcd_greater(score, hiscore[i]))
            continue;
        /* Сдвинуть хвост таблицы вниз и вставить новый результат */
        for (j = HS_COUNT - 1u; j > i; --j)
            memcpy(hiscore[j], hiscore[j - 1u], 3);
        memcpy(hiscore[i], score, 3);
        return (uint8_t)(i + 1u);       /* занятое место, 1 - лучшее */
    }
    return 0;
}

void menu_show(void)
{
    uint8_t i;

    screen_clear(0);
    text_init(3);
    text_string(13, 24, "DIGGER");

    text_init(2);
    text_string(11, 56, "HIGH SCORES");
    for (i = 0; i < HS_COUNT; ++i) {
        text_digit(9, (uint8_t)(72 + i * 12u), (uint8_t)(i + 1u));
        text_bcd6(12, (uint8_t)(72 + i * 12u), hiscore[i]);
    }

    text_init(1);
    text_string(9, 148, "PRESS FIRE");
    text_string(6, 168, "JOYSTICK TO DIG");

    /* Ждём отпускания и нового нажатия: иначе кнопка, которой закончили
     * прошлую партию, тут же начала бы следующую. */
    while (vbi_trig == 0)
        wait_frame();
    while (vbi_trig != 0)
        wait_frame();
}
