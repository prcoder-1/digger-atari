# Digger для ATARI 65XE / 130XE (cc65 + ca65)

# Основная цель - atarixl: ПЗУ ОС выключается, спрайты уезжают в ОЗУ под ним.
# Цель atari (27648 байт под код и данные) игру больше не вмещает - переход
# был предусмотрен планом и произошёл на подключении шрифта счёта.
TARGET    := atarixl
CFG       := cfg/digger-xl.cfg
BUILD     := build
XEX       := $(BUILD)/digger.xex
MAP       := $(BUILD)/digger.map
LBL       := $(BUILD)/digger.lbl

# -Osir: оптимизация + inline + register vars. --standard c99 обязателен:
# без него cc65 отдаёт fastcall/cdecl/asm в пространство имён пользователя.
# -j НЕ передавать: char в cc65 беззнаковый, знаковые таблицы - явный int8_t.
#
# -Cl (локальные переменные в статической памяти) когда-то был запрещён: он
# молча ломал рекурсивный move_bag(). Рекурсии в порте больше нет - толкание
# мешков переписано итерацией, - а прерывания написаны целиком на ассемблере
# (src/vbi.s, dli.s, music.s, pokey.s), то есть повторного входа в функции на
# Си неоткуда взяться. Флаг вернули: он снимает всю возню с кадром на стеке Си
# (pusha/decsp и чтение параметров через (sp),y) и стоит 990 байт кода из
# 22298, то есть 4,4%. Заодно код становится быстрее - демо-прогон за те же
# 35 секунд уходит с такта 236 на такт 285.
#
# --codesize НЕ помогает: и 50, и 20 дают код БОЛЬШЕ, чем 100 по умолчанию,
# на полтора килобайта - проверено, MAIN переполняется.
#
# Про -Osir надо помнить вот что: на M3 обнаружилось, что cc65 2.19 с
# -O/-Os/-Osir неверно собирает процедуру гашения прямоугольника - счётчик
# цикла по строкам инициализируется не нулём, и тело не выполняется ни разу.
# Тот же исходник без оптимизации работает. Если поведение выглядит
# невозможным, стоит пересобрать без -Osir и сравнить: это самый быстрый
# способ отличить свою ошибку от ошибки кодогенерации.
CFLAGS    := -t $(TARGET) --cpu 6502 -Osir -Cl --standard c99 -I src
ASFLAGS   := -t $(TARGET) --cpu 6502 -I src
LDFLAGS   := -C $(CFG) -m $(MAP) -Ln $(LBL)

# Демо-сборка добавляет к игре маршруты и телеметрию - около килобайта, - и
# MAIN от этого переполняется. Стек Си ей режется вдвое: замеренный максимум
# кадра 11 байт, рекурсии в порте нет, так что 176 байт - это полтора десятка
# вложенных вызовов с запасом. Релизной сборки это не касается, там $0200.


