#!/usr/bin/env python3
"""Generates src/level.hpp. Edit the layouts below, or edit the ASCII maps in level.hpp directly."""
import os

class Map:
    def __init__(s, W, H): s.W, s.H = W, H; s.g = [['.'] * W for _ in range(H)]
    def put(s, x, y, ch):
        assert s.g[y][x] == '.', (x, y, s.g[y][x], ch); s.g[y][x] = ch
    def block(s, x0, x1, top, bottom):
        for x in range(x0, x1 + 1):
            for y in range(top, bottom + 1): s.g[y][x] = '#'
    def ground(s, x0, x1, top): s.block(x0, x1, top, s.H - 1)
    def top(s, x): return next(y for y in range(s.H) if s.g[y][x] == '#')
    def on(s, x, ch): s.put(x, s.top(x) - 1, ch)
    def plat(s, x0, x1, y):
        for x in range(x0, x1 + 1): s.put(x, y, '=')
    def liana(s, x, y0, y1, orbs=()):
        for y in range(y0, y1 + 1): s.put(x, y, 'i' if y in orbs else '|')
    def lift(s, x, y0, y1, orbs=()):
        for y in range(y0, y1 + 1): s.put(x, y, ';' if y in orbs else ':')
    def rows(s): return [''.join(r) for r in s.g]

# ---------------------------------------------------------------- Level 1: Heidestein
m = Map(152, 15)
m.ground(0, 13, 11); m.ground(14, 22, 10)
m.on(3, 'P'); m.on(5, 'H'); m.on(1, 'T'); m.on(11, 'T'); m.on(20, 'T')
for x in (7, 8, 9): m.on(x, 'o')
for x in (16, 17, 18): m.put(x, 7, 'o')
m.put(23, 8, 'o'); m.put(24, 7, 'o'); m.put(25, 8, 'o')        # first pit
m.ground(26, 44, 11)
m.plat(29, 32, 8)
for x in (29, 30, 31, 32): m.put(x, 7, 'o')
m.plat(35, 38, 5); m.put(36, 4, 'o'); m.put(37, 4, 'o')
m.on(28, 'T'); m.on(35, 'E'); m.on(40, '^'); m.on(41, '^'); m.on(43, 'T')
m.put(47, 11, '~')                                              # drifting cloud over the wide pit
for x in (46, 48, 49): m.put(x, 8, 'o')
m.ground(51, 57, 11); m.ground(58, 70, 8)
m.on(53, 'T'); m.on(56, 'M'); m.put(56, 6, 'o'); m.put(57, 5, 'o')
m.on(60, 'C'); m.on(62, 'o'); m.on(63, 'o')
m.on(66, 'S'); m.on(64, 'b'); m.on(69, 'b'); m.on(68, 'E')      # the sheep shed, sheep, and a wolf
m.ground(71, 85, 12)                                            # bramble valley crossed on planks
for x in range(73, 84): m.on(x, '^')
m.plat(72, 74, 8); m.plat(77, 79, 7); m.plat(82, 84, 8)
m.put(73, 6, 'o'); m.put(78, 5, 'o'); m.put(83, 6, 'o'); m.put(80, 4, 'F')
m.ground(86, 100, 10)
m.on(87, 'T'); m.on(88, 'o'); m.on(90, 'E'); m.on(97, 'E'); m.on(99, 'T')
m.plat(92, 94, 7)
for x in (92, 93, 94): m.put(x, 6, 'o')
m.ground(103, 104, 10); m.on(104, 'M')                          # pillars over the big pit
m.ground(109, 110, 7)
m.put(106, 4, 'o'); m.put(107, 3, 'o'); m.put(108, 4, 'o')
m.put(112, 6, 'o'); m.put(113, 7, 'o'); m.put(114, 8, 'o')
m.ground(116, 135, 11)
m.on(118, 'C'); m.on(120, 'T'); m.on(123, '^'); m.on(124, '^')
m.plat(126, 129, 8)
for x in (126, 127, 128, 129): m.put(x, 7, 'o')
m.on(129, 'E'); m.on(131, 'T'); m.put(133, 6, 'F')
m.ground(136, 137, 10); m.ground(138, 139, 9); m.ground(140, 151, 8)
for x in (141, 142, 143): m.on(x, 'o')
m.on(145, 'T'); m.on(147, 'G'); m.on(150, 'T')
level1 = m.rows()

