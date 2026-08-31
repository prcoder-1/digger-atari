; score.s - счёт в упакованном двоично-десятичном виде.
;
; Три байта, старший разряд первым, всего шесть знаков - как потолок 999999
; в оригинале. Перенос из старшего байта отбрасывается, поэтому счёт
; переполняется в ноль сам собой, без отдельной проверки.
;
; Сложение обязано идти отсюда: десятичный режим включается командой SED,
; а из Си до флага D не добраться. Взамен вывод счёта не требует ни одного
; деления - в отличие от 32-битного счёта БК-порта, где print_dec вычитал
; степени десятки по девять раз на знак.

        .export _score, _score_add, _score_clear

        .segment "ZEROPAGE"
sc_lo:   .res 1
sc_hi:   .res 1

        .segment "DATA"
_score:  .byte 0, 0, 0                  ; [0] старшие два знака ... [2] младшие

        .segment "CODE"

; void __fastcall__ score_add(unsigned int bcd)
; Значение передаётся уже в BCD: 25 очков это $0025, 1000 очков это $1000.
; cc65 передаёт unsigned int в A (младший байт) и X (старший).
_score_add:
        sta     sc_lo
        stx     sc_hi
        sed
        clc
        lda     _score+2
        adc     sc_lo
        sta     _score+2
        lda     _score+1
        adc     sc_hi
        sta     _score+1
        lda     _score+0
        adc     #0
        sta     _score+0
        cld
        rts

; void score_clear(void)
_score_clear:
        lda     #0
        sta     _score+0
        sta     _score+1
        sta     _score+2
        rts
