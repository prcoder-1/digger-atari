#!/bin/bash
# sound.sh - снять дамп регистров POKEY и разобрать его.
#
#   tools/sound.sh [секунд_прогона]
#
# Скрипт САМ пересобирает демо-сборку перед замером. Это не удобство, а
# защита от конкретной ошибки: дамп уже один раз снимался с двоичного файла,
# собранного до того, как в исходники добавили огибающую, и по нему был
# сделан вывод "огибающая не работает". Кода огибающей в том файле не было.
#
# Каталог сборки СВОЙ, не build-demo: ключи здесь другие (DEMO_MISSILE), а
# make различает только даты файлов, не флаги. Собранное этим скриптом
# подменяло сборку для tools/check.sh, и проверка сверяла чужой двоичный файл.
#
# Звук нельзя проверить ни снимком экрана, ни счётчиками: счётчики
# показывают только, что значения дошли до регистров, но не какие они.
# По дампу восстанавливаются высота, тембр, длительность в кадрах и форма
# огибающей - то есть всё, что нужно и для эффектов, и для темпа музыки.

set -u
cd "$(dirname "$0")/.."

SECS=${1:-25}
SND_EXTRA=${SND_EXTRA:--DDEMO_MISSILE}
DUMP=build/pokey.dat

# Цель обязана совпадать с той, на которую собирается игра: линковка идёт
# библиотекой из Makefile. Здесь оставалась цель "atari", и после перехода
# на "atarixl" скрипт молча падал на сборке.
#
# Ключи ОБЯЗАНЫ совпадать с Makefile: там давно стоит -Cl, а здесь его не было,
# и сборка просто не влезала в MAIN - скрипт молча падал на "сборка не удалась".
# Стек демо-сборке урезан там же, поэтому и он повторён.
CFLAGS_DEMO="-t atarixl --cpu 6502 -Osir -Cl --standard c99 -I src -DDIGGER_DEMO ${SND_EXTRA:-}"

# Каталог сносится ПОЛНОСТЬЮ: make сверяет даты, а не ключи, и объектники от
# прошлого замера с другими -D молча переезжали в новую сборку. Один раз это
# уже стоило получаса - ld65 ругался переполнением на ровном месте.
echo "сборка демо..."
rm -rf build-sound
make BUILD=build-sound CFLAGS="$CFLAGS_DEMO" \
     LDFLAGS="-C cfg/digger-xl.cfg -m build-sound/digger.map -Ln build-sound/digger.lbl -D__STACKSIZE__=176" \
     >/dev/null 2>&1 \
    || { echo "сборка не удалась"; exit 1; }

# Свой дисплей и убийство ТОЛЬКО своего эмулятора - как в tools/shot.sh.
# На общем :99 работает эмулятор соседнего проекта, и "pkill -x atari800" глушил
# чужую работу.
export DISPLAY=${DIGGER_DISPLAY:-:97}
export SDL_VIDEODRIVER=x11
PIDFILE=${DIGGER_PIDFILE:-/tmp/.digger-atari800.pid}
if ! xdpyinfo >/dev/null 2>&1; then
    Xvfb "$DISPLAY" -screen 0 800x600x24 >/dev/null 2>&1 &
    sleep 1
fi

if [ -f "$PIDFILE" ]; then kill "$(cat "$PIDFILE")" 2>/dev/null; rm -f "$PIDFILE"; sleep 0.5; fi
rm -f "$DUMP"

# Интервал 30 строк растра - около десяти замеров на кадр. Раз в кадр (312)
# слишком грубо: ступени огибающей сливаются и выглядят как её отсутствие.
timeout "$SECS" atari800 -pal -windowed -win-width 336 -win-height 240 \
    -pokeyrec -pokeyrec-ascii -pokeyrec-interval 30 -pokeyrec-file "$DUMP" \
    -run build-sound/digger.xex >/dev/null 2>&1 &
echo "$!" > "$PIDFILE"
wait $! 2>/dev/null

if [ -f "$PIDFILE" ]; then kill "$(cat "$PIDFILE")" 2>/dev/null; rm -f "$PIDFILE"; fi
[ -s "$DUMP" ] || { echo "дамп не создан"; exit 1; }

python3 tools/pokeylog.py "$DUMP"
