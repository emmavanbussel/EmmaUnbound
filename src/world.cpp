// Sky, parallax scenery, terrain and level scenery. Level 1 is Heidestein (heath and pine forest),
// level 2 is Vinales (red soil, mogote hills, palms).
#include "draw.hpp"
#include <algorithm>
#include <cmath>

static const Theme THEMES[] = {
    {   // Heidestein: sandy soil, soft Dutch sky, brambles
        {0x4fb2f5, 0xb5e6ff, 0xfff3d9},
        {0xe3bd7e, 0xd3a968, 0xc09555, 0xab8046}, 0xf6ddb0,
        0x9bdc5a, 0x4fae4a, 0xd3f58a,
        0xf2bf78, 0xc98845, 0xffe0a8, 0xa56a35,
        0xe8457e, 0x9c1f4f, 0xffa3c4,
    },
    {   // Vinales: red earth, tropical sky, bamboo, cactus spikes
        {0x18b0e6, 0x8ee9f0, 0xfff2b5},
        {0xe07a4a, 0xcc623a, 0xb44f30, 0x9c4129}, 0xf5b48a,
        0x7dea4a, 0x23b04e, 0xc9ff7a,
        0xe9e37c, 0xa9b548, 0xf9f8bd, 0x7d8a2e,
        0x8fe05e, 0x2f8f47, 0xd6f7a3,
    },
};
const Theme& theme(const Game& g) { return THEMES[g.level == 0 ? 0 : 1]; }

// ------------------------------------------------------------ background

static void drawCloud(float x, float y, float s) {
    g_save(); g_translate(x, y); g_scale(s, s);
    g_fill(0xd4eeff);
    g_rrect(-6, 2, 78, 26, 13, FILL);
    g_fill(0xffffff);
    g_circle(4, 4, 20, FILL); g_circle(30, -10, 27, FILL); g_circle(58, 2, 21, FILL);
    g_rrect(-8, -2, 82, 24, 12, FILL);
    g_restore();
}

// Height of a scenery ridge. shape 0: rolling hills. shape 1: mogotes (steep rounded limestone domes).
static float ridgeY(int shape, float wx, float base, float amp, float f, float ph) {
    if (shape == 0) return base + amp * std::sin(wx * f + ph) + amp * 0.45f * std::sin(wx * f * 2.3f + ph * 2.1f);
    float dome = std::pow(std::max(0.f, std::sin(wx * f + ph)), 0.5f);
    float size = 0.55f + 0.45f * std::sin(wx * f * 0.37f + ph * 3);
    return base - amp * dome * size + 6 * std::sin(wx * f * 6);
}

static void drawRidge(int shape, float off, float base, float amp, float f, float ph, int top, int bottom) {
    g_lin(0, base - amp * (shape ? 1.0f : 1.5f), 0, VIEW_H, top, bottom);
    g_path(); g_move(0, VIEW_H);
    for (int sx = 0; sx <= VIEW_W + 8; sx += 8) g_line(sx, ridgeY(shape, sx + off, base, amp, f, ph));
    g_line(VIEW_W, VIEW_H); g_close(); g_paint(FILL);
}

static void pineSilhouette(float x, float y, float h, int color, int trunk) {
    g_fill(trunk); g_rect(x - h * 0.04f, y - h * 0.3f, h * 0.08f, h * 0.34f);
    g_fill(color);
    for (int i = 0; i < 3; i++) {
        float w = h * (0.30f - i * 0.07f), yb = y - h * (0.18f + i * 0.24f), yt = yb - h * 0.36f;
        g_path(); g_move(x - w, yb); g_quad(x, yb + h * 0.05f, x + w, yb); g_line(x, yt); g_close(); g_paint(FILL);
    }
}

