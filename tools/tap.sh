#!/bin/bash
# tap.sh - КОРОТКОЕ нажатие клавиши и снимок кадра позже.
#
#   tools/tap.sh <файл.xex> <выход.png> <до_нажатия> <после> [второе_нажатие]
#
# Третий аргумент - через сколько секунд ПОСЛЕ первого нажатия сделать второе.
# Заставка двухступенчатая: первое нажатие уводит с обложки в демо, второе
# запускает игру, и проверить весь путь одним прогоном можно только так.
#
# Отличие от keytest.sh: там клавиша УДЕРЖИВАЕТСЯ до самого снимка. Заставке
# это не годится - она ждёт любого нажатия и на удержании проскочила бы и
# обложку, и демо разом. Здесь нажатие короткое, а снимок берётся спустя время.
#
# Окно ищется так же, как в shot.sh: единственное дочернее окно корня.
set -u
XEX=${1:?нужен путь к .xex}
OUT=${2:?нужен путь к .png}
TAP=${3:-4}
AFTER=${4:-6}
TAP2=${5:-}

DISP=${DIGGER_DISPLAY:-:97}
PIDFILE=${DIGGER_PIDFILE:-/tmp/.digger-atari800.pid}
export DISPLAY=$DISP
export SDL_VIDEODRIVER=x11

if ! xdpyinfo >/dev/null 2>&1; then
    Xvfb $DISP -screen 0 800x600x24 >/dev/null 2>&1 &
    sleep 1
fi

if [ -f "$PIDFILE" ]; then kill "$(cat "$PIDFILE")" 2>/dev/null; rm -f "$PIDFILE"; fi
for _ in $(seq 20); do
    xwininfo -root -children 2>/dev/null | grep -q '^     0x' || break
    sleep 0.5
done

atari800 -pal -xl -nobasic -nosound -windowed \
         -win-width 336 -win-height 240 -run "$XEX" >/dev/null 2>&1 &
EMU_PID=$!
echo "$EMU_PID" > "$PIDFILE"

sleep "$TAP"
WID=$(xwininfo -root -children 2>/dev/null | awk '/^     0x/ {print $1}' | head -1)
# Клавиши шлются В ФОКУС, а не ключом --window: SDL читает ввод X, а адресные
# события XSendEvent до него не доходят. Ровно та же оговорка в keytest.sh.
[ -n "$WID" ] && xdotool windowfocus "$WID" 2>/dev/null
xdotool keydown space 2>/dev/null
sleep 0.2
xdotool keyup space 2>/dev/null

if [ -n "$TAP2" ]; then
    sleep "$TAP2"
    xdotool keydown space 2>/dev/null
    sleep 0.2
    xdotool keyup space 2>/dev/null
    sleep "$AFTER"
else
    sleep "$AFTER"
fi

if [ -n "$WID" ]; then
    import -window "$WID" "$OUT" 2>/dev/null
else
    import -window root "$OUT" 2>/dev/null
fi

kill "$EMU_PID" 2>/dev/null
wait "$EMU_PID" 2>/dev/null
rm -f "$PIDFILE"
[ -s "$OUT" ] || { echo "кадр не снят" >&2; exit 1; }
identify "$OUT"
