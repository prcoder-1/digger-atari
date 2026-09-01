; pmspr.s - СГЕНЕРИРОВАНО tools/spr2atari.py, не править вручную.
; Заливка Диггера для аппаратных спрайтов, правило сжатия: None
; По байту на строку растра, 8 бит на 16 точек.
; План 0 - корпус (краска 2), 1 - клюв (краска 1), 2 - колёса (3).

        .segment "RODATA"

        .export _pm_r0_0
_pm_r0_0:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $38
        .byte $78
        .byte $78
        .byte $38
        .byte $20
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_r0_1
_pm_r0_1:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $02
        .byte $02
        .byte $04
        .byte $04
        .byte $04
        .byte $04
        .byte $04
        .byte $02
        .byte $02
        .byte $00
        .byte $00

        .export _pm_r0_2
_pm_r0_2:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $10
        .byte $10
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_r1_0
_pm_r1_0:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $38
        .byte $78
        .byte $78
        .byte $E0
        .byte $20
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_r1_1
_pm_r1_1:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $02
        .byte $06
        .byte $04
        .byte $04
        .byte $04
        .byte $06
        .byte $02
        .byte $00
        .byte $00
        .byte $00

        .export _pm_r1_2
_pm_r1_2:            ; 15 строк
        .byte $00
        .byte $00
        .byte $10
        .byte $10
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_r2_0
_pm_r2_0:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $38
        .byte $78
        .byte $78
        .byte $38
        .byte $20
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_r2_1
_pm_r2_1:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $03
        .byte $07
        .byte $04
        .byte $07
        .byte $03
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_r2_2
_pm_r2_2:            ; 15 строк
        .byte $00
        .byte $10
        .byte $10
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_l0_0
_pm_l0_0:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $1C
        .byte $1E
        .byte $1E
        .byte $1C
        .byte $04
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_l0_1
_pm_l0_1:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $40
        .byte $40
        .byte $20
        .byte $20
        .byte $20
        .byte $20
        .byte $20
        .byte $40
        .byte $40
        .byte $00
        .byte $00

        .export _pm_l0_2
_pm_l0_2:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $08
        .byte $08
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_l1_0
_pm_l1_0:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $1C
        .byte $1E
        .byte $1E
        .byte $07
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_l1_1
_pm_l1_1:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $40
        .byte $60
        .byte $20
        .byte $20
        .byte $20
        .byte $60
        .byte $40
        .byte $00
        .byte $00
        .byte $00

        .export _pm_l1_2
_pm_l1_2:            ; 15 строк
        .byte $00
        .byte $00
        .byte $08
        .byte $08
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_l2_0
_pm_l2_0:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $1C
        .byte $1E
        .byte $1E
        .byte $1C
        .byte $04
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_l2_1
_pm_l2_1:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $C0
        .byte $E0
        .byte $20
        .byte $E0
        .byte $C0
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_l2_2
_pm_l2_2:            ; 15 строк
        .byte $00
        .byte $08
        .byte $08
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_d0_0
_pm_d0_0:            ; 15 строк
        .byte $00
        .byte $00
        .byte $08
        .byte $08
        .byte $0C
        .byte $0C
        .byte $0C
        .byte $08
        .byte $08
        .byte $0C
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_d0_1
_pm_d0_1:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $0C
        .byte $1C
        .byte $12
        .byte $02
        .byte $00

        .export _pm_d0_2
_pm_d0_2:            ; 15 строк
        .byte $02
        .byte $00
        .byte $00
        .byte $02
        .byte $00
        .byte $00
        .byte $20
        .byte $20
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_d1_0
_pm_d1_0:            ; 15 строк
        .byte $04
        .byte $00
        .byte $08
        .byte $0C
        .byte $0C
        .byte $08
        .byte $08
        .byte $08
        .byte $08
        .byte $08
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_d1_1
_pm_d1_1:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $08
        .byte $14
        .byte $10
        .byte $10
        .byte $10

        .export _pm_d1_2
_pm_d1_2:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $20
        .byte $02
        .byte $00
        .byte $20
        .byte $02
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_d2_0
_pm_d2_0:            ; 15 строк
        .byte $00
        .byte $00
        .byte $08
        .byte $08
        .byte $0C
        .byte $0C
        .byte $0C
        .byte $08
        .byte $08
        .byte $0C
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_d2_1
_pm_d2_1:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $08
        .byte $0C
        .byte $04
        .byte $04
        .byte $04

        .export _pm_d2_2
_pm_d2_2:            ; 15 строк
        .byte $02
        .byte $00
        .byte $00
        .byte $02
        .byte $00
        .byte $20
        .byte $40
        .byte $40
        .byte $20
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_u0_0
_pm_u0_0:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $30
        .byte $10
        .byte $10
        .byte $30
        .byte $30
        .byte $30
        .byte $10
        .byte $10
        .byte $00
        .byte $00

        .export _pm_u0_1
_pm_u0_1:            ; 15 строк
        .byte $00
        .byte $40
        .byte $48
        .byte $38
        .byte $30
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_u0_2
_pm_u0_2:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $04
        .byte $04
        .byte $00
        .byte $00
        .byte $40
        .byte $00
        .byte $00
        .byte $40

        .export _pm_u1_0
_pm_u1_0:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $10
        .byte $10
        .byte $10
        .byte $10
        .byte $10
        .byte $30
        .byte $30
        .byte $10
        .byte $00
        .byte $20

        .export _pm_u1_1
_pm_u1_1:            ; 15 строк
        .byte $08
        .byte $08
        .byte $28
        .byte $38
        .byte $30
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_u1_2
_pm_u1_2:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $40
        .byte $04
        .byte $00
        .byte $40
        .byte $04
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_u2_0
_pm_u2_0:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $30
        .byte $10
        .byte $10
        .byte $30
        .byte $30
        .byte $30
        .byte $10
        .byte $10
        .byte $00
        .byte $00

        .export _pm_u2_1
_pm_u2_1:            ; 15 строк
        .byte $20
        .byte $20
        .byte $20
        .byte $20
        .byte $10
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00

        .export _pm_u2_2
_pm_u2_2:            ; 15 строк
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $00
        .byte $04
        .byte $02
        .byte $02
        .byte $04
        .byte $00
        .byte $40
        .byte $00
        .byte $00
        .byte $40

