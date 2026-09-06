#!/bin/bash
# Сборка разведочной программы. SDK нужно сначала собрать и установить:
#
#   cd ~/ATARI/llvm-mos-sdk/build && make common atari8-common atari8
#   cmake --install . --prefix <куда-нибудь>
#
# В поставке лежит только исходное дерево платформ; готовых библиотек и
# link-скриптов в дереве компилятора нет, и без установки линковать нечем.
set -eu
MOS=${MOS:-$HOME/ATARI/llvm-mos-sdk/build/_deps/llvm-mos-src/bin}
SDK=${SDK:?укажи каталог, куда установлен mos-platform (там же bin/mos-atari8.cfg)}
cd "$(dirname "$0")"
$MOS/mos-clang --config "$SDK/bin/mos-atari8.cfg" -Oz \
    -o spike.xex spike.c init50.S -T link.ld -Wl,-Map,spike.map
echo "готово: spike.xex - граница экрана зелёная, если ОЗУ под ПЗУ работает"