static void palmSilhouette(float x, float y, float h, int color, int trunk) {
    g_stroke(trunk, h * 0.05f);
    g_path(); g_move(x, y); g_quad(x + h * 0.08f, y - h * 0.5f, x - h * 0.02f, y - h); g_paint(STROKE);
    g_fill(color);
    float cx = x - h * 0.02f, cy = y - h;
    for (int i = 0; i < 7; i++) {
        float a = -PI + i * PI / 6;
        float ex = cx + std::cos(a) * h * 0.34f, ey = cy + std::sin(a) * h * 0.16f + h * 0.12f;
        g_path(); g_move(cx, cy);
        g_quad(cx + std::cos(a) * h * 0.2f, cy + std::sin(a) * h * 0.3f - h * 0.04f, ex, ey);
        g_quad(cx + std::cos(a) * h * 0.14f, cy + std::sin(a) * h * 0.12f + h * 0.04f, cx, cy);
        g_paint(FILL);
    }
}

void drawBackground(const Game& g) {
    const Theme& th = theme(g);
    bool cuba = g.level != 0;
    float t = g.time;
    float vy = (g.H * TILE - VIEW_H - g.camY);  // how far the camera is above the bottom of the level
    float farUp = std::min(vy * 0.10f, 110.f), midUp = std::min(vy * 0.20f, 200.f), nearUp = std::min(vy * 0.34f, 330.f);
    g_lin3(0, 0, 0, VIEW_H, th.sky[0], th.sky[1], th.sky[2]);
    g_rect(0, 0, VIEW_W, VIEW_H);

    // sun with slowly turning rays
    float sx = 770 - g.camX * 0.02f, sy = 105 + std::min(vy * 0.05f, 40.f);
    g_rad(sx, sy, 260, 0xfff7c2, 0.85, 0xfff7c2, 0);
    g_circle(sx, sy, 260, FILL);
    g_save(); g_translate(sx, sy); g_rotate(t * 0.06f);
    g_alpha(0.13); g_fill(0xffffff);
    for (int i = 0; i < 12; i++) {
        g_rotate(PI / 6);
        g_path(); g_move(0, 0); g_line(-22, -330); g_line(22, -330); g_close(); g_paint(FILL);
    }
    g_alpha(1); g_restore();
    g_fill(0xfff9d8); g_circle(sx, sy, 50, FILL);
    g_fill(cuba ? 0xffd95a : 0xffe873); g_circle(sx, sy, 42, FILL);

    // clouds
    static const float clouds[][3] = {{80, 70, 1.3f}, {420, 120, 0.9f}, {700, 60, 1.1f}, {1050, 140, 1.5f},
                                      {1400, 80, 1.0f}, {1700, 150, 1.2f}};
    for (auto& c : clouds) {
        float span = 2000;
        float x = std::fmod(c[0] - g.camX * (0.06f + c[2] * 0.03f) + t * 5 * c[2], span);
        if (x < 0) x += span;
        g_alpha(0.92); drawCloud(x - 250, c[1] + std::min(vy * 0.04f, 60.f), c[2]); g_alpha(1);
    }

    if (!cuba) {
        // far: heath hills in bloom
        drawRidge(0, g.camX * 0.10f, 318 + farUp, 44, 0.0042f, 1.0f, 0xcbb2ea, 0xe9dcf6);
        // mid: a dark band of pine forest
        float off = g.camX * 0.22f, base = 372 + midUp;
        for (int k = int(std::floor(off / 38)) - 1; k <= int((off + VIEW_W) / 38) + 1; k++) {
            uint32_t h = hash2(k, 31);
            float wx = k * 38.f + h % 20;
            pineSilhouette(wx - off, ridgeY(0, wx, base, 26, 0.006f, 2.0f) + 8, 62 + (h >> 6) % 46,
                           (h >> 12) % 2 ? 0x4f9f84 : 0x5aab8e, 0x4f9f84);
        }
        drawRidge(0, off, base, 26, 0.006f, 2.0f, 0x66b596, 0x9bd4b4);
        // near: purple heather field with pines
        off = g.camX * 0.42f; base = 436 + nearUp;
        for (int k = int(std::floor(off / 130)) - 1; k <= int((off + VIEW_W) / 130) + 1; k++) {
            uint32_t h = hash2(k, 77);
            if (h % 4 == 0) continue;
            float wx = k * 130.f + (h >> 4) % 70;
            pineSilhouette(wx - off, ridgeY(0, wx, base, 26, 0.008f, 0.5f) + 8, 120 + (h >> 8) % 80,
                           (h >> 16) % 2 ? 0x3c9272 : 0x47a07c, 0x8a6a5a);
        }
        drawRidge(0, off, base, 26, 0.008f, 0.5f, 0xc58fd9, 0xe6c8ee);
        // heather speckles
        for (int k = int(std::floor(off / 34)) - 1; k <= int((off + VIEW_W) / 34) + 1; k++) {
            uint32_t h = hash2(k, 5);
            float wx = k * 34.f + h % 24, y = ridgeY(0, wx, base, 26, 0.008f, 0.5f) + 14 + (h >> 5) % 50;
            g_fill((h >> 11) % 2 ? 0xa869c9 : 0xdba6ea);
            g_ellipse(wx - off, y, 9 + (h >> 13) % 8, 4, 0, FILL);
        }
    } else {
        // far and mid: mogotes
        drawRidge(1, g.camX * 0.08f, 345 + farUp, 150, 0.0052f, 0.4f, 0x7fcdb9, 0xc4efdc);
        drawRidge(1, g.camX * 0.18f + 300, 415 + midUp, 200, 0.0038f, 2.2f, 0x4fba84, 0x9be0aa);
        // near: green fields with royal palms
        float off = g.camX * 0.42f, base = 442 + nearUp;
        for (int k = int(std::floor(off / 140)) - 1; k <= int((off + VIEW_W) / 140) + 1; k++) {
            uint32_t h = hash2(k, 91);
            if (h % 3 == 0) continue;
            float wx = k * 140.f + (h >> 4) % 80;
            palmSilhouette(wx - off, ridgeY(0, wx, base, 20, 0.008f, 0.5f) + 8, 130 + (h >> 8) % 90, 0x3fae6a, 0x9fb893);
        }
        drawRidge(0, off, base, 20, 0.008f, 0.5f, 0x8fdc5a, 0xd0f09a);
        // rows of tobacco plants
        for (int k = int(std::floor(off / 30)) - 1; k <= int((off + VIEW_W) / 30) + 1; k++) {
            float wx = k * 30.f, y = ridgeY(0, wx, base, 20, 0.008f, 0.5f) + 16;
            g_fill(k % 2 ? 0x5fc04a : 0x6fd055);
            g_ellipse(wx - off, y, 11, 5, 0, FILL); g_ellipse(wx - off + 9, y + 26, 13, 6, 0, FILL);
        }
    }
}

