#pragma once
// Shared bits for the drawing code. All art is procedural vector shapes - no image files.
#include "game.hpp"
#include "gfx.hpp"
#include <cstdint>

constexpr int INK = 0x2b1b3f;  // outline colour used for everything in the foreground
constexpr float PI = 3.14159265f;

// Colours that change per level.
struct Theme {
    int sky[3];
    int dirt[4], pebble;
    int grass0, grass1, grassHi;
    int plank0, plank1, plankHi, plankLine;
    int thorn0, thorn1, thornHi;
    int water0, water1, waterHi;
};
const Theme& theme(const Game& g);

uint32_t hash2(int x, int y);
void starPath(float cx, float cy, float R, float r, float rot, int n = 5);
void heartPath(float cx, float cy, float s);

// world.cpp: sky, parallax scenery, terrain, level scenery
void drawBackground(const Game& g);
void drawFog(const Game& g, bool front);

// japan.cpp: everything specific to level 4
void drawJapanBackground(const Game& g);
bool drawJapanDecor(const Game& g, const Decor& d);
void drawLightRoom(const Game& g, bool front);
void drawRedBridge(const Bridge& b);
const char* japanPlace(float x);
void drawWires(const Game& g);
void drawDecor(const Game& g, const Decor& d);

void drawTerrain(const Game& g, int tx0, int tx1, int ty0, int ty1);
void drawPlanks(const Game& g, int tx0, int tx1, int ty0, int ty1);
void drawThorns(const Game& g, int tx0, int tx1, int ty0, int ty1);
void drawWater(const Game& g, int tx0, int tx1, int ty0, int ty1);
void drawBridge(const Game& g, const Bridge& b);

// actors.cpp: things that move or can be touched
void drawBouncer(const Game& g, const Bouncer& b);
void drawMover(const Game& g, const Mover& m, int index);
void drawLiana(const Game& g, const Liana& l);
void drawFishingBoat(float x, float y, float scale, bool cabin);  // x,y = middle of the deck
void drawCheckpoint(const Checkpoint& c, float t);
void drawGoal(const Game& g);
void drawOrb(float x, float y, float t, float phase);
void drawEnemy(const Game& g, const Enemy& e);
void drawFist(const Game& g);
void drawEmma(const Game& g);
