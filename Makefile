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
# -Cl (static locals) НЕ передавать: молча ломает рекурсивный move_bag().
#
# Про -Osir надо помнить вот что: на M3 обнаружилось, что cc65 2.19 с
# -O/-Os/-Osir неверно собирает процедуру гашения прямоугольника - счётчик
# цикла по строкам инициализируется не нулём, и тело не выполняется ни разу.
# Тот же исходник без оптимизации работает. Если поведение выглядит
# невозможным, стоит пересобрать без -Osir и сравнить: это самый быстрый
# способ отличить свою ошибку от ошибки кодогенерации.
CFLAGS    := -t $(TARGET) --cpu 6502 -Osir --standard c99 -I src
ASFLAGS   := -t $(TARGET) --cpu 6502 -I src
LDFLAGS   := -C $(CFG) -m $(MAP) -Ln $(LBL)

# Демо-сборка добавляет к игре маршруты и телеметрию - около килобайта, - и
# MAIN от этого переполняется. Стек Си ей режется вдвое: замеренный максимум
# кадра 11 байт, рекурсии в порте нет, так что 192 байта - это полтора десятка
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

.PHONY: all clean run run130 size atr demo play sound

all: $(XEX)

# Сборка с зашитым маршрутом и телеметрией: единственный способ проверить
# геймплей автоматически, потому что послать ввод в эмулятор из песочницы
# нельзя. В релиз не входит.
demo:
	$(MAKE) BUILD=build-demo CFLAGS="$(CFLAGS) -DDIGGER_DEMO" \
	        LDFLAGS="-C $(CFG) -m build-demo/digger.map -Ln build-demo/digger.lbl -D__STACKSIZE__=192" all
	@echo "демо-сборка: build-demo/digger.xex"

# Замер звука: пересобирает демо-сборку и разбирает дамп регистров POKEY.
# Пересборка внутри скрипта не для удобства - однажды дамп был снят с файла,
# собранного до появления огибающей, и дал ложный вывод.
# Загрузочный образ дискеты: игра кладётся как AUTORUN.SYS в копию шаблона
# с уже установленной DOS. Своя файловая система через "atr mkfs" не годится -
# она без загрузочных секторов и не грузится.
atr: $(XEX)
	tools/mkatr.sh $(XEX) $(BUILD)/digger.atr

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
run: $(XEX)
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
	rm -rf $(BUILD)