// ------------------------------------------------------------ level scenery

static void drawPine(float x, float y, uint32_t h) {
    float s = 0.9f + (h % 40) / 100.f;
    g_save(); g_translate(x, y); g_scale(s, s);
    const int OUT = 0x1f4d40;
    g_stroke(OUT, 3);
    g_lin(-8, 0, 8, 0, 0xb5754a, 0x8a5233); g_rrect(-8, -84, 16, 88, 4, BOTH);
    for (int i = 0; i < 4; i++) {
        float w = 62 - i * 12, yb = -56 - i * 44, yt = yb - 72;
        g_path(); g_move(-w, yb);
        g_quad(-w * 0.5f, yb + 13, 0, yb + 5); g_quad(w * 0.5f, yb + 13, w, yb);
        g_quad(w * 0.35f, yb - 30, 0, yt); g_quad(-w * 0.35f, yb - 30, -w, yb);
        g_close();
        g_lin(0, yt, 0, yb + 10, 0x55b788, 0x2f8563); g_paint(BOTH);
        g_alpha(0.45); g_stroke(0x8fe0b0, 3);
        g_path(); g_move(-w * 0.45f, yb - 12); g_quad(-w * 0.25f, yb - 32, -4, yt + 16); g_paint(STROKE);
        g_alpha(1); g_stroke(OUT, 3);
    }
    g_restore();
}

