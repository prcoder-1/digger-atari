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

# ОТДЕЛЬНЫЙ дисплей и убийство ТОЛЬКО своего эмулятора - и то, и другое не
# прихоть. На :99 работает эмулятор соседнего проекта: "pkill -x atari800" бил
# по имени процесса и глушил чужую работу, а снимок брался с чужого окна и
# выходил пустым - выглядело это как поломка игры. Свой процесс запоминается по
# номеру в PIDFILE и добивается по нему.
DISP=${DIGGER_DISPLAY:-:97}
PIDFILE=${DIGGER_PIDFILE:-/tmp/.digger-atari800.pid}

kill_prev() {
    if [ -f "$PIDFILE" ]; then
        kill "$(cat "$PIDFILE")" 2>/dev/null
        rm -f "$PIDFILE"
        sleep 0.5
    fi
}
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
kill_prev
# Ждать ИСЧЕЗНОВЕНИЯ окна, а не спать наугад: полсекунды хватало не всегда, и
# снимок брался с мёртвого окна прошлого прогона - кадр выходил пустым, а
# выглядело это как поломка игры. Ровно так же сделано в tools/keytest.sh.
for _ in $(seq 20); do
    xwininfo -root -children 2>/dev/null | grep -q '^     0x' || break
    sleep 0.5
done

atari800 -pal -xl -nobasic -nosound -windowed \
         -win-width 336 -win-height 240 \
         "$@" -run "$XEX" >/dev/null 2>&1 &
EMU_PID=$!
echo "$EMU_PID" > "$PIDFILE"

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
