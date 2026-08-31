; earth.s - СГЕНЕРИРОВАНО tools/spr2atari.py, не править вручную.
; Текстуры земли (спрайты p1..p8 оригинала, 20x4 px -> 10x4 px).
; Каждая - блок 5 байт x 4 строки, повторяется по горизонтали 8 раз
; и по вертикали на всю высоту поля.
;
; Лежит в RODATA, а не вместе со спрайтами: под ПЗУ операционной
; системы места впритык, а текстуры земли мелкие.

        .segment "RODATA"

        .export _earth_p1
_earth_p1:            ; 5 байт x 4 строк
        .byte $BF,$AF,$EB,$FA,$FE
        .byte $EF,$BB,$EE,$FB,$BE
        .byte $EE,$FB,$BE,$EF,$BB
        .byte $FA,$FE,$BF,$AF,$EB

        .export _earth_p2
_earth_p2:            ; 5 байт x 4 строк
        .byte $66,$66,$66,$66,$66
        .byte $99,$99,$99,$99,$99
        .byte $99,$99,$99,$99,$99
        .byte $99,$99,$99,$99,$99

        .export _earth_p3
_earth_p3:            ; 5 байт x 4 строк
        .byte $BB,$BB,$BB,$BB,$BB
        .byte $EE,$EE,$EE,$EE,$EE
        .byte $EE,$EE,$EE,$EE,$EE
        .byte $BB,$BB,$BB,$BB,$BB

        .export _earth_p4
_earth_p4:            ; 5 байт x 4 строк
        .byte $7F,$5F,$D7,$F5,$FD
        .byte $DF,$77,$DD,$F7,$7D
        .byte $DD,$F7,$7D,$DF,$77
        .byte $F5,$FD,$7F,$5F,$D7

        .export _earth_p5
_earth_p5:            ; 5 байт x 4 строк
        .byte $BF,$AF,$EB,$FA,$FE
        .byte $DF,$77,$DD,$F7,$7D
        .byte $EE,$FB,$BE,$EF,$BB
        .byte $F5,$FD,$7F,$5F,$D7

        .export _earth_p6
_earth_p6:            ; 5 байт x 4 строк
        .byte $66,$66,$66,$66,$66
        .byte $99,$99,$99,$99,$99
        .byte $66,$66,$66,$66,$66
        .byte $99,$99,$99,$99,$99

        .export _earth_p7
_earth_p7:            ; 5 байт x 4 строк
        .byte $BB,$BB,$BB,$BB,$BB
        .byte $DD,$DD,$DD,$DD,$DD
        .byte $DD,$DD,$DD,$DD,$DD
        .byte $BB,$BB,$BB,$BB,$BB

        .export _earth_p8
_earth_p8:            ; 5 байт x 4 строк
        .byte $EE,$EE,$EE,$EE,$EE
        .byte $BB,$BB,$BB,$BB,$BB
        .byte $BB,$BB,$BB,$BB,$BB
        .byte $BB,$BB,$BB,$BB,$BB

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