static void drawPalm(float x, float y, uint32_t h, float t) {
    float hgt = 210 + h % 90, lean = (int(h >> 8) % 50) - 25;
    float tx = x + lean, ty = y - hgt;
    const int OUT = 0x1f5a3c;
    // trunk
    g_stroke(OUT, 19); g_path(); g_move(x, y + 2); g_quad(x - lean * 0.4f, y - hgt * 0.5f, tx, ty); g_paint(STROKE);
    g_stroke(0xcdbb9a, 13); g_paint(STROKE);
    g_stroke(0xa8946f, 2);
    for (int i = 1; i < 9; i++) {
        float k = i / 9.f, px = (1 - k) * (1 - k) * x + 2 * (1 - k) * k * (x - lean * 0.4f) + k * k * tx, py = y - hgt * k;
        g_path(); g_move(px - 6, py); g_line(px + 6, py); g_paint(STROKE);
    }
    // fronds
    float sway = std::sin(t * 1.5f + x) * 4;
    for (int i = 0; i < 7; i++) {
        float a = -PI * 1.08f + i * PI * 1.16f / 6;
        float len = 78 + (i % 2) * 10;
        float ex = tx + std::cos(a) * len + sway, ey = ty + std::sin(a) * len * 0.45f + 34;
        float mx = tx + std::cos(a) * len * 0.55f, my = ty + std::sin(a) * len * 0.8f - 14;
        g_path(); g_move(tx, ty); g_quad(mx, my, ex, ey);
        g_quad(mx * 0.5f + tx * 0.5f + std::cos(a) * 8, my * 0.4f + ty * 0.6f + 18, tx, ty); g_close();
        g_fill(i % 2 ? 0x43c463 : 0x62d86f); g_stroke(OUT, 3); g_paint(BOTH);
    }
    g_fill(0x8a5a36); g_stroke(OUT, 2.5);
    g_circle(tx - 7, ty + 8, 6.5, BOTH); g_circle(tx + 6, ty + 10, 6.5, BOTH);
}

// Heidestein's schaapskooi: a low barn under a big thatched roof.
static void drawSheepShed(float x, float y) {
    g_save(); g_translate(x, y);
    g_stroke(INK, 3.5);
    // wall
    g_lin(0, -48, 0, 0, 0x9a6740, 0x7a4c2c); g_rrect(-104, -48, 208, 50, 3, BOTH);
    g_stroke(0x5f3a20, 2);
    for (int i = -4; i <= 4; i++) { if (i >= -1 && i <= 1) continue; g_path(); g_move(i * 22, -44); g_line(i * 22, -2); g_paint(STROKE); }
    // thatched roof
    g_stroke(INK, 3.5);
    g_path(); g_move(-124, -40);
    g_quad(-112, -100, -84, -128); g_quad(0, -140, 84, -128); g_quad(112, -100, 124, -40);
    g_quad(0, -52, -124, -40); g_close();
    g_lin(0, -134, 0, -44, 0xe6c877, 0xb99548); g_paint(BOTH);
    g_stroke(0x9a7a38, 2);
    for (int i = -5; i <= 5; i++) {
        float bx = i * 21.f;
        g_path(); g_move(bx * 0.74f, -122); g_quad(bx * 0.9f, -90, bx, -56); g_paint(STROKE);
    }
    g_stroke(0xf5e2a4, 3); g_path(); g_move(-76, -126); g_quad(0, -136, 76, -126); g_paint(STROKE);
    // big double door with an eyebrow in the thatch
    g_stroke(INK, 3.5);
    g_fill(0xb99548); g_path(); g_move(-40, -52); g_quad(0, -92, 40, -52); g_close(); g_paint(BOTH);
    g_fill(0xc08a52); g_rrect(-30, -62, 60, 64, 4, BOTH);
    g_stroke(INK, 2.5);
    g_path(); g_move(0, -62); g_line(0, 2);
    g_move(-27, -58); g_line(-3, -2); g_move(-3, -58); g_line(-27, -2);
    g_move(27, -58); g_line(3, -2); g_move(3, -58); g_line(27, -2); g_paint(STROKE);
    // little name board
    g_fill(0xf2dcae); g_stroke(INK, 2.5); g_rrect(-43, -84, 86, 18, 5, BOTH);
    g_text("Schaapskooi", 0, -71, 12, 1, INK, INK, 0);
    g_restore();
}

