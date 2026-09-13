/*
 * title.c - заставка. Порт title.c из версии для БК 0010.
 *
 * Заставка идёт ДО игры и памяти у неё не отнимает: её кусок файла лежит
 * первым, запускается вектором INITAD, а по возвращении загрузчик DOS кладёт
 * игру поверх. Разбор устройства - в cfg/title.cfg и title/crt.s.
 *
 * Стадии, как на БК:
 *
 *      1. ОБЛОЖКА - картинка в четырёх цветах, ждёт нажатия;
 *      2. ДЕМО    - таблица с ходячими персонажами и призами, слева панель
 *                   кредитов и клавиш; нажатие запускает игру.
 *
 * Обложка выводится режимом ANTIC E (160 точек, четыре цвета) - это ближайший
 * к цветному режиму БК. Демо идёт в ANTIC F, том же, что и сама игра: там
 * нужны спрайты игры и её шрифт, а они нарисованы для 320 точек.
 */
#include <stdint.h>

/* Аппаратура и теневые ячейки ОС. Пока ПЗУ ОС включено (а в заставке оно
 * включено), цвета и дисплей-лист надо ставить через ТЕНЕВЫЕ ячейки: кадровое
 * прерывание ОС переписывает ими регистры каждый кадр. */
#define SDMCTL  (*(volatile uint8_t *)0x022F)   /* тень DMACTL   */
#define SDLSTL  (*(volatile uint8_t *)0x0230)   /* тень DLISTL   */
#define SDLSTH  (*(volatile uint8_t *)0x0231)
#define COLOR0  (*(volatile uint8_t *)0x02C4)   /* тень COLPF0   */
#define COLOR1  (*(volatile uint8_t *)0x02C5)
#define COLOR2  (*(volatile uint8_t *)0x02C6)
#define COLOR3  (*(volatile uint8_t *)0x02C7)
#define COLOR4  (*(volatile uint8_t *)0x02C8)   /* тень COLBK    */
#define CH      (*(volatile uint8_t *)0x02FC)   /* код нажатой клавиши */
#define STRIG0  (*(volatile uint8_t *)0x0284)   /* кнопка джойстика    */
#define RTCLOK2 (*(volatile uint8_t *)0x0014)   /* счётчик кадров ОС   */

#define KEY_NONE 0xFFu

/* Дисплей-лист заставки лежит в своей области, см. cfg/title.cfg */
extern uint8_t dlist[];
#pragma bss-name (push, "DLIST")
uint8_t dlist[512];
#pragma bss-name (pop)

/* Обложка грузится куском файла прямо в экранную память */
extern const uint8_t cover[];

/* Команды ANTIC */
#define DL_BLANK8   0x70        /* восемь пустых строк   */
#define DL_MODE_E   0x0E        /* 160 точек, 4 цвета    */
#define DL_LMS      0x40        /* + адрес следом        */
#define DL_JVB      0x41        /* ждать кадра и прыгнуть */

/* Цвета обложки - те же четыре, что на БК */
#define COL_BLACK   0x00
#define COL_RED     0x3A
#define COL_GREEN   0xCA
#define COL_BLUE    0x8A

#define COVER_ROWS  192u
#define COVER_BPR   40u         /* байт в строке ANTIC E */

/* Сохранённое состояние дисплея ОС: заставка обязана вернуть его загрузчику */
static uint8_t old_dmactl, old_dlistl, old_dlisth;

/*
 * Дисплей-лист обложки: адрес экрана перезагружается КАЖДОЙ строкой.
 *
 * Так короче и надёжнее, чем один LMS в начале: картинка занимает 7680 байт и
 * пересекает границу 4 КБ, за которой ANTIC сам адрес не продолжает. Лишних
 * тактов это не стоит ничего заметного - заставка не считает время.
 */
static void build_cover_dlist(void)
{
    uint16_t src = (uint16_t)cover;
    uint16_t i;
    uint8_t *d = dlist;

    *d++ = DL_BLANK8;
    *d++ = DL_BLANK8;
    *d++ = DL_BLANK8;

    for (i = 0; i < COVER_ROWS; ++i) {
        *d++ = DL_MODE_E | DL_LMS;
        *d++ = (uint8_t)src;
        *d++ = (uint8_t)(src >> 8);
        src += COVER_BPR;
    }

    *d++ = DL_JVB;
    *d++ = (uint8_t)(uint16_t)dlist;
    *d++ = (uint8_t)(((uint16_t)dlist) >> 8);
}

/* Нажата ли клавиша или кнопка джойстика */
static uint8_t any_key(void)
{
    if (CH != KEY_NONE) {
        CH = KEY_NONE;                  /* разобрать код, чтобы не залипал */
        return 1;
    }
    return (uint8_t)(STRIG0 == 0);      /* кнопка активна нулём */
}

/* Ждёт смены кадра: весь темп заставки отсчитывается по ним */
static void wait_frame(void)
{
    uint8_t t = RTCLOK2;
    while (RTCLOK2 == t)
        ;
}

static void show_cover(void)
{
    build_cover_dlist();

    COLOR4 = COL_BLACK;                 /* фон, значение точки 0 */
    COLOR0 = COL_RED;
    COLOR1 = COL_GREEN;
    COLOR2 = COL_BLUE;
    COLOR3 = COL_BLACK;

    SDLSTL = (uint8_t)(uint16_t)dlist;
    SDLSTH = (uint8_t)(((uint16_t)dlist) >> 8);
    SDMCTL = 0x22;                      /* поле обычной ширины, DMA включён */

    while (!any_key())
        wait_frame();
}

void title_main(void)
{
    old_dmactl = SDMCTL;
    old_dlistl = SDLSTL;
    old_dlisth = SDLSTH;

    CH = KEY_NONE;                      /* сбросить код, оставшийся от DOS */

    show_cover();

    /*
     * Вернуть дисплей ОС. Пока загрузчик дочитывает игру, кадровое прерывание
     * ОС продолжает работать, а память под нашим дисплей-листом уже занимают
     * куски игры - без возврата на экране был бы мусор.
     */
    SDMCTL = old_dmactl;
    SDLSTL = old_dlistl;
    SDLSTH = old_dlisth;
}
