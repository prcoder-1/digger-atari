; font.s - СГЕНЕРИРОВАНО tools/font2c.py, не править вручную.
; Цифры счёта из оригинала (через порт на БК-0010).
; Цифра 12 точек в ячейке 16 точек: 2 байта на строку, 12 строк.
; Один бит на точку (ANTIC F), старший бит слева.

; Шрифт уехал под ПЗУ вместе с картами уровней: MAIN упёрся в потолок.
; Переносит их copy_levels (src/util.s), сегмент общий.
        .segment "LEVELS"

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


; Буквы того же шрифта, прямо из оригинала (CLib/CHAR__X.CHR).
; Порядок - как в строке ниже; смотри font_letter в src/text.c.
        .export _font_alpha
_font_alpha:
; --- A ---
        .byte $0F,$C0
        .byte $1F,$E0
        .byte $18,$60
        .byte $18,$60
        .byte $18,$60
        .byte $1F,$E0
        .byte $3F,$F0
        .byte $38,$30
        .byte $38,$30
        .byte $38,$30
        .byte $38,$30
        .byte $38,$30
; --- E ---
        .byte $1F,$F0
        .byte $3F,$F0
        .byte $30,$00
        .byte $30,$00
        .byte $30,$00
        .byte $3F,$C0
        .byte $3F,$C0
        .byte $3C,$00
        .byte $3C,$00
        .byte $3C,$00
        .byte $3F,$F0
        .byte $1F,$F0
; --- G ---
        .byte $1F,$E0
        .byte $3F,$F0
        .byte $30,$30
        .byte $30,$30
        .byte $30,$00
        .byte $30,$00
        .byte $38,$E0
        .byte $3C,$F0
        .byte $3C,$30
        .byte $3C,$30
        .byte $3F,$F0
        .byte $1F,$E0
; --- K ---
        .byte $30,$C0
        .byte $30,$C0
        .byte $30,$C0
        .byte $30,$C0
        .byte $30,$C0
        .byte $3F,$C0
        .byte $3F,$F0
        .byte $3C,$30
        .byte $3C,$30
        .byte $3C,$30
        .byte $3C,$30
        .byte $3C,$30
; --- M ---
        .byte $1F,$E0
        .byte $3F,$F0
        .byte $33,$30
        .byte $33,$30
        .byte $33,$30
        .byte $33,$30
        .byte $3B,$30
        .byte $3B,$30
        .byte $3B,$30
        .byte $3B,$30
        .byte $3B,$30
        .byte $3B,$30
; --- N ---
        .byte $0F,$E0
        .byte $1F,$F0
        .byte $18,$30
        .byte $18,$30
        .byte $18,$30
        .byte $18,$30
        .byte $1C,$30
        .byte $1C,$30
        .byte $1C,$30
        .byte $1C,$30
        .byte $1C,$30
        .byte $1C,$30
; --- O ---
        .byte $1F,$E0
        .byte $3F,$F0
        .byte $30,$F0
        .byte $30,$F0
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $3F,$F0
        .byte $1F,$E0
; --- P ---
        .byte $1F,$E0
        .byte $3F,$F0
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $3F,$F0
        .byte $3F,$E0
        .byte $3C,$00
        .byte $3C,$00
        .byte $3C,$00
        .byte $3C,$00
        .byte $3C,$00
; --- R ---
        .byte $1F,$80
        .byte $3F,$C0
        .byte $30,$C0
        .byte $30,$C0
        .byte $30,$C0
        .byte $3F,$C0
        .byte $3F,$F0
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
; --- S ---
        .byte $1F,$E0
        .byte $3F,$F0
        .byte $30,$30
        .byte $30,$00
        .byte $30,$00
        .byte $3F,$E0
        .byte $1F,$F0
        .byte $00,$F0
        .byte $00,$F0
        .byte $30,$F0
        .byte $3F,$F0
        .byte $1F,$E0
; --- V ---
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $18,$C0
        .byte $18,$C0
        .byte $1F,$C0
        .byte $0F,$80
; --- Y ---
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $30,$30
        .byte $3F,$F0
        .byte $1F,$E0
        .byte $03,$80
        .byte $03,$80
        .byte $03,$80
        .byte $03,$80
        .byte $03,$80

