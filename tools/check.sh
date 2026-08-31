#!/bin/bash
# check.sh - автопроверка геймплея по демо-сборке.
#
#   tools/check.sh [секунд_прогона]
#
# Собирает сборку с зашитым маршрутом, доводит её в эмуляторе до нужного
# момента, снимает кадр и сверяет телеметрию с эталоном tools/expect.txt.
#
# Эталон - это НЕ снимок экрана, а числа: такт, счёт, положение Диггера и
# сколько камней осталось. Картинка может незначительно отличаться от прогона
# к прогону (момент снимка), а числа на заданном такте обязаны совпадать.
#
# Обновить эталон после осознанного изменения поведения:
#   tools/check.sh --bless

set -u
cd "$(dirname "$0")/.."

BLESS=0
SECS=20
for a in "$@"; do
    case "$a" in
        --bless) BLESS=1 ;;
        *) SECS=$a ;;
    esac
done

make demo >/dev/null 2>&1 || { echo "сборка демо не удалась"; exit 1; }

SHOT=build/check.png
tools/shot.sh build-demo/digger.xex "$SHOT" "$SECS" >/dev/null 2>&1 \
    || { echo "не удалось снять кадр"; exit 1; }

GOT=$(python3 tools/telemetry.py "$SHOT" | grep 'сырые байты')

if [ "$BLESS" = "1" ]; then
    echo "$GOT" > tools/expect.txt
    echo "эталон обновлён:"
    python3 tools/telemetry.py "$SHOT"
    exit 0
fi

if [ ! -f tools/expect.txt ]; then
    echo "эталона нет - создайте его: tools/check.sh --bless"
    python3 tools/telemetry.py "$SHOT"
    exit 1
fi

WANT=$(cat tools/expect.txt)
if [ "$GOT" = "$WANT" ]; then
    echo "СОВПАЛО"
    python3 tools/telemetry.py "$SHOT"
    exit 0
fi

echo "РАСХОЖДЕНИЕ"
echo "  ожидалось: $WANT"
echo "  получено:  $GOT"
echo
python3 tools/telemetry.py "$SHOT"
exit 1
