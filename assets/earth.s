; earth.s - СГЕНЕРИРОВАНО tools/spr2atari.py, не править вручную.
; Текстуры земли (спрайты p1..p8 оригинала, 20x4 точки).
; Каждая - блок 5 байт x 4 строки, повторяется по горизонтали 8 раз
; и по вертикали на всю высоту поля.
;
; Лежит в RODATA, а не вместе со спрайтами: под ПЗУ операционной
; системы места впритык, а текстуры земли мелкие.

; Узор земли уехал под ПЗУ вместе с картами уровней и шрифтом: MAIN упирается
; в потолок. Сегмент тот же, LEVELS - его целиком переносит copy_levels
; (src/util.s), а что внутри, ей всё равно.
        .segment "LEVELS"

        .export _earth_p1
_earth_p1:            ; 5 байт x 4 строк
        .byte $3F,$0F,$C3,$F0,$FC
        .byte $9E,$67,$99,$E6,$79
        .byte $CC,$F3,$3C,$CF,$33
        .byte $E1,$F8,$7E,$1F,$87

        .export _earth_p2
_earth_p2:            ; 5 байт x 4 строк
        .byte $33,$33,$33,$33,$33
        .byte $99,$99,$99,$99,$99
        .byte $CC,$CC,$CC,$CC,$CC
        .byte $99,$99,$99,$99,$99

        .export _earth_p3
_earth_p3:            ; 5 байт x 4 строк
        .byte $33,$33,$33,$33,$33
        .byte $99,$99,$99,$99,$99
        .byte $CC,$CC,$CC,$CC,$CC
        .byte $66,$66,$66,$66,$66

        .export _earth_p4
_earth_p4:            ; 5 байт x 4 строк
        .byte $3F,$0F,$C3,$F0,$FC
        .byte $9E,$67,$99,$E6,$79
        .byte $CC,$F3,$3C,$CF,$33
        .byte $E1,$F8,$7E,$1F,$87

        .export _earth_p5
_earth_p5:            ; 5 байт x 4 строк
        .byte $3F,$0F,$C3,$F0,$FC
        .byte $9E,$67,$99,$E6,$79
        .byte $CC,$F3,$3C,$CF,$33
        .byte $E1,$F8,$7E,$1F,$87

        .export _earth_p6
_earth_p6:            ; 5 байт x 4 строк
        .byte $33,$33,$33,$33,$33
        .byte $99,$99,$99,$99,$99
        .byte $33,$33,$33,$33,$33
        .byte $99,$99,$99,$99,$99

        .export _earth_p7
_earth_p7:            ; 5 байт x 4 строк
        .byte $33,$33,$33,$33,$33
        .byte $99,$99,$99,$99,$99
        .byte $CC,$CC,$CC,$CC,$CC
        .byte $66,$66,$66,$66,$66

        .export _earth_p8
_earth_p8:            ; 5 байт x 4 строк
        .byte $CC,$CC,$CC,$CC,$CC
        .byte $66,$66,$66,$66,$66
        .byte $33,$33,$33,$33,$33
        .byte $66,$66,$66,$66,$66

        .export _earth_tbl
_earth_tbl:
        .word _earth_p1
        .word _earth_p2
        .word _earth_p3
        .word _earth_p4
        .word _earth_p5
        .word _earth_p6
        .word _earth_p7
        .word _earth_p8