# Заголовки перечислены явно: без этой зависимости правка hw.h не вызывала
# пересборку, и палитра менялась в исходнике, но не в игре. Та же ошибка уже
# была с конфигом линковщика - и оба раза выглядела как "код не работает".
HDRS      := $(wildcard src/*.h)

CSRC      := $(wildcard src/*.c)
ASRC      := $(wildcard src/*.s)

GSRC      := $(wildcard assets/*.s)
OBJS      := $(patsubst src/%.c,$(BUILD)/%.o,$(CSRC)) \
             $(patsubst src/%.s,$(BUILD)/%.o,$(ASRC)) \
             $(patsubst assets/%.s,$(BUILD)/a_%.o,$(GSRC))

EMU       := atari800
# PAL задаётся и в ~/.atari800.cfg (DEFAULT_TV_MODE=PAL), и ключом - ключ
# надёжнее. -xl это 800XL с 64К, ближайший эквивалент 65XE; конфиг иначе
# поднял бы RAM_SIZE=576 и скрыл бы выход за пределы памяти.
EMUFLAGS  := -pal -nobasic

# Цель по умолчанию задана ЯВНО. Без этой строки ею становится первое же
# правило в файле, а блок сборки llvm-mos стоит выше all - и "make" без
# аргументов молча собирал пустой каталог build-mos вместо игры. Релизный
# digger.xex при этом не пересобирался вовсе, а проверки "make && ..."
# выглядели успешными.
.DEFAULT_GOAL := all

.PHONY: all clean run run-game run130 size atr demo play sound mos mos-demo title

# ---------------------------------------------------------------------------
# Сборка тем же исходником, но компилятором llvm-mos.
#
# Ассемблер НЕ дублируется: файлы src/*.s и assets/*.s переводятся на лету
# скриптом tools/ca65_to_mos.py. Источник правды остаётся один - иначе однажды
# поправят только одну редакцию блиттера.
#
# SDK надо один раз собрать и установить, в поставке его нет:
#   cd ~/ATARI/llvm-mos-sdk/build && make common atari8-common atari8
#   cmake --install . --prefix ~/ATARI/llvm-mos-sdk/install
# Разбор устройства и ловушек - в doc/llvm-mos.md.
# ---------------------------------------------------------------------------
MOSBIN    ?= $(HOME)/ATARI/llvm-mos-sdk/build/_deps/llvm-mos-src/bin
MOSSDK    ?= $(HOME)/ATARI/llvm-mos-sdk/install
MOSBUILD  := build-mos
MOSFLAGS  := --config $(MOSSDK)/bin/mos-atari8.cfg -Oz --std=c99 -I src \
             -Wno-unknown-pragmas
# Заголовок куска XEX с картами уровней - принадлежность cc65: там карты
# переносятся под ПЗУ отдельным куском файла. У llvm-mos своя раскладка, и
# перевод этого файла бессмысленен.
MOSSKIP   := src/lvlhdr.s
MOSASM    := $(patsubst src/%.s,$(MOSBUILD)/%.S,$(filter-out $(MOSSKIP),$(ASRC))) \
             $(patsubst assets/%.s,$(MOSBUILD)/a_%.S,$(GSRC))

$(MOSBUILD):
	@mkdir -p $(MOSBUILD)

$(MOSBUILD)/%.S: src/%.s tools/ca65_to_mos.py | $(MOSBUILD)
	python3 tools/ca65_to_mos.py $< $@

$(MOSBUILD)/a_%.S: assets/%.s tools/ca65_to_mos.py | $(MOSBUILD)
	python3 tools/ca65_to_mos.py $< $@

mos: $(MOSASM) $(CSRC) $(HDRS) mos/init.S mos/nmi.S mos/link.ld | $(MOSBUILD)
	$(MOSBIN)/mos-clang $(MOSFLAGS) $(MOSEXTRA) -o $(MOSBUILD)/digger.xex \
	    $(CSRC) $(MOSASM) mos/init.S mos/nmi.S -T mos/link.ld \
	    -Wl,-Map,$(MOSBUILD)/digger.map
	@echo "сборка llvm-mos: $(MOSBUILD)/digger.xex"

mos-demo:
	$(MAKE) MOSEXTRA="-DDIGGER_DEMO" MOSBUILD=build-mos-demo mos


# ---------------------------------------------------------------------------
# ЗАСТАВКА. Отдельная программа: свой конфиг, своя обвязка, свои ассеты.
# В файл игры она кладётся ПЕРВОЙ и запускается вектором INITAD - разбор в
# cfg/title.cfg. Исходники лежат в title/, а не в src/: там их подобрал бы
# wildcard игры.
# ---------------------------------------------------------------------------
TBUILD    := build-title
TCFG      := cfg/title.cfg
TCSRC     := $(wildcard title/*.c)
TASRC     := $(wildcard title/*.s) $(wildcard assets-title/*.s)
TOBJS     := $(patsubst title/%.c,$(TBUILD)/%.o,$(TCSRC)) \
             $(patsubst title/%.s,$(TBUILD)/%.o,$(filter title/%,$(TASRC))) \
             $(patsubst assets-title/%.s,$(TBUILD)/t_%.o,$(filter assets-title/%,$(TASRC)))

$(TBUILD):
	@mkdir -p $(TBUILD)

$(TBUILD)/%.o: title/%.c $(HDRS) | $(TBUILD)
	cc65 $(CFLAGS) -o $(@:.o=.s) $<
	ca65 $(ASFLAGS) -o $@ $(@:.o=.s)

$(TBUILD)/%.o: title/%.s | $(TBUILD)
	ca65 $(ASFLAGS) -o $@ $<

$(TBUILD)/t_%.o: assets-title/%.s | $(TBUILD)
	ca65 $(ASFLAGS) -o $@ $<

$(TBUILD)/title.xex: $(TOBJS) $(TCFG)
	ld65 -C $(TCFG) -m $(TBUILD)/title.map -o $@ $(TOBJS) $(TARGET).lib

title: $(TBUILD)/title.xex
	@echo "заставка: $(TBUILD)/title.xex"

# Игра с заставкой: куски заставки идут в файле ПЕРЕД кусками игры. Формат
# XEX это позволяет - признак $FFFF в начале каждого файла необязателен для
# продолжения, а загрузчик просто читает куски подряд.
$(BUILD)/digger_title.xex: $(TBUILD)/title.xex $(XEX)
	cat $(TBUILD)/title.xex $(XEX) > $@
	@echo "игра с заставкой: $@"

all: $(XEX) $(BUILD)/digger_title.xex

# Сборка с зашитым маршрутом и телеметрией: единственный способ проверить
# геймплей автоматически, потому что послать ввод в эмулятор из песочницы
# нельзя. В релиз не входит.
demo:
	$(MAKE) BUILD=build-demo CFLAGS="$(CFLAGS) -DDIGGER_DEMO" \
	        LDFLAGS="-C $(CFG) -m build-demo/digger.map -Ln build-demo/digger.lbl -D__STACKSIZE__=176" all
	@echo "демо-сборка: build-demo/digger.xex"

# Замер звука: пересобирает демо-сборку и разбирает дамп регистров POKEY.
# Пересборка внутри скрипта не для удобства - однажды дамп был снят с файла,
# собранного до появления огибающей, и дал ложный вывод.
# Загрузочный образ дискеты: игра кладётся как AUTORUN.SYS в копию шаблона
# с уже установленной DOS. Своя файловая система через "atr mkfs" не годится -
# она без загрузочных секторов и не грузится.
# В образ дискеты кладётся игра С ЗАСТАВКОЙ: это и есть то, что играют.
atr: $(BUILD)/digger_title.xex
	tools/mkatr.sh $(BUILD)/digger_title.xex $(BUILD)/digger.atr

sound:
	tools/sound.sh


$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD)/%.o: src/%.c $(HDRS) | $(BUILD)
	cc65 $(CFLAGS) -o $(BUILD)/$*.s $<
	ca65 $(ASFLAGS) -o $@ $(BUILD)/$*.s

$(BUILD)/%.o: src/%.s $(HDRS) | $(BUILD)
	ca65 $(ASFLAGS) -o $@ $<

# Сгенерированные данные (assets/*.s) - префикс a_, чтобы имена объектных
# файлов не сталкивались с одноимёнными модулями из src/
$(BUILD)/a_%.o: assets/%.s | $(BUILD)
	ca65 $(ASFLAGS) -o $@ $<

$(XEX): $(OBJS) $(CFG)
	ld65 $(LDFLAGS) -o $@ $(OBJS) $(TARGET).lib

# 65XE = 800XL с 64К - это основная цель
run: $(BUILD)/digger_title.xex
	$(EMU) $(EMUFLAGS) -xl -run $(BUILD)/digger_title.xex

# Игра без заставки - для проверок: заставка каждый раз требует нажатия
run-game: $(XEX)
	$(EMU) $(EMUFLAGS) -xl -run $(XEX)

run130: $(XEX)
	$(EMU) $(EMUFLAGS) -xe -run $(XEX)

# Прогон с подачей нажатий: в ~/.atari800.cfg джойстик 0 привязан к стрелкам
# клавиатуры, поэтому играть можно без джойстика.
#   make play KEYS=Left:2,Up:1.5,Right:2
play: $(XEX)
	tools/play.sh $(XEX) build/play.png "$(KEYS)"

# Сводка по размеру: сколько занято в MAIN и сколько осталось
size: $(XEX)
	@echo "--- размер XEX ---"
	@ls -l $(XEX) | awk '{print $$5 " байт"}'
	@echo "--- сегменты ---"
	@sed -n '/Segment list/,/^$$/p' $(MAP) | head -30

clean:
	rm -rf $(BUILD) $(TBUILD)
