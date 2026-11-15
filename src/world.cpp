// Sky, parallax scenery, terrain and level scenery. Level 1 is Heidestein (heath and pine forest),
// level 2 is Vinales (red soil, mogote hills, tobacco), level 3 is Lofoten (icy islands under northern lights).
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
        0x5fd8f0, 0x2a9fd6, 0xd6f7ff,
    },
    {   // Vinales: red earth, tropical sky, bamboo, cactus spikes
        {0x18b0e6, 0x8ee9f0, 0xfff2b5},
        {0xe07a4a, 0xcc623a, 0xb44f30, 0x9c4129}, 0xf5b48a,
        0x7dea4a, 0x23b04e, 0xc9ff7a,
        0xe9e37c, 0xa9b548, 0xf9f8bd, 0x7d8a2e,
        0x8fe05e, 0x2f8f47, 0xd6f7a3,
        0x62d9ee, 0x2a9fd6, 0xd6f7ff,
    },
    {   // Lofoten: dark rock under snow and ice, night sky, cold sea
        {0x0a1440, 0x1b3f7a, 0x4f9fc0},
        {0x8e9ab8, 0x79859f, 0x687389, 0x586176}, 0xc3cde0,
        0xffffff, 0xa9d8f7, 0xe9f7ff,
        0xd9b48a, 0xa97c50, 0xf2dcbc, 0x7d5a38,
        0xd9f3ff, 0x6fb8ea, 0xffffff,
        0x2f7fae, 0x154a7a, 0xbfe6ff,
    },
};
const Theme& theme(const Game& g) { return THEMES[g.level]; }

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

