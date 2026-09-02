; pmspr.s - СГЕНЕРИРОВАНО tools/spr2atari.py, не править вручную.
; Заливка для аппаратных спрайтов, правило сжатия: None
; По байту на строку растра, 8 бит на 16 точек.
; Диггер: план 0 - корпус (краска 2), план 1 - клюв (краска 1).
; Враги:  план 0 - весь силуэт, один оттенок на фигуру.

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

        .export _pm_m0_0
_pm_m0_0:            ; 15 строк
        .byte $00
        .byte $24
        .byte $58
        .byte $58
        .byte $7E
        .byte $3C
        .byte $24
        .byte $00
        .byte $00
        .byte $42
        .byte $42
        .byte $42
        .byte $E2
        .byte $02
        .byte $07

        .export _pm_m1_0
_pm_m1_0:            ; 15 строк
        .byte $00
        .byte $24
        .byte $7E
        .byte $58
        .byte $58
        .byte $3C
        .byte $3C
        .byte $00
        .byte $00
        .byte $5A
        .byte $42
        .byte $42
        .byte $42
        .byte $E7
        .byte $00

        .export _pm_m2_0
_pm_m2_0:            ; 15 строк
        .byte $00
        .byte $24
        .byte $7E
        .byte $1A
        .byte $1A
        .byte $3C
        .byte $3C
        .byte $18
        .byte $00
        .byte $5A
        .byte $42
        .byte $42
        .byte $47
        .byte $40
        .byte $E0

        .export _pm_c0_0
_pm_c0_0:            ; 15 строк
        .byte $10
        .byte $18
        .byte $48
        .byte $48
        .byte $56
        .byte $66
        .byte $7E
        .byte $30
        .byte $3E
        .byte $1C
        .byte $00
        .byte $00
        .byte $00
        .byte $18
        .byte $18

        .export _pm_c1_0
_pm_c1_0:            ; 15 строк
        .byte $10
        .byte $18
        .byte $40
        .byte $42
        .byte $56
        .byte $66
        .byte $7C
        .byte $30
        .byte $38
        .byte $1E
        .byte $02
        .byte $00
        .byte $18
        .byte $18
        .byte $00

        .export _pm_c2_0
_pm_c2_0:            ; 15 строк
        .byte $10
        .byte $00
        .byte $40
        .byte $5A
        .byte $56
        .byte $64
        .byte $78
        .byte $30
        .byte $38
        .byte $1C
        .byte $02
        .byte $18
        .byte $18
        .byte $00
        .byte $00

        .export _pm_a0_0
_pm_a0_0:            ; 15 строк
        .byte $08
        .byte $18
        .byte $12
        .byte $12
        .byte $6A
        .byte $66
        .byte $7E
        .byte $0C
        .byte $7C
        .byte $38
        .byte $00
        .byte $00
        .byte $00
        .byte $18
        .byte $18

        .export _pm_a1_0
_pm_a1_0:            ; 15 строк
        .byte $08
        .byte $18
        .byte $02
        .byte $42
        .byte $6A
        .byte $66
        .byte $3E
        .byte $0C
        .byte $1C
        .byte $78
        .byte $40
        .byte $00
        .byte $18
        .byte $18
        .byte $00

        .export _pm_a2_0
_pm_a2_0:            ; 15 строк
        .byte $08
        .byte $00
        .byte $02
        .byte $5A
        .byte $6A
        .byte $26
        .byte $1E
        .byte $0C
        .byte $1C
        .byte $38
        .byte $40
        .byte $18
        .byte $18
        .byte $00
        .byte $00

