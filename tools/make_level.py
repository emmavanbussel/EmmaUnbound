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
    def sand(s, x0, x1, top):
        for x in range(x0, x1 + 1):
            for y in range(top, s.H): s.g[y][x] = ','
    def top(s, x): return next(y for y in range(s.H) if s.g[y][x] in '#,')
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
m.sand(26, 44, 11)                                              # sandy plain with a dead tree, a log and a deer
m.plat(29, 32, 8)
for x in (29, 30, 31, 32): m.put(x, 7, 'o')
m.plat(35, 38, 5); m.put(36, 4, 'o'); m.put(37, 4, 'o')
m.on(28, 'T'); m.on(31, 'D'); m.on(33, 'l'); m.on(35, 'E'); m.on(38, 'd'); m.on(40, '^'); m.on(41, '^'); m.on(43, 'T'); m.on(44, 'k')
m.block(45, 50, 13, 14)                                         # small lake, crossed on the drifting cloud
for x in range(45, 51): m.put(x, 12, 'w'); m.put(x, 11, 'w')
m.g[11][45] = 'r'; m.g[11][50] = 'r'; m.put(47, 2, 'f')         # reeds; 'f' = fog hangs around here
m.put(47, 10, '~')
for x in (46, 48, 49): m.put(x, 7, 'o')
m.ground(51, 57, 11); m.ground(58, 70, 8)
m.on(53, 'T'); m.on(56, 'M'); m.put(56, 6, 'o'); m.put(57, 5, 'o')
m.on(60, 'C'); m.on(62, 'o'); m.on(63, 'o')
m.on(66, 'S'); m.on(64, 'b'); m.on(69, 'b'); m.on(68, 'E')      # the sheep shed, sheep, and a wolf
m.ground(71, 85, 12)                                            # bramble valley crossed on planks
for x in range(73, 84): m.on(x, '^')
m.plat(72, 74, 8); m.plat(77, 79, 7); m.plat(82, 84, 8)
m.put(73, 6, 'o'); m.put(78, 5, 'o'); m.put(83, 6, 'o'); m.put(80, 4, 'F')
m.ground(86, 100, 10)
m.on(87, 'T'); m.on(88, 'o'); m.on(90, 'E'); m.on(97, 'E'); m.on(99, 'k')
m.plat(92, 94, 7)
for x in (92, 93, 94): m.put(x, 6, 'o')
m.block(101, 115, 14, 14)                                       # the big misty lake, with two pillars standing in it
m.ground(103, 104, 10); m.on(104, 'M')
m.ground(109, 110, 7)
for x in range(101, 116):
    for y in (12, 13):
        if m.g[y][x] == '.': m.g[y][x] = 'w'
m.g[12][101] = 'r'; m.g[12][106] = 'r'; m.g[12][115] = 'r'; m.put(108, 1, 'f')
m.put(106, 4, 'o'); m.put(107, 3, 'o'); m.put(108, 4, 'o')
m.put(112, 6, 'o'); m.put(113, 7, 'o'); m.put(114, 8, 'o')
m.sand(116, 135, 11)                                            # second sandy plain
m.on(117, 'k'); m.on(118, 'C'); m.on(120, 'T'); m.on(121, 'D'); m.on(123, '^'); m.on(124, '^')
m.plat(126, 129, 8)
for x in (126, 127, 128, 129): m.put(x, 7, 'o')
m.on(129, 'E'); m.on(131, 'd'); m.on(133, 'l'); m.put(133, 6, 'F')
m.ground(136, 137, 10); m.ground(138, 139, 9); m.ground(140, 151, 8)
for x in (141, 142, 143): m.on(x, 'o')
m.on(145, 'T'); m.on(147, 'G'); m.on(150, 'T')
level1 = m.rows()

# ---------------------------------------------------------------- Level 2: Vinales (climbs up and to the right)
D = 43                                                          # everything after the river sits this much further right
m = Map(100 + D, 46)
m.ground(0, 37, 42); m.ground(38, 55, 45); m.ground(56, 19 + D, 42); m.ground(20 + D, 36 + D, 34)
m.on(1, 'T'); m.on(3, 'P'); m.on(5, 'H')                        # the village: cottages, power lines, the old bus
m.on(7, 'p'); m.on(10, 'c'); m.on(14, 'j'); m.on(17, 'p'); m.on(20, 'U'); m.on(24, 'p')
m.put(8, 39, 'o'); m.put(9, 39, 'o'); m.on(23, 'o')
m.on(27, 'S')                                                   # tobacco shed and field
for x in range(31, 37): m.on(x, 't')
for x in range(38, 56):                                         # the muddy river, crossed on a broken plank bridge
    m.put(x, 44, 'w'); m.put(x, 43, 'w')
    m.put(x, 42, 'w' if x in (43, 44, 50, 51) else 'u')
