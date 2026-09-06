/*
 * screen.c - инициализация дисплей-листа и таблиц адресов строк.
 *
 * Режим - ANTIC F: 40 байт на строку по одному биту на точку, 320 точек.
 */
#include <string.h>
#include "screen.h"
#include "dli.h"

/*
 * Статические данные модуля уезжают в ОЗУ под ПЗУ: MAIN забит под завязку.
 * Обнуляет этот сегмент clear_hi_bss() первым делом в main - стартовый код
 * cc65 сюда не дотягивается, он чистит только сегмент BSS.
 */
#pragma bss-name (push, "BSS_HI")


/* Сами таблицы лежат в отдельном сегменте, см. src/scrmem.s */
extern uint8_t screen_rowlo[SCR_LINES];
extern uint8_t screen_rowhi[SCR_LINES];

/* Сохранённое состояние экрана ОС, чтобы вернуть его при выходе */
static uint8_t   saved_sdmctl;
static uint16_t  saved_sdlstl;

/*
 * Начала трёх прогонов буфера. Смещение $10 у первого нужно потому, что
 * 4096 mod 40 = 16: прогон из 102 строк по 40 байт (4080) от +$10 упирается
 * ровно в границу 4К и не пересекает её.
 */
#define RUN0_OFS  0x0010u
#define RUN1_OFS  0x1000u
#define RUN2_OFS  0x2000u

/* Размер выделенного блока, см. scrmem.s и cfg/digger.cfg */
#define SCRBUF_BYTES  0x20A0u

/* Ждёт смены кадра по счётчику ОС */
static void wait_frame(void)
{
    uint8_t t = OS_RTCLOK2;
    while (OS_RTCLOK2 == t)
        ;
}

/* Заполняет screen_rowlo/hi адресами начал строк растра */
static void build_row_tables(void)
{
    uint8_t  *p;
    uint16_t  a;
    uint8_t   y;

    p = screen_buf + RUN0_OFS;
    for (y = 0; y < SCR_RUN0_LINES; ++y) {
        a = (uint16_t)p;
        screen_rowlo[y] = (uint8_t)a;
        screen_rowhi[y] = (uint8_t)(a >> 8);
        p += SCR_STRIDE;
    }

    p = screen_buf + RUN1_OFS;
    for (; y < SCR_RUN0_LINES + SCR_RUN1_LINES; ++y) {
        a = (uint16_t)p;
        screen_rowlo[y] = (uint8_t)a;
        screen_rowhi[y] = (uint8_t)(a >> 8);
        p += SCR_STRIDE;
    }

    p = screen_buf + RUN2_OFS;
    for (; y < SCR_LINES; ++y) {
        a = (uint16_t)p;
        screen_rowlo[y] = (uint8_t)a;
        screen_rowhi[y] = (uint8_t)(a >> 8);
        p += SCR_STRIDE;
    }
}

/* Дописывает LMS-инструкцию с адресом; возвращает новую позицию */
static uint8_t *emit_lms(uint8_t *d, uint16_t addr)
{
    *d++ = DL_MODE_F | DL_LMS;
    *d++ = (uint8_t)addr;
    *d++ = (uint8_t)(addr >> 8);
    return d;
}

/* Дописывает n обычных строк режима F */
static uint8_t *emit_lines(uint8_t *d, uint8_t n)
{
    while (n--)
        *d++ = DL_MODE_F;
    return d;
}

