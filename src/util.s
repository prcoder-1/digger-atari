; util.s - ожидания по растру.
;
; Эти циклы обязаны быть на ассемблере: в cc65 квалификатор volatile почти
; не влияет на оптимизатор, и цикл вида "while (*(char*)0xD40B < n);"
; сворачивается в одно чтение регистра, после чего программа зависает.

        .export _wait_frame, _wait_vcount, _set_colbk

VCOUNT  = $D40B                 ; счётчик строк ANTIC, считает строки парами
COLBK   = $D01A                 ; цвет фона GTIA
RTCLOK2 = $14                   ; младший байт счётчика кадров ОС

        .segment "ZEROPAGE"
wv_val: .res 1

        .segment "CODE"

; void wait_frame(void) - ждать смены кадра
_wait_frame:
        lda     RTCLOK2
wf_l:   cmp     RTCLOK2
        beq     wf_l
        rts

; void __fastcall__ wait_vcount(unsigned char v)
;
; Ждёт, пока луч не дойдёт до строки растра 2*v. Сначала дожидается перехода
; счётчика через ноль: после wait_frame() луч находится в кадровом гасящем
; импульсе, где VCOUNT уже больше любого разумного порога, и без этого шага
; ожидание завершалось бы мгновенно.
_wait_vcount:
        sta     wv_val
wv_hi:  lda     VCOUNT
        cmp     wv_val
        bcs     wv_hi                   ; пока VCOUNT >= v
wv_lo:  lda     VCOUNT
        cmp     wv_val
        bcc     wv_lo                   ; пока VCOUNT < v
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
