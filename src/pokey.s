; pokey.s - работа со звуковым чипом POKEY.
;
; Регистры пишутся только отсюда: в cc65 volatile почти не влияет на
; оптимизатор, и несколько подряд идущих записей по одному адресу
; схлопываются в последнюю (проверено на M2 на регистре цвета).
;
; Раскладка каналов:
;   CH1+CH2  связаны в 16 бит, тактируются от 1.773447 МГц -> ЗВУКОВЫЕ ЭФФЕКТЫ
;            громкость и тембр берутся со СТАРШЕГО канала (AUDC2),
;            у младшего AUDC1 обязан быть нулевым
;   CH3      8 бит от 63337 Гц -> мелодия
;   CH4      8 бит от 63337 Гц -> бас и второй голос эффектов
;
; Шестнадцатибитная пара выбрана не ради точности, а ради диапазона: период
; эффектов оригинала охватывает от 63 Гц (звук смерти) до 5 кГц (выстрел),
; и одним восьмибитным делителем это не покрыть без смены тактовой частоты.

        .export _pokey_init, _pokey_silence
        .export _sfx_f, _sfx_c, _sfx_apply
        .export _sfx_writes
        .export _sfx_env_tick, _sfx_env_vol, _sfx_env_timbre
        .export _sfx_sweep
        .export _sfx_money, _sfx_m1, _sfx_m2, _sfx_mvol
        .export _sfx_pulse, _sfx_pph, _sfx_pf, _sfx_pclick, _sfx_pvol
        .export _sfx_bugcnt, _sfx_bugset, _sfx_bugf, _sfx_bugvol
        .export _sfx_lifecnt, _sfx_lifef, _sfx_lifevol

AUDF1   = $D200
AUDC1   = $D201
AUDF2   = $D202
AUDC2   = $D203
AUDF3   = $D204
AUDC3   = $D205
AUDF4   = $D206
AUDC4   = $D207
AUDCTL  = $D208
SKCTL   = $D20F

; AUDCTL: 0x10 связать каналы 2+1, 0x40 тактировать канал 1 от 1.79 МГц
AUDCTL_VAL = $50

        .segment "ZEROPAGE"
_sfx_f:  .res 2                 ; 16-битный делитель эффекта
_sfx_c:  .res 1                 ; тембр и громкость (AUDC2)

        .segment "DATA"
; Огибающая: громкость спадает по кадру, а не по игровому такту.
; В оригинале звук шёл через ШИМ с фазами атаки, удержания и спада; такт
; игры идёт 10 раз в секунду, и на спад пришлось бы два-три шага - слышно
; как щелчок, а не как затухание. В кадровом прерывании шагов пятьдесят.
_sfx_env_vol:    .byte 0        ; текущая громкость 0..15, дробная часть ниже
_sfx_env_frac:   .byte 0
_sfx_env_timbre: .byte 0

; Скольжение высоты: столько прибавляется к делителю каждый кадр.
; Ноль - высота держится. Нужно для скрежета при гибели: если менять высоту
; раз в игровой такт, получаются девять отдельных нот с атакой на каждой, и
; звучит это как мелодия, а не как скрежет. В БК-порте свип идёт шагами по
; паре миллисекунд, то есть непрерывно.
_sfx_sweep:      .byte 0

; Звук съедаемого золота. В оригинале (snd_money/chk_money в Work/TH.C) это
; НЕ одиночный "динь", а тридцать коротких тонов подряд: чётные берут один
; делитель, нечётные другой. После каждого тона первый делитель растёт на
; одну шестнадцатую, второй на столько же убывает - высоты идут навстречу,
; расходятся и снова расходятся. Отсюда характерное дребезжание.
;
; Тона идут по кадрам, то есть 50 в секунду: вся последовательность звучит
; 0.6 с против примерно 0.3 с в оригинале, где счёт шёл от таймера. Чаще
; кадрового прерывания нам менять высоту негде.
;
; Огибающая на время последовательности отключена: в оригинале громкость
; держится, спад тут только смазал бы вторую половину.
_sfx_money:  .byte 0            ; сколько тонов осталось
_sfx_m1:     .res 2             ; делитель чётных тонов, ползёт ВВЕРХ
_sfx_m2:     .res 2             ; делитель нечётных тонов, ползёт ВНИЗ
_sfx_mvol:   .byte 0            ; тембр и громкость на всю последовательность
sfx_mt:      .res 2             ; временное под сдвиг на четыре

