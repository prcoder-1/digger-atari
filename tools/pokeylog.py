#!/usr/bin/env python3
"""
pokeylog.py - разбор дампа регистров POKEY из atari800.

Звук нельзя проверить снимком экрана, а счётчики показывают только, что
значения дошли до регистров, но не какие они. Эмулятор умеет писать сами
регистры, и по ним восстанавливаются и высота тона, и огибающая, и темп -
объективно, без прослушивания.

Снять дамп:
    atari800 -pal -pokeyrec -pokeyrec-ascii -pokeyrec-interval 30 \\
             -pokeyrec-file pk.dat -run игра.xex

Интервал задаётся в строках растра: 312 - раз в кадр, 30 - примерно десять
замеров на кадр, чего хватает, чтобы увидеть ступени огибающей.

    tools/pokeylog.py pk.dat
"""

import sys

# Порядок регистров в строке дампа: AUDF1 AUDC1 AUDF2 AUDC2 AUDF3 AUDC3
# AUDF4 AUDC4 AUDCTL
AUDF1, AUDC1, AUDF2, AUDC2, AUDF3, AUDC3, AUDF4, AUDC4, AUDCTL = range(9)

# Должен совпадать с -pokeyrec-interval, которым снят дамп
INTERVAL = 30

PAL_CLOCK = 1773447          # тактовая частота процессора PAL
CLOCK_64K = PAL_CLOCK / 28   # база "64 кГц" на PAL - 63337 Гц

TIMBRE = {0x00: 'poly5+17', 0x20: 'poly5', 0x40: 'poly5+4',
          0x80: 'poly17', 0xA0: 'чистый тон', 0xC0: 'poly4',
          0xE0: 'чистый тон'}


def freq_linked(n):
    """Связанная пара каналов 1+2 при тактовой 1.79 МГц."""
    return PAL_CLOCK / (2.0 * (n + 7)) if n + 7 else 0.0


def freq_single(audf):
    """Одиночный канал от базы 64 кГц."""
    return CLOCK_64K / (2.0 * (audf + 1))


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1

    rows = []
    for line in open(sys.argv[1]):
        line = line.strip()
        if len(line) < 18:
            continue
        rows.append([int(line[i * 2:i * 2 + 2], 16) for i in range(9)])

    if not rows:
        print('дамп пуст')
        return 1

    # AUDCTL берётся с момента, когда впервые появился звук: в начале дампа
    # игра ещё не стартовала, и там лежит значение после включения машины.
    ctl = rows[0][AUDCTL]
    for r in rows:
        if (r[AUDC2] & 0x0F) or (r[AUDC3] & 0x0F) or (r[AUDC4] & 0x0F):
            ctl = r[AUDCTL]
            break
    linked = bool(ctl & 0x10)
    print('замеров: %d, AUDCTL при первом звуке: $%02X%s'
          % (len(rows), ctl,
             '  (каналы 1+2 связаны)' if linked else '  (каналы НЕ связаны)'))

    # Сколько замеров приходится на кадр: нужно, чтобы показывать огибающую
    # по кадрам. Спад идёт из кадрового прерывания, и в пределах одного кадра
    # громкость постоянна - при мелком шаге это выглядит как "нет затухания".
    per_frame = max(1, round(312.0 / INTERVAL))
    print('интервал записи: %d строк растра, это ~%d замеров на кадр'
          % (INTERVAL, per_frame))
    print()

    # Голос эффектов - связанная пара, звук снимается со старшего канала
    events = []
    prev_vol = 0
    for i, r in enumerate(rows):
        vol = r[AUDC2] & 0x0F
        if vol and not prev_vol:                # начало звука
            n = r[AUDF1] | (r[AUDF2] << 8)
            events.append([i, n, r[AUDC2] & 0xF0, [vol]])
        elif vol and events:
            events[-1][3].append(vol)
        prev_vol = vol

    if not events:
        print('на голосе эффектов (каналы 1+2) звука не было')
    else:
        print('=== голос эффектов: %d нот ===' % len(events))
        for i, (t, n, timbre, vols) in enumerate(events[:12]):
            f = freq_linked(n)
            # Громкость прореживается до одного значения на кадр
            by_frame = vols[::per_frame][:10]
            print('  замер %5d  N=%-6d %7.1f Гц  %-11s  длит. %d кадров'
                  % (t, n, f, TIMBRE.get(timbre, '?'),
                     max(1, len(vols) // per_frame)))
            print('        громкость по кадрам: %s'
                  % '>'.join(str(v) for v in by_frame))
        if len(events) > 12:
            print('  ... ещё %d' % (len(events) - 12))

        # Огибающая: убывает ли громкость внутри ноты
        decaying = sum(1 for e in events if len(e[3]) > per_frame
                       and e[3][-1] < e[3][0])
        held = sum(1 for e in events if len(e[3]) > per_frame
                   and e[3][-1] == e[3][0])
        print()
        print('  с затуханием: %d, с постоянной громкостью: %d'
              % (decaying, held))

    # Метка профилирования: AUDC4 = $20 при идущем игровом такте. Громкость
    # в ней нулевая, звука нет, но доля замеров с меткой прямо даёт
    # длительность такта в долях кадра.
    marks = sum(1 for r in rows if r[AUDC4] == 0x20)
    if marks:
        frac = float(marks) / len(rows)
        print()
        print('=== профиль игрового такта ===')
        print('  такт занимает %.1f%% кадра = %.1f строк растра из 312'
              % (100.0 * frac, 312.0 * frac))
        print('  при такте раз в 5 кадров это %.1f%% бюджета 10 Гц'
              % (100.0 * frac / 5.0))

    # Мелодия и бас - одиночные каналы
    for name, af, ac in (('мелодия (канал 3)', AUDF3, AUDC3),
                         ('бас (канал 4)', AUDF4, AUDC4)):
        notes = sum(1 for i in range(1, len(rows))
                    if (rows[i][ac] & 0x0F) and
                    (not (rows[i - 1][ac] & 0x0F) or
                     rows[i][af] != rows[i - 1][af]))
        print()
        print('=== %s: %d нот ===' % (name, notes))
        if notes:
            # Начала нот и расстояния между ними: по ним считается темп.
            # Это и есть проверка темпа - на слух он не измеряется.
            # Начало ноты - это либо появление громкости, либо смена высоты
            # при уже звучащем канале. Считать только по громкости нельзя:
            # проигрыватель не гасит канал между нотами, и подряд идущие ноты
            # слились бы в одну.
            starts = [i for i in range(1, len(rows))
                      if (rows[i][ac] & 0x0F) and
                      (not (rows[i - 1][ac] & 0x0F) or
                       rows[i][af] != rows[i - 1][af])]
            for i in starts[:6]:
                print('  замер %5d  AUDF=%-4d %7.1f Гц'
                      % (i, rows[i][af], freq_single(rows[i][af])))
            gaps = [(starts[i + 1] - starts[i]) / float(per_frame)
                    for i in range(len(starts) - 1)]
            if gaps:
                print('  интервалы между нотами, кадров: %s'
                      % ', '.join('%.1f' % g for g in gaps[:10]))
                print('  ожидается: восьмая 9.1, четверть 18.2, '
                      'пять восьмых 45.4 кадра')
    return 0


if __name__ == '__main__':
    sys.exit(main())
