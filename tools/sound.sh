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
# Звук нельзя проверить ни снимком экрана, ни счётчиками: счётчики
# показывают только, что значения дошли до регистров, но не какие они.
# По дампу восстанавливаются высота, тембр, длительность в кадрах и форма
# огибающей - то есть всё, что нужно и для эффектов, и для темпа музыки.

set -u
cd "$(dirname "$0")/.."

SECS=${1:-25}
DUMP=build/pokey.dat

CFLAGS_DEMO="-t atari --cpu 6502 -Osir --standard c99 -I src -DDIGGER_DEMO -DDEMO_MISSILE"

echo "сборка демо..."
make BUILD=build-demo CFLAGS="$CFLAGS_DEMO" >/dev/null 2>&1 \
    || { echo "сборка не удалась"; exit 1; }

export DISPLAY=:99
export SDL_VIDEODRIVER=x11
if ! xdpyinfo >/dev/null 2>&1; then
    Xvfb :99 -screen 0 800x600x24 >/dev/null 2>&1 &
    sleep 1
fi

pkill -x atari800 2>/dev/null
sleep 0.5
rm -f "$DUMP"

# Интервал 30 строк растра - около десяти замеров на кадр. Раз в кадр (312)
# слишком грубо: ступени огибающей сливаются и выглядят как её отсутствие.
timeout "$SECS" atari800 -pal -windowed -win-width 336 -win-height 240 \
    -pokeyrec -pokeyrec-ascii -pokeyrec-interval 30 -pokeyrec-file "$DUMP" \
    -run build-demo/digger.xex >/dev/null 2>&1

pkill -x atari800 2>/dev/null
[ -s "$DUMP" ] || { echo "дамп не создан"; exit 1; }

python3 tools/pokeylog.py "$DUMP"
