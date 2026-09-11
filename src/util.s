; util.s - ожидания по растру.
;
; Эти циклы обязаны быть на ассемблере: в cc65 квалификатор volatile почти
; не влияет на оптимизатор, и цикл вида "while (*(char*)0xD40B < n);"
; сворачивается в одно чтение регистра, после чего программа зависает.

        .export _wait_frame, _set_colbk
        .export _clear_hi_bss, _copy_levels
        .import __BSS_HI_RUN__, __BSS_HI_SIZE__
        .import __LEVELS_LOAD__, __LEVELS_RUN__, __LEVELS_SIZE__
        .importzp ptr1, ptr2

VCOUNT  = $D40B                 ; счётчик строк ANTIC, считает строки парами
COLBK   = $D01A                 ; цвет фона GTIA
RTCLOK2 = $14                   ; младший байт счётчика кадров ОС

        .segment "CODE"

; void clear_hi_bss(void) - обнулить сегмент BSS_HI.
;
; Туда уехала статика половины модулей: MAIN забит под завязку, а под ПЗУ по
; $C000 простаивало три с лишним килобайта. Стартовый код cc65 этот сегмент не
; чистит - он знает только про BSS, - а перенести туда весь BSS нельзя:
; обнуление в библиотеке идёт РАНЬШЕ выключения ПЗУ, и запись по $C000 уходит
; в ПЗУ впустую. Игра тогда не запускается совсем.
;
; На ассемблере, а не на Си, потому что символы линковщика __BSS_HI_RUN__ и
; __BSS_HI_SIZE__ из Си не видны: cc65 приписывает именам подчёркивание, и
; extern-объявление ищет ___BSS_HI_RUN__.
_clear_hi_bss:
        lda     #<__BSS_HI_RUN__
        sta     ptr1
        lda     #>__BSS_HI_RUN__
        sta     ptr1+1
        lda     #0
        ldx     #>__BSS_HI_SIZE__       ; сначала целые страницы
        beq     chb_tail
chb_pg: ldy     #0
chb_p1: sta     (ptr1),y
        iny
        bne     chb_p1
        inc     ptr1+1
        dex
        bne     chb_pg
chb_tail:
        ldy     #<__BSS_HI_SIZE__       ; и остаток
        beq     chb_end
chb_t1: dey
        sta     (ptr1),y
        bne     chb_t1
chb_end:
        rts

; void copy_levels(void) - перенести карты уровней под ПЗУ.
;
; Грузятся они в кадровый буфер (LVLSTAGE в cfg/digger-xl.cfg): в момент
; загрузки тот ещё пуст, а в MAIN места уже нет. Звать ОБЯЗАТЕЛЬНО до
; screen_init, иначе буфер зальют землёй поверх карт.
;
; Меньше страницы не бывает и больше трёх не будет, поэтому цикл простой:
; целые страницы, потом остаток.
_copy_levels:
        lda     #<__LEVELS_LOAD__
        sta     ptr1
        lda     #>__LEVELS_LOAD__
        sta     ptr1+1
        lda     #<__LEVELS_RUN__
        sta     ptr2
        lda     #>__LEVELS_RUN__
        sta     ptr2+1

        ldy     #0
        ldx     #>__LEVELS_SIZE__       ; целые страницы
        beq     cl_tail
cl_pg:  lda     (ptr1),y
        sta     (ptr2),y
        iny
        bne     cl_pg
        inc     ptr1+1
        inc     ptr2+1
        dex
        bne     cl_pg
cl_tail:
        ldy     #0
        cpy     #<__LEVELS_SIZE__
        beq     cl_end
cl_t1:  lda     (ptr1),y
        sta     (ptr2),y
        iny
        cpy     #<__LEVELS_SIZE__
        bne     cl_t1
cl_end: rts

; void wait_frame(void) - ждать смены кадра
_wait_frame:
        lda     RTCLOK2
wf_l:   cmp     RTCLOK2
        beq     wf_l
        rts

; void __fastcall__ set_colbk(unsigned char c)
;
; Запись цвета фона обязана идти отсюда, а не из Си: cc65 схлопывает
; несколько подряд идущих записей по одному и тому же адресу, оставляя
; только последнюю (volatile на оптимизатор почти не влияет). Из-за этого
; растровые полосы измерителя пропадали целиком.
_set_colbk:
        sta     COLBK
        rts
