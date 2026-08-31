#!/bin/bash
# play.sh - запуск игры в atari800 в режиме PAL с подачей нажатий.
#
#   tools/play.sh <файл.xex> [выход.png] [сценарий]
#
# Сценарий - список шагов "клавиша:секунды" через запятую, например:
#   Left:2,Up:1.5,Right:2,ctrl:0.2
#
# Зачем: в ~/.atari800.cfg джойстик 0 привязан к стрелкам клавиатуры
# (SDL_JOY_0_LEFT=276 и далее, кнопка - правый Ctrl), поэтому игру можно
# гонять без настоящего джойстика, подавая нажатия через XTEST. Именно XTEST,
# а не XSendEvent: синтетические события SDL игнорирует.
#
# Режим PAL задаётся и в ~/.atari800.cfg (DEFAULT_TV_MODE=PAL), и ключом -pal
# здесь - ключ надёжнее, он не зависит от того, не переписали ли конфиг.

set -u
cd "$(dirname "$0")/.."

XEX=${1:?нужен путь к .xex}
OUT=${2:-build/play.png}
SCRIPT=${3:-}

DISP=:99
export DISPLAY=$DISP
export SDL_VIDEODRIVER=x11

if ! xdpyinfo >/dev/null 2>&1; then
    Xvfb $DISP -screen 0 800x600x24 >/dev/null 2>&1 &
    sleep 1
fi

pkill -x atari800 2>/dev/null
sleep 0.5

# Окно 336x240 - это ровно видимая область Atari без масштабирования,
# иначе кадр приходит увеличенным и обрезанным.
atari800 -pal -windowed -win-width 336 -win-height 240 \
         -run "$XEX" >/dev/null 2>&1 &
EMU=$!

sleep 9      # загрузка ОС и XEX

WID=$(xwininfo -root -children 2>/dev/null | awk '/^     0x/ {print $1}' | head -1)
[ -n "$WID" ] && xdotool windowfocus "$WID" 2>/dev/null
sleep 0.3

if [ -n "$SCRIPT" ]; then
    IFS=',' read -ra STEPS <<< "$SCRIPT"
    for step in "${STEPS[@]}"; do
        KEY=${step%%:*}
        SEC=${step##*:}
        xdotool keydown "$KEY" 2>/dev/null
        sleep "$SEC"
        xdotool keyup "$KEY" 2>/dev/null
    done
    sleep 0.5
fi

import -window "$WID" "$OUT" 2>/dev/null
kill "$EMU" 2>/dev/null
wait "$EMU" 2>/dev/null

[ -s "$OUT" ] || { echo "кадр не снят" >&2; exit 1; }
identify "$OUT"
