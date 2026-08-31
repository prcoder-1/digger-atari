#!/usr/bin/env python3
"""
music2c.py - мелодии из порта на БК -> данные для ca65.

Источник: digger_music_background.c порта на БК-0010. Там мелодии уже
разложены по байту на ноту, и эта кодировка переносится без изменений:

    n < 14      восьмая, высота n
    n + 14      четверть той же высоты
    n + 28      нота длиной пять восьмых

Высота - индекс в таблице из четырнадцати периодов. Периоды пересчитываются
в делители POKEY один раз здесь, на PC, а не в игре: AUDF = 63337/(2f) - 1,
где f = 59664/период. Восьмибитного делителя от базы 64 кГц хватает на весь
диапазон мелодии, поэтому связывать каналы не нужно - они заняты эффектами.

    tools/music2c.py --src <digger_music_background.c> --asm assets/music.s
"""

import argparse
import re
import sys

CLOCK_64K = 1773447.0 / 28.0    # база "64 кГц" на PAL - 63337 Гц


def parse_note_defines(path):
    """Ноты в digger_music.h заданы как (<делитель>u / N), где N = 20.

    Делитель - это делитель таймера оригинала для PC, поэтому период БК
    получается целочисленным делением, ровно как в самом порте.
    """
    text = open(path, errors='replace').read()
    m = re.search(r'#define\s+N\s+(\d+)', text)
    div = int(m.group(1)) if m else 20
    out = {}
    for name, raw in re.findall(r'#define\s+([A-G]S?\d)\s*\(\s*(\d+)u?\s*/\s*N\s*\)',
                                text):
        out[name] = int(raw) // div
    return out


def parse_array(text, name):
    m = re.search(r'\b' + re.escape(name) + r'\s*\[\s*\]\s*=\s*\{(.*?)\}',
                  text, re.S)
    if not m:
        return None
    body = re.sub(r'/\*.*?\*/', ' ', m.group(1), flags=re.S)
    body = re.sub(r'//[^\n]*', ' ', body)
    return [int(v) for v in re.findall(r'\b\d+\b', body)]


def audf_for_period(p):
    """Период БК -> восьмибитный делитель POKEY от базы 64 кГц."""
    f = 59664.0 / p
    n = int(round(CLOCK_64K / (2.0 * f) - 1.0))
    return max(0, min(255, n)), f


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--src', default='/home/prcoder/БК0010-develop/'
                    'digger-bk0010/digger_music_background.c')
    ap.add_argument('--asm', default='assets/music.s')
    a = ap.parse_args()

    text = open(a.src, errors='replace').read()

    # Периоды записаны именами нот, а не числами: разрешаем по заголовку
    notes_h = a.src.rsplit('/', 1)[0] + '/digger_music.h'
    defines = parse_note_defines(notes_h)
    m = re.search(r'note_period\s*\[\s*\]\s*=\s*\{([^}]*)\}', text, re.S)
    if not m:
        print('таблица note_period не найдена', file=sys.stderr)
        return 1
    names = re.findall(r'\b([A-G]S?\d)\b', m.group(1))
    periods = [defines[n] for n in names if n in defines]
    if len(periods) != len(names):
        print('не все ноты разрешены: %s' % names, file=sys.stderr)
        return 1

    tunes = {}
    for name in ('popcorn_notes', 'bonus_notes'):
        v = parse_array(text, name)
        if v:
            tunes[name] = v

    if not tunes:
        print('мелодии не найдены', file=sys.stderr)
        return 1

    lines = [
        '; music.s - СГЕНЕРИРОВАНО tools/music2c.py, не править вручную.',
        '; Источник: мелодии из порта Digger на БК-0010.',
        ';',
        '; Кодировка ноты (перенесена без изменений):',
        ';   n < 14   восьмая, высота n',
        ';   n + 14   четверть',
        ';   n + 28   пять восьмых',
        ';   0xFF     конец мелодии',
        ';',
        '; Делители POKEY посчитаны здесь, на PC: в игре не осталось ни',
        '; одного деления на ноту.',
        '',
        '        .segment "RODATA"',
        '',
        '        .export _music_audf',
        '_music_audf:',
    ]
    for i, p in enumerate(periods):
        n, f = audf_for_period(p)
        got = CLOCK_64K / (2.0 * (n + 1))
        cents = 1200.0 * (got / f - 1.0) / 0.0577623 * 0.0577623 if f else 0
        cents = 1731.2 * (got / f - 1.0)
        lines.append('        .byte %3d   ; период %3d -> %7.1f Гц, ошибка %+.0f центов'
                     % (n, p, got, cents))
    lines.append('')

    for name, notes in tunes.items():
        sym = '_music_' + name.replace('_notes', '')
        lines.append('        .export %s' % sym)
        lines.append('%s:            ; %d нот' % (sym, len(notes)))
        for i in range(0, len(notes), 16):
            chunk = notes[i:i + 16]
            lines.append('        .byte ' + ','.join(str(v) for v in chunk))
        lines.append('        .byte $FF')
        lines.append('')

    with open(a.asm, 'w') as f:
        f.write('\n'.join(lines) + '\n')

    print('нот в таблице высот: %d' % len(periods))
    for name, notes in tunes.items():
        print('  %-16s %d нот' % (name, len(notes)))
    print('asm:', a.asm)
    return 0


if __name__ == '__main__':
    sys.exit(main())
