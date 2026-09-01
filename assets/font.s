; font.s - СГЕНЕРИРОВАНО tools/font2c.py, не править вручную.
; Цифры счёта из оригинала (через порт на БК-0010).
; Цифра 12 точек в ячейке 16 точек: 2 байта на строку, 12 строк.
; Один бит на точку (ANTIC F), старший бит слева.

        .segment "RODATA"

        .export _font_digits
_font_digits:
; --- 0 ---
        .byte $1F,$E0
        .byte $3F,$F0
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$F0
        .byte $30,$F0
        .byte $30,$F0
        .byte $30,$F0
        .byte $30,$F0
        .byte $3F,$F0
        .byte $1F,$E0
; --- 1 ---
        .byte $01,$80
        .byte $01,$80
        .byte $01,$80
        .byte $01,$80
        .byte $01,$80
        .byte $03,$80
        .byte $03,$80
        .byte $03,$80
        .byte $03,$80
        .byte $03,$80
        .byte $03,$80
        .byte $03,$80
; --- 2 ---
        .byte $1F,$E0
        .byte $3F,$F0
        .byte $30,$30
        .byte $00,$30
        .byte $00,$30
        .byte $00,$30
        .byte $1F,$F0
        .byte $3F,$E0
        .byte $3C,$00
        .byte $3C,$00
        .byte $3F,$F0
        .byte $1F,$F0
; --- 3 ---
        .byte $1F,$80
        .byte $3F,$C0
        .byte $30,$C0
        .byte $30,$C0
        .byte $00,$C0
        .byte $0F,$E0
        .byte $0F,$F0
        .byte $00,$70
        .byte $30,$70
        .byte $30,$70
        .byte $3F,$F0
        .byte $1F,$E0
; --- 4 ---
        .byte $30,$00
        .byte $30,$00
        .byte $30,$C0
        .byte $30,$C0
        .byte $30,$C0
        .byte $30,$C0
        .byte $30,$C0
        .byte $3F,$F0
        .byte $1F,$F0
        .byte $01,$C0
        .byte $01,$C0
        .byte $01,$C0
; --- 5 ---
        .byte $1F,$C0
        .byte $3F,$C0
        .byte $30,$00
        .byte $30,$00
        .byte $30,$00
        .byte $3F,$E0
        .byte $1F,$F0
        .byte $00,$F0
        .byte $30,$F0
        .byte $30,$F0
        .byte $3F,$F0
        .byte $1F,$E0
; --- 6 ---
        .byte $1F,$E0
        .byte $3F,$F0
        .byte $30,$30
        .byte $30,$30
        .byte $30,$00
        .byte $3F,$E0
        .byte $3F,$F0
        .byte $30,$F0
        .byte $30,$F0
        .byte $30,$F0
        .byte $3F,$F0
        .byte $1F,$E0
; --- 7 ---
        .byte $1F,$E0
        .byte $1F,$F0
        .byte $00,$30
        .byte $00,$30
        .byte $00,$30
        .byte $00,$30
        .byte $00,$70
        .byte $00,$70
        .byte $00,$70
        .byte $00,$70
        .byte $00,$70
        .byte $00,$70
; --- 8 ---
        .byte $07,$C0
        .byte $0F,$E0
        .byte $0C,$60
        .byte $0C,$60
        .byte $0C,$60
        .byte $0F,$E0
        .byte $1F,$F0
        .byte $30,$70
        .byte $30,$70
        .byte $30,$70
        .byte $3F,$F0
        .byte $1F,$E0
; --- 9 ---
        .byte $1F,$E0
        .byte $3F,$F0
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $3F,$F0
        .byte $1F,$F0
        .byte $00,$F0
        .byte $00,$F0
        .byte $00,$F0
        .byte $00,$F0
        .byte $00,$F0

