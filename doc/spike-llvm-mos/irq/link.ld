/* Как в первой разведке, но регион hidden кончается на $FFF9: последние шесть
 * байт - векторы 6502, и с выключенным ПЗУ они наши. */
__rc0 = 0x80;
INCLUDE imag-regs.ld
ASSERT(__rc31 == 0x9f, "Inconsistent zero page map.")

MEMORY {
    zp : ORIGIN = __rc31 + 1, LENGTH = 0x100 - (__rc31 + 1)
    ram (rw) : ORIGIN = 0x2000, LENGTH = 0x7000
    hidden (rw) : ORIGIN = 0xe400, LENGTH = 0xfffa - 0xe400
}

REGION_ALIAS("c_readonly", ram)
REGION_ALIAS("c_writeable", ram)

SECTIONS {
    INCLUDE zp.ld
    INCLUDE text.ld
    INCLUDE rodata.ld
    INCLUDE data.ld
    INCLUDE bss.ld
    INCLUDE noinit.ld
}

OUTPUT_FORMAT {
    SHORT(0xffff)
    SHORT(0x02e0)
    SHORT(0x02e1)
    SHORT(_start)
    SHORT(0x2000)
    SHORT(--__data_end - 1)
    TRIM(ram)
}
