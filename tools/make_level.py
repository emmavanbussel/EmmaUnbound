#!/usr/bin/env python3
"""Generates src/level.hpp. Edit the layout below, or edit the ASCII map in level.hpp directly."""
import os
W, H = 152, 15
g = [['.'] * W for _ in range(H)]

def ground(x0, x1, top):
    for x in range(x0, x1 + 1):
        for y in range(top, H):
            g[y][x] = '#'

def put(x, y, ch): g[y][x] = ch
def top(x): return next(y for y in range(H) if g[y][x] == '#')
def on(x, ch): put(x, top(x) - 1, ch)
def plat(x0, x1, y):
    for x in range(x0, x1 + 1): put(x, y, '=')

# A: gentle start
ground(0, 13, 11); ground(14, 22, 10)
on(3, 'P')
for x in (7, 8, 9): on(x, 'o')
for x in (16, 17, 18): put(x, 7, 'o')
# B: first pit (23-25) with an arc of orbs
put(23, 8, 'o'); put(24, 7, 'o'); put(25, 8, 'o')
# C: platforms, first enemy, thorns
ground(26, 44, 11)
plat(29, 32, 8)
for x in (29, 30, 31, 32): put(x, 7, 'o')
plat(35, 38, 5); put(36, 4, 'o'); put(37, 4, 'o')
on(35, 'E')
on(40, '^'); on(41, '^')
# D: wide pit with a drifting cloud
put(47, 11, '~')
for x in (46, 48, 49): put(x, 8, 'o')
# E: mushroom up to the plateau, checkpoint
ground(51, 57, 11); ground(58, 70, 8)
on(56, 'M'); put(56, 6, 'o'); put(57, 5, 'o')
on(60, 'C'); on(62, 'o'); on(63, 'o'); on(66, 'E')
# F: thorn valley crossed on planks
ground(71, 85, 12)
for x in range(73, 84): on(x, '^')
plat(72, 74, 8); plat(77, 79, 7); plat(82, 84, 8)
put(73, 6, 'o'); put(78, 5, 'o'); put(83, 6, 'o')
put(80, 4, 'F')
# G: enemy meadow
ground(86, 100, 10)
on(88, 'o'); on(90, 'E'); on(97, 'E')
plat(92, 94, 7)
for x in (92, 93, 94): put(x, 6, 'o')
# H: pillars over the big pit
ground(103, 104, 10); on(104, 'M')
ground(109, 110, 7)
put(106, 4, 'o'); put(107, 3, 'o'); put(108, 4, 'o')
put(112, 6, 'o'); put(113, 7, 'o'); put(114, 8, 'o')
# I: last stretch
ground(116, 135, 11)
on(118, 'C')
on(123, '^'); on(124, '^')
plat(126, 129, 8)
for x in (126, 127, 128, 129): put(x, 7, 'o')
on(129, 'E')
put(133, 6, 'F')
ground(136, 137, 10); ground(138, 139, 9); ground(140, 151, 8)
for x in (141, 142, 143): on(x, 'o')
on(147, 'G')

rows = [''.join(r) for r in g]
out = '''#pragma once
// Level map. One character = one 48px tile.
//   #  ground        =  one-way plank     ^  thorns
//   o  orb           M  bouncy mushroom   ~  drifting cloud platform
//   E  walking enemy F  flying enemy      C  checkpoint
//   P  player start  G  goal star
// Regenerate with tools/make_level.py, or just edit the rows by hand
// (all rows must have the same length).
static const char* const LEVEL_ROWS[] = {
''' + ''.join('    "%s",\n' % r for r in rows) + '};\n'
open(os.path.join(os.path.dirname(__file__), '..', 'src', 'level.hpp'), 'w').write(out)
print('\n'.join(rows))
