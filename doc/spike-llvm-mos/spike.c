/*
 * Разведка llvm-mos: работает ли ОЗУ под ПЗУ.
 *
 * pattern лежит в секции .romdata, у которой адрес ИСПОЛНЕНИЯ $E400 (под ПЗУ
 * операционной системы), а адрес ЗАГРУЗКИ - в обычном ОЗУ. Перенос делает
 * init50.S до вызова main.
 *
 * Ответ виден по цвету фона: зелёный - узор доехал, красный - нет.
 */
#include <stdint.h>

/* volatile обязателен: без него LTO подставляет содержимое массива прямо в
 * сравнение, выбрасывает и сам массив, и секцию - проверять становится
 * нечего. Секция .romdata тогда выходит нулевого размера. */
extern volatile const uint8_t pattern[];

#define COLBK (*(volatile uint8_t *)0xD01A)

int main(void)
{
    uint8_t ok = 1, i;

    for (i = 0; i < 8u; ++i)
        if (pattern[i] != (uint8_t)(0x11u * (i + 1u)))
            ok = 0;

    COLBK = ok ? 0xC6u : 0x36u;         /* зелёный : красный */
    for (;;)
        ;
}

__attribute__((section(".romdata")))
volatile const uint8_t pattern[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
