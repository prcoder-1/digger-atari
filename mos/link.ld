/* Раскладка памяти порта Digger для llvm-mos.
 *
 * Повторяет cfg/digger-xl.cfg по адресам - они заданы железом (кадровый буфер
 * выровнен на 4К, буфер спрайтов на 2К) и логикой порта, - но устроена иначе.
 * У cc65 данные под ПЗУ едут через промежуточный кусок XEX и процедуру
 * библиотеки; здесь то же самое делается раздельными адресами загрузки и
 * исполнения (AT>ram) и своей секцией .init.50 в mos/init.S.
 *
 * Разбор механизма и четыре ловушки, на которых он ловится, - в doc/llvm-mos.md.
 */
/* Мнимые регистры уведены с $80 на $20. Порт держит в нулевой странице
 * параметры блиттера, спрайтов, прерывания по строке и звукового движка -
 * полутора сотен байт, и на штатных 96 они не помещаются. Ниже $20 не
 * опускаемся: там RTCLOK, по которому считает кадры wait_frame. */
__rc0 = 0x20;
INCLUDE imag-regs.ld
ASSERT(__rc31 == 0x3f, "Inconsistent zero page map.")

MEMORY {
    zp     : ORIGIN = __rc31 + 1, LENGTH = 0x100 - (__rc31 + 1)

    /* Код, данные и переменные. XEX грузится, пока ПЗУ ОС ещё включено,
     * поэтому весь образ обязан лечь сюда, ниже $9000. */
    ram    : ORIGIN = 0x2000, LENGTH = 0x7000

    /* Только резервируют адреса, в файл не попадают */
    scrbuf : ORIGIN = 0x9000, LENGTH = 0x20a0   /* кадровый буфер ANTIC F */
    dlist  : ORIGIN = 0xb400, LENGTH = 0x0100
    rowtab : ORIGIN = 0xb500, LENGTH = 0x0300   /* таблицы адресов строк */
    pmgbuf : ORIGIN = 0xb800, LENGTH = 0x0800   /* игроки, выравнено на 2К */

    /* Куда КЛАДЁТСЯ образ спрайтов при загрузке. Обе области в этот момент
     * ещё пусты: кадровый буфер заполняет screen_init, буфер игроков -
     * pmg_init, и оба зовутся уже после переноса. Держать образ в основном
     * ОЗУ нельзя - там не хватает девяти килобайт; у cc65 та же задача решена
     * отдельным куском XEX, который потом затирается основным.
     *
     * Области нарочно ОТДЕЛЬНЫЕ, хотя адреса те же: у линковщика на регион
     * один счётчик, и если класть образ прямо в scrbuf, он съест место у
     * самого кадрового буфера. */
    stage1 : ORIGIN = 0x9000, LENGTH = 0x20a0
    stage2 : ORIGIN = 0xb800, LENGTH = 0x0800

    /* ОЗУ под ПЗУ. Последние шесть байт не наши - там векторы 6502. */
    hidden3 : ORIGIN = 0xc000, LENGTH = 0x1000
    hidden2 : ORIGIN = 0xd800, LENGTH = 0x0800
    hidden  : ORIGIN = 0xe400, LENGTH = 0xfffa - 0xe400
}

REGION_ALIAS("c_readonly", ram)
REGION_ALIAS("c_writeable", ram)

SECTIONS {
    INCLUDE zp.ld
    INCLUDE text.ld
    INCLUDE rodata.ld

    /* Спрайты. Лежат ПЕРЕД .data не для красоты: секция с AT>ram, положенная
     * после .bss, получает адрес загрузки за концом образа, и переносить
     * становится нечего. */
    .romdata : { *(.romdata) } >hidden AT>stage1
    __romdata_load  = LOADADDR(.romdata);
    __romdata_start = ADDR(.romdata);
    __romdata_size  = SIZEOF(.romdata);

    .romdata2 : { *(.romdata2) } >hidden2 AT>stage2
    __romdata2_load  = LOADADDR(.romdata2);
    __romdata2_start = ADDR(.romdata2);
    __romdata2_size  = SIZEOF(.romdata2);

    INCLUDE data.ld
    INCLUDE bss.ld
    INCLUDE noinit.ld

    .scrbuf (NOLOAD) : { *(.scrbuf) } >scrbuf
    .dlist  (NOLOAD) : { *(.dlist)  } >dlist
    .rowtab (NOLOAD) : { *(.rowtab) } >rowtab
    .pmgbuf (NOLOAD) : { *(.pmgbuf) } >pmgbuf

    /* У cc65 часть статики модулей уезжала в ОЗУ под ПЗУ, потому что MAIN был
     * забит. Здесь места хватает, и она лежит в обычной .bss - а процедура
     * clear_hi_bss из util.s остаётся, ей просто нечего чистить. */
    __bss_hi_start = 0;
    __bss_hi_size  = 0;

    /* Карты уровней у cc65 переносятся под ПЗУ, здесь они лежат на месте:
     * нулевой размер делает copy_levels пустым циклом. */
    __levels_load  = 0;
    __levels_start = 0;
    __levels_size  = 0;
}

/* XEX из трёх кусков: вектор запуска, основной образ и образы спрайтов,
 * которые кладутся в ещё не занятые буферы и переносятся под ПЗУ. */
OUTPUT_FORMAT {
    SHORT(0xffff)
    SHORT(0x02e0)
    SHORT(0x02e1)
    SHORT(_start)

    SHORT(0x2000)
    SHORT(--__data_end - 1)
    TRIM(ram)

    SHORT(--__romdata_load)
    SHORT(--__romdata_load + --__romdata_size - 1)
    TRIM(stage1)

    SHORT(--__romdata2_load)
    SHORT(--__romdata2_load + --__romdata2_size - 1)
    TRIM(stage2)
}