m.put(43, 39, 'o'); m.put(44, 39, 'o'); m.put(50, 39, 'o'); m.put(51, 39, 'o'); m.put(47, 36, 'F')
m.on(57, 'C'); m.on(58, 'T')
m.plat(16 + D, 18 + D, 32); m.liana(17 + D, 33, 40, orbs=(35, 37, 39))      # liana up the cliff
m.on(21 + D, 'p'); m.on(23 + D, 'j'); m.on(25 + D, 'o'); m.on(27 + D, 'E'); m.on(29 + D, 'p')
m.on(31 + D, '^'); m.on(32 + D, '^'); m.put(31 + D, 31, 'o'); m.put(32 + D, 31, 'o'); m.on(34 + D, 'T'); m.on(35 + D, 'o')
m.lift(38 + D, 27, 34, orbs=(29, 31))                           # rope lift
m.block(42 + D, 50 + D, 27, 29)
m.on(43 + D, 'C'); m.on(45 + D, 'T'); m.on(47 + D, 'o'); m.on(49 + D, 'M')      # drum bounces you up
m.put(50 + D, 23, 'o'); m.put(51 + D, 21, 'o'); m.plat(52 + D, 54 + D, 22); m.put(53 + D, 21, 'o')
for x in range(56 + D, 61 + D): m.put(x, 22, '~')               # rope ferry
m.put(58 + D, 20, 'o'); m.put(59 + D, 20, 'o')
m.block(63 + D, 70 + D, 22, 24)
m.on(64 + D, 'T'); m.on(66 + D, 'E'); m.put(66 + D, 17, 'F'); m.on(69 + D, 'o')
m.plat(72 + D, 74 + D, 13); m.liana(73 + D, 14, 20, orbs=(15, 17, 19))      # two lianas, jump from one to the other
m.plat(76 + D, 78 + D, 8); m.liana(77 + D, 9, 16, orbs=(10, 12, 14)); m.put(75 + D, 12, 'o')
m.block(80 + D, 88 + D, 11, 13)
m.on(81 + D, 'C'); m.on(83 + D, 'T'); m.on(85 + D, 'E'); m.on(87 + D, 'o')
m.lift(90 + D, 5, 11, orbs=(7, 9))                              # last rope lift to the summit
m.block(94 + D, 99 + D, 5, 7)
m.on(95 + D, 'o'); m.on(96 + D, 'o'); m.on(97 + D, 'G'); m.on(99 + D, 'T')
level2 = m.rows()

# ---------------------------------------------------------------- Level 3: Lofoten (islands in an icy sea)
m = Map(150, 20)
def slope_up(x0, row, n):                                       # '/' tiles climbing to the right, starting in this row
    for i in range(n):
        m.put(x0 + i, row - i, '/'); m.block(x0 + i, x0 + i, row - i + 1, m.H - 1)
def slope_down(x0, row, n):                                     # '\' tiles descending to the right
    for i in range(n):
        m.put(x0 + i, row + i, '\\'); m.block(x0 + i, x0 + i, row + i + 1, m.H - 1)
def bridge(x0, x1, y, ch):
    for x in range(x0, x1 + 1): m.put(x, y, ch)
