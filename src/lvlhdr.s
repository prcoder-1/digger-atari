; lvlhdr.s - заголовок отдельного куска XEX с картами уровней.
;
; Четыре байта: адрес первого и последнего байта куска. Готового модуля для
; этого в библиотеке cc65 нет - там только EXEHDR и MAINHDR под свои куски, -
; поэтому он собран руками.
;
; Адреса берутся у линковщика: __LEVELS_LOAD__ это куда кусок грузится
; (кадровый буфер, см. LVLSTAGE в cfg/digger-xl.cfg), а не куда переносится.

        .import __LEVELS_LOAD__, __LEVELS_SIZE__

        .segment "LVLHDR"
        .word   __LEVELS_LOAD__
        .word   __LEVELS_LOAD__ + __LEVELS_SIZE__ - 1
