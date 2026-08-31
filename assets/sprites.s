; sprites.s - СГЕНЕРИРОВАНО tools/spr2atari.py, не править вручную.
; Источник: оригинальные спрайты Digger для PC (Sprites/*.SPR),
; сжатые вдвое по горизонтали под ANTIC E.
;
; Раскладка каждой фазы: по строке идут пары (маска, изображение),
; всего <байт_в_строке> пар на строку, <строк> строк.
; Вывод: экран = (экран И маска) ИЛИ изображение.

; Данные разложены по трём сегментам. Под ПЗУ операционной системы
; свободны два НЕСМЕЖНЫХ куска ОЗУ ($E400-$FFF9 и $D800-$DFFF), между
; ними лежит знакогенератор, поэтому одним сегментом их не занять.
; Имена SHADOW_RAM и SHADOW_RAM2 заданы библиотекой cc65: её процедура
; копирования под ПЗУ ссылается на __SHADOW_RAM_LOAD__ и подобные.
; Что не влезло под ПЗУ, уходит в RODATA, то есть в обычную память.

        .segment "SHADOW_RAM"

; --- 0: 8 x 15 px ---
        .export _spr_0_p0
_spr_0_p0:            ; 2 байт/строку, 15 строк
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00
        .byte $00,$00,$00,$00

        .export _spr_0_p1
_spr_0_p1:            ; 3 байт/строку, 15 строк
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$0F,$00

; --- 1: 8 x 15 px ---
        .export _spr_1_p0
_spr_1_p0:            ; 2 байт/строку, 15 строк
        .byte $F0,$00,$0F,$00
        .byte $F0,$03,$0F,$C0
        .byte $FC,$03,$3F,$C0
        .byte $F0,$03,$0F,$C0
        .byte $C0,$0F,$03,$F0
        .byte $C0,$3F,$03,$FC
        .byte $00,$30,$00,$0C
        .byte $00,$FF,$00,$FF
        .byte $00,$F0,$00,$0F
        .byte $00,$FF,$00,$FF
        .byte $00,$FF,$00,$FF
        .byte $00,$F0,$00,$0F
        .byte $00,$3F,$00,$FC
        .byte $C0,$0F,$03,$F0
        .byte $F0,$00,$0F,$00

        .export _spr_1_p1
_spr_1_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$00,$00,$FF,$00
        .byte $FF,$00,$00,$3C,$FF,$00
        .byte $FF,$00,$C3,$3C,$FF,$00
        .byte $FF,$00,$00,$3C,$FF,$00
        .byte $FC,$00,$00,$FF,$3F,$00
        .byte $FC,$03,$00,$FF,$3F,$C0
        .byte $F0,$03,$00,$00,$0F,$C0
        .byte $F0,$0F,$00,$FF,$0F,$F0
        .byte $F0,$0F,$00,$00,$0F,$F0
        .byte $F0,$0F,$00,$FF,$0F,$F0
        .byte $F0,$0F,$00,$FF,$0F,$F0
        .byte $F0,$0F,$00,$00,$0F,$F0
        .byte $F0,$03,$00,$FF,$0F,$C0
        .byte $FC,$00,$00,$FF,$3F,$00
        .byte $FF,$00,$00,$00,$FF,$00

; --- 15: 4 x 7 px ---
        .export _spr_15_p0
_spr_15_p0:            ; 1 байт/строку, 7 строк
        .byte $00,$3C
        .byte $00,$BB
        .byte $00,$EF
        .byte $00,$FE
        .byte $00,$FF
        .byte $00,$FF
        .byte $00,$F8

        .export _spr_15_p1
_spr_15_p1:            ; 2 байт/строку, 7 строк
        .byte $F0,$03,$0F,$C0
        .byte $F0,$0B,$0F,$B0
        .byte $F0,$0E,$0F,$F0
        .byte $F0,$0F,$0F,$E0
        .byte $F0,$0F,$0F,$F0
        .byte $F0,$0F,$0F,$F0
        .byte $F0,$0F,$0F,$80

; --- a0: 8 x 15 px ---
        .export _spr_a0_p0
_spr_a0_p0:            ; 2 байт/строку, 15 строк
        .byte $F0,$07,$03,$D0
        .byte $C0,$07,$03,$F4
        .byte $C0,$07,$00,$34
        .byte $C0,$17,$00,$35
        .byte $00,$17,$00,$D5
        .byte $00,$55,$00,$15
        .byte $00,$2A,$00,$54
        .byte $C0,$02,$03,$54
        .byte $00,$2A,$03,$50
        .byte $C0,$15,$0F,$50
        .byte $C0,$02,$0F,$80
        .byte $FC,$02,$3F,$80
        .byte $FC,$02,$3F,$80
        .byte $F0,$02,$0F,$80
        .byte $F0,$0A,$0F,$A0

        .export _spr_a0_p1
_spr_a0_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$00,$7D,$3F,$00
        .byte $FC,$00,$00,$7F,$3F,$40
        .byte $FC,$00,$00,$73,$0F,$40
        .byte $FC,$01,$00,$73,$0F,$50
        .byte $F0,$01,$00,$7D,$0F,$50
        .byte $F0,$05,$00,$51,$0F,$50
        .byte $F0,$02,$00,$A5,$0F,$40
        .byte $FC,$00,$00,$25,$3F,$40
        .byte $F0,$02,$00,$A5,$3F,$00
        .byte $FC,$01,$00,$55,$FF,$00
        .byte $FC,$00,$00,$28,$FF,$00
        .byte $FF,$00,$C3,$28,$FF,$00
        .byte $FF,$00,$C3,$28,$FF,$00
        .byte $FF,$00,$00,$28,$FF,$00
        .byte $FF,$00,$00,$AA,$FF,$00

; --- a1: 8 x 15 px ---
        .export _spr_a1_p0
_spr_a1_p0:            ; 2 байт/строку, 15 строк
        .byte $F0,$07,$03,$D0
        .byte $C0,$07,$03,$F4
        .byte $C0,$17,$00,$F4
        .byte $00,$17,$00,$F5
        .byte $00,$57,$00,$D5
        .byte $00,$A5,$00,$15
        .byte $00,$0A,$00,$54
        .byte $F0,$02,$03,$54
        .byte $C0,$0A,$03,$50
        .byte $00,$29,$0F,$50
        .byte $C0,$16,$0F,$80
        .byte $C0,$02,$3F,$80
        .byte $F0,$02,$0F,$80
        .byte $F0,$0A,$0F,$A0
        .byte $F0,$00,$0F,$00

        .export _spr_a1_p1
_spr_a1_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$00,$7D,$3F,$00
        .byte $FC,$00,$00,$7F,$3F,$40
        .byte $FC,$01,$00,$7F,$0F,$40
        .byte $F0,$01,$00,$7F,$0F,$50
        .byte $F0,$05,$00,$7D,$0F,$50
        .byte $F0,$0A,$00,$51,$0F,$50
        .byte $F0,$00,$00,$A5,$0F,$40
        .byte $FF,$00,$00,$25,$3F,$40
        .byte $FC,$00,$00,$A5,$3F,$00
        .byte $F0,$02,$00,$95,$FF,$00
        .byte $FC,$01,$00,$68,$FF,$00
        .byte $FC,$00,$03,$28,$FF,$00
        .byte $FF,$00,$00,$28,$FF,$00
        .byte $FF,$00,$00,$AA,$FF,$00
        .byte $FF,$00,$00,$00,$FF,$00

; --- a2: 8 x 15 px ---
        .export _spr_a2_p0
_spr_a2_p0:            ; 2 байт/строку, 15 строк
        .byte $F0,$07,$03,$D0
        .byte $C0,$07,$03,$F4
        .byte $C0,$17,$00,$F4
        .byte $00,$17,$00,$F5
        .byte $00,$A7,$00,$D5
        .byte $00,$29,$00,$15
        .byte $C0,$0A,$00,$54
        .byte $F0,$02,$03,$54
        .byte $C0,$0A,$03,$50
        .byte $C0,$29,$0F,$50
        .byte $C0,$26,$0F,$80
        .byte $C0,$02,$0F,$80
        .byte $F0,$0A,$0F,$A0
        .byte $F0,$00,$0F,$00
        .byte $FF,$00,$FF,$00

        .export _spr_a2_p1
_spr_a2_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$00,$7D,$3F,$00
        .byte $FC,$00,$00,$7F,$3F,$40
        .byte $FC,$01,$00,$7F,$0F,$40
        .byte $F0,$01,$00,$7F,$0F,$50
        .byte $F0,$0A,$00,$7D,$0F,$50
        .byte $F0,$02,$00,$91,$0F,$50
        .byte $FC,$00,$00,$A5,$0F,$40
        .byte $FF,$00,$00,$25,$3F,$40
        .byte $FC,$00,$00,$A5,$3F,$00
        .byte $FC,$02,$00,$95,$FF,$00
        .byte $FC,$02,$00,$68,$FF,$00
        .byte $FC,$00,$00,$28,$FF,$00
        .byte $FF,$00,$00,$AA,$FF,$00
        .byte $FF,$00,$00,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00

; --- a3: 8 x 14 px ---
        .export _spr_a3_p0
_spr_a3_p0:            ; 2 байт/строку, 14 строк
        .byte $F0,$00,$0F,$00
        .byte $F0,$07,$03,$D0
        .byte $C0,$07,$03,$F4
        .byte $C0,$17,$00,$F4
        .byte $00,$17,$00,$F5
        .byte $00,$A7,$00,$D5
        .byte $00,$29,$00,$15
        .byte $C0,$0A,$00,$54
        .byte $C0,$0A,$03,$54
        .byte $C0,$29,$03,$50
        .byte $00,$A5,$0F,$50
        .byte $C0,$02,$0F,$80
        .byte $F0,$0A,$0F,$A0
        .byte $F0,$00,$0F,$00

        .export _spr_a3_p1
_spr_a3_p1:            ; 3 байт/строку, 14 строк
        .byte $FF,$00,$00,$00,$FF,$00
        .byte $FF,$00,$00,$7D,$3F,$00
        .byte $FC,$00,$00,$7F,$3F,$40
        .byte $FC,$01,$00,$7F,$0F,$40
        .byte $F0,$01,$00,$7F,$0F,$50
        .byte $F0,$0A,$00,$7D,$0F,$50
        .byte $F0,$02,$00,$91,$0F,$50
        .byte $FC,$00,$00,$A5,$0F,$40
        .byte $FC,$00,$00,$A5,$3F,$40
        .byte $FC,$02,$00,$95,$3F,$00
        .byte $F0,$0A,$00,$55,$FF,$00
        .byte $FC,$00,$00,$28,$FF,$00
        .byte $FF,$00,$00,$AA,$FF,$00
        .byte $FF,$00,$00,$00,$FF,$00

; --- b3: 4 x 7 px ---
        .export _spr_b3_p0
_spr_b3_p0:            ; 1 байт/строку, 7 строк
        .byte $3C,$82
        .byte $0C,$82
        .byte $C0,$02
        .byte $C3,$3C
        .byte $C3,$00
        .byte $30,$8A
        .byte $3C,$82

        .export _spr_b3_p1
_spr_b3_p1:            ; 2 байт/строку, 7 строк
        .byte $F3,$08,$CF,$20
        .byte $F0,$08,$CF,$20
        .byte $FC,$00,$0F,$20
        .byte $FC,$03,$3F,$C0
        .byte $FC,$00,$3F,$00
        .byte $F3,$08,$0F,$A0
        .byte $F3,$08,$CF,$20

; --- b4: 8 x 15 px ---
        .export _spr_b4_p0
_spr_b4_p0:            ; 2 байт/строку, 15 строк
        .byte $F0,$00,$3F,$00
        .byte $F0,$0F,$3F,$C0
        .byte $F0,$03,$3F,$00
        .byte $F0,$0F,$0F,$C0
        .byte $C0,$0F,$0F,$F0
        .byte $C0,$3C,$03,$F0
        .byte $00,$30,$03,$3C
        .byte $00,$CC,$00,$FC
        .byte $00,$C0,$00,$0C
        .byte $00,$FC,$00,$CC
        .byte $00,$CC,$00,$CC
        .byte $00,$F0,$03,$3C
        .byte $00,$3C,$03,$F0
        .byte $C0,$0F,$0F,$C0
        .byte $F0,$00,$3F,$00

        .export _spr_b4_p1
_spr_b4_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$03,$00,$FF,$00
        .byte $FF,$00,$03,$FC,$FF,$00
        .byte $FF,$00,$03,$30,$FF,$00
        .byte $FF,$00,$00,$FC,$FF,$00
        .byte $FC,$00,$00,$FF,$FF,$00
        .byte $FC,$03,$00,$CF,$3F,$00
        .byte $F0,$03,$00,$03,$3F,$C0
        .byte $F0,$0C,$00,$CF,$0F,$C0
        .byte $F0,$0C,$00,$00,$0F,$C0
        .byte $F0,$0F,$00,$CC,$0F,$C0
        .byte $F0,$0C,$00,$CC,$0F,$C0
        .byte $F0,$0F,$00,$03,$3F,$C0
        .byte $F0,$03,$00,$CF,$3F,$00
        .byte $FC,$00,$00,$FC,$FF,$00
        .byte $FF,$00,$03,$00,$FF,$00

; --- b5: 8 x 15 px ---
        .export _spr_b5_p0
_spr_b5_p0:            ; 2 байт/строку, 15 строк
        .byte $FC,$00,$0F,$00
        .byte $FC,$03,$0F,$F0
        .byte $FC,$00,$0F,$C0
        .byte $F0,$03,$0F,$F0
        .byte $F0,$0F,$03,$F0
        .byte $C0,$0F,$03,$3C
        .byte $C0,$3C,$00,$0C
        .byte $00,$33,$00,$3F
        .byte $00,$30,$00,$03
        .byte $00,$3F,$00,$33
        .byte $00,$33,$00,$33
        .byte $C0,$3C,$00,$0C
        .byte $C0,$0F,$03,$3C
        .byte $F0,$03,$03,$F0
        .byte $FC,$00,$0F,$00

        .export _spr_b5_p1
_spr_b5_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$C0,$00,$FF,$00
        .byte $FF,$00,$C0,$3F,$FF,$00
        .byte $FF,$00,$C0,$0C,$FF,$00
        .byte $FF,$00,$00,$3F,$FF,$00
        .byte $FF,$00,$00,$FF,$3F,$00
        .byte $FC,$00,$00,$F3,$3F,$C0
        .byte $FC,$03,$00,$C0,$0F,$C0
        .byte $F0,$03,$00,$33,$0F,$F0
        .byte $F0,$03,$00,$00,$0F,$30
        .byte $F0,$03,$00,$F3,$0F,$30
        .byte $F0,$03,$00,$33,$0F,$30
        .byte $FC,$03,$00,$C0,$0F,$C0
        .byte $FC,$00,$00,$F3,$3F,$C0
        .byte $FF,$00,$00,$3F,$3F,$00
        .byte $FF,$00,$C0,$00,$FF,$00

; --- b6: 8 x 15 px ---
        .export _spr_b6_p0
_spr_b6_p0:            ; 2 байт/строку, 15 строк
        .byte $F0,$03,$0F,$C0
        .byte $FC,$03,$3F,$C0
        .byte $FC,$03,$3F,$C0
        .byte $F0,$03,$0F,$C0
        .byte $F0,$0F,$0F,$F0
        .byte $C0,$0F,$03,$F0
        .byte $C0,$30,$03,$0C
        .byte $00,$3F,$00,$FC
        .byte $00,$30,$00,$0C
        .byte $00,$3F,$00,$FC
        .byte $00,$3F,$00,$FC
        .byte $C0,$30,$03,$0C
        .byte $C0,$0F,$03,$F0
        .byte $F0,$0F,$0F,$F0
        .byte $F0,$03,$0F,$C0

        .export _spr_b6_p1
_spr_b6_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$00,$3C,$FF,$00
        .byte $FF,$00,$C3,$3C,$FF,$00
        .byte $FF,$00,$C3,$3C,$FF,$00
        .byte $FF,$00,$00,$3C,$FF,$00
        .byte $FF,$00,$00,$FF,$FF,$00
        .byte $FC,$00,$00,$FF,$3F,$00
        .byte $FC,$03,$00,$00,$3F,$C0
        .byte $F0,$03,$00,$FF,$0F,$C0
        .byte $F0,$03,$00,$00,$0F,$C0
        .byte $F0,$03,$00,$FF,$0F,$C0
        .byte $F0,$03,$00,$FF,$0F,$C0
        .byte $FC,$03,$00,$00,$3F,$C0
        .byte $FC,$00,$00,$FF,$3F,$00
        .byte $FF,$00,$00,$FF,$FF,$00
        .byte $FF,$00,$00,$3C,$FF,$00

; --- b7: 8 x 15 px ---
        .export _spr_b7_p0
_spr_b7_p0:            ; 2 байт/строку, 15 строк
        .byte $FC,$00,$3F,$00
        .byte $FC,$03,$3F,$C0
        .byte $F0,$03,$0F,$C0
        .byte $C0,$0F,$0F,$C0
        .byte $C0,$3F,$0F,$F0
        .byte $00,$3F,$03,$F0
        .byte $00,$3F,$03,$FC
        .byte $C0,$3F,$03,$FC
        .byte $C0,$0C,$03,$F0
        .byte $C0,$33,$0F,$00
        .byte $00,$3F,$0F,$F0
        .byte $00,$FF,$03,$F0
        .byte $00,$FF,$03,$FC
        .byte $00,$3F,$03,$FC
        .byte $C0,$3F,$03,$F0

        .export _spr_b7_p1
_spr_b7_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$C3,$00,$FF,$00
        .byte $FF,$00,$C3,$3C,$FF,$00
        .byte $FF,$00,$00,$3C,$FF,$00
        .byte $FC,$00,$00,$FC,$FF,$00
        .byte $FC,$03,$00,$FF,$FF,$00
        .byte $F0,$03,$00,$FF,$3F,$00
        .byte $F0,$03,$00,$FF,$3F,$C0
        .byte $FC,$03,$00,$FF,$3F,$C0
        .byte $FC,$00,$00,$CF,$3F,$00
        .byte $FC,$03,$00,$30,$FF,$00
        .byte $F0,$03,$00,$FF,$FF,$00
        .byte $F0,$0F,$00,$FF,$3F,$00
        .byte $F0,$0F,$00,$FF,$3F,$C0
        .byte $F0,$03,$00,$FF,$3F,$C0
        .byte $FC,$03,$00,$FF,$3F,$00

; --- b8: 8 x 15 px ---
        .export _spr_b8_p0
_spr_b8_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$CF,$00
        .byte $F3,$00,$03,$30
        .byte $C0,$0C,$03,$FC
        .byte $C0,$3F,$00,$FC
        .byte $00,$3F,$00,$FC
        .byte $00,$3F,$03,$3C
        .byte $C0,$3F,$03,$F0
        .byte $C0,$0F,$0F,$F0
        .byte $00,$33,$03,$F0
        .byte $00,$FF,$03,$FC
        .byte $00,$FC,$00,$FC
        .byte $00,$3F,$00,$FC
        .byte $C0,$3F,$03,$FC

        .export _spr_b8_p1
_spr_b8_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FC,$00,$FF,$00
        .byte $FF,$00,$30,$03,$3F,$00
        .byte $FC,$00,$00,$CF,$3F,$C0
        .byte $FC,$03,$00,$FF,$0F,$C0
        .byte $F0,$03,$00,$FF,$0F,$C0
        .byte $F0,$03,$00,$F3,$3F,$C0
        .byte $FC,$03,$00,$FF,$3F,$00
        .byte $FC,$00,$00,$FF,$FF,$00
        .byte $F0,$03,$00,$3F,$3F,$00
        .byte $F0,$0F,$00,$FF,$3F,$C0
        .byte $F0,$0F,$00,$CF,$0F,$C0
        .byte $F0,$03,$00,$FF,$0F,$C0
        .byte $FC,$03,$00,$FF,$3F,$C0

; --- b9: 8 x 15 px ---
        .export _spr_b9_p0
_spr_b9_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $C3,$00,$C3,$00
        .byte $C0,$3C,$03,$3C
        .byte $00,$3F,$00,$FC
        .byte $00,$FF,$00,$FF
        .byte $00,$FF,$00,$FF
        .byte $00,$3C,$00,$F0
        .byte $00,$3F,$00,$FC
        .byte $00,$FC,$00,$3F
        .byte $00,$FF,$00,$FF
        .byte $00,$3F,$00,$FC

        .export _spr_b9_p1
_spr_b9_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FC,$00,$3C,$00,$3F,$00
        .byte $FC,$03,$00,$C3,$3F,$C0
        .byte $F0,$03,$00,$FF,$0F,$C0
        .byte $F0,$0F,$00,$FF,$0F,$F0
        .byte $F0,$0F,$00,$FF,$0F,$F0
        .byte $F0,$03,$00,$CF,$0F,$00
        .byte $F0,$03,$00,$FF,$0F,$C0
        .byte $F0,$0F,$00,$C3,$0F,$F0
        .byte $F0,$0F,$00,$FF,$0F,$F0
        .byte $F0,$03,$00,$FF,$0F,$C0

; --- bd: 12 x 8 px ---
        .export _spr_bd_p0
_spr_bd_p0:            ; 3 байт/строку, 8 строк
        .byte $C0,$00,$00,$00,$03,$00
        .byte $C0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$03,$00
        .byte $C0,$00,$00,$00,$03,$00
        .byte $C0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$03,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $F3,$00,$33,$00,$3F,$00

        .export _spr_bd_p1
_spr_bd_p1:            ; 4 байт/строку, 8 строк
        .byte $FC,$00,$00,$00,$00,$00,$3F,$00
        .byte $FC,$00,$00,$00,$00,$00,$FF,$00
        .byte $FF,$00,$00,$00,$00,$00,$3F,$00
        .byte $FC,$00,$00,$00,$00,$00,$3F,$00
        .byte $FC,$00,$00,$00,$00,$00,$FF,$00
        .byte $FF,$00,$00,$00,$00,$00,$3F,$00
        .byte $FF,$00,$00,$00,$00,$00,$FF,$00
        .byte $FF,$00,$33,$00,$33,$00,$FF,$00

; --- bf: 12 x 6 px ---
        .export _spr_bf_p0
_spr_bf_p0:            ; 3 байт/строку, 6 строк
        .byte $CC,$00,$CC,$00,$CF,$00
        .byte $C0,$00,$00,$00,$03,$00
        .byte $C0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$03,$00
        .byte $F0,$00,$00,$00,$03,$00
        .byte $F0,$00,$00,$00,$0F,$00

        .export _spr_bf_p1
_spr_bf_p1:            ; 4 байт/строку, 6 строк
        .byte $FC,$00,$CC,$00,$CC,$00,$FF,$00
        .byte $FC,$00,$00,$00,$00,$00,$3F,$00
        .byte $FC,$00,$00,$00,$00,$00,$FF,$00
        .byte $FF,$00,$00,$00,$00,$00,$3F,$00
        .byte $FF,$00,$00,$00,$00,$00,$3F,$00
        .byte $FF,$00,$00,$00,$00,$00,$FF,$00

; --- c0: 8 x 15 px ---
        .export _spr_c0_p0
_spr_c0_p0:            ; 2 байт/строку, 15 строк
        .byte $C0,$07,$0F,$D0
        .byte $C0,$1F,$03,$D0
        .byte $00,$1C,$03,$D0
        .byte $00,$5C,$03,$D4
        .byte $00,$57,$00,$D4
        .byte $00,$54,$00,$55
        .byte $00,$15,$00,$68
        .byte $C0,$15,$03,$80
        .byte $C0,$05,$00,$68
        .byte $F0,$05,$03,$54
        .byte $F0,$02,$03,$80
        .byte $FC,$02,$3F,$80
        .byte $FC,$02,$3F,$80
        .byte $F0,$02,$0F,$80
        .byte $F0,$0A,$0F,$A0

        .export _spr_c0_p1
_spr_c0_p1:            ; 3 байт/строку, 15 строк
        .byte $FC,$00,$00,$7D,$FF,$00
        .byte $FC,$01,$00,$FD,$3F,$00
        .byte $F0,$01,$00,$CD,$3F,$00
        .byte $F0,$05,$00,$CD,$3F,$40
        .byte $F0,$05,$00,$7D,$0F,$40
        .byte $F0,$05,$00,$45,$0F,$50
        .byte $F0,$01,$00,$56,$0F,$80
        .byte $FC,$01,$00,$58,$3F,$00
        .byte $FC,$00,$00,$56,$0F,$80
        .byte $FF,$00,$00,$55,$3F,$40
        .byte $FF,$00,$00,$28,$3F,$00
        .byte $FF,$00,$C3,$28,$FF,$00
        .byte $FF,$00,$C3,$28,$FF,$00
        .byte $FF,$00,$00,$28,$FF,$00
        .byte $FF,$00,$00,$AA,$FF,$00

; --- c1: 8 x 15 px ---
        .export _spr_c1_p0
_spr_c1_p0:            ; 2 байт/строку, 15 строк
        .byte $C0,$07,$0F,$D0
        .byte $C0,$1F,$03,$D0
        .byte $00,$1F,$03,$D4
        .byte $00,$5F,$03,$D4
        .byte $00,$57,$00,$D5
        .byte $00,$54,$00,$5A
        .byte $00,$15,$03,$60
        .byte $C0,$15,$0F,$80
        .byte $C0,$05,$03,$60
        .byte $F0,$05,$00,$58
        .byte $F0,$02,$03,$94
        .byte $FC,$02,$03,$80
        .byte $F0,$02,$0F,$80
        .byte $F0,$0A,$0F,$A0
        .byte $F0,$00,$0F,$00

        .export _spr_c1_p1
_spr_c1_p1:            ; 3 байт/строку, 15 строк
        .byte $FC,$00,$00,$7D,$FF,$00
        .byte $FC,$01,$00,$FD,$3F,$00
        .byte $F0,$01,$00,$FD,$3F,$40
        .byte $F0,$05,$00,$FD,$3F,$40
        .byte $F0,$05,$00,$7D,$0F,$50
        .byte $F0,$05,$00,$45,$0F,$A0
        .byte $F0,$01,$00,$56,$3F,$00
        .byte $FC,$01,$00,$58,$FF,$00
        .byte $FC,$00,$00,$56,$3F,$00
        .byte $FF,$00,$00,$55,$0F,$80
        .byte $FF,$00,$00,$29,$3F,$40
        .byte $FF,$00,$C0,$28,$3F,$00
        .byte $FF,$00,$00,$28,$FF,$00
        .byte $FF,$00,$00,$AA,$FF,$00
        .byte $FF,$00,$00,$00,$FF,$00

; --- c2: 8 x 15 px ---
        .export _spr_c2_p0
_spr_c2_p0:            ; 2 байт/строку, 15 строк
        .byte $C0,$07,$0F,$D0
        .byte $C0,$1F,$03,$D0
        .byte $00,$1F,$03,$D4
        .byte $00,$5F,$00,$D4
        .byte $00,$57,$00,$D6
        .byte $00,$54,$00,$58
        .byte $00,$15,$03,$60
        .byte $C0,$15,$0F,$80
        .byte $C0,$05,$03,$60
        .byte $F0,$05,$00,$58
        .byte $F0,$02,$00,$94
        .byte $F0,$02,$03,$80
        .byte $F0,$0A,$0F,$A0
        .byte $F0,$00,$0F,$00
        .byte $FF,$00,$FF,$00

        .export _spr_c2_p1
_spr_c2_p1:            ; 3 байт/строку, 15 строк
        .byte $FC,$00,$00,$7D,$FF,$00
        .byte $FC,$01,$00,$FD,$3F,$00
        .byte $F0,$01,$00,$FD,$3F,$40
        .byte $F0,$05,$00,$FD,$0F,$40
        .byte $F0,$05,$00,$7D,$0F,$60
        .byte $F0,$05,$00,$45,$0F,$80
        .byte $F0,$01,$00,$56,$3F,$00
        .byte $FC,$01,$00,$58,$FF,$00
        .byte $FC,$00,$00,$56,$3F,$00
        .byte $FF,$00,$00,$55,$0F,$80
        .byte $FF,$00,$00,$29,$0F,$40
        .byte $FF,$00,$00,$28,$3F,$00
        .byte $FF,$00,$00,$AA,$FF,$00
        .byte $FF,$00,$00,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00

; --- c3: 8 x 15 px ---
        .export _spr_c3_p0
_spr_c3_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $F0,$00,$0F,$00
        .byte $C0,$07,$0F,$D0
        .byte $C0,$1F,$03,$D0
        .byte $00,$1F,$03,$D4
        .byte $00,$5F,$00,$D4
        .byte $00,$57,$00,$D6
        .byte $00,$54,$00,$58
        .byte $00,$15,$03,$60
        .byte $C0,$15,$0F,$A0
        .byte $C0,$05,$03,$68
        .byte $F0,$05,$00,$5A
        .byte $F0,$02,$00,$80
        .byte $F0,$0A,$0F,$A0
        .byte $F0,$00,$0F,$00

        .export _spr_c3_p1
_spr_c3_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$00,$00,$FF,$00
        .byte $FC,$00,$00,$7D,$FF,$00
        .byte $FC,$01,$00,$FD,$3F,$00
        .byte $F0,$01,$00,$FD,$3F,$40
        .byte $F0,$05,$00,$FD,$0F,$40
        .byte $F0,$05,$00,$7D,$0F,$60
        .byte $F0,$05,$00,$45,$0F,$80
        .byte $F0,$01,$00,$56,$3F,$00
        .byte $FC,$01,$00,$5A,$FF,$00
        .byte $FC,$00,$00,$56,$3F,$80
        .byte $FF,$00,$00,$55,$0F,$A0
        .byte $FF,$00,$00,$28,$0F,$00
        .byte $FF,$00,$00,$AA,$FF,$00
        .byte $FF,$00,$00,$00,$FF,$00

; --- cn: 8 x 10 px ---
        .export _spr_cn_p0
_spr_cn_p0:            ; 2 байт/строку, 10 строк
        .byte $F0,$00,$03,$00
        .byte $F0,$01,$00,$54
        .byte $C0,$05,$00,$54
        .byte $C0,$07,$00,$55
        .byte $C0,$05,$00,$54
        .byte $F0,$01,$00,$54
        .byte $F0,$01,$03,$50
        .byte $FC,$00,$03,$50
        .byte $FC,$00,$0F,$40
        .byte $FF,$00,$0F,$00

        .export _spr_cn_p1
_spr_cn_p1:            ; 3 байт/строку, 10 строк
        .byte $FF,$00,$00,$00,$3F,$00
        .byte $FF,$00,$00,$15,$0F,$40
        .byte $FC,$00,$00,$55,$0F,$40
        .byte $FC,$00,$00,$75,$0F,$50
        .byte $FC,$00,$00,$55,$0F,$40
        .byte $FF,$00,$00,$15,$0F,$40
        .byte $FF,$00,$00,$15,$3F,$00
        .byte $FF,$00,$C0,$05,$3F,$00
        .byte $FF,$00,$C0,$04,$FF,$00
        .byte $FF,$00,$F0,$00,$FF,$00

; --- d0: 8 x 15 px ---
        .export _spr_d0_p0
_spr_d0_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$00,$2C
        .byte $FF,$00,$00,$B3
        .byte $FC,$00,$00,$B3
        .byte $FC,$02,$00,$AC
        .byte $F0,$02,$03,$A0
        .byte $F0,$0F,$00,$A0
        .byte $C0,$0E,$00,$AF
        .byte $C0,$0E,$00,$AF
        .byte $F0,$0F,$00,$AF
        .byte $F0,$02,$00,$AF
        .byte $FC,$01,$00,$50
        .byte $F0,$01,$03,$54
        .byte $F0,$05,$00,$14
        .byte $F0,$05,$C0,$04
        .byte $F0,$00,$F3,$00

        .export _spr_d0_p1
_spr_d0_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$F0,$02,$0F,$C0
        .byte $FF,$00,$F0,$0B,$0F,$30
        .byte $FF,$00,$C0,$0B,$0F,$30
        .byte $FF,$00,$C0,$2A,$0F,$C0
        .byte $FF,$00,$00,$2A,$3F,$00
        .byte $FF,$00,$00,$FA,$0F,$00
        .byte $FC,$00,$00,$EA,$0F,$F0
        .byte $FC,$00,$00,$EA,$0F,$F0
        .byte $FF,$00,$00,$FA,$0F,$F0
        .byte $FF,$00,$00,$2A,$0F,$F0
        .byte $FF,$00,$C0,$15,$0F,$00
        .byte $FF,$00,$00,$15,$3F,$40
        .byte $FF,$00,$00,$51,$0F,$40
        .byte $FF,$00,$0C,$50,$0F,$40
        .byte $FF,$00,$0F,$00,$3F,$00

; --- d1: 8 x 15 px ---
        .export _spr_d1_p0
_spr_d1_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$00,$2F
        .byte $FF,$00,$00,$AF
        .byte $FC,$00,$00,$AF
        .byte $FC,$02,$00,$AF
        .byte $F0,$02,$00,$A0
        .byte $C0,$0F,$03,$A0
        .byte $C0,$3E,$00,$AC
        .byte $C0,$3E,$00,$B3
        .byte $C0,$0F,$00,$B3
        .byte $F0,$02,$00,$AC
        .byte $FC,$01,$03,$50
        .byte $F0,$01,$03,$54
        .byte $F0,$01,$03,$14
        .byte $F0,$01,$03,$14
        .byte $F0,$01,$03,$14

        .export _spr_d1_p1
_spr_d1_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$F0,$02,$0F,$F0
        .byte $FF,$00,$F0,$0A,$0F,$F0
        .byte $FF,$00,$C0,$0A,$0F,$F0
        .byte $FF,$00,$C0,$2A,$0F,$F0
        .byte $FF,$00,$00,$2A,$0F,$00
        .byte $FC,$00,$00,$FA,$3F,$00
        .byte $FC,$03,$00,$EA,$0F,$C0
        .byte $FC,$03,$00,$EB,$0F,$30
        .byte $FC,$00,$00,$FB,$0F,$30
        .byte $FF,$00,$00,$2A,$0F,$C0
        .byte $FF,$00,$C0,$15,$3F,$00
        .byte $FF,$00,$00,$15,$3F,$40
        .byte $FF,$00,$00,$11,$3F,$40
        .byte $FF,$00,$00,$11,$3F,$40
        .byte $FF,$00,$00,$11,$3F,$40

; --- d2: 8 x 15 px ---
        .export _spr_d2_p0
_spr_d2_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$00,$2C
        .byte $FF,$00,$00,$B3
        .byte $FC,$00,$00,$B3
        .byte $FC,$02,$00,$AC
        .byte $C0,$02,$03,$A0
        .byte $C0,$3F,$00,$A0
        .byte $00,$32,$00,$AF
        .byte $00,$32,$00,$AF
        .byte $C0,$3F,$00,$AF
        .byte $C0,$02,$00,$AF
        .byte $FC,$00,$00,$50
        .byte $FC,$01,$03,$50
        .byte $FC,$01,$03,$50
        .byte $FC,$01,$03,$50
        .byte $FC,$01,$03,$50

        .export _spr_d2_p1
_spr_d2_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$F0,$02,$0F,$C0
        .byte $FF,$00,$F0,$0B,$0F,$30
        .byte $FF,$00,$C0,$0B,$0F,$30
        .byte $FF,$00,$C0,$2A,$0F,$C0
        .byte $FC,$00,$00,$2A,$3F,$00
        .byte $FC,$03,$00,$FA,$0F,$00
        .byte $F0,$03,$00,$2A,$0F,$F0
        .byte $F0,$03,$00,$2A,$0F,$F0
        .byte $FC,$03,$00,$FA,$0F,$F0
        .byte $FC,$00,$00,$2A,$0F,$F0
        .byte $FF,$00,$C0,$05,$0F,$00
        .byte $FF,$00,$C0,$15,$3F,$00
        .byte $FF,$00,$C0,$15,$3F,$00
        .byte $FF,$00,$C0,$15,$3F,$00
        .byte $FF,$00,$C0,$15,$3F,$00

; --- d4: 8 x 15 px ---
        .export _spr_d4_p0
_spr_d4_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$00,$2C
        .byte $FF,$00,$00,$B3
        .byte $FC,$00,$00,$B3
        .byte $FC,$02,$00,$AC
        .byte $FC,$02,$03,$A0
        .byte $F0,$03,$00,$A0
        .byte $F0,$0F,$00,$AF
        .byte $F0,$0F,$00,$AF
        .byte $F0,$03,$00,$AF
        .byte $FC,$02,$00,$AF
        .byte $FC,$01,$00,$50
        .byte $F0,$01,$03,$54
        .byte $F0,$05,$00,$14
        .byte $F0,$05,$C0,$04
        .byte $F0,$00,$F3,$00

        .export _spr_d4_p1
_spr_d4_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$F0,$02,$0F,$C0
        .byte $FF,$00,$F0,$0B,$0F,$30
        .byte $FF,$00,$C0,$0B,$0F,$30
        .byte $FF,$00,$C0,$2A,$0F,$C0
        .byte $FF,$00,$C0,$2A,$3F,$00
        .byte $FF,$00,$00,$3A,$0F,$00
        .byte $FF,$00,$00,$FA,$0F,$F0
        .byte $FF,$00,$00,$FA,$0F,$F0
        .byte $FF,$00,$00,$3A,$0F,$F0
        .byte $FF,$00,$C0,$2A,$0F,$F0
        .byte $FF,$00,$C0,$15,$0F,$00
        .byte $FF,$00,$00,$15,$3F,$40
        .byte $FF,$00,$00,$51,$0F,$40
        .byte $FF,$00,$0C,$50,$0F,$40
        .byte $FF,$00,$0F,$00,$3F,$00

; --- d5: 8 x 15 px ---
        .export _spr_d5_p0
_spr_d5_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$00,$2F
        .byte $FF,$00,$00,$AF
        .byte $FC,$00,$00,$AF
        .byte $FC,$02,$00,$AF
        .byte $FC,$02,$00,$A0
        .byte $F0,$03,$03,$A0
        .byte $F0,$0F,$00,$AC
        .byte $F0,$0F,$00,$B3
        .byte $F0,$03,$00,$B3
        .byte $FC,$02,$00,$AC
        .byte $FC,$01,$03,$50
        .byte $F0,$01,$03,$54
        .byte $F0,$01,$03,$14
        .byte $F0,$01,$03,$14
        .byte $F0,$01,$03,$14

        .export _spr_d5_p1
_spr_d5_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$F0,$02,$0F,$F0
        .byte $FF,$00,$F0,$0A,$0F,$F0
        .byte $FF,$00,$C0,$0A,$0F,$F0
        .byte $FF,$00,$C0,$2A,$0F,$F0
        .byte $FF,$00,$C0,$2A,$0F,$00
        .byte $FF,$00,$00,$3A,$3F,$00
        .byte $FF,$00,$00,$FA,$0F,$C0
        .byte $FF,$00,$00,$FB,$0F,$30
        .byte $FF,$00,$00,$3B,$0F,$30
        .byte $FF,$00,$C0,$2A,$0F,$C0
        .byte $FF,$00,$C0,$15,$3F,$00
        .byte $FF,$00,$00,$15,$3F,$40
        .byte $FF,$00,$00,$11,$3F,$40
        .byte $FF,$00,$00,$11,$3F,$40
        .byte $FF,$00,$00,$11,$3F,$40

; --- d6: 8 x 15 px ---
        .export _spr_d6_p0
_spr_d6_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$00,$2C
        .byte $FF,$00,$00,$B3
        .byte $FC,$00,$00,$B3
        .byte $FC,$02,$00,$AC
        .byte $FC,$02,$03,$A0
        .byte $F0,$03,$00,$A0
        .byte $F0,$0F,$00,$AF
        .byte $F0,$0F,$00,$AF
        .byte $F0,$03,$00,$AF
        .byte $FC,$02,$00,$AF
        .byte $FC,$00,$00,$50
        .byte $FC,$01,$03,$50
        .byte $FC,$01,$03,$50
        .byte $FC,$01,$03,$50
        .byte $FC,$01,$03,$50

        .export _spr_d6_p1
_spr_d6_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$F0,$02,$0F,$C0
        .byte $FF,$00,$F0,$0B,$0F,$30
        .byte $FF,$00,$C0,$0B,$0F,$30
        .byte $FF,$00,$C0,$2A,$0F,$C0
        .byte $FF,$00,$C0,$2A,$3F,$00
        .byte $FF,$00,$00,$3A,$0F,$00
        .byte $FF,$00,$00,$FA,$0F,$F0
        .byte $FF,$00,$00,$FA,$0F,$F0
        .byte $FF,$00,$00,$3A,$0F,$F0
        .byte $FF,$00,$C0,$2A,$0F,$F0
        .byte $FF,$00,$C0,$05,$0F,$00
        .byte $FF,$00,$C0,$15,$3F,$00
        .byte $FF,$00,$C0,$15,$3F,$00
        .byte $FF,$00,$C0,$15,$3F,$00
        .byte $FF,$00,$C0,$15,$3F,$00

; --- dn: 12 x 6 px ---
        .export _spr_dn_p0
_spr_dn_p0:            ; 3 байт/строку, 6 строк
        .byte $C0,$00,$00,$00,$03,$00
        .byte $C0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$03,$00
        .byte $F0,$00,$00,$00,$0F,$00
        .byte $FC,$00,$00,$00,$3F,$00
        .byte $FF,$00,$00,$00,$FF,$00

        .export _spr_dn_p1
_spr_dn_p1:            ; 4 байт/строку, 6 строк
        .byte $FC,$00,$00,$00,$00,$00,$3F,$00
        .byte $FC,$00,$00,$00,$00,$00,$FF,$00
        .byte $FF,$00,$00,$00,$00,$00,$3F,$00
        .byte $FF,$00,$00,$00,$00,$00,$FF,$00
        .byte $FF,$00,$C0,$00,$03,$00,$FF,$00
        .byte $FF,$00,$F0,$00,$0F,$00,$FF,$00

; --- e1: 4 x 7 px ---
        .export _spr_e1_p0
_spr_e1_p0:            ; 1 байт/строку, 7 строк
        .byte $3C,$00
        .byte $00,$C3
        .byte $00,$38
        .byte $C3,$28
        .byte $00,$38
        .byte $00,$C3
        .byte $3C,$00

        .export _spr_e1_p1
_spr_e1_p1:            ; 2 байт/строку, 7 строк
        .byte $F3,$00,$CF,$00
        .byte $F0,$0C,$0F,$30
        .byte $F0,$03,$0F,$80
        .byte $FC,$02,$3F,$80
        .byte $F0,$03,$0F,$80
        .byte $F0,$0C,$0F,$30
        .byte $F3,$00,$CF,$00

; --- e2: 4 x 7 px ---
        .export _spr_e2_p0
_spr_e2_p0:            ; 1 байт/строку, 7 строк
        .byte $3C,$C3
        .byte $00,$C3
        .byte $00,$28
        .byte $C3,$00
        .byte $00,$28
        .byte $00,$C3
        .byte $3C,$C3

        .export _spr_e2_p1
_spr_e2_p1:            ; 2 байт/строку, 7 строк
        .byte $F3,$0C,$CF,$30
        .byte $F0,$0C,$0F,$30
        .byte $F0,$02,$0F,$80
        .byte $FC,$00,$3F,$00
        .byte $F0,$02,$0F,$80
        .byte $F0,$0C,$0F,$30
        .byte $F3,$0C,$CF,$30

; --- e3: 4 x 7 px ---
        .export _spr_e3_p0
_spr_e3_p0:            ; 1 байт/строку, 7 строк
        .byte $3C,$82
        .byte $00,$C3
        .byte $3C,$00
        .byte $FF,$00
        .byte $3C,$00
        .byte $00,$C3
        .byte $3C,$82

        .export _spr_e3_p1
_spr_e3_p1:            ; 2 байт/строку, 7 строк
        .byte $F3,$08,$CF,$20
        .byte $F0,$0C,$0F,$30
        .byte $F3,$00,$CF,$00
        .byte $FF,$00,$FF,$00
        .byte $F3,$00,$CF,$00
        .byte $F0,$0C,$0F,$30
        .byte $F3,$08,$CF,$20

; --- f1: 8 x 15 px ---
        .export _spr_f1_p0
_spr_f1_p0:            ; 2 байт/строку, 15 строк
        .byte $FC,$00,$3F,$00
        .byte $F0,$01,$0F,$40
        .byte $C0,$07,$03,$D0
        .byte $00,$1F,$03,$F4
        .byte $00,$1F,$03,$F4
        .byte $00,$43,$00,$FC
        .byte $00,$7F,$00,$FC
        .byte $00,$43,$00,$C4
        .byte $00,$4F,$00,$F4
        .byte $00,$7F,$00,$C4
        .byte $00,$7F,$00,$FC
        .byte $00,$7F,$00,$FC
        .byte $00,$7F,$00,$FC
        .byte $00,$7F,$00,$FC
        .byte $00,$7F,$00,$FC

        .export _spr_f1_p1
_spr_f1_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$C3,$00,$FF,$00
        .byte $FF,$00,$00,$14,$FF,$00
        .byte $FC,$00,$00,$7D,$3F,$00
        .byte $F0,$01,$00,$FF,$3F,$40
        .byte $F0,$01,$00,$FF,$3F,$40
        .byte $F0,$04,$00,$3F,$0F,$C0
        .byte $F0,$07,$00,$FF,$0F,$C0
        .byte $F0,$04,$00,$3C,$0F,$40
        .byte $F0,$04,$00,$FF,$0F,$40
        .byte $F0,$07,$00,$FC,$0F,$40
        .byte $F0,$07,$00,$FF,$0F,$C0
        .byte $F0,$07,$00,$FF,$0F,$C0
        .byte $F0,$07,$00,$FF,$0F,$C0
        .byte $F0,$07,$00,$FF,$0F,$C0
        .byte $F0,$07,$00,$FF,$0F,$C0

; --- f2: 8 x 15 px ---
        .export _spr_f2_p0
_spr_f2_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FC,$00,$3F,$00
        .byte $F0,$01,$0F,$40
        .byte $C0,$07,$03,$D0
        .byte $00,$1F,$03,$F4
        .byte $00,$1F,$03,$F4
        .byte $00,$43,$00,$FC
        .byte $00,$7F,$00,$FC
        .byte $00,$43,$00,$C4
        .byte $00,$4F,$00,$F4
        .byte $00,$7F,$00,$C4
        .byte $00,$7F,$00,$FC
        .byte $00,$7F,$00,$FC
        .byte $00,$00,$00,$00

        .export _spr_f2_p1
_spr_f2_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$C3,$00,$FF,$00
        .byte $FF,$00,$00,$14,$FF,$00
        .byte $FC,$00,$00,$7D,$3F,$00
        .byte $F0,$01,$00,$FF,$3F,$40
        .byte $F0,$01,$00,$FF,$3F,$40
        .byte $F0,$04,$00,$3F,$0F,$C0
        .byte $F0,$07,$00,$FF,$0F,$C0
        .byte $F0,$04,$00,$3C,$0F,$40
        .byte $F0,$04,$00,$FF,$0F,$40
        .byte $F0,$07,$00,$FC,$0F,$40
        .byte $F0,$07,$00,$FF,$0F,$C0
        .byte $F0,$07,$00,$FF,$0F,$C0
        .byte $F0,$00,$00,$00,$0F,$00

; --- f3: 8 x 15 px ---
        .export _spr_f3_p0
_spr_f3_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FC,$00,$3F,$00
        .byte $F0,$01,$0F,$40
        .byte $C0,$07,$03,$D0
        .byte $00,$1F,$03,$F4
        .byte $00,$1F,$03,$F4
        .byte $00,$43,$00,$FC
        .byte $00,$7F,$00,$FC
        .byte $00,$43,$00,$C4
        .byte $00,$4F,$00,$F4
        .byte $00,$00,$00,$00

        .export _spr_f3_p1
_spr_f3_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$C3,$00,$FF,$00
        .byte $FF,$00,$00,$14,$FF,$00
        .byte $FC,$00,$00,$7D,$3F,$00
        .byte $F0,$01,$00,$FF,$3F,$40
        .byte $F0,$01,$00,$FF,$3F,$40
        .byte $F0,$04,$00,$3F,$0F,$C0
        .byte $F0,$07,$00,$FF,$0F,$C0
        .byte $F0,$04,$00,$3C,$0F,$40
        .byte $F0,$04,$00,$FF,$0F,$40
        .byte $F0,$00,$00,$00,$0F,$00

; --- f4: 8 x 15 px ---
        .export _spr_f4_p0
_spr_f4_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FC,$00,$3F,$00
        .byte $F0,$01,$0F,$40
        .byte $C0,$07,$03,$D0
        .byte $00,$1F,$03,$F4
        .byte $00,$1F,$03,$F4
        .byte $00,$43,$00,$FC
        .byte $00,$7F,$00,$FC
        .byte $00,$00,$00,$00

        .export _spr_f4_p1
_spr_f4_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$C3,$00,$FF,$00
        .byte $FF,$00,$00,$14,$FF,$00
        .byte $FC,$00,$00,$7D,$3F,$00
        .byte $F0,$01,$00,$FF,$3F,$40
        .byte $F0,$01,$00,$FF,$3F,$40
        .byte $F0,$04,$00,$3F,$0F,$C0
        .byte $F0,$07,$00,$FF,$0F,$C0
        .byte $F0,$00,$00,$00,$0F,$00

; --- f5: 8 x 15 px ---
        .export _spr_f5_p0
_spr_f5_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FC,$00,$3F,$00
        .byte $F0,$01,$0F,$40
        .byte $C0,$07,$03,$D0
        .byte $00,$1F,$03,$F4
        .byte $00,$1F,$03,$F4
        .byte $00,$00,$00,$00

        .export _spr_f5_p1
_spr_f5_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$C3,$00,$FF,$00
        .byte $FF,$00,$00,$14,$FF,$00
        .byte $FC,$00,$00,$7D,$3F,$00
        .byte $F0,$01,$00,$FF,$3F,$40
        .byte $F0,$01,$00,$FF,$3F,$40
        .byte $F0,$00,$00,$00,$0F,$00

; --- f9: 8 x 15 px ---
        .export _spr_f9_p0
_spr_f9_p0:            ; 2 байт/строку, 15 строк
        .byte $FC,$00,$30,$00
        .byte $FC,$03,$30,$CF
        .byte $F0,$03,$00,$CF
        .byte $C0,$03,$00,$CF
        .byte $00,$17,$00,$EF
        .byte $00,$56,$00,$AA
        .byte $00,$46,$00,$AA
        .byte $00,$16,$00,$A8
        .byte $00,$56,$03,$A8
        .byte $00,$43,$03,$30
        .byte $3C,$00,$03,$F0
        .byte $FC,$03,$03,$30
        .byte $FC,$00,$03,$F0
        .byte $FF,$00,$0F,$F0
        .byte $FF,$00,$0F,$00

        .export _spr_f9_p1
_spr_f9_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$C3,$00,$0F,$00
        .byte $FF,$00,$C3,$3C,$0F,$F0
        .byte $FF,$00,$00,$3C,$0F,$F0
        .byte $FC,$00,$00,$3C,$0F,$F0
        .byte $F0,$01,$00,$7E,$0F,$F0
        .byte $F0,$05,$00,$6A,$0F,$A0
        .byte $F0,$04,$00,$6A,$0F,$A0
        .byte $F0,$01,$00,$6A,$0F,$80
        .byte $F0,$05,$00,$6A,$3F,$80
        .byte $F0,$04,$00,$33,$3F,$00
        .byte $F3,$00,$C0,$0F,$3F,$00
        .byte $FF,$00,$C0,$33,$3F,$00
        .byte $FF,$00,$C0,$0F,$3F,$00
        .byte $FF,$00,$F0,$0F,$FF,$00
        .byte $FF,$00,$F0,$00,$FF,$00

; --- k1: 4 x 7 px ---
        .export _spr_k1_p0
_spr_k1_p0:            ; 1 байт/строку, 7 строк
        .byte $00,$2A
        .byte $00,$FB
        .byte $00,$2E
        .byte $00,$FA
        .byte $00,$BB
        .byte $00,$BA
        .byte $00,$BA

        .export _spr_k1_p1
_spr_k1_p1:            ; 2 байт/строку, 7 строк
        .byte $F0,$02,$0F,$A0
        .byte $F0,$0F,$0F,$B0
        .byte $F0,$02,$0F,$E0
        .byte $F0,$0F,$0F,$A0
        .byte $F0,$0B,$0F,$B0
        .byte $F0,$0B,$0F,$A0
        .byte $F0,$0B,$0F,$A0

; --- k2: 4 x 7 px ---
        .export _spr_k2_p0
_spr_k2_p0:            ; 1 байт/строку, 7 строк
        .byte $00,$2A
        .byte $00,$BA
        .byte $00,$AA
        .byte $00,$BA
        .byte $00,$AA
        .byte $00,$BB
        .byte $00,$2A

        .export _spr_k2_p1
_spr_k2_p1:            ; 2 байт/строку, 7 строк
        .byte $F0,$02,$0F,$A0
        .byte $F0,$0B,$0F,$A0
        .byte $F0,$0A,$0F,$A0
        .byte $F0,$0B,$0F,$A0
        .byte $F0,$0A,$0F,$A0
        .byte $F0,$0B,$0F,$B0
        .byte $F0,$02,$0F,$A0

; --- l0: 8 x 15 px ---
        .export _spr_l0_p0
_spr_l0_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$3F,$00
        .byte $CC,$00,$0F,$C0
        .byte $00,$13,$0F,$F0
        .byte $00,$17,$03,$30
        .byte $C0,$16,$03,$A8
        .byte $C0,$06,$00,$A8
        .byte $C0,$06,$00,$AA
        .byte $C0,$06,$00,$AA
        .byte $C0,$16,$00,$AF
        .byte $00,$17,$00,$EF
        .byte $00,$13,$00,$CF
        .byte $C0,$03,$00,$CF
        .byte $FC,$03,$30,$C0

        .export _spr_l0_p1
_spr_l0_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$F3,$00,$FF,$00
        .byte $FC,$00,$C0,$0C,$FF,$00
        .byte $F0,$01,$00,$3F,$FF,$00
        .byte $F0,$01,$00,$73,$3F,$00
        .byte $FC,$01,$00,$6A,$3F,$80
        .byte $FC,$00,$00,$6A,$0F,$80
        .byte $FC,$00,$00,$6A,$0F,$A0
        .byte $FC,$00,$00,$6A,$0F,$A0
        .byte $FC,$01,$00,$6A,$0F,$F0
        .byte $F0,$01,$00,$7E,$0F,$F0
        .byte $F0,$01,$00,$3C,$0F,$F0
        .byte $FC,$00,$00,$3C,$0F,$F0
        .byte $FF,$00,$C3,$3C,$0F,$00

; --- l1: 8 x 15 px ---
        .export _spr_l1_p0
_spr_l1_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$3F,$00
        .byte $FC,$00,$0F,$C0
        .byte $FC,$03,$0F,$F0
        .byte $00,$03,$0F,$30
        .byte $00,$57,$03,$30
        .byte $00,$56,$03,$A8
        .byte $00,$16,$00,$A8
        .byte $C0,$06,$00,$AA
        .byte $00,$16,$00,$AA
        .byte $00,$57,$00,$EA
        .byte $00,$57,$00,$CF
        .byte $00,$03,$00,$CF
        .byte $FC,$03,$00,$CF
        .byte $FC,$00,$30,$0F

        .export _spr_l1_p1
_spr_l1_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$F3,$00,$FF,$00
        .byte $FF,$00,$C0,$0C,$FF,$00
        .byte $FF,$00,$C0,$3F,$FF,$00
        .byte $F0,$00,$00,$33,$FF,$00
        .byte $F0,$05,$00,$73,$3F,$00
        .byte $F0,$05,$00,$6A,$3F,$80
        .byte $F0,$01,$00,$6A,$0F,$80
        .byte $FC,$00,$00,$6A,$0F,$A0
        .byte $F0,$01,$00,$6A,$0F,$A0
        .byte $F0,$05,$00,$7E,$0F,$A0
        .byte $F0,$05,$00,$7C,$0F,$F0
        .byte $F0,$00,$00,$3C,$0F,$F0
        .byte $FF,$00,$C0,$3C,$0F,$F0
        .byte $FF,$00,$C3,$00,$0F,$F0

; --- l2: 8 x 15 px ---
        .export _spr_l2_p0
_spr_l2_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$3F,$00
        .byte $FC,$00,$0F,$C0
        .byte $FC,$03,$0F,$F0
        .byte $FC,$03,$0F,$30
        .byte $FC,$03,$0F,$30
        .byte $00,$03,$03,$30
        .byte $00,$56,$03,$A8
        .byte $00,$56,$00,$A8
        .byte $00,$06,$00,$AA
        .byte $00,$56,$00,$AA
        .byte $00,$56,$00,$AF
        .byte $00,$03,$00,$EF
        .byte $F0,$03,$00,$CF
        .byte $F0,$03,$00,$CF
        .byte $FC,$03,$30,$C0

        .export _spr_l2_p1
_spr_l2_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$F3,$00,$FF,$00
        .byte $FF,$00,$C0,$0C,$FF,$00
        .byte $FF,$00,$C0,$3F,$FF,$00
        .byte $FF,$00,$C0,$33,$FF,$00
        .byte $FF,$00,$C0,$33,$FF,$00
        .byte $F0,$00,$00,$33,$3F,$00
        .byte $F0,$05,$00,$6A,$3F,$80
        .byte $F0,$05,$00,$6A,$0F,$80
        .byte $F0,$00,$00,$6A,$0F,$A0
        .byte $F0,$05,$00,$6A,$0F,$A0
        .byte $F0,$05,$00,$6A,$0F,$F0
        .byte $F0,$00,$00,$3E,$0F,$F0
        .byte $FF,$00,$00,$3C,$0F,$F0
        .byte $FF,$00,$00,$3C,$0F,$F0
        .byte $FF,$00,$C3,$3C,$0F,$00

; --- l4: 8 x 15 px ---
        .export _spr_l4_p0
_spr_l4_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $CF,$00,$3F,$00
        .byte $00,$10,$0F,$C0
        .byte $00,$17,$03,$F0
        .byte $C0,$16,$03,$A8
        .byte $C0,$06,$00,$A8
        .byte $C0,$06,$00,$AA
        .byte $C0,$06,$00,$AA
        .byte $C0,$16,$00,$AF
        .byte $00,$17,$00,$EF
        .byte $00,$13,$00,$CF
        .byte $C0,$03,$00,$CF
        .byte $FC,$03,$30,$C0

        .export _spr_l4_p1
_spr_l4_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FC,$00,$F3,$00,$FF,$00
        .byte $F0,$01,$00,$0C,$FF,$00
        .byte $F0,$01,$00,$7F,$3F,$00
        .byte $FC,$01,$00,$6A,$3F,$80
        .byte $FC,$00,$00,$6A,$0F,$80
        .byte $FC,$00,$00,$6A,$0F,$A0
        .byte $FC,$00,$00,$6A,$0F,$A0
        .byte $FC,$01,$00,$6A,$0F,$F0
        .byte $F0,$01,$00,$7E,$0F,$F0
        .byte $F0,$01,$00,$3C,$0F,$F0
        .byte $FC,$00,$00,$3C,$0F,$F0
        .byte $FF,$00,$C3,$3C,$0F,$00

; --- l5: 8 x 15 px ---
        .export _spr_l5_p0
_spr_l5_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$3F,$00
        .byte $00,$00,$0F,$C0
        .byte $00,$57,$03,$F0
        .byte $00,$56,$03,$A8
        .byte $00,$16,$00,$A8
        .byte $C0,$06,$00,$AA
        .byte $00,$16,$00,$AA
        .byte $00,$57,$00,$EA
        .byte $00,$57,$00,$CF
        .byte $00,$03,$00,$CF
        .byte $FC,$03,$00,$CF
        .byte $FC,$00,$30,$0F

        .export _spr_l5_p1
_spr_l5_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$F3,$00,$FF,$00
        .byte $F0,$00,$00,$0C,$FF,$00
        .byte $F0,$05,$00,$7F,$3F,$00
        .byte $F0,$05,$00,$6A,$3F,$80
        .byte $F0,$01,$00,$6A,$0F,$80
        .byte $FC,$00,$00,$6A,$0F,$A0
        .byte $F0,$01,$00,$6A,$0F,$A0
        .byte $F0,$05,$00,$7E,$0F,$A0
        .byte $F0,$05,$00,$7C,$0F,$F0
        .byte $F0,$00,$00,$3C,$0F,$F0
        .byte $FF,$00,$C0,$3C,$0F,$F0
        .byte $FF,$00,$C3,$00,$0F,$F0

; --- l6: 8 x 15 px ---
        .export _spr_l6_p0
_spr_l6_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$3F,$00
        .byte $FC,$00,$0F,$C0
        .byte $00,$03,$03,$F0
        .byte $00,$56,$03,$A8
        .byte $00,$56,$00,$A8
        .byte $00,$06,$00,$AA
        .byte $00,$56,$00,$AA
        .byte $00,$56,$00,$AF
        .byte $00,$03,$00,$EF
        .byte $F0,$03,$00,$CF
        .byte $F0,$03,$00,$CF
        .byte $FC,$03,$30,$C0

        .export _spr_l6_p1
_spr_l6_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$F3,$00,$FF,$00
        .byte $FF,$00,$C0,$0C,$FF,$00
        .byte $F0,$00,$00,$3F,$3F,$00
        .byte $F0,$05,$00,$6A,$3F,$80
        .byte $F0,$05,$00,$6A,$0F,$80
        .byte $F0,$00,$00,$6A,$0F,$A0
        .byte $F0,$05,$00,$6A,$0F,$A0
        .byte $F0,$05,$00,$6A,$0F,$F0
        .byte $F0,$00,$00,$3E,$0F,$F0
        .byte $FF,$00,$00,$3C,$0F,$F0
        .byte $FF,$00,$00,$3C,$0F,$F0
        .byte $FF,$00,$C3,$3C,$0F,$00

; --- lt: 4 x 18 px ---
        .export _spr_lt_p0
_spr_lt_p0:            ; 1 байт/строку, 18 строк
        .byte $FC,$00
        .byte $F0,$00
        .byte $C0,$00
        .byte $C0,$00
        .byte $C0,$00
        .byte $00,$00
        .byte $00,$00
        .byte $00,$00
        .byte $00,$00
        .byte $00,$00
        .byte $00,$00
        .byte $00,$00
        .byte $00,$00
        .byte $C0,$00
        .byte $C0,$00
        .byte $C0,$00
        .byte $F0,$00
        .byte $F3,$00

        .export _spr_lt_p1
_spr_lt_p1:            ; 2 байт/строку, 18 строк
        .byte $FF,$00,$CF,$00
        .byte $FF,$00,$0F,$00
        .byte $FC,$00,$0F,$00
        .byte $FC,$00,$0F,$00
        .byte $FC,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $FC,$00,$0F,$00
        .byte $FC,$00,$0F,$00
        .byte $FC,$00,$0F,$00
        .byte $FF,$00,$0F,$00
        .byte $FF,$00,$3F,$00

; --- m0: 8 x 15 px ---
        .export _spr_m0_p0
_spr_m0_p0:            ; 2 байт/строку, 15 строк
        .byte $C3,$00,$C3,$00
        .byte $C0,$3C,$03,$3C
        .byte $00,$33,$00,$7C
        .byte $00,$33,$00,$7C
        .byte $00,$3F,$00,$7C
        .byte $C0,$3D,$03,$7C
        .byte $C0,$05,$03,$50
        .byte $C0,$05,$03,$50
        .byte $C0,$29,$03,$68
        .byte $00,$21,$00,$48
        .byte $00,$21,$00,$48
        .byte $00,$A8,$00,$08
        .byte $00,$A8,$C0,$08
        .byte $03,$00,$C0,$2A
        .byte $FF,$00,$00,$2A

        .export _spr_m0_p1
_spr_m0_p1:            ; 3 байт/строку, 15 строк
        .byte $FC,$00,$3C,$00,$3F,$00
        .byte $FC,$03,$00,$C3,$3F,$C0
        .byte $F0,$03,$00,$37,$0F,$C0
        .byte $F0,$03,$00,$37,$0F,$C0
        .byte $F0,$03,$00,$F7,$0F,$C0
        .byte $FC,$03,$00,$D7,$3F,$C0
        .byte $FC,$00,$00,$55,$3F,$00
        .byte $FC,$00,$00,$55,$3F,$00
        .byte $FC,$02,$00,$96,$3F,$80
        .byte $F0,$02,$00,$14,$0F,$80
        .byte $F0,$02,$00,$14,$0F,$80
        .byte $F0,$0A,$00,$80,$0F,$80
        .byte $F0,$0A,$0C,$80,$0F,$80
        .byte $F0,$00,$3C,$02,$0F,$A0
        .byte $FF,$00,$F0,$02,$0F,$A0

; --- m1: 8 x 15 px ---
        .export _spr_m1_p0
_spr_m1_p0:            ; 2 байт/строку, 15 строк
        .byte $C3,$00,$C3,$00
        .byte $C0,$3C,$03,$3C
        .byte $00,$3F,$00,$7C
        .byte $00,$33,$00,$7C
        .byte $00,$33,$00,$7C
        .byte $C0,$3D,$03,$7C
        .byte $C0,$05,$03,$50
        .byte $C0,$05,$03,$50
        .byte $C0,$29,$03,$68
        .byte $00,$21,$00,$48
        .byte $00,$21,$00,$48
        .byte $00,$20,$00,$08
        .byte $03,$A8,$C0,$2A
        .byte $03,$A8,$00,$2A
        .byte $03,$00,$C0,$00

        .export _spr_m1_p1
_spr_m1_p1:            ; 3 байт/строку, 15 строк
        .byte $FC,$00,$3C,$00,$3F,$00
        .byte $FC,$03,$00,$C3,$3F,$C0
        .byte $F0,$03,$00,$F7,$0F,$C0
        .byte $F0,$03,$00,$37,$0F,$C0
        .byte $F0,$03,$00,$37,$0F,$C0
        .byte $FC,$03,$00,$D7,$3F,$C0
        .byte $FC,$00,$00,$55,$3F,$00
        .byte $FC,$00,$00,$55,$3F,$00
        .byte $FC,$02,$00,$96,$3F,$80
        .byte $F0,$02,$00,$14,$0F,$80
        .byte $F0,$02,$00,$14,$0F,$80
        .byte $F0,$02,$00,$00,$0F,$80
        .byte $F0,$0A,$3C,$82,$0F,$A0
        .byte $F0,$0A,$30,$82,$0F,$A0
        .byte $F0,$00,$3C,$00,$0F,$00

; --- m2: 8 x 15 px ---
        .export _spr_m2_p0
_spr_m2_p0:            ; 2 байт/строку, 15 строк
        .byte $C3,$00,$C3,$00
        .byte $C0,$3C,$03,$3C
        .byte $00,$3F,$00,$7C
        .byte $00,$3F,$00,$4C
        .byte $00,$3F,$00,$4C
        .byte $C0,$3D,$03,$7C
        .byte $C0,$05,$03,$50
        .byte $C0,$05,$03,$50
        .byte $C0,$29,$03,$68
        .byte $00,$21,$00,$48
        .byte $00,$21,$00,$48
        .byte $00,$20,$00,$2A
        .byte $03,$20,$00,$2A
        .byte $03,$A8,$C0,$00
        .byte $00,$A8,$FF,$00

        .export _spr_m2_p1
_spr_m2_p1:            ; 3 байт/строку, 15 строк
        .byte $FC,$00,$3C,$00,$3F,$00
        .byte $FC,$03,$00,$C3,$3F,$C0
        .byte $F0,$03,$00,$F7,$0F,$C0
        .byte $F0,$03,$00,$F4,$0F,$C0
        .byte $F0,$03,$00,$F4,$0F,$C0
        .byte $FC,$03,$00,$D7,$3F,$C0
        .byte $FC,$00,$00,$55,$3F,$00
        .byte $FC,$00,$00,$55,$3F,$00
        .byte $FC,$02,$00,$96,$3F,$80
        .byte $F0,$02,$00,$14,$0F,$80
        .byte $F0,$02,$00,$14,$0F,$80
        .byte $F0,$02,$00,$02,$0F,$A0
        .byte $F0,$02,$30,$02,$0F,$A0
        .byte $F0,$0A,$3C,$80,$0F,$00
        .byte $F0,$0A,$0F,$80,$FF,$00

; --- m3: 8 x 15 px ---
        .export _spr_m3_p0
_spr_m3_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $C3,$00,$C3,$00
        .byte $C0,$3C,$03,$3C
        .byte $00,$3F,$00,$7C
        .byte $00,$3F,$00,$7C
        .byte $00,$3F,$00,$7C
        .byte $C0,$3D,$03,$7C
        .byte $C0,$05,$03,$50
        .byte $C0,$05,$03,$50
        .byte $C0,$29,$03,$68
        .byte $00,$21,$00,$48
        .byte $00,$A9,$00,$6A
        .byte $00,$A8,$00,$2A
        .byte $03,$00,$C0,$00

        .export _spr_m3_p1
_spr_m3_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FC,$00,$3C,$00,$3F,$00
        .byte $FC,$03,$00,$C3,$3F,$C0
        .byte $F0,$03,$00,$F7,$0F,$C0
        .byte $F0,$03,$00,$F7,$0F,$C0
        .byte $F0,$03,$00,$F7,$0F,$C0
        .byte $FC,$03,$00,$D7,$3F,$C0
        .byte $FC,$00,$00,$55,$3F,$00
        .byte $FC,$00,$00,$55,$3F,$00
        .byte $FC,$02,$00,$96,$3F,$80
        .byte $F0,$02,$00,$14,$0F,$80
        .byte $F0,$0A,$00,$96,$0F,$A0
        .byte $F0,$0A,$00,$82,$0F,$A0
        .byte $F0,$00,$3C,$00,$0F,$00

; --- nc: 8 x 10 px ---
        .export _spr_nc_p0
_spr_nc_p0:            ; 2 байт/строку, 10 строк
        .byte $FC,$00,$03,$00
        .byte $F0,$00,$03,$00
        .byte $F0,$00,$00,$00
        .byte $C0,$00,$00,$00
        .byte $F0,$00,$00,$00
        .byte $F0,$00,$03,$00
        .byte $FC,$00,$03,$00
        .byte $FC,$00,$0F,$00
        .byte $FF,$00,$0F,$00
        .byte $FF,$00,$3F,$00

        .export _spr_nc_p1
_spr_nc_p1:            ; 3 байт/строку, 10 строк
        .byte $FF,$00,$C0,$00,$3F,$00
        .byte $FF,$00,$00,$00,$3F,$00
        .byte $FF,$00,$00,$00,$0F,$00
        .byte $FC,$00,$00,$00,$0F,$00
        .byte $FF,$00,$00,$00,$0F,$00
        .byte $FF,$00,$00,$00,$3F,$00
        .byte $FF,$00,$C0,$00,$3F,$00
        .byte $FF,$00,$C0,$00,$FF,$00
        .byte $FF,$00,$F0,$00,$FF,$00
        .byte $FF,$00,$F3,$00,$FF,$00

; --- p1: 10 x 4 px ---
        .export _spr_p1_p0
_spr_p1_p0:            ; 3 байт/строку, 4 строк
        .byte $00,$BF,$00,$AF,$0F,$E0
        .byte $00,$EF,$00,$BB,$0F,$E0
        .byte $00,$EE,$00,$FB,$0F,$B0
        .byte $00,$FA,$00,$FE,$0F,$B0

        .export _spr_p1_p1
_spr_p1_p1:            ; 3 байт/строку, 4 строк
        .byte $F0,$0B,$00,$FA,$00,$FE
        .byte $F0,$0E,$00,$FB,$00,$BE
        .byte $F0,$0E,$00,$EF,$00,$BB
        .byte $F0,$0F,$00,$AF,$00,$EB

; --- p2: 10 x 4 px ---
        .export _spr_p2_p0
_spr_p2_p0:            ; 3 байт/строку, 4 строк
        .byte $00,$66,$00,$66,$0F,$60
        .byte $00,$99,$00,$99,$0F,$90
        .byte $00,$99,$00,$99,$0F,$90
        .byte $00,$99,$00,$99,$0F,$90

        .export _spr_p2_p1
_spr_p2_p1:            ; 3 байт/строку, 4 строк
        .byte $F0,$06,$00,$66,$00,$66
        .byte $F0,$09,$00,$99,$00,$99
        .byte $F0,$09,$00,$99,$00,$99
        .byte $F0,$09,$00,$99,$00,$99

; --- p3: 10 x 4 px ---
        .export _spr_p3_p0
_spr_p3_p0:            ; 3 байт/строку, 4 строк
        .byte $00,$BB,$00,$BB,$0F,$B0
        .byte $00,$EE,$00,$EE,$0F,$E0
        .byte $00,$EE,$00,$EE,$0F,$E0
        .byte $00,$BB,$00,$BB,$0F,$B0

        .export _spr_p3_p1
_spr_p3_p1:            ; 3 байт/строку, 4 строк
        .byte $F0,$0B,$00,$BB,$00,$BB
        .byte $F0,$0E,$00,$EE,$00,$EE
        .byte $F0,$0E,$00,$EE,$00,$EE
        .byte $F0,$0B,$00,$BB,$00,$BB

; --- p4: 10 x 4 px ---
        .export _spr_p4_p0
_spr_p4_p0:            ; 3 байт/строку, 4 строк
        .byte $00,$7F,$00,$5F,$0F,$D0
        .byte $00,$DF,$00,$77,$0F,$D0
        .byte $00,$DD,$00,$F7,$0F,$70
        .byte $00,$F5,$00,$FD,$0F,$70

        .export _spr_p4_p1
_spr_p4_p1:            ; 3 байт/строку, 4 строк
        .byte $F0,$07,$00,$F5,$00,$FD
        .byte $F0,$0D,$00,$F7,$00,$7D
        .byte $F0,$0D,$00,$DF,$00,$77
        .byte $F0,$0F,$00,$5F,$00,$D7

; --- p5: 10 x 4 px ---
        .export _spr_p5_p0
_spr_p5_p0:            ; 3 байт/строку, 4 строк
        .byte $00,$BF,$00,$AF,$0F,$E0
        .byte $00,$DF,$00,$77,$0F,$D0
        .byte $00,$EE,$00,$FB,$0F,$B0
        .byte $00,$F5,$00,$FD,$0F,$70

        .export _spr_p5_p1
_spr_p5_p1:            ; 3 байт/строку, 4 строк
        .byte $F0,$0B,$00,$FA,$00,$FE
        .byte $F0,$0D,$00,$F7,$00,$7D
        .byte $F0,$0E,$00,$EF,$00,$BB
        .byte $F0,$0F,$00,$5F,$00,$D7

; --- p6: 10 x 4 px ---
        .export _spr_p6_p0
_spr_p6_p0:            ; 3 байт/строку, 4 строк
        .byte $00,$66,$00,$66,$0F,$60
        .byte $00,$99,$00,$99,$0F,$90
        .byte $00,$66,$00,$66,$0F,$60
        .byte $00,$99,$00,$99,$0F,$90

        .export _spr_p6_p1
_spr_p6_p1:            ; 3 байт/строку, 4 строк
        .byte $F0,$06,$00,$66,$00,$66
        .byte $F0,$09,$00,$99,$00,$99
        .byte $F0,$06,$00,$66,$00,$66
        .byte $F0,$09,$00,$99,$00,$99

; --- p7: 10 x 4 px ---
        .export _spr_p7_p0
_spr_p7_p0:            ; 3 байт/строку, 4 строк
        .byte $00,$BB,$00,$BB,$0F,$B0
        .byte $00,$DD,$00,$DD,$0F,$D0
        .byte $00,$DD,$00,$DD,$0F,$D0
        .byte $00,$BB,$00,$BB,$0F,$B0

        .export _spr_p7_p1
_spr_p7_p1:            ; 3 байт/строку, 4 строк
        .byte $F0,$0B,$00,$BB,$00,$BB
        .byte $F0,$0D,$00,$DD,$00,$DD
        .byte $F0,$0D,$00,$DD,$00,$DD
        .byte $F0,$0B,$00,$BB,$00,$BB

; --- p8: 10 x 4 px ---
        .export _spr_p8_p0
_spr_p8_p0:            ; 3 байт/строку, 4 строк
        .byte $00,$EE,$00,$EE,$0F,$E0
        .byte $00,$BB,$00,$BB,$0F,$B0
        .byte $00,$BB,$00,$BB,$0F,$B0
        .byte $00,$BB,$00,$BB,$0F,$B0

        .export _spr_p8_p1
_spr_p8_p1:            ; 3 байт/строку, 4 строк
        .byte $F0,$0E,$00,$EE,$00,$EE
        .byte $F0,$0B,$00,$BB,$00,$BB
        .byte $F0,$0B,$00,$BB,$00,$BB
        .byte $F0,$0B,$00,$BB,$00,$BB

; --- r0: 8 x 15 px ---
        .export _spr_r0_p0
_spr_r0_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FC,$00,$FF,$00
        .byte $F0,$03,$33,$00
        .byte $F0,$0F,$00,$C4
        .byte $C0,$0C,$00,$D4
        .byte $C0,$2A,$03,$94
        .byte $00,$2A,$03,$90
        .byte $00,$AA,$03,$90
        .byte $00,$AA,$03,$90
        .byte $00,$FA,$03,$94
        .byte $00,$FB,$00,$D4
        .byte $00,$F3,$00,$C4
        .byte $00,$F3,$03,$C0
        .byte $0C,$03,$3F,$C0

        .export _spr_r0_p1
_spr_r0_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$CF,$00,$FF,$00
        .byte $FF,$00,$03,$30,$3F,$00
        .byte $FF,$00,$00,$FC,$0F,$40
        .byte $FC,$00,$00,$CD,$0F,$40
        .byte $FC,$02,$00,$A9,$3F,$40
        .byte $F0,$02,$00,$A9,$3F,$00
        .byte $F0,$0A,$00,$A9,$3F,$00
        .byte $F0,$0A,$00,$A9,$3F,$00
        .byte $F0,$0F,$00,$A9,$3F,$40
        .byte $F0,$0F,$00,$BD,$0F,$40
        .byte $F0,$0F,$00,$3C,$0F,$40
        .byte $F0,$0F,$00,$3C,$3F,$00
        .byte $F0,$00,$C3,$3C,$FF,$00

        .segment "SHADOW_RAM2"

; --- r1: 8 x 15 px ---
        .export _spr_r1_p0
_spr_r1_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FC,$00,$FF,$00
        .byte $F0,$03,$3F,$00
        .byte $F0,$0F,$3F,$C0
        .byte $F0,$0C,$30,$C0
        .byte $C0,$0C,$00,$C5
        .byte $C0,$2A,$00,$95
        .byte $00,$2A,$00,$94
        .byte $00,$AA,$03,$90
        .byte $00,$AA,$00,$94
        .byte $00,$AB,$00,$D5
        .byte $00,$FB,$00,$C5
        .byte $00,$F3,$00,$C0
        .byte $00,$F3,$3F,$C0
        .byte $0C,$F0,$3F,$00

        .export _spr_r1_p1
_spr_r1_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$CF,$00,$FF,$00
        .byte $FF,$00,$03,$30,$FF,$00
        .byte $FF,$00,$03,$FC,$FF,$00
        .byte $FF,$00,$03,$CC,$0F,$00
        .byte $FC,$00,$00,$CC,$0F,$50
        .byte $FC,$02,$00,$A9,$0F,$50
        .byte $F0,$02,$00,$A9,$0F,$40
        .byte $F0,$0A,$00,$A9,$3F,$00
        .byte $F0,$0A,$00,$A9,$0F,$40
        .byte $F0,$0A,$00,$BD,$0F,$50
        .byte $F0,$0F,$00,$BC,$0F,$50
        .byte $F0,$0F,$00,$3C,$0F,$00
        .byte $F0,$0F,$03,$3C,$FF,$00
        .byte $F0,$0F,$C3,$00,$FF,$00

; --- r2: 8 x 15 px ---
        .export _spr_r2_p0
_spr_r2_p0:            ; 2 байт/строку, 15 строк
        .byte $FC,$00,$FF,$00
        .byte $F0,$03,$3F,$00
        .byte $F0,$0F,$3F,$C0
        .byte $F0,$0C,$3F,$C0
        .byte $F0,$0C,$3F,$C0
        .byte $C0,$0C,$00,$C0
        .byte $C0,$2A,$00,$95
        .byte $00,$2A,$00,$95
        .byte $00,$AA,$00,$90
        .byte $00,$AA,$00,$95
        .byte $00,$FA,$00,$95
        .byte $00,$FB,$00,$C0
        .byte $00,$F3,$0F,$C0
        .byte $00,$F3,$0F,$C0
        .byte $0C,$03,$3F,$C0

        .export _spr_r2_p1
_spr_r2_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$CF,$00,$FF,$00
        .byte $FF,$00,$03,$30,$FF,$00
        .byte $FF,$00,$03,$FC,$FF,$00
        .byte $FF,$00,$03,$CC,$FF,$00
        .byte $FF,$00,$03,$CC,$FF,$00
        .byte $FC,$00,$00,$CC,$0F,$00
        .byte $FC,$02,$00,$A9,$0F,$50
        .byte $F0,$02,$00,$A9,$0F,$50
        .byte $F0,$0A,$00,$A9,$0F,$00
        .byte $F0,$0A,$00,$A9,$0F,$50
        .byte $F0,$0F,$00,$A9,$0F,$50
        .byte $F0,$0F,$00,$BC,$0F,$00
        .byte $F0,$0F,$00,$3C,$FF,$00
        .byte $F0,$0F,$00,$3C,$FF,$00
        .byte $F0,$00,$C3,$3C,$FF,$00

; --- r4: 8 x 15 px ---
        .export _spr_r4_p0
_spr_r4_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FC,$00,$F3,$00
        .byte $F0,$03,$00,$04
        .byte $C0,$0F,$00,$D4
        .byte $C0,$2A,$03,$94
        .byte $00,$2A,$03,$90
        .byte $00,$AA,$03,$90
        .byte $00,$AA,$03,$90
        .byte $00,$FA,$03,$94
        .byte $00,$FB,$00,$D4
        .byte $00,$F3,$00,$C4
        .byte $00,$F3,$03,$C0
        .byte $0C,$03,$3F,$C0

        .export _spr_r4_p1
_spr_r4_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$CF,$00,$3F,$00
        .byte $FF,$00,$00,$30,$0F,$40
        .byte $FC,$00,$00,$FD,$0F,$40
        .byte $FC,$02,$00,$A9,$3F,$40
        .byte $F0,$02,$00,$A9,$3F,$00
        .byte $F0,$0A,$00,$A9,$3F,$00
        .byte $F0,$0A,$00,$A9,$3F,$00
        .byte $F0,$0F,$00,$A9,$3F,$40
        .byte $F0,$0F,$00,$BD,$0F,$40
        .byte $F0,$0F,$00,$3C,$0F,$40
        .byte $F0,$0F,$00,$3C,$3F,$00
        .byte $F0,$00,$C3,$3C,$FF,$00

; --- r5: 8 x 15 px ---
        .export _spr_r5_p0
_spr_r5_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FC,$00,$FF,$00
        .byte $F0,$03,$30,$00
        .byte $C0,$0F,$00,$C5
        .byte $C0,$2A,$00,$95
        .byte $00,$2A,$00,$94
        .byte $00,$AA,$03,$90
        .byte $00,$AA,$00,$94
        .byte $00,$AB,$00,$D5
        .byte $00,$FB,$00,$C5
        .byte $00,$F3,$00,$C0
        .byte $00,$F3,$3F,$C0
        .byte $0C,$F0,$3F,$00

        .export _spr_r5_p1
_spr_r5_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$CF,$00,$FF,$00
        .byte $FF,$00,$03,$30,$0F,$00
        .byte $FC,$00,$00,$FC,$0F,$50
        .byte $FC,$02,$00,$A9,$0F,$50
        .byte $F0,$02,$00,$A9,$0F,$40
        .byte $F0,$0A,$00,$A9,$3F,$00
        .byte $F0,$0A,$00,$A9,$0F,$40
        .byte $F0,$0A,$00,$BD,$0F,$50
        .byte $F0,$0F,$00,$BC,$0F,$50
        .byte $F0,$0F,$00,$3C,$0F,$00
        .byte $F0,$0F,$03,$3C,$FF,$00
        .byte $F0,$0F,$C3,$00,$FF,$00

; --- r6: 8 x 15 px ---
        .export _spr_r6_p0
_spr_r6_p0:            ; 2 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00
        .byte $FC,$00,$FF,$00
        .byte $F0,$03,$3F,$00
        .byte $C0,$0F,$00,$C0
        .byte $C0,$2A,$00,$95
        .byte $00,$2A,$00,$95
        .byte $00,$AA,$00,$90
        .byte $00,$AA,$00,$95
        .byte $00,$FA,$00,$95
        .byte $00,$FB,$00,$C0
        .byte $00,$F3,$0F,$C0
        .byte $00,$F3,$0F,$C0
        .byte $0C,$03,$3F,$C0

        .export _spr_r6_p1
_spr_r6_p1:            ; 3 байт/строку, 15 строк
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$FF,$00,$FF,$00
        .byte $FF,$00,$CF,$00,$FF,$00
        .byte $FF,$00,$03,$30,$FF,$00
        .byte $FC,$00,$00,$FC,$0F,$00
        .byte $FC,$02,$00,$A9,$0F,$50
        .byte $F0,$02,$00,$A9,$0F,$50
        .byte $F0,$0A,$00,$A9,$0F,$00
        .byte $F0,$0A,$00,$A9,$0F,$50
        .byte $F0,$0F,$00,$A9,$0F,$50
        .byte $F0,$0F,$00,$BC,$0F,$00
        .byte $F0,$0F,$00,$3C,$FF,$00
        .byte $F0,$0F,$00,$3C,$FF,$00
        .byte $F0,$00,$C3,$3C,$FF,$00

; --- rt: 4 x 18 px ---
        .export _spr_rt_p0
_spr_rt_p0:            ; 1 байт/строку, 18 строк
        .byte $CF,$00
        .byte $0F,$00
        .byte $03,$00
        .byte $03,$00
        .byte $03,$00
        .byte $00,$00
        .byte $00,$00
        .byte $00,$00
        .byte $00,$00
        .byte $00,$00
        .byte $00,$00
        .byte $00,$00
        .byte $00,$00
        .byte $03,$00
        .byte $03,$00
        .byte $03,$00
        .byte $0F,$00
        .byte $3F,$00

        .export _spr_rt_p1
_spr_rt_p1:            ; 2 байт/строку, 18 строк
        .byte $FC,$00,$FF,$00
        .byte $F0,$00,$FF,$00
        .byte $F0,$00,$3F,$00
        .byte $F0,$00,$3F,$00
        .byte $F0,$00,$3F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$0F,$00
        .byte $F0,$00,$3F,$00
        .byte $F0,$00,$3F,$00
        .byte $F0,$00,$3F,$00
        .byte $F0,$00,$FF,$00
        .byte $F3,$00,$FF,$00

; --- s0: 8 x 12 px ---
        .export _spr_s0_p0
_spr_s0_p0:            ; 2 байт/строку, 12 строк
        .byte $F0,$03,$3F,$00
        .byte $F0,$0F,$3F,$C0
        .byte $C0,$0C,$00,$C0
        .byte $C0,$2A,$00,$95
        .byte $00,$2A,$00,$95
        .byte $00,$AA,$00,$90
        .byte $00,$AA,$00,$95
        .byte $00,$AA,$00,$95
        .byte $00,$FB,$00,$C0
        .byte $00,$F3,$0F,$C0
        .byte $00,$F3,$0F,$C0
        .byte $0C,$F3,$3F,$C0

        .export _spr_s0_p1
_spr_s0_p1:            ; 3 байт/строку, 12 строк
        .byte $FF,$00,$03,$30,$FF,$00
        .byte $FF,$00,$03,$FC,$FF,$00
        .byte $FC,$00,$00,$CC,$0F,$00
        .byte $FC,$02,$00,$A9,$0F,$50
        .byte $F0,$02,$00,$A9,$0F,$50
        .byte $F0,$0A,$00,$A9,$0F,$00
        .byte $F0,$0A,$00,$A9,$0F,$50
        .byte $F0,$0A,$00,$A9,$0F,$50
        .byte $F0,$0F,$00,$BC,$0F,$00
        .byte $F0,$0F,$00,$3C,$FF,$00
        .byte $F0,$0F,$00,$3C,$FF,$00
        .byte $F0,$0F,$C3,$3C,$FF,$00

; --- u0: 8 x 15 px ---
        .export _spr_u0_p0
_spr_u0_p0:            ; 2 байт/строку, 15 строк
        .byte $CF,$00,$0F,$00
        .byte $03,$10,$0F,$50
        .byte $00,$14,$0F,$50
        .byte $C0,$15,$0F,$40
        .byte $00,$05,$3F,$40
        .byte $00,$FA,$0F,$80
        .byte $00,$FA,$0F,$B0
        .byte $00,$FA,$03,$B0
        .byte $00,$FA,$03,$B0
        .byte $00,$0A,$0F,$B0
        .byte $C0,$0A,$0F,$80
        .byte $00,$3A,$3F,$80
        .byte $00,$CE,$3F,$00
        .byte $00,$CE,$FF,$00
        .byte $00,$38,$FF,$00

        .export _spr_u0_p1
_spr_u0_p1:            ; 3 байт/строку, 15 строк
        .byte $FC,$00,$F0,$00,$FF,$00
        .byte $F0,$01,$30,$05,$FF,$00
        .byte $F0,$01,$00,$45,$FF,$00
        .byte $FC,$01,$00,$54,$FF,$00
        .byte $F0,$00,$03,$54,$FF,$00
        .byte $F0,$0F,$00,$A8,$FF,$00
        .byte $F0,$0F,$00,$AB,$FF,$00
        .byte $F0,$0F,$00,$AB,$3F,$00
        .byte $F0,$0F,$00,$AB,$3F,$00
        .byte $F0,$00,$00,$AB,$FF,$00
        .byte $FC,$00,$00,$A8,$FF,$00
        .byte $F0,$03,$03,$A8,$FF,$00
        .byte $F0,$0C,$03,$E0,$FF,$00
        .byte $F0,$0C,$0F,$E0,$FF,$00
        .byte $F0,$03,$0F,$80,$FF,$00

; --- u1: 8 x 15 px ---
        .export _spr_u1_p0
_spr_u1_p0:            ; 2 байт/строку, 15 строк
        .byte $C0,$14,$0F,$40
        .byte $C0,$14,$0F,$40
        .byte $C0,$15,$0F,$40
        .byte $C0,$15,$0F,$40
        .byte $C0,$05,$3F,$40
        .byte $00,$3A,$0F,$80
        .byte $00,$CE,$03,$B0
        .byte $00,$CE,$03,$BC
        .byte $00,$3A,$03,$BC
        .byte $C0,$0A,$03,$B0
        .byte $00,$0A,$0F,$80
        .byte $00,$FA,$3F,$80
        .byte $00,$FA,$3F,$00
        .byte $00,$FA,$FF,$00
        .byte $03,$F8,$FF,$00

        .export _spr_u1_p1
_spr_u1_p1:            ; 3 байт/строку, 15 строк
        .byte $FC,$01,$00,$44,$FF,$00
        .byte $FC,$01,$00,$44,$FF,$00
        .byte $FC,$01,$00,$54,$FF,$00
        .byte $FC,$01,$00,$54,$FF,$00
        .byte $FC,$00,$03,$54,$FF,$00
        .byte $F0,$03,$00,$A8,$FF,$00
        .byte $F0,$0C,$00,$EB,$3F,$00
        .byte $F0,$0C,$00,$EB,$3F,$C0
        .byte $F0,$03,$00,$AB,$3F,$C0
        .byte $FC,$00,$00,$AB,$3F,$00
        .byte $F0,$00,$00,$A8,$FF,$00
        .byte $F0,$0F,$03,$A8,$FF,$00
        .byte $F0,$0F,$03,$A0,$FF,$00
        .byte $F0,$0F,$0F,$A0,$FF,$00
        .byte $F0,$0F,$3F,$80,$FF,$00

; --- u2: 8 x 15 px ---
        .export _spr_u2_p0
_spr_u2_p0:            ; 2 байт/строку, 15 строк
        .byte $C0,$05,$3F,$40
        .byte $C0,$05,$3F,$40
        .byte $C0,$05,$3F,$40
        .byte $C0,$05,$3F,$40
        .byte $00,$05,$3F,$00
        .byte $00,$FA,$03,$80
        .byte $00,$FA,$03,$BC
        .byte $00,$FA,$00,$8C
        .byte $00,$FA,$00,$8C
        .byte $00,$0A,$03,$BC
        .byte $C0,$0A,$03,$80
        .byte $00,$3A,$3F,$80
        .byte $00,$CE,$3F,$00
        .byte $00,$CE,$FF,$00
        .byte $00,$38,$FF,$00

        .export _spr_u2_p1
_spr_u2_p1:            ; 3 байт/строку, 15 строк
        .byte $FC,$00,$03,$54,$FF,$00
        .byte $FC,$00,$03,$54,$FF,$00
        .byte $FC,$00,$03,$54,$FF,$00
        .byte $FC,$00,$03,$54,$FF,$00
        .byte $F0,$00,$03,$50,$FF,$00
        .byte $F0,$0F,$00,$A8,$3F,$00
        .byte $F0,$0F,$00,$AB,$3F,$C0
        .byte $F0,$0F,$00,$A8,$0F,$C0
        .byte $F0,$0F,$00,$A8,$0F,$C0
        .byte $F0,$00,$00,$AB,$3F,$C0
        .byte $FC,$00,$00,$A8,$3F,$00
        .byte $F0,$03,$03,$A8,$FF,$00
        .byte $F0,$0C,$03,$E0,$FF,$00
        .byte $F0,$0C,$0F,$E0,$FF,$00
        .byte $F0,$03,$0F,$80,$FF,$00

; --- u4: 8 x 15 px ---
        .export _spr_u4_p0
_spr_u4_p0:            ; 2 байт/строку, 15 строк
        .byte $CF,$00,$0F,$00
        .byte $03,$10,$0F,$50
        .byte $00,$14,$0F,$50
        .byte $C0,$15,$0F,$40
        .byte $00,$05,$3F,$40
        .byte $00,$FA,$3F,$80
        .byte $00,$FA,$0F,$80
        .byte $00,$FA,$0F,$B0
        .byte $00,$FA,$0F,$B0
        .byte $00,$0A,$0F,$80
        .byte $C0,$0A,$3F,$80
        .byte $00,$3A,$3F,$80
        .byte $00,$CE,$3F,$00
        .byte $00,$CE,$FF,$00
        .byte $00,$38,$FF,$00

        .export _spr_u4_p1
_spr_u4_p1:            ; 3 байт/строку, 15 строк
        .byte $FC,$00,$F0,$00,$FF,$00
        .byte $F0,$01,$30,$05,$FF,$00
        .byte $F0,$01,$00,$45,$FF,$00
        .byte $FC,$01,$00,$54,$FF,$00
        .byte $F0,$00,$03,$54,$FF,$00
        .byte $F0,$0F,$03,$A8,$FF,$00
        .byte $F0,$0F,$00,$A8,$FF,$00
        .byte $F0,$0F,$00,$AB,$FF,$00
        .byte $F0,$0F,$00,$AB,$FF,$00
        .byte $F0,$00,$00,$A8,$FF,$00
        .byte $FC,$00,$03,$A8,$FF,$00
        .byte $F0,$03,$03,$A8,$FF,$00
        .byte $F0,$0C,$03,$E0,$FF,$00
        .byte $F0,$0C,$0F,$E0,$FF,$00
        .byte $F0,$03,$0F,$80,$FF,$00

; --- u5: 8 x 15 px ---
        .export _spr_u5_p0
_spr_u5_p0:            ; 2 байт/строку, 15 строк
        .byte $C0,$14,$0F,$40
        .byte $C0,$14,$0F,$40
        .byte $C0,$15,$0F,$40
        .byte $C0,$15,$0F,$40
        .byte $C0,$05,$3F,$40
        .byte $00,$3A,$3F,$80
        .byte $00,$CE,$0F,$80
        .byte $00,$CE,$0F,$B0
        .byte $00,$3A,$0F,$B0
        .byte $C0,$0A,$0F,$80
        .byte $00,$0A,$3F,$80
        .byte $00,$FA,$3F,$80
        .byte $00,$FA,$3F,$00
        .byte $00,$FA,$FF,$00
        .byte $00,$F8,$FF,$00

        .export _spr_u5_p1
_spr_u5_p1:            ; 3 байт/строку, 15 строк
        .byte $FC,$01,$00,$44,$FF,$00
        .byte $FC,$01,$00,$44,$FF,$00
        .byte $FC,$01,$00,$54,$FF,$00
        .byte $FC,$01,$00,$54,$FF,$00
        .byte $FC,$00,$03,$54,$FF,$00
        .byte $F0,$03,$03,$A8,$FF,$00
        .byte $F0,$0C,$00,$E8,$FF,$00
        .byte $F0,$0C,$00,$EB,$FF,$00
        .byte $F0,$03,$00,$AB,$FF,$00
        .byte $FC,$00,$00,$A8,$FF,$00
        .byte $F0,$00,$03,$A8,$FF,$00
        .byte $F0,$0F,$03,$A8,$FF,$00
        .byte $F0,$0F,$03,$A0,$FF,$00
        .byte $F0,$0F,$0F,$A0,$FF,$00
        .byte $F0,$0F,$0F,$80,$FF,$00

; --- u6: 8 x 15 px ---
        .export _spr_u6_p0
_spr_u6_p0:            ; 2 байт/строку, 15 строк
        .byte $C0,$05,$3F,$40
        .byte $C0,$05,$3F,$40
        .byte $C0,$05,$3F,$40
        .byte $C0,$05,$3F,$40
        .byte $00,$05,$3F,$00
        .byte $00,$FA,$3F,$80
        .byte $00,$FA,$0F,$80
        .byte $00,$FA,$0F,$B0
        .byte $00,$FA,$0F,$B0
        .byte $00,$0A,$0F,$80
        .byte $C0,$0A,$3F,$80
        .byte $00,$3A,$3F,$80
        .byte $00,$CE,$3F,$00
        .byte $00,$CE,$FF,$00
        .byte $00,$38,$FF,$00

        .export _spr_u6_p1
_spr_u6_p1:            ; 3 байт/строку, 15 строк
        .byte $FC,$00,$03,$54,$FF,$00
        .byte $FC,$00,$03,$54,$FF,$00
        .byte $FC,$00,$03,$54,$FF,$00
        .byte $FC,$00,$03,$54,$FF,$00
        .byte $F0,$00,$03,$50,$FF,$00
        .byte $F0,$0F,$03,$A8,$FF,$00
        .byte $F0,$0F,$00,$A8,$FF,$00
        .byte $F0,$0F,$00,$AB,$FF,$00
        .byte $F0,$0F,$00,$AB,$FF,$00
        .byte $F0,$00,$00,$A8,$FF,$00
        .byte $FC,$00,$03,$A8,$FF,$00
        .byte $F0,$03,$03,$A8,$FF,$00
        .byte $F0,$0C,$03,$E0,$FF,$00
        .byte $F0,$0C,$0F,$E0,$FF,$00
        .byte $F0,$03,$0F,$80,$FF,$00

; --- up: 12 x 6 px ---
        .export _spr_up_p0
_spr_up_p0:            ; 3 байт/строку, 6 строк
        .byte $FF,$00,$00,$00,$FF,$00
        .byte $FC,$00,$00,$00,$3F,$00
        .byte $FC,$00,$00,$00,$0F,$00
        .byte $C0,$00,$00,$00,$03,$00
        .byte $C0,$00,$00,$00,$0F,$00
        .byte $F0,$00,$00,$00,$03,$00

        .export _spr_up_p1
_spr_up_p1:            ; 4 байт/строку, 6 строк
        .byte $FF,$00,$F0,$00,$0F,$00,$FF,$00
        .byte $FF,$00,$C0,$00,$03,$00,$FF,$00
        .byte $FF,$00,$C0,$00,$00,$00,$FF,$00
        .byte $FC,$00,$00,$00,$00,$00,$3F,$00
        .byte $FC,$00,$00,$00,$00,$00,$FF,$00
        .byte $FF,$00,$00,$00,$00,$00,$3F,$00

        .segment "RODATA"

; --- z1: 8 x 15 px ---
        .export _spr_z1_p0
_spr_z1_p0:            ; 2 байт/строку, 15 строк
        .byte $00,$00,$00,$50
        .byte $00,$01,$00,$50
        .byte $00,$01,$00,$50
        .byte $00,$01,$00,$14
        .byte $00,$01,$00,$04
        .byte $00,$05,$00,$15
        .byte $00,$05,$00,$15
        .byte $00,$15,$00,$04
        .byte $00,$15,$00,$28
        .byte $00,$28,$00,$2E
        .byte $00,$2E,$00,$BA
        .byte $00,$AA,$00,$AA
        .byte $00,$AA,$00,$2A
        .byte $00,$2A,$00,$28
        .byte $00,$28,$00,$00

        .export _spr_z1_p1
_spr_z1_p1:            ; 3 байт/строку, 15 строк
        .byte $F0,$00,$00,$05,$0F,$00
        .byte $F0,$00,$00,$15,$0F,$00
        .byte $F0,$00,$00,$15,$0F,$00
        .byte $F0,$00,$00,$11,$0F,$40
        .byte $F0,$00,$00,$10,$0F,$40
        .byte $F0,$00,$00,$51,$0F,$50
        .byte $F0,$00,$00,$51,$0F,$50
        .byte $F0,$01,$00,$50,$0F,$40
        .byte $F0,$01,$00,$52,$0F,$80
        .byte $F0,$02,$00,$82,$0F,$E0
        .byte $F0,$02,$00,$EB,$0F,$A0
        .byte $F0,$0A,$00,$AA,$0F,$A0
        .byte $F0,$0A,$00,$A2,$0F,$A0
        .byte $F0,$02,$00,$A2,$0F,$80
        .byte $F0,$02,$00,$80,$0F,$00