// Height of a scenery ridge. shape 0: rolling hills. shape 1: rounded domes. shapes 2, 3: jagged peaks.
static float ridgeY(int shape, float wx, float base, float amp, float f, float ph) {
    if (shape == 0) return base + amp * std::sin(wx * f + ph) + amp * 0.45f * std::sin(wx * f * 2.3f + ph * 2.1f);
    if (shape >= 2) {  // 2: jagged alpine ridge, 3: the same cut off at sea level, leaving separate islands
        auto tri = [](float u) { return 1 - std::fabs(std::fmod(std::fabs(u), 2.f) - 1); };
        float v = tri(wx * f + ph) * 0.62f + tri(wx * f * 2.3f + ph * 1.7f) * 0.28f + tri(wx * f * 5.1f + ph) * 0.10f;
        if (shape == 3) v = std::max(0.f, v - 0.42f) / 0.58f;
        return base - amp * v;
    }
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

// One mogote: a tall dome with steep sides. Detailed ones get pale limestone cliffs and tree clumps on top.
static void drawMogote(float x, float base, float w, float h, int top, int bottom, bool detail, uint32_t seed) {
    if (x < -w || x > VIEW_W + w) return;
    g_lin(0, base - h, 0, base, top, bottom);
    g_path(); g_move(x - w * 0.5f - 12, base);
    g_cubic(x - w * 0.5f + 8, base - h * 0.55f, x - w * 0.42f, base - h, x - w * 0.12f, base - h);
    g_quad(x, base - h * 1.05f, x + w * 0.14f, base - h * 0.98f);
    g_cubic(x + w * 0.44f, base - h * 0.95f, x + w * 0.5f - 6, base - h * 0.5f, x + w * 0.5f + 14, base);
    g_close(); g_paint(FILL);
    if (!detail) return;
    g_alpha(0.5); g_fill(0xeef3e2);  // limestone cliff streaks
    for (int i = -2; i <= 2; i++) {
        uint32_t r = hash2(int(seed % 977) + i, 3);
        float sx = x + i * w * 0.13f + (r % 14), topY = base - h * (0.66f - 0.07f * std::abs(i)) + (r >> 5) % 18;
        g_rrect(sx - 6, topY, 9 + (r >> 9) % 8, h * 0.4f, 5, FILL);
    }
    g_alpha(1); g_fill(0x329a68);    // tree clumps along the top
    for (int i = -3; i <= 3; i++) {
        float u = i * 0.085f;
        g_circle(x + u * w + 4, base - h * (1.0f - 2.6f * u * u) + 2, 15 + (hash2(int(seed % 911), i) % 9), FILL);
    }
    g_fill(0x58c488);
    for (int i = -3; i <= 3; i++) {
        float u = i * 0.085f + 0.03f;
        g_circle(x + u * w, base - h * (1.0f - 2.6f * u * u) - 5, 8, FILL);
    }
}

// Lofoten: night sky, stars, northern lights, jagged snowy peaks and the sea with a far fishing village.
static void drawArcticBackground(const Game& g, float vy) {
    const Theme& th = theme(g);
    float t = g.time;
    g_lin3(0, 0, 0, VIEW_H, th.sky[0], th.sky[1], th.sky[2]);
    g_rect(0, 0, VIEW_W, VIEW_H);
    g_fill(0xffffff);
    for (int i = 0; i < 80; i++) {
        uint32_t h = hash2(i, 999);
        float x = std::fmod(float(h % 1100) - g.camX * 0.01f + 1100, 1100.f) - 60, y = (h >> 11) % 330;
        g_alpha(0.25 + 0.6 * (0.5f + 0.5f * std::sin(t * (1.5f + (h >> 20) % 3) + i)));
        g_circle(x, y, 0.8f + ((h >> 24) % 3) * 0.5f, FILL);
    }
    // northern lights: three slow curtains of light
    static const int aur[][2] = {{0x9d7bff, 0x4dffb4}, {0x4fe0ff, 0x6dffc0}, {0x4dffb4, 0xc9ff7a}};
    for (int b = 0; b < 3; b++) {
        g_lin3a(0, 0, 0, 100, aur[b][0], 0.0, aur[b][1], 0.55, aur[b][1], 0.0);
        float by = 40 + b * 46;
        for (float sx = -12; sx < VIEW_W; sx += 12) {
            float wx = sx + g.camX * 0.02f;
            float yTop = by + std::sin(wx * 0.006f + t * 0.22f + b * 2.1f) * 44 + std::sin(wx * 0.017f - t * 0.35f + b) * 16;
            float hgt = 150 + 60 * std::sin(wx * 0.011f + t * 0.4f + b * 1.7f);
            g_alpha(0.5 + 0.45 * std::sin(wx * 0.028f + t * 0.7f + b * 3));
            g_save(); g_translate(sx, yTop); g_scale(1, hgt / 100); g_rect(0, 0, 12.6, 100); g_restore();
        }
    }
    g_alpha(1);
    // moon
    float mx = 800 - g.camX * 0.01f, my = 90;
    g_rad(mx, my, 120, 0xdfeeff, 0.5, 0xdfeeff, 0); g_circle(mx, my, 120, FILL);
    g_fill(0xf4f8ff); g_circle(mx, my, 30, FILL);
    g_fill(0xd9e4f5); g_circle(mx - 9, my - 6, 6, FILL); g_circle(mx + 10, my + 8, 4, FILL); g_circle(mx + 6, my - 13, 3, FILL);

    // far peaks
    float seaY = 398 + std::min(vy * 0.25f, 150.f);
    {
        float off = g.camX * 0.06f, base = seaY + 6;
        g_lin(0, base - 250, 0, base, 0xeef6ff, 0x3b5a9c);
        g_path(); g_move(0, base);
        for (int sx = 0; sx <= VIEW_W + 6; sx += 6) g_line(sx, ridgeY(2, sx + off, base, 250, 0.0052f, 0.3f));
        g_line(VIEW_W, base); g_close(); g_paint(FILL);
    }
    // sea
    g_lin(0, seaY, 0, VIEW_H, th.water0, th.water1); g_rect(0, seaY, VIEW_W, VIEW_H - seaY + 1);
    for (int i = 0; i < 26; i++) {  // aurora glints and wave lines on the water
        uint32_t h = hash2(i, 4242);
        float x = std::fmod(float(h % 1000) - g.camX * 0.1f + std::sin(t * 0.5f + i) * 12 + 2000, 1000.f) - 20;
        float y = seaY + 8 + (h >> 10) % 130;
        g_alpha(0.16 + 0.14 * std::sin(t * 1.3f + i)); g_fill(i % 3 ? 0x6dffc0 : 0xffffff);
        g_rrect(x, y, 26 + (h >> 18) % 40, 2.5, 1.2, FILL);
    }
    g_alpha(1);
    // mid: steep islands rising from the sea, with villages and boats at the waterline
    {
        float off = g.camX * 0.16f, base = seaY + 26;
        g_lin(0, base - 200, 0, base, 0xe4f0ff, 0x2a4680);
        g_path(); g_move(0, base);
        for (int sx = 0; sx <= VIEW_W + 6; sx += 6) g_line(sx, ridgeY(3, sx + off, base, 210, 0.0036f, 1.1f));
        g_line(VIEW_W, base); g_close(); g_paint(FILL);
        for (int k = int(std::floor(off / 64)) - 1; k <= int((off + VIEW_W) / 64) + 1; k++) {
            uint32_t h = hash2(k, 321);
            float wx = k * 64.f + h % 40, x = wx - off;
            bool land = ridgeY(3, wx, base, 210, 0.0036f, 1.1f) < base - 14;
            if (land && h % 3 != 0) {         // rorbu cabin with a lit window
                g_fill((h >> 6) % 4 == 0 ? 0xe8c25a : 0xc8402f); g_rect(x - 8, base - 12, 16, 11);
                g_fill(0x26345c); g_path(); g_move(x - 10, base - 12); g_line(x, base - 20); g_line(x + 10, base - 12); g_close(); g_paint(FILL);
                g_fill(0xffe07a); g_rect(x - 2.5, base - 9, 5, 4.5);
            } else if (!land && h % 4 == 0) {  // fishing boat
                float bob = std::sin(t * 1.4f + k) * 1.5f, y = base + 6 + (h >> 8) % 20 + bob;
                g_fill(0xb8372b); g_path(); g_move(x - 13, y - 6); g_line(x + 14, y - 6); g_line(x + 10, y); g_line(x - 10, y); g_close(); g_paint(FILL);
                g_fill(0xf4f0e6); g_rect(x - 6, y - 12, 8, 6);
                g_stroke(0xf4f0e6, 1.5); g_path(); g_move(x + 5, y - 6); g_line(x + 5, y - 20); g_paint(STROKE);
            }
        }
    }
}

void drawBackground(const Game& g) {
    const Theme& th = theme(g);
    bool cuba = g.level == 1;
    float t = g.time;
    float vy = (g.H * TILE - VIEW_H - g.camY);  // how far the camera is above the bottom of the level
    if (g.level == 2) { drawArcticBackground(g, vy); return; }
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
        // far: hazy mogotes, the steep rounded limestone hills of the valley
        float off = g.camX * 0.08f, base = 404 + farUp;
        for (int k = int(std::floor(off / 210)) - 1; k <= int((off + VIEW_W) / 210) + 1; k++) {
            uint32_t h = hash2(k, 13);
            if (h % 5 == 0) continue;
            drawMogote(k * 210.f + (h >> 4) % 80 - off, base, 170 + (h >> 8) % 110, 110 + (h >> 14) % 110, 0x7fd0bb, 0xbdeedb, false, h);
        }
        drawRidge(0, off, base - 6, 9, 0.006f, 0.4f, 0xb4ead6, 0xd4f5e6);
        // mid: big green mogotes with pale cliff faces
        off = g.camX * 0.18f; base = 446 + midUp;
        for (int k = int(std::floor(off / 330)) - 1; k <= int((off + VIEW_W) / 330) + 1; k++) {
            uint32_t h = hash2(k, 57);
            if (h % 6 == 0) continue;
            drawMogote(k * 330.f + (h >> 4) % 110 - off, base, 240 + (h >> 8) % 130, 190 + (h >> 14) % 120, 0x3fb476, 0x86db9a, true, h);
        }
        drawRidge(0, off, base - 8, 10, 0.007f, 2.2f, 0x7fd88c, 0xb5ecae);
        // near: a tobacco field on red earth, with drying sheds and royal palms
        off = g.camX * 0.42f; base = 448 + nearUp;
        for (int k = int(std::floor(off / 150)) - 1; k <= int((off + VIEW_W) / 150) + 1; k++) {
            uint32_t h = hash2(k, 91);
            float wx = k * 150.f + (h >> 4) % 80, x = wx - off, y = ridgeY(0, wx, base, 14, 0.008f, 0.5f) + 8;
            if (h % 3 == 0) {  // shed
                float sc = 0.8f + (h >> 9) % 30 / 100.f;
                g_fill(0xb9a468); g_path(); g_move(x - 46 * sc, y); g_line(x - 4 * sc, y - 74 * sc); g_line(x + 4 * sc, y - 74 * sc); g_line(x + 46 * sc, y); g_close(); g_paint(FILL);
                g_fill(0xd9c690); g_path(); g_move(x - 30 * sc, y); g_line(x, y - 56 * sc); g_line(x + 30 * sc, y); g_close(); g_paint(FILL);
                g_fill(0x7a5a3a); g_rect(x - 8 * sc, y - 26 * sc, 16 * sc, 26 * sc);
            } else if (h % 3 == 1) {
                palmSilhouette(x, y, 130 + (h >> 8) % 90, 0x3fae6a, 0x9fb893);
            }
        }
        drawRidge(0, off, base, 14, 0.008f, 0.5f, 0xe08a58, 0xeeb083);
        for (int row = 0; row < 4; row++) {
            float sp = 26 + row * 6, shift = row * 13.f;
            for (int k = int(std::floor((off + shift) / sp)) - 1; k <= int((off + shift + VIEW_W) / sp) + 1; k++) {
                float wx = k * sp - shift, y = ridgeY(0, wx, base, 14, 0.008f, 0.5f) + 12 + row * 22;
                g_fill((k + row) % 2 ? 0x4fbf4a : 0x6fd655);
                g_ellipse(wx - off, y, 9 + row * 2, 5 + row, 0, FILL);
                g_fill(0x8fe66a); g_ellipse(wx - off - 2, y - 2 - row * 0.5f, 4 + row, 2 + row * 0.4f, 0, FILL);
            }
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

// Vinales: a tobacco-drying shed (casa de tabaco) - a steep palm-thatch roof over a plank gable,
// with the leaves hung up inside.
static void drawTobaccoShed(float x, float y) {
    g_save(); g_translate(x, y);
    g_stroke(INK, 3.5);
    // the long roof running back behind the gable
    g_path(); g_move(8, -158); g_line(96, -150); g_quad(150, -70, 176, 3); g_line(96, 3); g_close();
    g_lin(0, -158, 0, 0, 0xb09a52, 0x857034); g_paint(BOTH);
    g_stroke(0x6a5a28, 2);
    for (int i = 1; i <= 4; i++) { float k = i / 5.f; g_path(); g_move(14 + k * 86, -156 + k * 6); g_quad(70 + k * 78, -80, 100 + k * 74, 0); g_paint(STROKE); }
    g_stroke(INK, 3.5);
    // thatched roof
    g_path(); g_move(-100, 3); g_quad(-52, -70, -12, -158); g_line(12, -158); g_quad(52, -70, 100, 3);
    g_line(78, 3); g_line(0, -128); g_line(-78, 3); g_close();
    g_lin(0, -158, 0, 0, 0xcdb96c, 0x9c8440); g_paint(BOTH);
    g_stroke(0x7d6a30, 2);
    for (int i = 0; i < 6; i++) {
        float k = (i + 0.5f) / 6;
        g_path(); g_move(-12 - k * 82, -150 + k * 150); g_line(-4 - k * 76, -140 + k * 144);
        g_move(12 + k * 82, -150 + k * 150); g_line(4 + k * 76, -140 + k * 144); g_paint(STROKE);
    }
    // plank gable wall
    g_stroke(INK, 3);
    g_path(); g_move(-78, 3); g_line(0, -128); g_line(78, 3); g_close();
    g_lin(0, -128, 0, 0, 0xd9b185, 0xb98a5a); g_paint(BOTH);
    g_stroke(0x94683e, 2);
    for (int i = -4; i <= 4; i++) {
        if (i >= -1 && i <= 1) continue;
        float px = i * 15.f, topY = -128 + std::fabs(px) * (131.f / 78) + 6;
        g_path(); g_move(px, topY); g_line(px, 0); g_paint(STROKE);
    }
    // vent under the ridge
    g_stroke(INK, 2.5); g_fill(0x4a2f2a); g_rrect(-9, -98, 18, 16, 3, BOTH);
    // open doorway with tobacco leaves drying inside
    g_stroke(INK, 3); g_fill(0x3d2924); g_rrect(-27, -70, 54, 73, 4, BOTH);
    for (int row = 0; row < 2; row++) {
        float py = -60 + row * 27;
        g_stroke(0xd9b185, 2.5); g_path(); g_move(-24, py); g_line(24, py); g_paint(STROKE);
        g_stroke(INK, 1.6);
        for (int i = -2; i <= 2; i++) {
            g_fill((i + row) % 2 ? 0xc99a3a : 0x8fb347);
            g_ellipse(i * 9.5f, py + 12, 4.3, 11, i * 0.05f, BOTH);
        }
    }
    // doors swung open
    g_stroke(INK, 3);
    for (int side = -1; side <= 1; side += 2) {
        g_fill(0xc49762); g_rrect(side > 0 ? 27 : -45, -68, 18, 71, 3, BOTH);
        g_stroke(INK, 2); g_path(); g_move(side * 29, -64); g_line(side * 43, 0); g_move(side * 43, -64); g_line(side * 29, 0); g_paint(STROKE);
        g_stroke(INK, 3);
    }
    g_restore();
}

static void drawTobaccoPlant(float x, float y, uint32_t h, float t) {
    float sway = std::sin(t * 1.6f + x * 0.05f) * 2.5f, hgt = 62 + h % 14;
    g_stroke(INK, 5.5); g_path(); g_move(x, y + 2); g_quad(x, y - hgt * 0.5f, x + sway, y - hgt); g_paint(STROKE);
    g_stroke(0x4fae3c, 3); g_paint(STROKE);
    for (int i = 0; i < 4; i++) {
        for (int side = -1; side <= 1; side += 2) {
            float ly = y - 6 - i * 15, len = 34 - i * 5, lx = x + sway * (i / 4.f);
            float ex = lx + side * len, ey = ly - 6 + i * 1.5f + (side > 0 ? sway : -sway) * 0.4f;
            g_path(); g_move(lx, ly);
            g_quad(lx + side * len * 0.45f, ly - 20, ex, ey);
            g_quad(lx + side * len * 0.5f, ly + 7, lx, ly); g_close();
            g_fill((i + (side > 0)) % 2 ? 0x56c44a : 0x7fe05a); g_stroke(INK, 2.4); g_paint(BOTH);
            g_stroke(0x2f9a4a, 1.6); g_path(); g_move(lx + side * 4, ly - 2); g_quad(lx + side * len * 0.5f, ly - 9, ex - side * 5, ey); g_paint(STROKE);
        }
    }
    g_fill(0xffb3cf); g_stroke(INK, 1.8);
    g_circle(x + sway - 5, y - hgt - 2, 4, BOTH); g_circle(x + sway + 5, y - hgt - 3, 4, BOTH); g_circle(x + sway, y - hgt - 8, 4.2, BOTH);
}

// The guava tree on the river bank. Its long branch reaches out over the water; the big guava hangs from the tip.
static void drawGuavaTree(float x, float y, float t) {
    const int OUT = INK;
    g_stroke(OUT, 21); g_path(); g_move(x, y + 2); g_quad(x - 14, y - 70, x + 2, y - 128); g_paint(STROKE);
    g_stroke(0xc9a27a, 15); g_paint(STROKE);
    g_stroke(OUT, 14); g_path(); g_move(x, y - 118); g_quad(x + 60, y - 168, x + 148, y - 150); g_paint(STROKE);
    g_stroke(0xc9a27a, 8.5); g_paint(STROKE);
    static const float blobs[][3] = {{-34, -150, 30}, {-4, -182, 36}, {40, -186, 32}, {78, -176, 26}, {-44, -118, 22}, {112, -170, 20}, {18, -150, 26}};
    float sway = std::sin(t * 1.3f) * 2;
    g_fill(OUT);
    for (auto& b : blobs) g_circle(x + b[0] + sway, y + b[1], b[2] + 3.5f, FILL);
    for (auto& b : blobs) { g_fill(0x4cc24a); g_circle(x + b[0] + sway, y + b[1], b[2], FILL); }
    g_fill(0x7fe05a);
    for (auto& b : blobs) g_circle(x + b[0] + sway - b[2] * 0.25f, y + b[1] - b[2] * 0.3f, b[2] * 0.45f, FILL);
    g_stroke(OUT, 2);  // little guavas in the leaves
    static const float fruit[][2] = {{-22, -142}, {30, -160}, {70, -158}, {4, -200}};
    for (auto& f : fruit) { g_fill(0xe9f07a); g_circle(x + f[0] + sway, y + f[1], 6.5, BOTH); }
}

// Lofoten: a red rorbu (fisherman's cabin) on stilts, snow on the roof, light in the window.
static void drawRorbu(float x, float y, uint32_t h, float t) {
    bool yellow = h % 4 == 0;
    g_save(); g_translate(x, y);
    g_stroke(INK, 3);
    g_fill(0x6b4a35);
    for (int i = -1; i <= 1; i++) g_rrect(i * 40 - 4, -16, 8, 19, 2, BOTH);
    if (yellow) g_lin(0, -76, 0, -12, 0xf2c94c, 0xd9a52e); else g_lin(0, -76, 0, -12, 0xd9483b, 0xb5332a);
    g_rrect(-54, -76, 108, 62, 3, BOTH);
    g_stroke(yellow ? 0xb5861f : 0x8f241d, 2);
    for (int i = -4; i <= 4; i++) { g_path(); g_move(i * 11.5f, -72); g_line(i * 11.5f, -17); g_paint(STROKE); }
    g_stroke(INK, 2.5);
    g_fill(0xfdf8ee); g_rrect(-56, -78, 9, 66, 2, BOTH); g_rrect(47, -78, 9, 66, 2, BOTH);
    // door and window
    g_fill(0xfdf8ee); g_rrect(-36, -56, 22, 42, 2, BOTH);
    g_fill(0x3f6fae); g_rrect(-32, -52, 14, 34, 1.5, FILL);
    g_fill(0xfdf8ee); g_rrect(2, -60, 34, 28, 2, BOTH);
    g_fill(0xffe07a); g_rect(6, -56, 26, 20);
    g_stroke(0xfdf8ee, 2.5); g_path(); g_move(19, -56); g_line(19, -36); g_move(6, -46); g_line(32, -46); g_paint(STROKE);
    g_rad(19, -46, 46, 0xffe07a, 0.35, 0xffe07a, 0); g_circle(19, -46, 46, FILL);
    // roof with snow
    g_stroke(INK, 3);
    g_fill(0x3a4466); g_path(); g_move(-64, -74); g_line(-44, -108); g_line(44, -108); g_line(64, -74); g_close(); g_paint(BOTH);
    g_fill(0xffffff); g_path(); g_move(-49, -100); g_quad(-46, -114, -34, -113); g_line(34, -113); g_quad(46, -114, 49, -100);
    g_quad(30, -94, 14, -100); g_quad(0, -92, -16, -100); g_quad(-32, -94, -49, -100); g_close(); g_paint(BOTH);
    // chimney smoke
    g_fill(0x6b4a35); g_rrect(22, -124, 12, 16, 2, BOTH);
    for (int i = 0; i < 3; i++) {
        float k = std::fmod(t * 0.35f + i / 3.f + (h % 10) * 0.1f, 1.f);
        g_alpha(0.5 * (1 - k)); g_fill(0xe6eefc); g_circle(28 + std::sin(k * 5 + i) * 6 + k * 14, -130 - k * 46, 5 + k * 9, FILL);
    }
    g_alpha(1);
    g_restore();
}

// Lofoten: a hjell, the wooden rack where cod hangs to dry into stockfish.
static void drawStockfishRack(float x, float y, float t) {
    g_save(); g_translate(x, y);
    auto pole = [](float x0, float y0, float x1, float y1, float w) {
        g_stroke(INK, w + 4); g_path(); g_move(x0, y0); g_line(x1, y1); g_paint(STROKE);
        g_stroke(0xb98a5a, w); g_paint(STROKE);
    };
    for (int side = -1; side <= 1; side += 2) { pole(side * 84, 2, side * 58, -112, 6); pole(side * 34, 2, side * 60, -112, 6); }
    for (int row = 0; row < 3; row++) {
        float py = -100 + row * 32, half = 64 + row * 7;
        pole(-half, py, half, py, 5);
        for (float fx = -half + 14; fx <= half - 10; fx += 15) {  // the fish hang in pairs, tails up
            float sw = std::sin(t * 1.8f + fx * 0.3f + row) * 0.09f;
            g_save(); g_translate(fx, py + 2); g_rotate(sw);
            g_stroke(INK, 1.8); g_fill(row % 2 ? 0xe9dcc0 : 0xd9c9a8);
            g_path(); g_move(-3, 0); g_line(0, 5); g_line(3, 0); g_close(); g_paint(BOTH);
            g_path(); g_move(0, 4); g_quad(5.5, 12, 0, 25); g_quad(-5.5, 12, 0, 4); g_close(); g_paint(BOTH);
            g_restore();
        }
    }
    g_stroke(0xffffff, 5); g_path(); g_move(-58, -105); g_line(58, -105); g_paint(STROKE);  // snow on the top pole
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
    float t = g.time;
    switch (d.kind) {
        case 'T': if (g.level == 0) drawPine(d.x, d.y, h); else drawPalm(d.x, d.y, h, t); break;
        case 'S':
            if (g.level == 0) drawSheepShed(d.x, d.y);
            else if (g.level == 1) drawTobaccoShed(d.x, d.y);
            else drawRorbu(d.x, d.y, h, t);
            break;
        case 'b': drawSheep(d.x, d.y, t, h); break;
        case 'H': drawSign(d.x, d.y, g.name.c_str()); break;
        case 't': drawTobaccoPlant(d.x, d.y, h, t); break;
        case 'Y': drawGuavaTree(d.x, d.y, t); break;
        case 'R': drawStockfishRack(d.x, d.y, t); break;
        case 'B':  // small boat moored on the water; d.y is the bottom of the top water tile
            drawFishingBoat(d.x, d.y - TILE + WATER_TOP - 34 + std::sin(t * 1.6f + d.x) * 2.5f, 0.6f, false);
            break;
    }
}

// ------------------------------------------------------------ terrain

static bool isTop(const Game& g, int tx, int ty) { return g.solid(tx, ty) && !g.filled(tx, ty - 1); }

void drawTerrain(const Game& g, int tx0, int tx1, int ty0, int ty1) {
    const Theme& th = theme(g);
    float t = g.time;
    // dirt
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (tx < 0 || tx >= g.W) continue;
            float x = tx * TILE, y = ty * TILE;
            char sc = g.at(tx, ty);
            if (sc == '/' || sc == '\\') {  // slope: a triangle of rock
                g_fill(th.dirt[0]); g_path();
                if (sc == '/') { g_move(x - 0.5, y + TILE + 0.5); g_line(x + TILE + 0.5, y + TILE + 0.5); g_line(x + TILE + 0.5, y - 0.5); }
                else { g_move(x - 0.5, y - 0.5); g_line(x - 0.5, y + TILE + 0.5); g_line(x + TILE + 0.5, y + TILE + 0.5); }
                g_close(); g_paint(FILL);
                continue;
            }
            if (!g.solid(tx, ty)) continue;
            int depth = 0;
            while (depth < 3 && g.filled(tx, ty - depth - 1)) depth++;
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
            if (!g.filled(tx - 1, ty)) { g_move(x, y); g_line(x, y + TILE); }
            if (!g.filled(tx + 1, ty)) { g_move(x + TILE, y); g_line(x + TILE, y + TILE); }
            if (ty + 1 < g.H && !g.filled(tx, ty + 1)) { g_move(x, y + TILE); g_line(x + TILE, y + TILE); }
            g_paint(STROKE);
        }
    }
    // grass caps, one path per run of top tiles
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (!isTop(g, tx, ty) || tx < 0 || tx >= g.W) continue;
            int end = tx;
            while (end + 1 < g.W && isTop(g, end + 1, ty)) end++;
            bool capL = !g.filled(tx - 1, ty) || tx == 0, capR = !g.filled(end + 1, ty) || end == g.W - 1;
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
    // slopes get a band of the same cap along the incline
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            char sc = g.at(tx, ty);
            if ((sc != '/' && sc != '\\') || tx < 0 || tx >= g.W) continue;
            float x = tx * TILE, y = ty * TILE;
            float xa = x, ya = sc == '/' ? y + TILE : y, xb = x + TILE, yb = sc == '/' ? y : y + TILE;
            g_path(); g_move(xa, ya - 1); g_line(xb, yb - 1);
            g_stroke(INK, 17); g_paint(STROKE);
            g_stroke(th.grass1, 10); g_paint(STROKE);
            g_path(); g_move(xa, ya - 4); g_line(xb, yb - 4); g_stroke(th.grass0, 5); g_paint(STROKE);
        }
    }
    if (g.level == 2) {  // Lofoten: snow drifts and ice crystals instead of plants
        for (int ty = ty0; ty <= ty1; ty++) {
            for (int tx = tx0; tx <= tx1; tx++) {
                if (!isTop(g, tx, ty) || g.at(tx, ty - 1) != '.' || tx < 0 || tx >= g.W) continue;
                uint32_t h = hash2(tx * 7 + 3, ty * 13 + 1);
                int kind = h % 8;
                float x = tx * TILE + 10 + (h >> 6) % 28, y = ty * TILE - 8;
                if (kind <= 1) {
                    g_fill(0xffffff); g_stroke(INK, 2.5);
                    g_path(); g_move(x - 16, y + 3); g_quad(x - 6, y - 14 - (h >> 12) % 6, x + 6, y - 6); g_quad(x + 14, y - 10, x + 18, y + 3); g_close(); g_paint(BOTH);
                } else if (kind == 2) {
                    float tw = 0.6f + 0.4f * std::sin(t * 3 + tx);
                    g_stroke(INK, 2);
                    for (int i = -1; i <= 1; i++) {
                        float hgt = 14 + (i == 0 ? 8 : 0), bx = x + i * 8;
                        g_fill(i == 0 ? 0xd9f3ff : 0x9fd6f7);
                        g_path(); g_move(bx - 4, y + 3); g_line(bx + i * 3, y - hgt); g_line(bx + 4, y + 3); g_close(); g_paint(BOTH);
                    }
                    g_alpha(tw); g_fill(0xffffff); starPath(x + 3, y - 20, 4 * tw + 1, 1, t, 4); g_paint(FILL); g_alpha(1);
                }
            }
        }
        return;
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
            if (g.level != 1) {  // wood grain
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

void drawWater(const Game& g, int tx0, int tx1, int ty0, int ty1) {
    const Theme& th = theme(g);
    float t = g.time;
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = std::max(0, tx0); tx <= std::min(g.W - 1, tx1); tx++) {
            if (g.at(tx, ty) != 'w') continue;
            int end = tx;
            bool top = g.at(tx, ty - 1) != 'w';
            while (end + 1 < g.W && g.at(end + 1, ty) == 'w' && (g.at(end + 1, ty - 1) != 'w') == top) end++;
            float x0 = tx * TILE, x1 = (end + 1) * TILE, y = ty * TILE;
            g_alpha(0.8);
            if (top) {
                g_lin(0, y + WATER_TOP, 0, y + TILE, th.water0, th.water1);
                g_path(); g_move(x0, y + TILE + 0.5);
                for (float x = x0; x < x1; x += 12) g_line(x, y + WATER_TOP + std::sin(x * 0.07f + t * 2.2f) * 2.5f);
                g_line(x1, y + WATER_TOP + std::sin(x1 * 0.07f + t * 2.2f) * 2.5f); g_line(x1, y + TILE + 0.5); g_close(); g_paint(FILL);
                g_alpha(0.9); g_stroke(th.waterHi, 3);
                g_path();
                for (float x = x0; x <= x1 + 1; x += 12) {
                    float xx = std::min(x, x1), wy = y + WATER_TOP + std::sin(xx * 0.07f + t * 2.2f) * 2.5f;
                    if (x == x0) g_move(xx, wy); else g_line(xx, wy);
                }
                g_paint(STROKE);
                g_alpha(0.35); g_fill(th.waterHi);
                for (int i = tx; i <= end; i++) {
                    uint32_t h = hash2(i, ty);
                    g_rrect(i * TILE + std::fmod(h % 30 + t * 9, 30.f), y + WATER_TOP + 8 + (h >> 6) % 12, 14, 2.5, 1.2, FILL);
                }
            } else {
                g_fill(th.water1); g_rect(x0, y - 0.5, x1 - x0, TILE + 1);
            }
            g_alpha(1);
            tx = end;
        }
    }
}

