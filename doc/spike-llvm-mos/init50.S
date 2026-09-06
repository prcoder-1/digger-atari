; Выключает ПЗУ операционной системы и переносит секцию .hidden под него.
;
; Ложится в .init.50 - раньше, чем инициализация мягкого стека (.init.100 в
; mos-platform/atari8-common/init-stack.S), потому что после выключения ПЗУ
; ни одна процедура ОС больше не работает и делать это надо разом.
;
; NMI гасится до переключения: обработчик вертикального гашения живёт в ПЗУ,
; и после его исчезновения прерывание ушло бы в данные.
        .include "imag.inc"

        .global __do_hidden_ram
        .section .init.50,"axR",@progbits
__do_hidden_ram:
        sei
        lda #0
        sta $D40E               ; NMIEN: прерывания ANTIC прочь

        lda $D301               ; PORTB
        and #$FE                ; бит 0: ПЗУ ОС прочь
        sta $D301

        ; перенос .hidden с адреса загрузки на адрес исполнения
        lda #<__hidden_load
        sta __rc2
        lda #>__hidden_load
        sta __rc3
        lda #<__hidden_start
        sta __rc4
        lda #>__hidden_start
        sta __rc5

        ldx #>__hidden_size     ; сначала целые страницы
        beq hr_tail_chk
        ldy #0
hr_page:
hr_p1:  lda (__rc2),y
        sta (__rc4),y
        iny
        bne hr_p1
        inc __rc3
        inc __rc5
        dex
        bne hr_page
hr_tail_chk:
        ldy #0
hr_tail:
        cpy #<__hidden_size
        beq hr_done
        lda (__rc2),y
        sta (__rc4),y
        iny
        bne hr_tail
hr_done:
        ; rts здесь НЕЛЬЗЯ: секции .init.* идут сквозным потоком, одна за
        ; другой, и никто их не вызывает - в init-stack.S (.init.100) тоже нет
        ; возврата. Поставленный сюда rts снимал со стека чужой адрес и уводил
        ; управление в никуда: main не отрабатывал вовсе.
