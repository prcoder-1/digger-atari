/*
 * screen.c - инициализация дисплей-листа и таблиц адресов строк.
 *
 * Режим - ANTIC F: 40 байт на строку по одному биту на точку, 320 точек.
 */
#include <string.h>
#include "screen.h"

uint8_t screen_rowlo[SCR_LINES];
uint8_t screen_rowhi[SCR_LINES];

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

    /* 24 пустые строки растра до начала картинки */
    *d++ = DL_BLANK8;
    *d++ = DL_BLANK8;
    *d++ = DL_BLANK8;

    d = emit_lms(d, (uint16_t)(screen_buf + RUN0_OFS));
    d = emit_lines(d, SCR_RUN0_LINES - 1);

    d = emit_lms(d, (uint16_t)(screen_buf + RUN1_OFS));
    d = emit_lines(d, SCR_RUN1_LINES - 1);

    d = emit_lms(d, (uint16_t)(screen_buf + RUN2_OFS));
    d = emit_lines(d, SCR_RUN2_LINES - 1);

    *d++ = DL_JVB;
    *d++ = (uint8_t)(uint16_t)dlist;
    *d++ = (uint8_t)((uint16_t)dlist >> 8);
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

    OS_COLOR4 = COL_BACK;
    OS_COLOR0 = COL_PF0;
    OS_COLOR1 = COL_PF1;
    OS_COLOR2 = COL_PF2;

    OS_SDLSTL = (uint16_t)dlist;
    OS_SDMCTL = SDMCTL_GAME;
    wait_frame();
}

void screen_done(void)
{
    OS_SDMCTL = 0;
    wait_frame();
    OS_SDLSTL = saved_sdlstl;
    OS_SDMCTL = saved_sdmctl;
    wait_frame();
}