; Звук съедаемого камня. В оригинале (snd_coin/snd_2_coin и chk_coin/chk_2_coin
; в Work/TH.C) он из ДВУХ частей, и это слышно:
;
;   1. Низкий щелчок - один тик периодом 1000. Он перебивает ноту, потому что
;      chk_coin вызывается ПОСЛЕ chk_2_coin и переписывает t_period.
;   2. Пульсирующая нота - семь повторов по восемь тиков, из которых звучат
;      только первые два (coin_2_type 0 и 1). Высота ноты растёт с каждым
;      камнем подряд: C5, D5, E5 ... до C6, где даётся бонус за восемь.
;
; Ровный тон вместо этого звучал как "пик" и с оригиналом не имел общего.
; Последовательность идёт по кадрам, как и золото: чаще кадрового прерывания
; менять регистры негде.
_sfx_pulse:  .byte 0            ; сколько импульсов осталось (7 в начале)
_sfx_pph:    .byte 0            ; фаза внутри импульса, 0..7
_sfx_pf:     .res 2             ; делитель ноты
_sfx_pclick: .res 2             ; делитель щелчка, звучит только первый кадр
_sfx_pvol:   .byte 0            ; тембр и громкость

; Звук съедаемого врага в бонус-режиме. В оригинале (snd_bug/chk_bug в
; Work/TH.C) это ТРИ восходящих свипа подряд, а не один тон: делитель начинает
; с 2000 счётчиков PIT и на каждом тике убавляется на одну шестнадцатую, то
; есть высота ползёт вверх примерно на две октавы за двадцать тиков. Потом
; делитель возвращается к началу, и всё повторяется трижды.
;
; Звучит не непрерывно: тон выдаётся только на тиках, где остаток от деления
; счётчика на четыре равен единице или трём. Остальные два тика тишина, отчего
; свип получается пульсирующим.
_sfx_bugcnt:    .byte 0            ; тиков в текущем свипе (20 в начале)
_sfx_bugset: .byte 0            ; свипов осталось (3 в начале)
_sfx_bugf:   .res 2             ; текущий делитель, ползёт ВНИЗ
_sfx_bugvol: .byte 0            ; тембр и громкость
sfx_bt:      .res 2             ; временное под сдвиг на четыре

; Звук новой жизни. В оригинале (snd_life/chk_life в Work/TH.C) это не тон, а
; ТРЕЛЬ с поднимающейся высотой:
;
;       life_count = 96;
;       if((life_count/3) % 2) t_period = 600 + (life_count << 2);
;       life_count--;
;
; То есть три тика звучит, три молчит - шестнадцать импульсов, - а период PIT
; идёт с 984 к 604, и высота растёт с 1212 к 1975 Гц.
;
; Пересчёт. Период PIT делится на 20, получаются единицы БК 49.2..30.2, а из
; них по формуле движка (N = 14.86 * P - 7, см. sfx.h) выходят делители POKEY
; 724..442. Делитель убавляется на 2 за кадр, фаза - четыре кадра; сколько
; кадров всего, задаёт sfx_life в src/sfx.c, там же и разбор чисел.
_sfx_lifecnt: .byte 0           ; кадров осталось
_sfx_lifef:   .res 2            ; текущий делитель, ползёт ВНИЗ
_sfx_lifevol: .byte 0           ; тембр и громкость

; Накопительный счётчик записей в POKEY. Проверять звук снимком экрана
; нельзя, а счётчик показывает, доходит ли дело до чипа вообще.
_sfx_writes: .byte 0

        .segment "CODE"

; void pokey_init(void)
_pokey_init:
        lda     #0
        sta     AUDCTL
        lda     #3                      ; вывести POKEY из двухтонового режима
        sta     SKCTL
        lda     #AUDCTL_VAL
        sta     AUDCTL
        lda     #0
        sta     AUDC1                   ; у младшего в паре громкость всегда 0
        sta     AUDC2
        sta     AUDC3
        sta     AUDC4
        sta     _sfx_writes
        rts

; void pokey_silence(void) - гасит ТОЛЬКО голос эффектов.
;
; Каналы 3 и 4 трогать нельзя: там играет музыка, а эта процедура
; вызывается всякий раз, когда кончается звуковой эффект. Раньше она гасила
; все три канала, и каждый выстрел обрывал мелодию.
_pokey_silence:
        lda     #0
        sta     AUDC2
        rts

