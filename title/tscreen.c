/*
 * tscreen.c - экран заставки.
 *
 * Два разных экрана на одном месте:
 *
 *      ОБЛОЖКА - ANTIC E, 160 точек в строке, четыре цвета. Ближайший к
 *                цветному режиму БК, в котором нарисован оригинал картинки.
 *      ДЕМО    - ANTIC F, 320 точек, один бит на точку. Тот же режим, что у
 *                самой игры, и это не совпадение: в демо выводятся спрайты
 *                игры и её шрифт, а они нарисованы под 320 точек.
 *
 * Память под оба - одна и та же ($9000). Обложка грузится туда куском файла,
 * демо потом рисует поверх неё: одновременно они не нужны.
 *
 * Дисплей-лист в обоих случаях перезагружает адрес экрана КАЖДОЙ строкой.
 * Картинка в 7680 байт пересекает границу 4 КБ, за которой ANTIC сам адрес не
 * продолжает, а раскладывать буфер на три прогона, как в игре (см.
 * src/screen.c), здесь незачем: заставка не считает такты.
 */
#include "tscreen.h"

/* Теневые ячейки ОС: пока ПЗУ включено, регистры каждый кадр переписывает
 * кадровое прерывание, и ставить надо именно тени. */
#define SDMCTL  (*(volatile unsigned char *)0x022F)
#define SDLSTL  (*(volatile unsigned char *)0x0230)
#define SDLSTH  (*(volatile unsigned char *)0x0231)
#define COLOR0  (*(volatile unsigned char *)0x02C4)
#define COLOR1  (*(volatile unsigned char *)0x02C5)
#define COLOR2  (*(volatile unsigned char *)0x02C6)
#define COLOR3  (*(volatile unsigned char *)0x02C7)
#define COLOR4  (*(volatile unsigned char *)0x02C8)

/* Команды ANTIC */
#define DL_BLANK8   0x70
#define DL_MODE_E   0x0E
#define DL_MODE_F   0x0F
#define DL_LMS      0x40
#define DL_JVB      0x41

#pragma bss-name (push, "DLIST")
static unsigned char dlist[1024];
#pragma bss-name (pop)

#pragma bss-name (push, "SCRBUF")
unsigned char screen_buf[SCR_BYTES * SCR_ROWS];
#pragma bss-name (pop)

/*
 * Таблицы адресов строк. Имена не произвольные: по ним их ищет блиттер игры
 * (src/blit.s), который заставка использует как есть.
 */
unsigned char screen_rowlo[SCR_ROWS];
unsigned char screen_rowhi[SCR_ROWS];

static unsigned char old_dmactl, old_dlistl, old_dlisth;

/* Строит дисплей-лист: mode строк по bpr байт, адрес в каждой строке */
static void build_dlist(unsigned char mode, unsigned int base, unsigned char bpr)
{
    unsigned char *d = dlist;
    unsigned int src = base;
    unsigned int i;

    *d++ = DL_BLANK8;
    *d++ = DL_BLANK8;
    *d++ = DL_BLANK8;

    for (i = 0; i < SCR_ROWS; ++i) {
        *d++ = (unsigned char)(mode | DL_LMS);
        *d++ = (unsigned char)src;
        *d++ = (unsigned char)(src >> 8);
        src += bpr;
    }

    *d++ = DL_JVB;
    *d++ = (unsigned char)(unsigned int)dlist;
    *d++ = (unsigned char)(((unsigned int)dlist) >> 8);
}

static void show_dlist(void)
{
    SDLSTL = (unsigned char)(unsigned int)dlist;
    SDLSTH = (unsigned char)(((unsigned int)dlist) >> 8);
    SDMCTL = 0x22;                      /* поле обычной ширины, DMA включён */
}

void screen_save(void)
{
    old_dmactl = SDMCTL;
    old_dlistl = SDLSTL;
    old_dlisth = SDLSTH;
}

void screen_restore(void)
{
    SDMCTL = old_dmactl;
    SDLSTL = old_dlistl;
    SDLSTH = old_dlisth;
}

void screen_cover(void)
{
    build_dlist(DL_MODE_E, (unsigned int)screen_buf, COVER_BYTES);

    COLOR4 = COL_COVER_BG;              /* точка 0 - фон */
    COLOR0 = COL_COVER_RED;
    COLOR1 = COL_COVER_GREEN;
    COLOR2 = COL_COVER_BLUE;
    COLOR3 = COL_COVER_BG;

    show_dlist();
}

void screen_demo(void)
{
    unsigned int a;
    unsigned int i;

    /* Таблицы строк: буфер сплошной, поэтому просто шаг в 40 байт */
    a = (unsigned int)screen_buf;
    for (i = 0; i < SCR_ROWS; ++i) {
        screen_rowlo[i] = (unsigned char)a;
        screen_rowhi[i] = (unsigned char)(a >> 8);
        a += SCR_BYTES;
    }

    screen_clear();
    build_dlist(DL_MODE_F, (unsigned int)screen_buf, SCR_BYTES);

    /*
     * В ANTIC F оттенок И фона, и рисунка задаёт COLPF2, а яркость рисунка
     * приходит из COLPF1 - правило измерено в doc/graphics.md. Поэтому фон
     * тёмный, а надписи светлые той же гаммы.
     */
    COLOR4 = COL_DEMO_BG;
    COLOR2 = COL_DEMO_PF2;
    COLOR1 = COL_DEMO_PF1;

    show_dlist();
}

void screen_clear(void)
{
    unsigned int i;
    for (i = 0; i < (unsigned int)(SCR_BYTES * SCR_ROWS); ++i)
        screen_buf[i] = 0;
}