# ---------------------------------------------------------------- Level 2: Vinales (climbs up and to the right)
m = Map(100, 46)
m.ground(0, 19, 42); m.ground(20, 36, 34)                       # valley floor and the first cliff
m.on(3, 'P'); m.on(5, 'H'); m.on(1, 'T'); m.on(11, 'S'); m.on(15, 'T')
m.on(7, 'o'); m.on(8, 'o')
m.plat(16, 18, 32); m.liana(17, 33, 40, orbs=(35, 37, 39))      # liana up the cliff
m.on(22, 'T'); m.on(24, 'o'); m.on(25, 'o'); m.on(27, 'E')
m.on(31, '^'); m.on(32, '^'); m.put(31, 31, 'o'); m.put(32, 31, 'o'); m.on(34, 'T'); m.on(35, 'o')
m.lift(38, 27, 34, orbs=(29, 31))                               # fruit lift
m.block(42, 50, 27, 29)
m.on(43, 'C'); m.on(45, 'T'); m.on(47, 'o'); m.on(49, 'M')      # drum bounces you up
m.put(50, 23, 'o'); m.put(51, 21, 'o'); m.plat(52, 54, 22); m.put(53, 21, 'o')
for x in range(56, 61): m.put(x, 22, '~')                       # fruit ferry
m.put(58, 20, 'o'); m.put(59, 20, 'o')
m.block(63, 70, 22, 24)
m.on(64, 'T'); m.on(66, 'E'); m.put(66, 17, 'F'); m.on(69, 'o')
m.plat(72, 74, 13); m.liana(73, 14, 20, orbs=(15, 17, 19))      # two lianas, jump from one to the other
m.plat(76, 78, 8); m.liana(77, 9, 16, orbs=(10, 12, 14)); m.put(75, 12, 'o')
m.block(80, 88, 11, 13)
m.on(81, 'C'); m.on(83, 'T'); m.on(85, 'E'); m.on(87, 'o')
m.lift(90, 5, 11, orbs=(7, 9))                                  # last fruit lift to the summit
m.block(94, 99, 5, 7)
m.on(95, 'o'); m.on(96, 'o'); m.on(97, 'G'); m.on(99, 'T')
level2 = m.rows()

def arr(name, rows): return 'static const char* const %s[] = {\n' % name + ''.join('    "%s",\n' % r for r in rows) + '};\n'
out = '''#pragma once
// Level maps. One character = one 48px tile.
//   #  ground        =  one-way plank       ^  thorns
//   o  orb           M  bouncer (mushroom / drum)
//   ~  platform that drifts sideways (a row of ~ sets how far it travels)
//   :  platform that rides up and down this column   ;  same, with an orb
//   |  liana to climb (a column of them)             i  same, with an orb
//   E  walking enemy F  flying enemy        C  checkpoint
//   P  player start  G  goal star
//   Scenery: T tree, S hut / sheep shed, b sheep, H name sign
// Regenerate with tools/make_level.py, or just edit the rows by hand
// (all rows of a level must have the same length).
''' + arr('LEVEL1_ROWS', level1) + '\n' + arr('LEVEL2_ROWS', level2) + '''
struct LevelDef { const char* name; const char* const* rows; int height; };
static const LevelDef LEVELS[] = {
    {"Heidestein", LEVEL1_ROWS, int(sizeof(LEVEL1_ROWS) / sizeof(LEVEL1_ROWS[0]))},
    {"Vi\\u00f1ales", LEVEL2_ROWS, int(sizeof(LEVEL2_ROWS) / sizeof(LEVEL2_ROWS[0]))},
};
static const int LEVEL_COUNT = int(sizeof(LEVELS) / sizeof(LEVELS[0]));
'''
open(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'src', 'level.hpp'), 'w').write(out)
print('\n'.join(level2))