static void build_dlist(void)
{
    uint8_t *d = dlist;

    /*
     * 24 пустые строки растра до начала картинки, но ЧЕТЫРЬМЯ командами, а не
     * тремя: 8 + 8 + 7 + 1. Последняя - управляемая, её длину меняет тряска
     * поля при ударе мешка (screen_shake в src/vbi.s). Команда пустых строк
     * задаёт от одной до восьми, поэтому одним байтом картинка опускается на
     * 0..7 строк, и перестраивать список не нужно.
     */
    *d++ = DL_BLANK8;
    *d++ = DL_BLANK8;
    *d++ = DL_BLANK7;
    *d++ = DL_BLANK1;

    /*
     * Бит DLI стоит НЕ на последней строке панели, а на DLI_LEAD строк выше.
     * Строка режима F - одна строка растра, а ANTIC успевает украсть из неё
     * больше трети тактов на выборку картинки, поэтому отклик на прерывание
     * вместе с диспетчером ПЗУ перехлёстывает за конец своей строки, и WSYNC
     * внутри обработчика встаёт уже на следующую. Измерено: с битом на строке
     * перед границей краски поля ложились строкой ниже, то есть верхняя строка
     * поля оставалась в цветах панели.
     */
    d = emit_lms(d, (uint16_t)(screen_buf + RUN0_OFS));  /* строка 0       */
    d = emit_lines(d, HUD_BAND_LINES - DLI_LEAD - 1);    /* строки 1..13   */
    *d++ = DL_MODE_F | DL_DLI;                           /* строка 14      */
    d = emit_lines(d, SCR_RUN0_LINES - HUD_BAND_LINES + DLI_LEAD - 1);
                                                         /* строки 15..101 */

    d = emit_lms(d, (uint16_t)(screen_buf + RUN1_OFS));
    d = emit_lines(d, SCR_RUN1_LINES - 1);

    d = emit_lms(d, (uint16_t)(screen_buf + RUN2_OFS));
    d = emit_lines(d, SCR_RUN2_LINES - 1);

    *d++ = DL_JVB;
    *d++ = (uint8_t)(uint16_t)dlist;
    *d++ = (uint8_t)((uint16_t)dlist >> 8);

    /*
     * Сетка прерываний по полю: горизонтальная позиция игроков-врагов
     * перезагружается каждые DLI_GRID строк. Биты ставятся ЗДЕСЬ, один раз на
     * всю игру, и больше не трогаются: от этого число прерываний в кадре
     * постоянно, а вместе с ним не может сбиться и порядковый счётчик слотов
     * (src/dli.s). Подвижные биты, по одному на врага, этим свойством не
     * обладали и давали призраков - см. шапку src/pmgbug.c.
     */
    {
        uint8_t i;
        for (i = 0; i < DLI_GRID_N; ++i)
            *screen_dlist_line((uint8_t)(FIELD_Y_OFFSET + i * DLI_GRID
                                         - DLI_LEAD)) |= DL_DLI;
    }
}

/*
 * Байт режима строки растра в дисплей-листе - туда ставится бит DLI.
 *
 * Раскладка задана build_dlist() выше и повторена здесь числами, потому что
 * прогоны начинаются с трёхбайтовой LMS и сдвигают индекс:
 *
 *      строка 0        -> 3            (LMS первого прогона)
 *      строки 1..101   -> 5 + n
 *      строка 102      -> 107          (LMS второго прогона)
 *      строки 103..203 -> 7 + n
 *      строка 204      -> 211          (LMS третьего прогона)
 *      строки 205..207 -> 9 + n
 *
 * Обе раскладки лежат в одном файле нарочно: разъедься они, бит DLI встал бы
 * посреди адреса LMS и увёл бы картинку в мусор.
 */
uint8_t *screen_dlist_line(uint8_t line)
{
    /* Смещения на единицу больше, чем можно ожидать: пустых строк в начале
     * четыре команды, а не три (см. build_dlist). */
    if (line == 0)
        return dlist + 4;
    if (line < SCR_RUN0_LINES)
        return dlist + 6 + line;
    if (line == SCR_RUN0_LINES)
        return dlist + 108;
    if (line < SCR_RUN0_LINES + SCR_RUN1_LINES)
        return dlist + 8 + line;
    if (line == SCR_RUN0_LINES + SCR_RUN1_LINES)
        return dlist + 212;
    return dlist + 10 + line;
}

void screen_clear(uint8_t value)
{
    /* Дырка между прогонами тоже затирается - она невидима, это безвредно */
    memset(screen_buf, value, SCRBUF_BYTES);
}

void screen_init(void)
{
    saved_sdmctl = OS_SDMCTL;
    saved_sdlstl = OS_SDLSTL;

    /* Гасим ANTIC, пока перестраиваем дисплей-лист */
    OS_SDMCTL = 0;
    wait_frame();

    build_row_tables();
    build_dlist();
    screen_clear(0);

    /*
     * В тенях ОС - краски ПАНЕЛИ: вторая стадия кадрового прерывания
     * переписывает их в железо, то есть сама возвращает панельную палитру к
     * строке 0. Краски поля кладёт DLI на границе строк 15/16.
     */
    OS_COLOR4 = COL_BACK;
    OS_COLOR0 = COL_PF0;
    OS_COLOR1 = COL_HUD_PF1;
    OS_COLOR2 = COL_HUD_PF2;
    /* Краска пятого игрока: ею выводится вишенка. Полосы палитры
     * её не трогают - в режиме высокого разрешения COLPF3 в выводе
     * поля не участвует. */
    OS_COLOR3 = COL_CHERRY;

    dli_pf1 = COL_FIELD_PF1;
    dli_pf2 = COL_FIELD_PF2;
    dli_install();

    OS_SDLSTL = (uint16_t)dlist;
    OS_SDMCTL = SDMCTL_GAME;
    wait_frame();
}

void screen_done(void)
{
    OS_SDMCTL = 0;
    wait_frame();
    dli_remove();
    OS_SDLSTL = saved_sdlstl;
    OS_SDMCTL = saved_sdmctl;
    wait_frame();
}

#pragma bss-name (pop)