# island 1: fishing village, first slope
m.ground(0, 16, 15); slope_up(17, 14, 3); m.ground(20, 26, 12)
m.on(3, 'P'); m.on(5, 'H'); m.on(6, 'o'); m.on(7, 'o'); m.on(9, 'S'); m.on(11, 'o'); m.on(13, 'S')
m.put(17, 13, 'o'); m.put(18, 12, 'o'); m.put(19, 11, 'o'); m.on(21, 'o'); m.on(23, 'R')
bridge(27, 33, 12, 'n')                                         # arched bridge
for x in (29, 30, 31): m.put(x, 10, 'o')
# island 2: first polar bear, a dip down to the shore and back up
m.ground(34, 40, 12); m.on(38, 'E'); slope_down(41, 12, 3); m.ground(44, 52, 15)
m.on(45, 'C'); m.on(47, 'S'); m.on(49, 'o'); m.on(51, '^'); slope_up(53, 14, 4); m.ground(57, 60, 11); m.on(58, 'o')
bridge(61, 68, 11, 'u')                                         # sagging rope bridge
m.put(63, 10, 'o'); m.put(65, 10, 'o')
# island 3: bear guarding the bridge, long slide down to the boat
m.ground(69, 73, 11); m.on(71, 'E'); slope_down(74, 11, 4); m.ground(78, 84, 15)
m.on(80, 'R'); m.on(83, 'o'); m.put(82, 11, 'F')
for x in range(86, 96): m.put(x, 16, '~')                       # fishing boat ferry
m.put(89, 13, 'o'); m.put(92, 13, 'o')
# island 4: climb to the high plateau
m.ground(97, 103, 15); m.on(98, 'C'); m.on(100, 'S'); m.on(102, 'o'); slope_up(104, 14, 5); m.ground(109, 114, 10)
m.on(111, 'E'); m.plat(110, 112, 7)
for x in (110, 111, 112): m.put(x, 6, 'o')
bridge(115, 121, 10, 'n')
for x in (117, 118, 119): m.put(x, 8, 'o')
# island 5: last bear, slide down into the harbour
m.ground(122, 127, 10); m.on(125, 'E'); slope_down(128, 10, 4); m.ground(132, 149, 14)
m.on(134, 'S'); m.put(135, 9, 'F'); m.on(137, 'R'); m.on(139, 'o'); m.on(141, 'S'); m.on(143, 'o'); m.on(146, 'G'); m.on(148, 'S')
for y in range(17, 20):                                         # the sea
    for x in range(m.W):
        if m.g[y][x] == '.': m.g[y][x] = 'w'
for x in (30, 64, 118): m.g[17][x] = 'B'                        # moored boats
level3 = m.rows()

def arr(name, rows): return 'static const char* const %s[] = {\n' % name + ''.join('    "%s",\n' % r.replace('\\', '\\\\') for r in rows) + '};\n'
out = '''#pragma once
// Level maps. One character = one 48px tile.
//   #  ground        ,  sandy ground        =  one-way plank       ^  spikes
//   /  \\  slopes (put ground underneath)    w  water (falling in sends you back to the checkpoint)
//   n  arched bridge, u  sagging rope / plank bridge (a row of them is one bridge)
//   o  orb           M  bouncer (mushroom / drum)
//   ~  platform that drifts sideways (a row of ~ sets how far it travels)
//   :  platform that rides up and down this column   ;  same, with an orb
//   |  liana to climb (a column of them)             i  same, with an orb
//   E  walking enemy F  flying enemy        C  checkpoint
//   P  player start  G  goal star           f  fog hangs around this column
//   Scenery: T tree, S shed / cabin, b sheep, H name sign, t tobacco plant, R stockfish rack,
//            D deer, l fallen log, d dead tree, k birch, c j cottages, p power pole, U old bus,
//            r reeds and B moored boat (place those two in water)
// Regenerate with tools/make_level.py, or just edit the rows by hand
// (all rows of a level must have the same length).
''' + arr('LEVEL1_ROWS', level1) + '\n' + arr('LEVEL2_ROWS', level2) + '\n' + arr('LEVEL3_ROWS', level3) + '''
struct LevelDef { const char* name; const char* const* rows; int height; };
static const LevelDef LEVELS[] = {
    {"Heidestein", LEVEL1_ROWS, int(sizeof(LEVEL1_ROWS) / sizeof(LEVEL1_ROWS[0]))},
    {"Vi\\u00f1ales", LEVEL2_ROWS, int(sizeof(LEVEL2_ROWS) / sizeof(LEVEL2_ROWS[0]))},
    {"Lofoten", LEVEL3_ROWS, int(sizeof(LEVEL3_ROWS) / sizeof(LEVEL3_ROWS[0]))},
};
static const int LEVEL_COUNT = int(sizeof(LEVELS) / sizeof(LEVELS[0]));
'''
open(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'src', 'level.hpp'), 'w').write(out)
print('\n'.join(r[:152] for r in level1)); print('\n'.join(r[:70] for r in level2[30:]))
