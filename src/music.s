; music.s - фоновая музыка на канале 3 POKEY.
;
; Тикает из кадрового прерывания, то есть 50 раз в секунду. Порт на БК был
; вынужден дробить ноту на кусочки и доигрывать их в остатке кадрового
; времени, потому что там звук занимал процессор целиком. POKEY держит тон
; сам, поэтому от всего планировщика остаётся один счётчик длительности.
;
; Темп задаётся накопителем в формате 8.8: длительность восьмой ноты хранится
; с дробной частью, из счётчика раз в кадр вычитается ровно единица. Иначе
; восьмая укладывалась бы в целое число кадров, и темп уезжал бы на проценты:
; при 50 Гц восьмая мелодии Popcorn - это 9.08 кадра, а не 9.
;
; Кодировка ноты перенесена из порта на БК без изменений:
;   n < 14   восьмая, высота n      n + 14   четверть
;   n + 28   пять восьмых           $FF      конец, мелодия зациклится

        .export _music_start, _music_stop, _music_tick
        .export _music_notes_played

        .import _music_audf

AUDF3   = $D204
AUDC3   = $D205

MUS_VOL = $A8                   ; чистый тон, громкость 8

        .segment "ZEROPAGE"
mus_ptr:  .res 2                ; текущая позиция в мелодии
mus_base: .res 2                ; начало мелодии, чтобы зациклить
mus_left: .res 2                ; кадров до следующей ноты, формат 8.8
mus_egt:  .res 2                ; длительность восьмой, формат 8.8

        .segment "DATA"
mus_on:   .byte 0
; Накопительный счётчик сыгранных нот: по нему видно, что проигрыватель
; движется, даже если высоты окажутся неверными.
_music_notes_played: .byte 0

        .segment "CODE"

; void __fastcall__ music_start(void)  - мелодия и темп берутся из mus_base/mus_egt,
; их выставляет обёртка на Си через music_select
_music_start:
        lda     mus_base
        sta     mus_ptr
        lda     mus_base+1
        sta     mus_ptr+1
        lda     #0
        sta     mus_left
        sta     mus_left+1              ; первая нота зазвучит сразу
        sta     _music_notes_played
        lda     #1
        sta     mus_on
        rts

; void __fastcall__ music_note(unsigned char audf)
;
; Выдаёт одиночную ноту на канал мелодии, пока проигрыватель остановлен.
; Нужна для траурного марша: голос эффектов работает связанной 16-битной
; парой от тактовой 1.79 МГц, а таблица периодов мелодии рассчитана на
; одиночный канал от базы 64 кГц - через sfx_play марш звучал октавой выше.
        .export _music_note
_music_note:
        sta     AUDF3
        lda     #MUS_VOL
        sta     AUDC3
        rts

_music_stop:
        lda     #0
        sta     mus_on
        sta     AUDC3
        rts

; Вызывается из кадрового прерывания
_music_tick:
        lda     mus_on
        bne     :+
        rts
:
        ; mus_left -= 1.0 (то есть $0100 в формате 8.8)
        sec
        lda     mus_left+1
        sbc     #1
        sta     mus_left+1
        bcs     mus_done                ; ещё звучит

; --- пора брать следующую ноту ---
mus_next:
        ldy     #0
        lda     (mus_ptr),y
        cmp     #$FF
        bne     :+
        lda     mus_base                ; конец - зациклить
        sta     mus_ptr
        lda     mus_base+1
        sta     mus_ptr+1
        lda     (mus_ptr),y
:
        ; длительность: 1, 2 или 5 восьмых по диапазону кода ноты
        ldx     #1
        cmp     #28
        bcc     :+
        sbc     #28
        ldx     #5
        bne     mus_have
:
        cmp     #14
        bcc     mus_have
        sbc     #14
        ldx     #2
mus_have:
        tay                             ; Y = высота
        lda     _music_audf,y
        sta     AUDF3
        lda     #MUS_VOL
        sta     AUDC3
        inc     _music_notes_played

        ; mus_left += mus_egt * X, умножение сложением: X не больше пяти.
        ;
        ; Именно ПРИБАВЛЯЕТСЯ к остатку, а не присваивается. Счётчик к этому
        ; моменту уже ушёл чуть ниже нуля, и этот остаток - дробная часть
        ; длительности плюс кадр, на котором произошло переполнение. Если его
        ; отбросить, каждая нота удлиняется на кадр и теряет дробную часть:
        ; измерено 10.4 кадра на восьмую вместо 9.1, то есть темп на 13% ниже.
mus_mul:
        clc
        lda     mus_left
        adc     mus_egt
        sta     mus_left
        lda     mus_left+1
        adc     mus_egt+1
        sta     mus_left+1
        dex
        bne     mus_mul

        ; шагнуть на следующую ноту
        inc     mus_ptr
        bne     mus_done
        inc     mus_ptr+1
mus_done:
        rts

; Мелодию и темп выставляет обёртка music_select в sfx.c: она пишет прямо
; в mus_base и mus_egt, поэтому отдельной процедуры-установщика не нужно.
        .export _mus_base, _mus_egt
_mus_base = mus_base
_mus_egt  = mus_egt