// Vinales: a thatched tobacco-drying hut.
static void drawTobaccoHut(float x, float y) {
    g_save(); g_translate(x, y);
    g_stroke(INK, 3.5);
    g_path(); g_move(-84, 2); g_quad(-40, -70, -6, -138); g_line(6, -138); g_quad(40, -70, 84, 2); g_close();
    g_lin(0, -138, 0, 0, 0xd9c574, 0xa98f45); g_paint(BOTH);
    g_stroke(0x8a7434, 2);
    for (int i = -4; i <= 4; i++) { g_path(); g_move(i * 2.f, -124); g_quad(i * 9.f, -70, i * 17.f, -4); g_paint(STROKE); }
    g_stroke(INK, 4);
    g_path(); g_move(-12, -128); g_line(12, -152); g_move(12, -128); g_line(-12, -152); g_paint(STROKE);
    g_stroke(INK, 3.5);
    g_fill(0x4a2f2a); g_path(); g_move(-24, 2); g_line(-24, -44); g_quad(0, -66, 24, -44); g_line(24, 2); g_close(); g_paint(BOTH);
    // tobacco leaves hung up to dry
    g_stroke(0xc9a45c, 3); g_path(); g_move(-22, -42); g_line(22, -42); g_paint(STROKE);
    g_stroke(INK, 2);
    for (int i = -2; i <= 2; i++) { g_fill(i % 2 ? 0xa9903c : 0x7fae45); g_ellipse(i * 8.5f, -30, 4.2, 11, i * 0.06f, BOTH); }
    g_restore();
}

static void drawSheep(float x, float y, float t, uint32_t h) {
    float ph = t * 0.35f + (h % 100) * 0.1f;
    float dx = std::sin(ph) * 22, dir = std::cos(ph) > 0 ? 1.f : -1.f;
    float walk = std::fabs(std::cos(ph)) > 0.25f ? std::sin(t * 7 + h % 7) : 0;
    bool graze = std::fabs(std::cos(ph)) <= 0.25f;
    g_save(); g_translate(x + dx, y); g_scale(dir, 1);
    g_stroke(INK, 2.5); g_fill(0x4a3b52);
    g_rrect(-13 + walk * 2, -12, 5, 13, 2, BOTH); g_rrect(7 - walk * 2, -12, 5, 13, 2, BOTH);
    g_rrect(-8 - walk * 2, -12, 5, 13, 2, BOTH); g_rrect(11 + walk * 2, -12, 5, 13, 2, BOTH);
    static const float wool[][3] = {{-13, -22, 10}, {-3, -29, 11}, {9, -27, 10}, {14, -19, 9}, {2, -17, 11}, {-9, -15, 9}};
    g_stroke(INK, 5.5); g_fill(INK);
    for (auto& w : wool) g_circle(w[0], w[1], w[2], BOTH);
    g_fill(0xfffaf0);
    for (auto& w : wool) g_circle(w[0], w[1], w[2], FILL);
    g_fill(0xe9e1d3); g_circle(-6, -14, 5, FILL); g_circle(8, -15, 5, FILL);
    // head
    float hy = graze ? -8 : -27, hx = graze ? 24 : 21;
    g_stroke(INK, 2.5); g_fill(0x4a3b52);
    g_ellipse(hx - 6, hy - 6, 5, 2.6, -0.5, BOTH);
    g_ellipse(hx, hy, 8, 9.5, graze ? 0.5 : 0.15, BOTH);
    g_fill(0xfffaf0); g_circle(hx - 3, hy - 9, 5, FILL);
    g_fill(0xffffff); g_circle(hx + 2.5, hy - 2, 2.1, FILL);
    g_fill(INK); g_circle(hx + 3, hy - 1.8, 1, FILL);
    g_restore();
}

