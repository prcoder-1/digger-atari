; init.S - что делается до main: выключение ПЗУ и перенос спрайтов под него.
;
; У cc65 это делала библиотека цели atarixl: отдельный кусок XEX грузился в
; промежуточную область, её процедура переносила SHADOW_RAM под ПЗУ, и только
; потом загружался основной кусок. Здесь то же самое устроено раздельными
; адресами загрузки и исполнения (AT>ram в mos/link.ld), а перенос делает эта
; секция.
;
; Возврата в конце НЕТ намеренно: секции .init.* идут сквозным потоком, одна за
; другой, и никто их не вызывает - в штатной init-stack.S возврата тоже нет.
; Поставленный сюда rts снимет со стека чужой адрес (проверено, main тогда не
; отрабатывает вовсе).
;
; NMI гасится ДО переключения: обработчик вертикального гашения живёт в ПЗУ, и
; после его исчезновения прерывание ушло бы в данные. Свой обработчик ставит
; уже сама игра, через векторы по $FFFA - с выключенным ПЗУ они наши.

        .include "imag.inc"

        .globl __do_rom_off, mos_ptr

; Рабочий указатель для перенесённых с ca65 процедур: там он назывался ptr1 и
; брался из потрохов cc65. Свой - надёжнее, чем занимать мнимый регистр.
        .zeropage mos_ptr, cp_src, cp_dst, cp_cnt
        .section .zp.bss,"aw",@nobits
mos_ptr: .zero 2
cp_src:  .zero 2
cp_dst:  .zero 2

        .section .init.50,"axR",@progbits
__do_rom_off:
        sei
        lda #0
        sta $D40E               ; NMIEN прочь на время переключения

        ; Знакогенератор сохраняется САМ В СЕБЯ, пока ПЗУ ещё включено: чтение
        ; идёт из ПЗУ, а запись по тому же адресу попадает в ОЗУ под ним. После
        ; переключения по $E000 остаётся та же таблица. Цель cc65 atarixl
        ; делает ровно это (сегмент CHARGEN), и на неё опирается text_char -
        ; надпись GAME OVER рисуется буквами оттуда.
        ; Младшие байты ставятся ЯВНО: обнуление .bss идёт в .init.200, то
        ; есть позже нас, и до него в ячейках лежит что попало.
        lda #0
        sta cp_src
        sta cp_dst
        lda #$E0
        sta cp_src+1
        sta cp_dst+1
        ldy #0
        ldx #4                  ; четыре страницы, $E000..$E3FF
cg_pg:  lda (cp_src),y
        sta (cp_dst),y
        iny
        bne cg_pg
        inc cp_src+1
        inc cp_dst+1
        dex
        bne cg_pg

        lda $D301               ; PORTB
        and #$FE                ; бит 0: ПЗУ ОС прочь
        sta $D301

        ; Спрайты лежат в двух областях под ПЗУ: между ними знакогенератор,
        ; одним куском их не занять - так же было и на cc65.
        lda #<__romdata_load
        sta cp_src
        lda #>__romdata_load
        sta cp_src+1
        lda #<__romdata_start
        sta cp_dst
        lda #>__romdata_start
        sta cp_dst+1
        lda #<__romdata_size
        sta cp_cnt
        lda #>__romdata_size
        sta cp_cnt+1
        jsr copy_block

        lda #<__romdata2_load
        sta cp_src
        lda #>__romdata2_load
        sta cp_src+1
        lda #<__romdata2_start
        sta cp_dst
        lda #>__romdata2_start
        sta cp_dst+1
        lda #<__romdata2_size
        sta cp_cnt
        lda #>__romdata2_size
        sta cp_cnt+1
        jsr copy_block
        ; возврата нет: падаем в следующую секцию .init

; Мягкий стек Си ставится СВОЙ, и это обязательно. Штатная init-stack.S
; (.init.100) берёт его от MEMTOP - у ОС это верх свободной памяти, около
; $9C00, - а у нас с $9000 начинается кадровый буфер ANTIC. Стек растёт вниз
; прямо по нему, и картинка рассыпается ещё до первого кадра: экран остаётся
; чёрным, потому что затирается и дисплей-лист.
;
; Секция .init.150 идёт ПОСЛЕ .init.100, поэтому наше значение остаётся.
        .section .init.150,"axR",@progbits
        lda #<0x8fff
        sta __rc0
        lda #>0x8fff
        sta __rc1

        .section .text,"ax",@progbits

; Перенос cp_cnt байт из cp_src в cp_dst. Сначала целые страницы, потом
; остаток: так на страницу приходится один inc старшего байта, а не проверка
; на каждый байт.
copy_block:
        ldy #0
        ldx cp_cnt+1
        beq cb_tail
cb_pg:  lda (cp_src),y
        sta (cp_dst),y
        iny
        bne cb_pg
        inc cp_src+1
        inc cp_dst+1
        dex
        bne cb_pg
cb_tail:
        ldy #0
        cpy cp_cnt
        beq cb_done
cb_t1:  lda (cp_src),y
        sta (cp_dst),y
        iny
        cpy cp_cnt
        bne cb_t1
cb_done:
        rts

        .section .zp.bss,"aw",@nobits
cp_cnt: .zero 2
