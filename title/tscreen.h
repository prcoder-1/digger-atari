/*
 * tscreen.h - экран заставки: обложка (ANTIC E) и демо (ANTIC F).
 */
#ifndef TSCREEN_H
#define TSCREEN_H

#define SCR_BYTES     40u       /* байт в строке ANTIC F, 320 точек */
#define SCR_ROWS      192u
#define COVER_BYTES   40u       /* байт в строке ANTIC E, 160 точек */

/* Цвета обложки - те же четыре, что на БК */
#define COL_COVER_BG     0x00
#define COL_COVER_RED    0x3A
#define COL_COVER_GREEN  0xCA
#define COL_COVER_BLUE   0x8A

/* Цвета демо: фон тёмный, рисунок светлый той же гаммы */
#define COL_DEMO_BG      0x00
#define COL_DEMO_PF2     0xC0   /* фон чёрный, оттенок рисунка зелёный */
#define COL_DEMO_PF1     0x0C   /* яркость рисунка */

extern unsigned char screen_buf[];

void screen_save(void);
void screen_restore(void);
void screen_cover(void);
void screen_demo(void);
void screen_clear(void);

#endif /* TSCREEN_H */