static void drawSign(float x, float y, const char* text) {
    int chars = 0;
    for (const char* c = text; *c; c++) if ((*c & 0xC0) != 0x80) chars++;
    float w = chars * 13.5f + 30;
    g_save(); g_translate(x, y);
    g_stroke(INK, 3);
    g_fill(0xb07a48); g_rrect(-5, -66, 10, 70, 4, BOTH);
    g_rotate(-0.045f);
    g_lin(0, -98, 0, -58, 0xf7dba2, 0xdcae6c); g_rrect(-w / 2, -98, w, 40, 8, BOTH);
    g_stroke(0xc08f52, 2); g_path(); g_move(-w / 2 + 12, -65); g_line(w / 2 - 12, -65); g_paint(STROKE);
    g_fill(0x8a8298); g_circle(-w / 2 + 9, -78, 2.5, FILL); g_circle(w / 2 - 9, -78, 2.5, FILL);
    g_text(text, 0, -70, 22, 1, INK, INK, 0);
    g_restore();
}

void drawDecor(const Game& g, const Decor& d) {
    uint32_t h = hash2(int(d.x), int(d.y));
    switch (d.kind) {
        case 'T': if (g.level == 0) drawPine(d.x, d.y, h); else drawPalm(d.x, d.y, h, g.time); break;
        case 'S': if (g.level == 0) drawSheepShed(d.x, d.y); else drawTobaccoHut(d.x, d.y); break;
        case 'b': drawSheep(d.x, d.y, g.time, h); break;
        case 'H': drawSign(d.x, d.y, g.name.c_str()); break;
    }
}

// ------------------------------------------------------------ terrain

static bool isTop(const Game& g, int tx, int ty) { return g.solid(tx, ty) && !g.solid(tx, ty - 1); }

