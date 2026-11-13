#!/bin/bash
# mkatr.sh - собрать загрузочный образ дискеты .ATR с игрой.
#
#   tools/mkatr.sh [файл.xex] [выход.atr]
#
# Образ делается копией готового шаблона с DOS 2.0S, в котором игра кладётся
# под именем AUTORUN.SYS - тогда она запускается сразу после загрузки DOS.
#
# ВАЖНО: "atr <образ> mkfs" использовать нельзя. Он создаёт файловую систему,
# но НЕ пишет загрузочные сектора и не кладёт DOS.SYS, поэтому такая дискета
# не грузится. Нужен именно шаблон с уже установленной DOS.

set -u
cd "$(dirname "$0")/.."

XEX=${1:-build/digger.xex}
OUT=${2:-build/digger.atr}

# Шаблон берётся из Python-пакета atrcopy; другой можно задать в ATR_TEMPLATE
TEMPLATE=${ATR_TEMPLATE:-$(python3 -c 'import atrcopy, os; print(os.path.dirname(atrcopy.__file__))' 2>/dev/null)/templates/dos2sd+2.0s.atr}

[ -f "$XEX" ] || { echo "нет файла $XEX - сначала make"; exit 1; }
[ -f "$TEMPLATE" ] || { echo "не найден шаблон DOS: $TEMPLATE (pip install atrcopy или ATR_TEMPLATE)"; exit 1; }
command -v atr >/dev/null || { echo "нет утилиты atr (atari-tools)"; exit 1; }

cp "$TEMPLATE" "$OUT"

# В шаблоне уже есть AUTORUN.SYS - его надо убрать, иначе игра не встанет
atr "$OUT" rm AUTORUN.SYS >/dev/null 2>&1

atr "$OUT" put "$XEX" AUTORUN.SYS >/dev/null 2>&1 \
    || { echo "не удалось записать игру в образ"; exit 1; }

echo "образ: $OUT"
atr "$OUT" ls -la
echo
atr "$OUT" free
