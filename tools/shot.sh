#!/bin/bash
# shot.sh - запускает XEX в atari800 под Xvfb и снимает кадр в PNG.
#
#   tools/shot.sh <файл.xex> <выход.png> [секунд_до_снимка] [доп.флаги atari800]
#
# Кадр берётся прямо из X-окна эмулятора через import: это надёжнее, чем F10,
# потому что не зависит от того, дошло ли нажатие клавиши до SDL.
# Обязателен SDL_VIDEODRIVER=x11 - иначе SDL молча уходит в другой бэкенд.
#
# ВНИМАНИЕ: не использовать "pkill -f atari800" - под шаблон попадает и сама
# оболочка, из которой запущен скрипт. Только "pkill -x atari800".

set -u
XEX=${1:?нужен путь к .xex}
OUT=${2:?нужен путь к .png}
DELAY=${3:-4}
shift 3 2>/dev/null || shift $#

DISP=:99
export DISPLAY=$DISP
export SDL_VIDEODRIVER=x11

# Поднимаем Xvfb, если ещё не запущен
if ! xdpyinfo >/dev/null 2>&1; then
    Xvfb $DISP -screen 0 800x600x24 >/dev/null 2>&1 &
    sleep 1
fi

# Добить эмулятор от прошлого запуска ОБЯЗАТЕЛЬНО: окно ищется как первое
# дочернее окно корня, и живой эмулятор с прошлого раза перехватывает снимок.
# Это уже приводило к ложному выводу - три сборки с разными цветами дали
# побайтово одинаковые кадры.
pkill -x atari800 2>/dev/null
sleep 0.5

atari800 -pal -xl -nobasic -nosound -windowed \
         -win-width 336 -win-height 240 \
         "$@" -run "$XEX" >/dev/null 2>&1 &
EMU_PID=$!

sleep "$DELAY"

# Окно эмулятора - единственное дочернее окно корня
WID=$(xwininfo -root -children 2>/dev/null \
      | awk '/^     0x/ {print $1}' | head -1)

if [ -n "$WID" ]; then
    import -window "$WID" "$OUT" 2>/dev/null
else
    import -window root "$OUT" 2>/dev/null
fi

kill "$EMU_PID" 2>/dev/null
wait "$EMU_PID" 2>/dev/null

if [ ! -s "$OUT" ]; then
    echo "кадр не снят" >&2
    exit 1
fi
identify "$OUT"