; void __fastcall__ prof_mark(unsigned char on)
;
; Метка для профилирования: пишет в AUDC4 признак "идёт игровой такт".
; Громкость в значении нулевая, поэтому метка беззвучна, но эмулятор
; записывает регистр в дамп, и длительность такта восстанавливается оттуда.
;
; Полосой цвета фона это померить не вышло: такт короче кадрового гасящего
; импульса и целиком в него укладывается, а видимой области луч достигает,
; когда цвет уже возвращён. Дамп регистров от положения луча не зависит.
        .export _prof_mark
_prof_mark:
        beq     :+
        lda     #$20                    ; идёт такт, громкость 0 - тишина
        sta     AUDC4
        rts
:       lda     #0
        sta     AUDC4
        rts

; void pokey_all_off(void) - полное молчание, для выхода из игры
        .export _pokey_all_off
_pokey_all_off:
        lda     #0
        sta     AUDC2
        sta     AUDC3
        sta     AUDC4
        rts

; void sfx_env_tick(void) - шаг огибающей, вызывается из кадрового прерывания
;
; Громкость хранится с дробной частью (8.8), иначе при пятидесяти шагах в
; секунду затухание с 12 до нуля заняло бы четверть секунды на любой звук.
_sfx_env_tick:
        lda     _sfx_lifecnt
        beq     :+
        jmp     life_tick               ; новая жизнь ведёт канал сама
:       lda     _sfx_money
        beq     :+
        jmp     money_tick              ; золото ведёт канал само
:       lda     _sfx_pulse
        beq     :+
        jmp     pulse_tick              ; камень ведёт канал сам
:       lda     _sfx_bugset
        beq     env_normal
        jmp     bug_tick                ; съеденный враг ведёт канал сам

env_normal:
        lda     _sfx_env_vol
        beq     env_done                ; уже тихо

        ; скольжение высоты, если включено
        lda     _sfx_sweep
        beq     env_novol
        clc
        adc     _sfx_f
        sta     _sfx_f
        sta     AUDF1
        lda     _sfx_f+1
        adc     #0
        sta     _sfx_f+1
        sta     AUDF2
env_novol:
        sec
        lda     _sfx_env_frac
        sbc     #$60                    ; скорость спада
        sta     _sfx_env_frac
        lda     _sfx_env_vol
        sbc     #0
        bcc     env_off
        sta     _sfx_env_vol
        ora     _sfx_env_timbre
        sta     AUDC2
        rts
env_off:
        lda     #0
        sta     _sfx_env_vol
        sta     _sfx_env_frac
        sta     AUDC2
env_done:
        rts

; Очередной кадр звука новой жизни: трель с поднимающейся высотой, см. выше.
life_tick:
        lda     _sfx_lifecnt
        and     #4                      ; четыре кадра звук, четыре тишина
        beq     life_quiet
        lda     _sfx_lifef
        sta     AUDF1
        lda     _sfx_lifef+1
        sta     AUDF2
        lda     #0
        sta     AUDC1                   ; связанная пара: звук с AUDC2
        lda     _sfx_lifevol
        sta     AUDC2
        jmp     life_step
life_quiet:
        lda     #0
        sta     AUDC2
life_step:
        sec                             ; высота растёт: делитель на 2 меньше
        lda     _sfx_lifef
        sbc     #2
        sta     _sfx_lifef
        lda     _sfx_lifef+1
        sbc     #0
        sta     _sfx_lifef+1
        dec     _sfx_lifecnt
        bne     :+
        lda     #0
        sta     AUDC2                   ; отзвучала - замолчать
:       rts

; Очередной кадр звука съеденного врага.
bug_tick:
        lda     _sfx_bugcnt
        bne     bug_have
        ; свип кончился - начать следующий
        dec     _sfx_bugset
        beq     bug_stop
        lda     #20
        sta     _sfx_bugcnt
        lda     #<1480                  ; период БК 100 в делителях POKEY
        sta     _sfx_bugf
        lda     #>1480
        sta     _sfx_bugf+1
bug_have:
        lda     _sfx_bugcnt
        and     #3
        cmp     #1
        beq     bug_snd
        cmp     #3
        bne     bug_quiet
bug_snd:
        lda     _sfx_bugf
        sta     AUDF1
        lda     _sfx_bugf+1
        sta     AUDF2
        lda     #0
        sta     AUDC1                   ; связанная пара: звук с AUDC2
        lda     _sfx_bugvol
        sta     AUDC2
        jmp     bug_step
bug_quiet:
        lda     #0
        sta     AUDC2
