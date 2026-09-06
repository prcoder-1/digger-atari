; Разведка прерываний на llvm-mos.
;
; С выключенным ПЗУ векторы 6502 по $FFFA..$FFFF лежат в обычном ОЗУ, то есть
; принадлежат нам - ровно так и работает наш порт на cc65. Здесь проверяется,
; что этим можно пользоваться из программы, собранной llvm-mos: свой обработчик
; вертикального гашения ставится руками, считает кадры, и main видит счётчик.
;
; Возврата в конце НЕТ: секции .init.* идут сквозным потоком.
        .include "imag.inc"

        .globl __do_irq_setup, vbi_count

        .section .zp.bss,"aw",@nobits
        .zeropage vbi_count
vbi_count: .zero 1

        .section .init.50,"axR",@progbits
__do_irq_setup:
        sei
        lda #0
        sta $D40E               ; NMIEN прочь на время переключения
        sta vbi_count

        lda $D301               ; PORTB
        and #$FE                ; ПЗУ ОС прочь - векторы теперь наши
        sta $D301

        lda #<nmi_handler       ; свой вектор NMI по $FFFA
        sta $FFFA
        lda #>nmi_handler
        sta $FFFB

        lda #$40                ; NMIEN: только вертикальное гашение
        sta $D40E
        cli

        .section .text,"ax",@progbits
nmi_handler:
        pha
        lda $D40F               ; NMIST: чьё прерывание
        sta $D40F               ; NMIRES: сбросить
        inc vbi_count
        pla
        rti