void drawTerrain(const Game& g, int tx0, int tx1, int ty0, int ty1) {
    const Theme& th = theme(g);
    float t = g.time;
    // dirt
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (!g.solid(tx, ty) || tx < 0 || tx >= g.W) continue;
            int depth = 0;
            while (depth < 3 && g.solid(tx, ty - depth - 1)) depth++;
            float x = tx * TILE, y = ty * TILE;
            g_fill(th.dirt[depth]); g_rect(x - 0.5, y - 0.5, TILE + 1, TILE + 1);
            uint32_t h = hash2(tx, ty);
            g_fill(th.dirt[std::min(depth + 1, 3)] - 0x0a0806);
            g_ellipse(x + 8 + h % 30, y + 10 + (h >> 5) % 28, 5 + (h >> 10) % 4, 3.5, 0, FILL);
            if ((h >> 14) % 3 == 0) g_ellipse(x + 6 + (h >> 16) % 34, y + 8 + (h >> 21) % 32, 3, 2.2, 0, FILL);
            if ((h >> 24) % 5 == 0) { g_fill(th.pebble); g_circle(x + 10 + (h >> 8) % 28, y + 14 + (h >> 3) % 24, 2.2, FILL); }
        }
    }
    // outlines on exposed sides
    g_stroke(INK, 3.5);
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (!g.solid(tx, ty) || tx < 0 || tx >= g.W) continue;
            float x = tx * TILE, y = ty * TILE;
            g_path();
            if (!g.solid(tx - 1, ty)) { g_move(x, y); g_line(x, y + TILE); }
            if (!g.solid(tx + 1, ty)) { g_move(x + TILE, y); g_line(x + TILE, y + TILE); }
            if (ty + 1 < g.H && !g.solid(tx, ty + 1)) { g_move(x, y + TILE); g_line(x + TILE, y + TILE); }
            g_paint(STROKE);
        }
    }
    // grass caps, one path per run of top tiles
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (!isTop(g, tx, ty) || tx < 0 || tx >= g.W) continue;
            int end = tx;
            while (end + 1 < g.W && isTop(g, end + 1, ty)) end++;
            bool capL = !g.solid(tx - 1, ty) || tx == 0, capR = !g.solid(end + 1, ty) || end == g.W - 1;
            float x0 = tx * TILE - (capL ? 7 : 0), x1 = (end + 1) * TILE + (capR ? 7 : 0), y = ty * TILE;
            g_path();
            g_move(x0, y + 4);
            g_quad(x0, y - 9, x0 + 12, y - 9);
            g_line(x1 - 12, y - 9);
            g_quad(x1, y - 9, x1, y + 4);
            int n = std::max(1, int((x1 - x0) / 17));
            float w = (x1 - x0) / n;
            for (int i = 0; i < n; i++) {  // scalloped fringe hanging over the dirt
                float a = x1 - i * w, b = a - w;
                float drop = 15 + (hash2(tx + (n - i), ty) % 7);
                g_quad((a + b) / 2, y + drop + 6, b, y + 5);
            }
            g_close();
            g_lin(0, y - 9, 0, y + 16, th.grass0, th.grass1);
            g_stroke(INK, 3.5); g_paint(BOTH);
            g_stroke(th.grassHi, 3);
            g_path(); g_move(x0 + 10, y - 4.5); g_line(x1 - 10, y - 4.5); g_paint(STROKE);
            tx = end;
        }
    }
    // little plants on top: heather and grass in Heidestein, tropical flowers and ferns in Vinales
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (!isTop(g, tx, ty) || g.at(tx, ty - 1) != '.' || tx < 0 || tx >= g.W) continue;
            uint32_t h = hash2(tx * 7 + 3, ty * 13 + 1);
            int kind = h % 9;
            float x = tx * TILE + 10 + (h >> 6) % 28, y = ty * TILE - 7;
            float sway = std::sin(t * 2 + tx * 1.7f) * 3;
            bool heather = g.level == 0 && kind <= 3, tuft = g.level == 0 ? kind == 4 : kind <= 1;
            bool flower = g.level == 0 ? kind == 5 : (kind >= 2 && kind <= 4), fern = g.level != 0 && kind == 5;
            if (heather) {  // a clump of purple heather spikes
                for (int i = -1; i <= 1; i++) {
                    float hgt = 17 + ((h >> (8 + i * 2)) % 9), bx = x + i * 8, tipX = bx + i * 4 + sway * 0.7f, tipY = y - hgt;
                    g_stroke(INK, 5); g_path(); g_move(bx, y + 2); g_line(tipX, tipY); g_paint(STROKE);
                    g_stroke(0x4fae4a, 2.5); g_paint(STROKE);
                    g_stroke(INK, 1.8);
                    for (int k = 0; k < 4; k++) {
                        float f = 0.35f + k * 0.2f;
                        g_fill((k + i + kind) % 2 ? 0xb565d9 : 0xe08ff0);
                        g_circle(bx + (tipX - bx) * f + ((k % 2) ? 3 : -3), y + 2 + (tipY - y - 2) * f, 3.6f - k * 0.3f, BOTH);
                    }
                }
            } else if (tuft) {
                g_fill(0x6fdc55); g_stroke(INK, 2);
                for (int i = -1; i <= 1; i++) {
                    float tipX = x + i * 10 + sway, tipY = y - 13 - (i == 0 ? 7 : 0);
                    g_path(); g_move(x + i * 5 - 5, y + 3);
                    g_quad(x + i * 6 - 4, y - 7, tipX, tipY);
                    g_quad(x + i * 6 + 5, y - 5, x + i * 5 + 5, y + 3);
                    g_close(); g_paint(BOTH);
                }
            } else if (flower) {
                static const int petals[] = {0xff4d5e, 0xffffff, 0xffb21f};
                int col = g.level == 0 ? 0xffffff : petals[kind - 2];
                float hgt = 18 + (h >> 12) % 10, fx = x + sway, fy = y - hgt;
                g_stroke(INK, 5); g_path(); g_move(x, y + 2); g_quad(x, y - hgt / 2, fx, fy); g_paint(STROKE);
                g_stroke(0x4cc24a, 2.5); g_paint(STROKE);
                g_fill(col); g_stroke(INK, 2);
                float r = g.level == 0 ? 4.2f : 5.2f;
                for (int i = 0; i < 5; i++) {
                    float a = i * 2 * PI / 5 + t * 0.4f;
                    g_circle(fx + std::cos(a) * (r + 1.3f), fy + std::sin(a) * (r + 1.3f), r, BOTH);
                }
                g_fill(0xffe45c); g_circle(fx, fy, 3.6, BOTH);
            } else if (fern) {  // broad tobacco-like leaves
                g_stroke(INK, 2.2);
                for (int i = -2; i <= 2; i++) {
                    float a = -PI / 2 + i * 0.5f + sway * 0.02f, len = 22 - std::abs(i) * 3;
                    float ex = x + std::cos(a) * len, ey = y + 2 + std::sin(a) * len;
                    g_fill(i % 2 ? 0x4cc24a : 0x7fe05a);
                    g_path(); g_move(x, y + 3);
                    g_quad(x + std::cos(a - 0.5f) * len * 0.6f, y + 2 + std::sin(a - 0.5f) * len * 0.6f, ex, ey);
                    g_quad(x + std::cos(a + 0.5f) * len * 0.6f, y + 2 + std::sin(a + 0.5f) * len * 0.6f, x, y + 3);
                    g_close(); g_paint(BOTH);
                }
            }
        }
    }
}

