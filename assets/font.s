; font.s - СГЕНЕРИРОВАНО tools/font2c.py, не править вручную.
; Цифры счёта из оригинала (через порт на БК-0010).
; Каждая цифра - 12x12 пикселей: 3 байта на строку, 12 строк.
; Порядок пикселей в байте переставлен под ANTIC E.

        .segment "RODATA"

        .export _font_digits
_font_digits:
; --- 0 ---
        .byte $3F,$FF,$C0
        .byte $FF,$FF,$F0
        .byte $F0,$00,$F0
        .byte $F0,$00,$F0
        .byte $F0,$00,$F0
        .byte $F0,$0F,$F0
        .byte $F0,$0F,$F0
        .byte $F0,$0F,$F0
        .byte $F0,$0F,$F0
        .byte $F0,$0F,$F0
        .byte $FF,$FF,$F0
        .byte $3F,$FF,$C0
; --- 1 ---
        .byte $00,$3C,$00
        .byte $00,$3C,$00
        .byte $00,$3C,$00
        .byte $00,$3C,$00
        .byte $00,$3C,$00
        .byte $00,$FC,$00
        .byte $00,$FC,$00
        .byte $00,$FC,$00
        .byte $00,$FC,$00
        .byte $00,$FC,$00
        .byte $00,$FC,$00
        .byte $00,$FC,$00
; --- 2 ---
        .byte $3F,$FF,$C0
        .byte $FF,$FF,$F0
        .byte $F0,$00,$F0
        .byte $00,$00,$F0
        .byte $00,$00,$F0
        .byte $00,$00,$F0
        .byte $3F,$FF,$F0
        .byte $FF,$FF,$C0
        .byte $FF,$00,$00
        .byte $FF,$00,$00
        .byte $FF,$FF,$F0
        .byte $3F,$FF,$F0
; --- 3 ---
        .byte $3F,$FC,$00
        .byte $FF,$FF,$00
        .byte $F0,$0F,$00
        .byte $F0,$0F,$00
        .byte $00,$0F,$00
        .byte $0F,$FF,$C0
        .byte $0F,$FF,$F0
        .byte $00,$03,$F0
        .byte $F0,$03,$F0
        .byte $F0,$03,$F0
        .byte $FF,$FF,$F0
        .byte $3F,$FF,$C0
; --- 4 ---
        .byte $F0,$00,$00
        .byte $F0,$00,$00
        .byte $F0,$0F,$00
        .byte $F0,$0F,$00
        .byte $F0,$0F,$00
        .byte $F0,$0F,$00
        .byte $F0,$0F,$00
        .byte $FF,$FF,$F0
        .byte $3F,$FF,$F0
        .byte $00,$3F,$00
        .byte $00,$3F,$00
        .byte $00,$3F,$00
; --- 5 ---
        .byte $3F,$FF,$00
        .byte $FF,$FF,$00
        .byte $F0,$00,$00
        .byte $F0,$00,$00
        .byte $F0,$00,$00
        .byte $FF,$FF,$C0
        .byte $3F,$FF,$F0
        .byte $00,$0F,$F0
        .byte $F0,$0F,$F0
        .byte $F0,$0F,$F0
        .byte $FF,$FF,$F0
        .byte $3F,$FF,$C0
; --- 6 ---
        .byte $3F,$FF,$C0
        .byte $FF,$FF,$F0
        .byte $F0,$00,$F0
        .byte $F0,$00,$F0
        .byte $F0,$00,$00
        .byte $FF,$FF,$C0
        .byte $FF,$FF,$F0
        .byte $F0,$0F,$F0
        .byte $F0,$0F,$F0
        .byte $F0,$0F,$F0
        .byte $FF,$FF,$F0
        .byte $3F,$FF,$C0
; --- 7 ---
        .byte $3F,$FF,$C0
        .byte $3F,$FF,$F0
        .byte $00,$00,$F0
        .byte $00,$00,$F0
        .byte $00,$00,$F0
        .byte $00,$00,$F0
        .byte $00,$03,$F0
        .byte $00,$03,$F0
        .byte $00,$03,$F0
        .byte $00,$03,$F0
        .byte $00,$03,$F0
        .byte $00,$03,$F0
; --- 8 ---
        .byte $03,$FF,$00
        .byte $0F,$FF,$C0
        .byte $0F,$03,$C0
        .byte $0F,$03,$C0
        .byte $0F,$03,$C0
        .byte $0F,$FF,$C0
        .byte $3F,$FF,$F0
        .byte $F0,$03,$F0
        .byte $F0,$03,$F0
        .byte $F0,$03,$F0
        .byte $FF,$FF,$F0
        .byte $3F,$FF,$C0
; --- 9 ---
        .byte $3F,$FF,$C0
        .byte $FF,$FF,$F0
        .byte $F0,$00,$F0
        .byte $F0,$00,$F0
        .byte $F0,$00,$F0
        .byte $FF,$FF,$F0
        .byte $3F,$FF,$F0
        .byte $00,$0F,$F0
        .byte $00,$0F,$F0
        .byte $00,$0F,$F0
        .byte $00,$0F,$F0
        .byte $00,$0F,$F0

