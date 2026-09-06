; blit_mos.S - перенос src/blit.s на ассемблер llvm-mos.
;
; Что менялось против ca65:
;   * .segment "ZEROPAGE" -> .section .zp.bss + директива .zeropage, которая
;     велит ассемблеру брать однобайтную адресацию (адрес известен только
;     линковщику);
;   * .export/.import -> .globl (символы на Си идут БЕЗ ведущего подчёркивания,
;     в отличие от cc65);
;   * .macro ... .endmacro -> .macro ... .endm, параметры через \имя;
;   * .res N -> .zero N (или .fill).
;
; Самомодификация операндов (bm_and+1 и т.д.) переносится дословно: она про
; машину, а не про ассемблер.

        .globl  bl_x, bl_y, bl_w, bl_h, bl_src
        .globl  blit_mask
        .globl  cp_x, cp_y, cp_w, cp_h, clear_px
        .globl  screen_rowlo, screen_rowhi

        .zeropage bl_x, bl_y, bl_w, bl_h, bl_src, bl_row, bl_cnt
        .zeropage cp_x, cp_y, cp_w, cp_h, cp_ptr, cp_x0, cp_x1, cp_lm, cp_rm

        .section .zp.bss,"aw",@nobits
bl_x:   .zero 1
bl_y:   .zero 1
bl_w:   .zero 1
bl_h:   .zero 1
bl_src: .zero 2
bl_row: .zero 1
bl_cnt: .zero 1
cp_x:   .zero 1
cp_y:   .zero 1
cp_w:   .zero 1
cp_h:   .zero 1
cp_ptr: .zero 2
cp_x0:  .zero 1
cp_x1:  .zero 1
cp_lm:  .zero 1
cp_rm:  .zero 1

        .section .text,"ax",@progbits

.macro SET_ROW_ADDR op1, op2
        ldx     bl_row
        lda     screen_rowlo,x
        clc
        adc     bl_x
        sta     \op1+1
        sta     \op2+1
        lda     screen_rowhi,x
        adc     #0
        sta     \op1+2
        sta     \op2+2
.endm

blit_mask:
        lda     bl_y
        sta     bl_row
        lda     bl_h
        sta     bl_cnt
        ldy     #0
bm_row:
        SET_ROW_ADDR bm_and, bm_sta
        ldx     #0
bm_col:
        lda     (bl_src),y
        iny
bm_and: and     $FFFF,x
        ora     (bl_src),y
        iny
bm_sta: sta     $FFFF,x
        inx
        cpx     bl_w
        bne     bm_col
        inc     bl_row
        dec     bl_cnt
        bne     bm_row
        rts

clear_px:
        lda     cp_x
        lsr     a
        sta     cp_x0

        lda     cp_x
        clc
        adc     cp_w
        sec
        sbc     #1
        pha
        lsr     a
        sta     cp_x1

        lda     cp_x
        and     #1
        tax
        lda     cp_lmask,x
        sta     cp_lm

        pla
        and     #1
        tax
        lda     cp_rmask,x
        sta     cp_rm

        lda     cp_y
        sta     bl_row
        lda     cp_h
        sta     bl_cnt

cp_row:
        ldx     bl_row
        lda     screen_rowlo,x
        sta     cp_ptr
        lda     screen_rowhi,x
        sta     cp_ptr+1

        ldy     cp_x0
        cpy     cp_x1
        beq     cp_single

        lda     cp_lm
        eor     #$FF
        and     (cp_ptr),y
        sta     (cp_ptr),y
        iny

        lda     #0
cp_mid: cpy     cp_x1
        beq     cp_right
        sta     (cp_ptr),y
        iny
        bne     cp_mid

cp_right:
        lda     cp_rm
        eor     #$FF
        and     (cp_ptr),y
        sta     (cp_ptr),y
        jmp     cp_next

cp_single:
        lda     cp_lm
        and     cp_rm
        eor     #$FF
        and     (cp_ptr),y
        sta     (cp_ptr),y

cp_next:
        inc     bl_row
        dec     bl_cnt
        bne     cp_row
        rts

        .section .rodata,"a",@progbits
cp_lmask:
        .byte   $FF, $0F
cp_rmask:
        .byte   $F0, $FF
