/*
 * hw.h - железо ATARI 65XE/130XE и геометрия экрана для Digger.
 *
 * Регистры берутся из cc65 <atari.h> (структуры ANTIC/GTIA_WRITE/POKEY_WRITE),
 * здесь только то, что удобнее иметь константами, и вся геометрия поля.
 *
 * Геометрия - точное уменьшение оригинала для PC вдвое по горизонтали и
 * один к одному по вертикали (пиксель CGA выше своей ширины ровно во столько
 * же раз, во сколько пиксель ANTIC E шире своей высоты):
 *
 *              оригинал PC      Atari
 *   экран      320 x 200        160 x 208
 *   клетка     20 x 18 px       10 x 18 px
 *   спрайт     16 x 15          8 x 15  (2 байта)
 *   подшаг X   4 px (5/клетку)  2 px (5/клетку)
 *   подшаг Y   3 px (6/клетку)  3 строки (6/клетку)
 *   поле       300 x 180 px     150 x 180 px
 *
 * Запас 2 px по бокам и 3 строки по вертикали между спрайтом и клеткой - это
 * разделители земли между клетками, которых лишился порт на БК.
 */
#ifndef HW_H
#define HW_H

#include <stdint.h>

/* ---------------------------------------------------------------- экран */

/*
 * ANTIC E (DL_MAP160x1x4), НОРМАЛЬНОЕ поле: 40 байт = 160 пикселей.
 * Широкое поле (48 байт) не используется: измерено на M0, что видны только
 * байты 3..44 - по 3 байта с каждого края уходит за пределы картинки.
 */
#define SCR_STRIDE      40u             /* байт в строке растра            */
#define SCR_LINES       208u            /* строк растра                    */
#define SCR_WIDTH_PX    160u            /* пикселей по горизонтали         */
#define SCR_PX_PER_BYTE 4u              /* 2 бита на пиксель               */

/* Прогоны буфера между LMS (ни один не пересекает границу 4096) */
#define SCR_RUN0_LINES  102u            /* строки   0..101 -> $9010        */
#define SCR_RUN1_LINES  102u            /* строки 102..203 -> $A000        */
#define SCR_RUN2_LINES  4u              /* строки 204..207 -> $B000        */

/* --------------------------------------------------------------- поле */

#define W_MAX           15u             /* клеток по горизонтали           */
#define H_MAX           10u             /* клеток по вертикали             */

#define CELL_W_PX       10u
#define CELL_H_PX       18u

#define SPRITE_W_PX     8u              /* 2 байта                         */
#define SPRITE_W_BYTES  2u
#define SPRITE_H        15u

/*
 * x_graph считается в ПОДШАГАХ по 2 пикселя (полбайта), y_graph - в строках
 * растра. Отсюда: байт экрана = x_graph >> 1, фаза сдвига = x_graph & 1.
 * Спрайт хранится в двух предсдвинутых фазах: фаза 0 - 2 байта, фаза 1 - 3.
 */
#define SUBSTEP_X_PX    2u              /* пикселей в подшаге по X         */
#define SUBSTEP_Y_LINES 3u              /* строк растра в подшаге по Y     */

#define POS_X_STEP      5u              /* подшагов в клетке по X          */
#define POS_Y_STEP      6u              /* подшагов в клетке по Y          */
#define MOVE_X_STEP     1u
#define MOVE_Y_STEP     SUBSTEP_Y_LINES /* шаг по Y сразу в строках растра */

/* HUD занимает строки 0..27, поле - строки 28..207 */
#define HUD_LINES       28u
#define FIELD_Y_OFFSET  HUD_LINES
#define FIELD_H_LINES   (H_MAX * CELL_H_PX)             /* 180 */

/* Поле 150 px из 160: начинается с пикселя 4 = байт 1 = подшаг 2 */
#define FIELD_X0_PX     4u
#define FIELD_X_OFFSET  (FIELD_X0_PX / SUBSTEP_X_PX)    /* 2 подшага */
#define FIELD_W_PX      (W_MAX * CELL_W_PX)             /* 150 */

/* --------------------------------------------------------------- цвета */

/* Байт цвета Atari: оттенок в битах 4-7, яркость в битах 1-3 */
#define ATARI_COLOR(hue, lum)   (uint8_t)(((hue) << 4) | ((lum) & 0x0Eu))

/*
 * Палитра поля. Индекс пикселя -> регистр:
 *   00 -> COLBK, 01 -> COLPF0, 10 -> COLPF1, 11 -> COLPF2
 *
 * Взята из оригинала: он работал в режиме 4 CGA, и в Work/DIGGER.H перечислена
 * его палитра - чёрный, зелёный, красный, жёлтый.
 *
 * Яркость поднята до предельной: на БК цвета были полностью насыщенными,
 * и приглушённые оттенки CGA рядом с ними выглядят блёкло.
 *
 * Вторую и третью краски надо подбирать ПО СПРАЙТАМ, а не по виду уровня:
 * на уровне преобладает земля, и по нему не заметно, что краски слились.
 * Спрайты используют все три сразу - именно на них видно, различимы ли они.
 * Контрольный лист build/sprites_sheet.png рисуется теми же цветами.
 */
#define COL_BACK        ATARI_COLOR(0x0, 0)     /* чёрный - прокопанный ход */
#define COL_PF0         ATARI_COLOR(0xC, 8)     /* зелёный                  */
#define COL_PF1         ATARI_COLOR(0x3, 8)     /* красный                  */
#define COL_PF2         ATARI_COLOR(0x1, 10)    /* жёлтый                   */

/* -------------------------------------------------------- байты ANTIC */

#define DL_BLANK8       0x70            /* 8 пустых строк растра           */
#define DL_MODE_E       0x0E            /* 160 x 1 x 4 цвета               */
#define DL_LMS          0x40            /* + 2 байта адреса                */
#define DL_JVB          0x41            /* переход с ожиданием кадра       */
#define DL_DLI          0x80

/* DMACTL / SDMCTL: выборка дисплей-листа + нормальное поле, PMG выключен */
#define SDMCTL_GAME     0x22

/* --------------------------------------------------- теневые ячейки ОС */

#define OS_SDMCTL       (*(volatile uint8_t *)0x022Fu)
#define OS_SDLSTL       (*(volatile uint16_t *)0x0230u)
#define OS_COLOR0       (*(volatile uint8_t *)0x02C4u)  /* -> COLPF0 */
#define OS_COLOR1       (*(volatile uint8_t *)0x02C5u)  /* -> COLPF1 */
#define OS_COLOR2       (*(volatile uint8_t *)0x02C6u)  /* -> COLPF2 */
#define OS_COLOR3       (*(volatile uint8_t *)0x02C7u)  /* -> COLPF3 */
#define OS_COLOR4       (*(volatile uint8_t *)0x02C8u)  /* -> COLBK  */
#define OS_RTCLOK2      (*(volatile uint8_t *)0x0014u)  /* счётчик кадров ОС */
#define OS_STICK0       (*(volatile uint8_t *)0x0278u)
#define OS_STRIG0       (*(volatile uint8_t *)0x0284u)
#define OS_CH           (*(volatile uint8_t *)0x02FCu)

/* --------------------------------------------------------- регистры */

#define ANTIC_VCOUNT    (*(volatile uint8_t *)0xD40Bu)
#define ANTIC_WSYNC     (*(volatile uint8_t *)0xD40Au)
#define GTIA_COLBK      (*(volatile uint8_t *)0xD01Au)
#define GTIA_CONSOL     (*(volatile uint8_t *)0xD01Fu)

#endif /* HW_H */