// Curved bridges. arch > 0: a humped stone bridge with a red railing. arch < 0: a sagging rope bridge.
void drawBridge(const Game& g, const Bridge& b) {
    bool rope = b.arch < 0;
    float deck = rope ? 8 : 12;
    auto curve = [&](float dy, bool reverse) {
        if (!reverse) for (float x = b.x0; x <= b.x1; x += 8) g_line(x, bridgeY(b, x) + dy);
        else for (float x = b.x1; x >= b.x0; x -= 8) g_line(x, bridgeY(b, x) + dy);
    };
    if (!rope) {  // stone arch underneath
        g_fill(0x8e9ab8); g_stroke(INK, 3.5);
        g_path(); g_move(b.x0, b.y + deck);
        curve(deck, false);
        for (float x = b.x1; x >= b.x0; x -= 8) {
            float k = std::sin(PI * (x - b.x0) / (b.x1 - b.x0));
            g_line(x, std::max(bridgeY(b, x) + deck, b.y + 44 - (b.arch + 46) * k));
        }
        g_close(); g_paint(BOTH);
    }
    // rail posts and handrail (behind Emma)
    float railH = rope ? 30 : 27;
    for (float x = b.x0 + (rope ? 4 : 18); x <= b.x1 - 3; x += rope ? 28 : 46) {
        float y = bridgeY(b, x);
        g_path(); g_move(x, y + 2); g_line(x, y - railH - (rope ? std::sin(PI * (x - b.x0) / (b.x1 - b.x0)) * -4 : 0));
        if (rope) { g_stroke(INK, 3.2); g_paint(STROKE); g_stroke(0xe9d8a6, 1.4); g_paint(STROKE); }
        else { g_stroke(INK, 7); g_paint(STROKE); g_stroke(0xd9483b, 3.5); g_paint(STROKE); }
    }
    g_path(); g_move(b.x0, b.y - railH); curve(-railH, false);
    if (rope) { g_stroke(INK, 5); g_paint(STROKE); g_stroke(0xe9d8a6, 2.4); g_paint(STROKE); }
    else { g_stroke(INK, 8); g_paint(STROKE); g_stroke(0xd9483b, 4.5); g_paint(STROKE); g_stroke(0xff8a7a, 1.5); g_paint(STROKE); }
    if (rope) {  // anchor posts
        g_fill(0xb98a5a); g_stroke(INK, 3);
        g_rrect(b.x0 - 5, b.y - 40, 10, 44, 3, BOTH); g_rrect(b.x1 - 5, b.y - 40, 10, 44, 3, BOTH);
    }
    // deck planks
    const Theme& th = theme(g);
    g_lin(0, b.y - std::fabs(b.arch), 0, b.y + std::fabs(b.arch) + deck, th.plank0, th.plank1);
    g_stroke(INK, 3.5);
    g_path(); g_move(b.x0, b.y); curve(0, false); curve(deck, true); g_close(); g_paint(BOTH);
    g_stroke(th.plankLine, 2);
    for (float x = b.x0 + 14; x < b.x1 - 6; x += 15) {
        float y = bridgeY(b, x);
        g_path(); g_move(x, y + 2.5); g_line(x, y + deck - 2.5); g_paint(STROKE);
    }
    g_stroke(0xffffff, 2.5); g_alpha(0.8);  // a dusting of snow
    g_path(); g_move(b.x0 + 6, b.y + 1); for (float x = b.x0 + 6; x <= b.x1 - 6; x += 8) g_line(x, bridgeY(b, x) + 1.5f); g_paint(STROKE);
    g_alpha(1);
}
