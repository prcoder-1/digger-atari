#!/bin/bash
# keytest.sh - запускает XEX, УДЕРЖИВАЕТ клавишу и снимает кадр.
#
#   tools/keytest.sh <файл.xex> <клавиша> <выход.png> [до_нажатия] [удержание]
#
# Единственный способ проверить ввод: демо-маршрут его не касается, а клавиши в
# сборке llvm-mos сделаны своими руками (ПЗУ ОС выключено, ячейку CH заполнять
# некому, см. mos/nmi.S). Клавиша подаётся эмулятору через xdotool в его окно.
#
# Собирать проверяемое надо с -DDIGGER_DEMO -DDEMO_KEYS: тогда ввод настоящий,
# а телеметрия на месте, и результат читается tools/telemetry.py.
#
# Имя клавиши - как у xdotool: minus, equal, plus, asterisk.

set -u
XEX=${1:?нужен путь к .xex}
KEY=${2:?нужно имя клавиши для xdotool}
OUT=${3:?нужен путь к .png}
WAIT=${4:-12}
HOLD=${5:-3}

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

if ! xdpyinfo >/dev/null 2>&1; then
    Xvfb $DISP -screen 0 800x600x24 >/dev/null 2>&1 &
    sleep 1
fi

kill_prev

atari800 -pal -xl -nobasic -nosound -windowed \
         -win-width 336 -win-height 240 \
         -run "$XEX" >/dev/null 2>&1 &
echo "$!" > "$PIDFILE"
sleep "$WAIT"

# Окно эмулятора ищется так же, как в shot.sh: единственное дочернее у корня.
WID=$(xwininfo -root -children 2>/dev/null | awk '/^     0x/ {print $1}' | head -1)
WIN=$WID

# Удержание, а не одиночное нажатие: игра забирает код из CH и ставит обратно
# $FF, а обработчик кладёт его заново, пока клавиша прижата.
# windowfocus, а НЕ windowactivate: под Xvfb оконного менеджера нет, и
# windowactivate --sync ждёт его до бесконечности. Клавиши шлются в фокус, а не
# ключом --window: SDL читает их через ввод X, а не адресными событиями.
xdotool windowfocus "$WIN" 2>/dev/null
xdotool keydown "$KEY" 2>/dev/null
sleep "$HOLD"
xdotool keyup "$KEY" 2>/dev/null

if [ -n "$WID" ]; then
    import -window "$WID" "$OUT" 2>/dev/null
else
    import -window root "$OUT" 2>/dev/null
fi
kill_prev