bug_step:
        ; делитель убавляется на одну шестнадцатую: высота ползёт вверх
        lda     _sfx_bugf
        sta     sfx_bt
        lda     _sfx_bugf+1
        sta     sfx_bt+1
        lsr     sfx_bt+1
        ror     sfx_bt
        lsr     sfx_bt+1
        ror     sfx_bt
        lsr     sfx_bt+1
        ror     sfx_bt
        lsr     sfx_bt+1
        ror     sfx_bt
        sec
        lda     _sfx_bugf
        sbc     sfx_bt
        sta     _sfx_bugf
        lda     _sfx_bugf+1
        sbc     sfx_bt+1
        sta     _sfx_bugf+1
        dec     _sfx_bugcnt
        rts
bug_stop:
        lda     #0
        sta     _sfx_bugcnt
        sta     AUDC2
        rts

; Очередной кадр звука камня.
;
; Фазы 0 и 1 звучат, 2..7 молчат - ровно как coin_2_type в оригинале. На
; ПЕРВОМ кадре вместо ноты выдаётся низкий щелчок: там chk_coin переписывает
; период уже после chk_2_coin, и слышно именно щелчок, а следом ноту.
pulse_tick:
        lda     _sfx_pph
        cmp     #2
        bcs     pulse_off               ; фазы 2..7 - тишина

        ; звучащая фаза: на самом первом кадре щелчок, дальше нота
        ldx     _sfx_pf
        ldy     _sfx_pf+1
        lda     _sfx_pclick+1
        beq     pulse_note              ; щелчок уже отыграл
        ldx     _sfx_pclick
        ldy     _sfx_pclick+1
        lda     #0                      ; щелчок звучит один кадр
        sta     _sfx_pclick+1
pulse_note:
        stx     AUDF1
        sty     AUDF2
        lda     #0
        sta     AUDC1                   ; связанная пара: звук выходит из AUDC2
        lda     _sfx_pvol
        sta     AUDC2
        jmp     pulse_step
pulse_off:
        lda     #0
        sta     AUDC2
pulse_step:
        inc     _sfx_pph
        lda     _sfx_pph
        cmp     #8
        bcc     :+
        lda     #0
        sta     _sfx_pph
        dec     _sfx_pulse
:       rts

; Очередной тон последовательности золота. Делитель берётся по чётности
; номера, потом оба делителя расходятся на одну шестнадцатую.
money_tick:
        lda     _sfx_money
        and     #1
        bne     m_odd
        lda     _sfx_m1
        ldx     _sfx_m1+1
        jmp     m_out
m_odd:
        lda     _sfx_m2
        ldx     _sfx_m2+1
m_out:
        sta     AUDF1
        stx     AUDF2
        lda     #0
        sta     AUDC1
        lda     _sfx_mvol
        sta     AUDC2

        lda     _sfx_m1                 ; m1 += m1 >> 4
        sta     sfx_mt
        lda     _sfx_m1+1
        sta     sfx_mt+1
        ldx     #4
m_sh1:  lsr     sfx_mt+1
        ror     sfx_mt
        dex
        bne     m_sh1
        clc
        lda     _sfx_m1
        adc     sfx_mt
        sta     _sfx_m1
        lda     _sfx_m1+1
        adc     sfx_mt+1
        sta     _sfx_m1+1

        lda     _sfx_m2                 ; m2 -= m2 >> 4
        sta     sfx_mt
        lda     _sfx_m2+1
        sta     sfx_mt+1
        ldx     #4
m_sh2:  lsr     sfx_mt+1
        ror     sfx_mt
        dex
        bne     m_sh2
        sec
        lda     _sfx_m2
        sbc     sfx_mt
        sta     _sfx_m2
        lda     _sfx_m2+1
        sbc     sfx_mt+1
        sta     _sfx_m2+1

        dec     _sfx_money
        bne     m_done
        lda     #0
        sta     AUDC2                   ; последовательность кончилась
m_done:
        rts

; void sfx_apply(void) - выдать в чип текущие _sfx_f и _sfx_c
_sfx_apply:
        lda     _sfx_f
        sta     AUDF1                   ; младший байт делителя
        lda     _sfx_f+1
        sta     AUDF2                   ; старший байт
        lda     #0
        sta     AUDC1
        lda     _sfx_c
        sta     AUDC2                   ; звук снимается со старшего канала
        and     #$0F
        sta     _sfx_env_vol            ; с этой громкости начнётся спад
        lda     #0
        sta     _sfx_env_frac
        lda     _sfx_c
        and     #$F0
        sta     _sfx_env_timbre
        inc     _sfx_writes
        rts
