/* Разведочный link-скрипт: ОЗУ под ПЗУ на llvm-mos.
 *
 * Отличие от штатного mos-platform/atari8/link.ld - регион hidden с адресом
 * ИСПОЛНЕНИЯ $E400 (под ПЗУ ОС) и адресом ЗАГРУЗКИ в обычном ОЗУ (AT>ram).
 *
 * Секции разложены поимённо, а не через INCLUDE c.ld: .hidden обязана лечь
 * МЕЖДУ rodata и data. Если положить её после c.ld, её адрес загрузки уезжает
 * за .bss (та NOLOAD, но место в регионе занимает), то есть за конец
 * загружаемого образа, и переносить оказывается нечего - проверено, заголовок
 * XEX обещал 591 байт при файле в 472.
 */
__rc0 = 0x80;
INCLUDE imag-regs.ld
ASSERT(__rc31 == 0x9f, "Inconsistent zero page map.")

MEMORY {
    zp : ORIGIN = __rc31 + 1, LENGTH = 0x100 - (__rc31 + 1)
    ram (rw) : ORIGIN = 0x2000, LENGTH = 0x7000
    hidden (rw) : ORIGIN = 0xe400, LENGTH = 0x1000
}

REGION_ALIAS("c_readonly", ram)
REGION_ALIAS("c_writeable", ram)

SECTIONS {
    INCLUDE zp.ld
    INCLUDE text.ld
    INCLUDE rodata.ld

    .romdata : { *(.romdata) } >hidden AT>ram
    __hidden_load  = LOADADDR(.romdata);
    __hidden_start = ADDR(.romdata);
    __hidden_size  = SIZEOF(.romdata);

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