void drawPlanks(const Game& g, int tx0, int tx1, int ty0, int ty1) {
    const Theme& th = theme(g);
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = std::max(0, tx0); tx <= std::min(g.W - 1, tx1); tx++) {
            if (g.at(tx, ty) != '=') continue;
            int end = tx;
            while (end + 1 < g.W && g.at(end + 1, ty) == '=') end++;
            float x0 = tx * TILE - 4, x1 = (end + 1) * TILE + 4, y = ty * TILE;
            // hanging leaves
            for (int i = tx; i <= end; i++) {
                uint32_t h = hash2(i, ty * 5);
                if (h % 2) continue;
                float lx = i * TILE + 12 + (h >> 4) % 24, len = 12 + (h >> 9) % 14;
                if (std::fabs(lx - (i * TILE + 24)) < 9) lx += 14;  // keep clear of a liana tied to the middle
                float sw = std::sin(g.time * 2 + i) * 2;
                g_stroke(INK, 5); g_path(); g_move(lx, y + 12); g_quad(lx, y + 12 + len / 2, lx + sw, y + 12 + len); g_paint(STROKE);
                g_stroke(0x4cc24a, 2.5); g_paint(STROKE);
                g_fill(0x6fdc55); g_stroke(INK, 2); g_ellipse(lx + sw, y + 14 + len, 4.5, 6, 0.3, BOTH);
            }
            g_lin(0, y, 0, y + 18, th.plank0, th.plank1);
            g_stroke(INK, 3.5); g_rrect(x0, y, x1 - x0, 18, 8, BOTH);
            g_stroke(th.plankHi, 2.5); g_path(); g_move(x0 + 9, y + 5); g_line(x1 - 9, y + 5); g_paint(STROKE);
            g_stroke(th.plankLine, 2);
            if (g.level == 0) {  // wood grain
                for (float x = x0 + 30; x < x1 - 14; x += 34) { g_path(); g_move(x, y + 9); g_line(x + 12, y + 9); g_move(x - 8, y + 13); g_line(x, y + 13); g_paint(STROKE); }
            } else {  // bamboo joints
                for (float x = x0 + 26; x < x1 - 16; x += 30) { g_path(); g_move(x, y + 3); g_quad(x + 3, y + 9, x, y + 15); g_paint(STROKE); }
            }
            tx = end;
        }
    }
}

void drawThorns(const Game& g, int tx0, int tx1, int ty0, int ty1) {
    const Theme& th = theme(g);
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = std::max(0, tx0); tx <= std::min(g.W - 1, tx1); tx++) {
            if (g.at(tx, ty) != '^') continue;
            float x = tx * TILE, base = (ty + 1) * TILE - 4;
            for (int i = 0; i < 3; i++) {
                float cx = x + 8 + i * 16, hgt = 24 + hash2(tx * 3 + i, ty) % 10;
                float lean = (int(hash2(tx + i, 9) % 7) - 3);
                g_path(); g_move(cx - 9, base + 6); g_line(cx + lean, base - hgt); g_line(cx + 9, base + 6); g_close();
                g_lin(cx - 9, 0, cx + 9, 0, th.thorn0, th.thorn1); g_stroke(INK, 3); g_paint(BOTH);
                g_stroke(th.thornHi, 2); g_path(); g_move(cx - 4, base); g_line(cx + lean - 1, base - hgt + 9); g_paint(STROKE);
            }
        }
    }
}
