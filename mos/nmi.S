; nmi.S - замена обработчика NMI операционной системы.
;
; Зачем. Порт писался под цель cc65 atarixl, а та выключает ПЗУ ОС и подставляет
; ВЗАМЕН свою заглушку в низкой памяти: она принимает NMI, раздаёт его по
; векторам ОС (VDSLST для прерывания по строке, VVBLKD для кадрового) и
; переносит теневые ячейки в регистры. Игра на это опирается насквозь:
; screen.c ставит дисплей-лист записью в ТЕНЕВУЮ SDLSTL, палитра идёт через
; COLOR0..COLOR4, джойстик читается из STICK0.
;
; Без заглушки экран остаётся чёрным - теневые ячейки некому переносить.
; Проверено: первая сборка llvm-mos именно так и вела себя.
;
; Здесь заглушка своя. Она делает ровно то, чем пользуется игра, и ничего
; сверх: часы, перенос теней, защёлка джойстика и раздача по векторам.

        .globl __do_nmi_setup, mos_xitvbv

; --- аппаратура ---
NMIST   = $D40F                 ; кто прервал: $80 строчное, $40 кадровое
NMIRES  = $D40F                 ; сброс (та же ячейка на запись)
DMACTL  = $D400
DLISTL  = $D402
CHBASE  = $D409
PRIOR   = $D01B
COLPM0  = $D012
COLPF0  = $D016
PORTA   = $D300
TRIG0   = $D010

; --- ячейки ОС, которыми пользуется игра ---
RTCLOK  = $12                   ; трёхбайтные часы, младший байт по $14
VDSLST  = $0200                 ; вектор прерывания по строке
VVBLKD  = $0224                 ; отложенный кадровый вектор
SDMCTL  = $022F
SDLSTL  = $0230
GPRIOR  = $026F
PCOLR0  = $02C0                 ; четыре краски игроков
COLOR0  = $02C4                 ; четыре краски поля и фон
CHBAS   = $02F4
STICK0  = $0278
STRIG0  = $0284

        .section .init.50,"axR",@progbits
; Ставится ПОСЛЕ выключения ПЗУ (mos/init.S): пока оно включено, по $FFFA
; лежит вектор ОС, а не наш.
__do_nmi_setup:
        lda #<nmi_entry
        sta $FFFA
        lda #>nmi_entry
        sta $FFFB
        lda #<irq_rti           ; IRQ маскирован через sei, но вектор всё равно
        sta $FFFE               ; должен указывать на что-то осмысленное
        lda #>irq_rti
        sta $FFFF
        ; Векторы ОС до установки игрой должны вести в безопасное место:
        ; кадровое прерывание разрешается прямо сейчас, а vbi_install
        ; отработает только из main.
        lda #<mos_xitvbv
        sta VVBLKD
        lda #>mos_xitvbv
        sta VVBLKD+1
        lda #<dli_none
        sta VDSLST
        lda #>dli_none
        sta VDSLST+1

        lda #$40                ; пока только кадровое; строчное включит dli.s
        sta $D40E

        .section .text,"ax",@progbits

irq_rti:
dli_none:
        rti

; Порядок повторяет обработчик ОС дословно. Важно, что на пути прерывания по
; строке НИЧЕГО не портится: обработчики в src/dli.s сами кладут A и X на стек
; и выходят по rti, как принято у ОС. Сброс NMIRES там тоже не делается - его
; делает кадровая ветка, и именно так устроено в ПЗУ.
nmi_entry:
        bit NMIST
        bpl vbi                 ; бит 7 снят - значит кадровое
        jmp (VDSLST)

vbi:
        pha
        txa
        pha
        tya
        pha
        lda #$00
        sta NMIRES

        inc RTCLOK+2            ; часы: по ним считает кадры wait_frame
        bne vb_shadow
        inc RTCLOK+1
        bne vb_shadow
        inc RTCLOK

vb_shadow:
        lda SDMCTL              ; выборка ANTIC
        sta DMACTL
        lda SDLSTL              ; дисплей-лист
        sta DLISTL
        lda SDLSTL+1
        sta DLISTL+1
        lda CHBAS               ; знакогенератор
        sta CHBASE
        lda GPRIOR              ; приоритеты игроков
        sta PRIOR

        ldx #3                  ; краски игроков
vb_pm:  lda PCOLR0,x
        sta COLPM0,x
        dex
        bpl vb_pm

        ldx #4                  ; краски поля и фон
vb_pf:  lda COLOR0,x
        sta COLPF0,x
        dex
        bpl vb_pf

        lda PORTA               ; защёлка джойстика: у ОС это делает VBI
        and #$0F
        sta STICK0
        lda TRIG0
        sta STRIG0

        jmp (VVBLKD)            ; дальше обработчик игры, см. src/vbi.s

; Сюда игра уходит из своего обработчика (jmp XITVBV в src/vbi.s). У ОС это
; выход из кадрового прерывания; у нас - снятие регистров и возврат.
mos_xitvbv:
        pla
        tay
        pla
        tax
        pla
        rti
