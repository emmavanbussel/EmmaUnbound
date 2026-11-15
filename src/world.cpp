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
        0xb4cde0, 0x7f9fbe, 0xf2f7fb,
    },
    {   // Vinales at dusk: red-brown earth, dull greens, weathered wood, muddy water
        {0x7d8cc4, 0xe6a6ae, 0xfad3a0},
        {0xb86c44, 0xa55d3a, 0x914f32, 0x7d432b}, 0xd69c72,
        0x93b458, 0x537f3f, 0xbdd682,
        0xb7a07c, 0x86705a, 0xd8c8a6, 0x5c4c3a,
        0x8ab661, 0x3f7646, 0xc6dea0,
        0xc79a60, 0x966b3c, 0xe8cc9c,
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
static void drawMogote(float x, float base, float w, float h, int top, int bottom, bool detail, uint32_t seed,
                       int cliff = 0, int clumpDark = 0, int clumpLight = 0) {
    if (x < -w || x > VIEW_W + w) return;
    g_lin(0, base - h, 0, base, top, bottom);
    g_path(); g_move(x - w * 0.5f - 12, base);
    g_cubic(x - w * 0.5f + 8, base - h * 0.55f, x - w * 0.42f, base - h, x - w * 0.12f, base - h);
    g_quad(x, base - h * 1.05f, x + w * 0.14f, base - h * 0.98f);
    g_cubic(x + w * 0.44f, base - h * 0.95f, x + w * 0.5f - 6, base - h * 0.5f, x + w * 0.5f + 14, base);
    g_close(); g_paint(FILL);
    if (!detail) return;
    g_alpha(0.42); g_fill(cliff);  // limestone cliff streaks
    for (int i = -2; i <= 2; i++) {
        uint32_t r = hash2(int(seed % 977) + i, 3);
        float sx = x + i * w * 0.13f + (r % 14), topY = base - h * (0.66f - 0.07f * std::abs(i)) + (r >> 5) % 18;
        g_rrect(sx - 6, topY, 9 + (r >> 9) % 8, h * 0.4f, 5, FILL);
    }
    g_alpha(1); g_fill(clumpDark);    // tree clumps along the top
    for (int i = -3; i <= 3; i++) {
        float u = i * 0.085f;
        g_circle(x + u * w + 4, base - h * (1.0f - 2.6f * u * u) + 2, 15 + (hash2(int(seed % 911), i) % 9), FILL);
    }
    g_fill(clumpLight);
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

    if (cuba) {  // dusk: a low orange sun and clouds lit pink and orange from below
        float sx = 690 - g.camX * 0.02f, sy = 318 + farUp * 0.6f;
        g_rad(sx, sy, 360, 0xffc08a, 0.75, 0xffc08a, 0); g_circle(sx, sy, 360, FILL);
        g_fill(0xfff0c4); g_circle(sx, sy, 34, FILL);
        for (int i = 0; i < 9; i++) {
            uint32_t h = hash2(i, 606);
            float span = 1500, sc = 0.9f + (h >> 4) % 70 / 100.f;
            float x = std::fmod(float(h % 1500) - g.camX * (0.04f + sc * 0.02f) + t * 3 * sc, span);
            if (x < 0) x += span;
            x -= 260;
            float y = 40 + (h >> 12) % 230 + std::min(vy * 0.04f, 60.f);
            g_alpha(0.85);
            g_fill(0xa99ccb); g_ellipse(x, y - 7 * sc, 120 * sc, 13 * sc, 0, FILL); g_ellipse(x + 70 * sc, y - 12 * sc, 80 * sc, 11 * sc, 0, FILL);
            g_fill(y > 150 ? 0xffab7e : 0xf7a9a6); g_ellipse(x + 12 * sc, y, 110 * sc, 9 * sc, 0, FILL); g_ellipse(x - 60 * sc, y + 6 * sc, 60 * sc, 6 * sc, 0, FILL);
            g_fill(0xffd9a8); g_ellipse(x + 30 * sc, y + 4 * sc, 60 * sc, 4 * sc, 0, FILL);
            g_alpha(1);
        }
    } else {
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
        // far: mogotes fading into blue haze
        float off = g.camX * 0.08f, base = 404 + farUp;
        for (int k = int(std::floor(off / 210)) - 1; k <= int((off + VIEW_W) / 210) + 1; k++) {
            uint32_t h = hash2(k, 13);
            if (h % 5 == 0) continue;
            drawMogote(k * 210.f + (h >> 4) % 80 - off, base, 190 + (h >> 8) % 120, 110 + (h >> 14) % 100, 0x8f9cc0, 0xbfb6cc, false, h);
        }
        drawRidge(0, off, base - 6, 9, 0.006f, 0.4f, 0xa9a8c4, 0xc9b9c8);
        // mid: dark blue-green mogotes with pale cliff faces
        off = g.camX * 0.18f; base = 446 + midUp;
        for (int k = int(std::floor(off / 330)) - 1; k <= int((off + VIEW_W) / 330) + 1; k++) {
            uint32_t h = hash2(k, 57);
            if (h % 6 == 0) continue;
            drawMogote(k * 330.f + (h >> 4) % 110 - off, base, 260 + (h >> 8) % 140, 180 + (h >> 14) % 120, 0x4a6a80, 0x6c8a8c, true, h,
                       0xc9cfc6, 0x3f5f72, 0x587c88);
        }
        drawRidge(0, off, base - 8, 10, 0.007f, 2.2f, 0x5f8272, 0x7f9a7c);
        // near: ploughed red-brown fields with palms, thatched sheds, cottages and a power line
        off = g.camX * 0.42f; base = 448 + nearUp;
        auto ground = [&](float wx) { return ridgeY(0, wx, base, 14, 0.008f, 0.5f) + 8; };
        for (int k = int(std::floor(off / 150)) - 1; k <= int((off + VIEW_W) / 150) + 1; k++) {
            uint32_t h = hash2(k, 91);
            float wx = k * 150.f + (h >> 4) % 80, x = wx - off, y = ground(wx);
            int kind = h % 4;
            if (kind == 0) {         // thatched tobacco shed: a dark, steep roof almost down to the ground
                float sc = 0.8f + (h >> 9) % 30 / 100.f;
                g_fill(0x5f5440); g_path(); g_move(x - 40 * sc, y); g_line(x - 26 * sc, y - 60 * sc); g_line(x + 26 * sc, y - 60 * sc); g_line(x + 40 * sc, y); g_close(); g_paint(FILL);
                g_fill(0x7a6c50); g_path(); g_move(x - 34 * sc, y - 14 * sc); g_line(x - 24 * sc, y - 56 * sc); g_line(x + 24 * sc, y - 56 * sc); g_line(x + 34 * sc, y - 14 * sc); g_close(); g_paint(FILL);
            } else if (kind == 1) {
                palmSilhouette(x, y, 130 + (h >> 8) % 90, 0x3f7358, 0x7f8a7c);
            } else if (kind == 2) {  // cottage
                static const int walls[] = {0x6fc4b6, 0xd9625a, 0xe9e2d0};
                g_fill(walls[(h >> 7) % 3]); g_rect(x - 24, y - 20, 48, 20);
                g_fill(0x8a4038); g_path(); g_move(x - 29, y - 20); g_line(x - 18, y - 30); g_line(x + 18, y - 30); g_line(x + 29, y - 20); g_close(); g_paint(FILL);
                g_fill(0xf4efe2); g_rect(x - 16, y - 15, 8, 10); g_rect(x + 6, y - 15, 8, 15);
            }
        }
        for (int k = int(std::floor(off / 190)) - 1; k <= int((off + VIEW_W) / 190) + 1; k++) {  // poles and wires
            float wx = k * 190.f + 40, wx2 = wx + 190, x = wx - off, x2 = wx2 - off;
            float y = ground(wx) - 78, y2 = ground(wx2) - 78;
            g_stroke(0x4c4450, 1.6);
            for (int w = 0; w < 2; w++) { g_path(); g_move(x - 6 + w * 12, y); g_quad((x + x2) / 2, (y + y2) / 2 + 20 + w * 4, x2 - 6 + w * 12, y2); g_paint(STROKE); }
            g_fill(0x5a5058); g_rect(x - 2, y - 4, 4, 86); g_rect(x - 9, y - 1, 18, 3);
        }
        drawRidge(0, off, base, 14, 0.008f, 0.5f, 0xa8623e, 0xbd7c52);
        for (int row = 0; row < 4; row++) {
            float sp = 26 + row * 6, shift = row * 13.f;
            for (int k = int(std::floor((off + shift) / sp)) - 1; k <= int((off + shift + VIEW_W) / sp) + 1; k++) {
                float wx = k * sp - shift, y = ridgeY(0, wx, base, 14, 0.008f, 0.5f) + 12 + row * 22;
                g_fill((k + row) % 2 ? 0x5a8f45 : 0x6f9f50);
                g_ellipse(wx - off, y, 9 + row * 2, 5 + row, 0, FILL);
                g_fill(0x8fb866); g_ellipse(wx - off - 2, y - 2 - row * 0.5f, 4 + row, 2 + row * 0.4f, 0, FILL);
            }
        }
    }
}

// Fog (Heidestein's lakes). The first call hazes the far scenery, the second lays drifting banks over the level.
void drawFog(const Game& g, bool front) {
    float f = g.fogAt(g.camX + VIEW_W / 2.f);
    if (f <= 0) return;
    if (!front) {
        g_alpha(0.6 * f); g_fill(0xe6ecf1); g_rect(0, 0, VIEW_W, VIEW_H); g_alpha(1);
        return;
    }
    for (int i = 0; i < 7; i++) {
        float x = std::fmod(i * 230.f + g.time * (10 + (i % 3) * 5) - g.camX * 0.7f, 1500.f);
        if (x < 0) x += 1500;
        float y = 300 + (i * 67 % 170);
        g_save(); g_translate(x - 250, y); g_scale(3.6, 1);
        g_rad(0, 0, 80, 0xffffff, 0.42 * f, 0xffffff, 0); g_circle(0, 0, 80, FILL);
        g_restore();
    }
    g_alpha(0.16 * f); g_fill(0xf2f5f8); g_rect(0, 0, VIEW_W, VIEW_H); g_alpha(1);
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
    g_stroke(0xb5a78c, 13); g_paint(STROKE);
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
        g_fill(i % 2 ? 0x4f9455 : 0x6ba862); g_stroke(OUT, 3); g_paint(BOTH);
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

// Vinales: a tobacco-drying shed (casa de tabaco) - a dark, steep thatched roof that comes almost down to
// the ground, with the leaves hung up inside.
static void drawTobaccoShed(float x, float y) {
    g_save(); g_translate(x, y);
    g_stroke(INK, 3.5);
    // the thatch: steep sides, a long flat ridge, a ragged lower edge
    g_path(); g_move(-112, -8); g_line(-88, -122); g_quad(-84, -134, -68, -134); g_line(72, -134); g_quad(88, -134, 92, -122); g_line(116, -8);
    for (int i = 0; i < 14; i++) { float fx = 116 - (i + 0.5f) * 16.3f; g_line(fx, -2 + (i % 2) * 5); g_line(fx - 8, -9); }
    g_close();
    g_lin(0, -134, 0, 0, 0x93846a, 0x5d5244); g_paint(BOTH);
    g_stroke(0x4a4136, 2);
    for (int row = 0; row < 5; row++) {
        float ry = -118 + row * 23, half = 84 + row * 5.2f;
        for (float fx = -half + 8 + (row % 2) * 9; fx < half - 4; fx += 19) { g_path(); g_move(fx, ry); g_line(fx - 2, ry + 13); g_paint(STROKE); }
    }
    g_alpha(0.3); g_fill(0xd6c9a8); g_ellipse(34, -74, 26, 30, 0.2, FILL); g_ellipse(-50, -100, 30, 9, 0, FILL); g_alpha(1);  // sun-bleached patches
    g_stroke(0xb3a482, 3); g_path(); g_move(-66, -131); g_line(70, -131); g_paint(STROKE);
    // doorway with tobacco leaves drying inside
    g_stroke(INK, 3); g_fill(0x2f2622); g_rrect(-27, -66, 54, 67, 3, BOTH);
    for (int row = 0; row < 2; row++) {
        float py = -57 + row * 26;
        g_stroke(0xb59a7a, 2.5); g_path(); g_move(-24, py); g_line(24, py); g_paint(STROKE);
        g_stroke(INK, 1.6);
        for (int i = -2; i <= 2; i++) {
            g_fill((i + row) % 2 ? 0xb58a3a : 0x7f9a45);
            g_ellipse(i * 9.5f, py + 12, 4.3, 10.5f, i * 0.05f, BOTH);
        }
    }
    g_stroke(INK, 3); g_fill(0x8c7256);
    g_rrect(-33, -70, 8, 72, 2, BOTH); g_rrect(25, -70, 8, 72, 2, BOTH); g_rrect(-35, -74, 70, 8, 2, BOTH);  // door frame
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
            g_fill((i + (side > 0)) % 2 ? 0x5a9a48 : 0x7db45c); g_stroke(INK, 2.4); g_paint(BOTH);
            g_stroke(0x2f9a4a, 1.6); g_path(); g_move(lx + side * 4, ly - 2); g_quad(lx + side * len * 0.5f, ly - 9, ex - side * 5, ey); g_paint(STROKE);
        }
    }
    g_fill(0xf2c4cf); g_stroke(INK, 1.8);
    g_circle(x + sway - 5, y - hgt - 2, 4, BOTH); g_circle(x + sway + 5, y - hgt - 3, 4, BOTH); g_circle(x + sway, y - hgt - 8, 4.2, BOTH);
}

// Heidestein: a roe deer. It turns to watch Emma when she comes close.
// Bolting: a flat-out gallop to the right, legs gathering and stretching, white tail up.
static void drawDeerRunning(const Decor& d) {
    const int FUR = 0xc98c52, DARK = 0xa56c3c;
    float ph = d.t * 13, s = std::sin(ph), reach = 0.35f + 0.65f * d.lift;   // fully stretched in mid-leap
    float hop = -std::fabs(std::sin(ph * 0.5f)) * 16 - d.lift * 34;
    g_save(); g_translate(d.x, d.y + hop); g_rotate(-0.10f - d.lift * 0.1f + s * 0.05f);
    auto leg = [&](float x0, float swing, int col) {
        g_path(); g_move(x0, -32); g_line(x0 + swing * 12, -17); g_line(x0 + swing * 26, -4 + std::fabs(swing) * 4);
        g_stroke(INK, 7); g_paint(STROKE); g_stroke(col, 3.6); g_paint(STROKE);
    };
    float f = std::max(s, reach), b = -std::max(-s * 0.2f + reach, reach);
    leg(-15, b * 0.8f, DARK); leg(13, f * 0.8f, DARK);
    g_stroke(INK, 3);
    g_fill(0xfaf3e3); g_ellipse(-23, -42, 7, 9, 0, BOTH);
    g_path(); g_move(-24, -48); g_line(-31, -58); g_line(-21, -52); g_close(); g_paint(BOTH);   // tail up
    g_lin(0, -50, 0, -24, 0xd9a063, 0xb57a45); g_ellipse(-1, -37, 25, 12.5, 0, BOTH);
    g_fill(0xe9c79a); g_ellipse(2, -29, 13, 4, 0, FILL);
    leg(-11, b, FUR); leg(18, f, FUR);
    g_save(); g_translate(17, -42); g_rotate(0.35f);
    g_stroke(INK, 3); g_fill(FUR);
    g_path(); g_move(-6, -2); g_quad(2, -16, 4, -30); g_line(14, -26); g_quad(12, -10, 8, 6); g_close(); g_paint(BOTH);
    g_fill(DARK); g_ellipse(2, -38, 4, 9, -1.1f, BOTH);
    g_stroke(INK, 5); g_path(); g_move(9, -36); g_line(6, -49); g_move(8, -43); g_line(12, -49); g_paint(STROKE);
    g_stroke(0x8f7355, 2.2); g_paint(STROKE);
    g_stroke(INK, 3); g_fill(FUR); g_ellipse(8, -40, 4, 9, -0.8f, BOTH);
    g_ellipse(12, -29, 9.5, 7.5, -0.15, BOTH);
    g_fill(0xe9c79a); g_ellipse(20, -26, 6.5, 4.8, 0.1, BOTH);
    g_fill(INK); g_circle(25, -26.5, 2.3, FILL); g_circle(14, -31.5, 2.3, FILL);
    g_fill(0xffffff); g_circle(14.6, -32.2, 0.8, FILL);
    g_restore();
    g_restore();
}

static void drawDeer(float x, float y, float t, float playerX, uint32_t h) {
    bool alert = std::fabs(playerX - x) < 330;
    float dir = alert ? (playerX > x ? 1.f : -1.f) : (h % 2 ? 1.f : -1.f);
    const int FUR = 0xc98c52, DARK = 0xa56c3c;
    g_save(); g_translate(x, y); g_scale(dir, 1);
    auto leg = [&](float x0, float x1, float xk, int col) {
        g_path(); g_move(x0, -32); g_line(xk, -15); g_line(x1, 0);
        g_stroke(INK, 7); g_paint(STROKE); g_stroke(col, 3.6); g_paint(STROKE);
    };
    leg(-17, -19, -21, DARK); leg(13, 12, 14, DARK);
    g_stroke(INK, 3);
    g_fill(0xfaf3e3); g_ellipse(-23, -40, 6, 8, 0, BOTH);                       // white rump
    g_lin(0, -50, 0, -24, 0xd9a063, 0xb57a45); g_ellipse(-1, -37, 24, 13.5, 0, BOTH);
    g_fill(0xe9c79a); g_ellipse(2, -29, 13, 4.5, 0, FILL);
    leg(-12, -11, -15, FUR); leg(18, 20, 19, FUR);
    // neck and head: up and watching when alert, otherwise nodding gently
    float nod = alert ? 0 : 0.5f + std::sin(t * 0.7f + h % 9) * 0.45f;
    g_save(); g_translate(15, -42); g_rotate(nod);
    g_stroke(INK, 3); g_fill(FUR);
    g_path(); g_move(-6, -2); g_quad(2, -16, 4, -30); g_line(14, -26); g_quad(12, -10, 8, 6); g_close(); g_paint(BOTH);
    float ear = std::sin(t * 5 + h % 5) > 0.92f ? 0.35f : 0;
    g_fill(DARK); g_ellipse(4, -40, 4, 9, -0.55f - ear, BOTH);
    g_stroke(INK, 5); g_path(); g_move(9, -36); g_line(8, -50); g_move(8.5, -43); g_line(13, -49); g_paint(STROKE);   // small antlers
    g_stroke(0x8f7355, 2.2); g_paint(STROKE);
    g_stroke(INK, 3); g_fill(FUR); g_ellipse(11, -41, 4, 9, 0.25f + ear, BOTH);
    g_ellipse(12, -29, 9.5, 7.5, -0.15, BOTH);
    g_fill(0xe9c79a); g_ellipse(20, -26, 6.5, 4.8, 0.1, BOTH);
    g_fill(INK); g_circle(25, -26.5, 2.3, FILL); g_circle(14, -31.5, 2, FILL);
    g_fill(0xffffff); g_circle(14.6, -32.2, 0.7, FILL);
    g_restore();
    g_restore();
}

// Heidestein: a Scots pine whose roots stand clear of the sand, as on the drifting dunes.
static void drawRootPine(float x, float y, uint32_t h) {
    const int BARK = 0x9a7458, BARK_L = 0xc49a74, ROOT = 0x8d7a66;
    float lean = (int(h % 30) - 15) * 0.6f;
    g_save(); g_translate(x, y);
    // crown: flat, irregular clumps high up
    static const float crown[][3] = {{-46, -268, 30}, {-8, -292, 36}, {36, -280, 32}, {70, -262, 24}, {-70, -250, 20}, {10, -258, 30}};
    g_fill(0x1f4d40);
    for (auto& c : crown) g_circle(c[0] + lean, c[1], c[2] + 3.5f, FILL);
    for (auto& c : crown) { g_fill(0x3d8f68); g_circle(c[0] + lean, c[1], c[2], FILL); }
    g_fill(0x5fb486);
    for (auto& c : crown) g_circle(c[0] + lean - c[2] * 0.25f, c[1] - c[2] * 0.3f, c[2] * 0.45f, FILL);
    auto limb = [](float x0, float y0, float cx, float cy, float x1, float y1, float w, int col) {
        g_path(); g_move(x0, y0); g_quad(cx, cy, x1, y1);
        g_stroke(INK, w + 5); g_paint(STROKE); g_stroke(col, w); g_paint(STROKE);
    };
    limb(lean * 0.8f, -210, lean - 30, -232, lean - 44, -250, 5, BARK);
    limb(lean * 0.8f, -200, lean + 34, -224, lean + 50, -256, 5, BARK);
    // roots behind the trunk
    limb(-4, -58, -70, -70, -128, -2, 6, ROOT);
    limb(6, -56, 60, -84, 132, -4, 6, ROOT);
    limb(-2, -44, -30, -6, -74, 2, 5, ROOT);
    // trunk: thick at the foot, red-brown and bare all the way up
    g_path(); g_move(-17, -44); g_quad(-8, -150, lean - 7, -262); g_line(lean + 7, -262); g_quad(10, -150, 17, -44); g_close();
    g_lin(0, -262, 0, -44, BARK_L, BARK); g_stroke(INK, 3.5); g_paint(BOTH);
    g_stroke(0x6f5340, 2);
    for (int i = 0; i < 7; i++) { float py = -70 - i * 27; g_path(); g_move(-5 + (i % 2) * 6, py); g_line(-3 + (i % 2) * 6, py - 12); g_paint(STROKE); }
    // roots in front: arching out, crossing, with dangling rootlets
    limb(-12, -50, -52, -44, -96, 2, 7, ROOT);
    limb(12, -50, 44, -40, 92, 2, 7, ROOT);
    limb(-6, -46, -16, -14, -40, 3, 6, ROOT);
    limb(6, -46, 22, -18, 48, 3, 6, ROOT);
    limb(0, -44, 2, -20, -6, 3, 5, ROOT);
    limb(-58, -36, -42, -22, -56, 2, 3.5f, ROOT);
    limb(58, -32, 70, -36, 112, -30, 3.5f, ROOT);
    limb(-88, -22, -100, -30, -136, -26, 3, ROOT);
    limb(96, -28, 100, -14, 96, -4, 2.5f, ROOT);
    limb(-112, -27, -114, -16, -110, -8, 2.5f, ROOT);
    g_fill(0xf3e4bb); g_stroke(INK, 2.5);                       // a little sand heaped against the foot
    g_path(); g_move(-26, 4); g_quad(0, -12, 28, 4); g_close(); g_paint(BOTH);
    g_restore();
}

// Heidestein's Theehuis: a white lower storey, a dark timber upper storey with a balcony and a red parasol,
// on a wooded mound above the lake, with the little white battlemented gate below it.
static void drawTheehuis(float x, float y) {
    g_save(); g_translate(x, y);
    g_stroke(INK, 3.5);
    // the mound
    g_path(); g_move(-150, 4); g_quad(-110, -44, -50, -46); g_line(56, -46); g_quad(92, -40, 150, 4); g_close();
    g_lin(0, -46, 0, 4, 0x86b860, 0x5f8f48); g_paint(BOTH);
    g_stroke(0x4f7a3c, 2); g_path(); g_move(-96, -22); g_quad(-88, -30, -80, -22); g_move(-36, -30); g_quad(-28, -38, -20, -30); g_move(84, -18); g_quad(92, -26, 100, -18); g_paint(STROKE);
    g_stroke(INK, 2.5); g_fill(0xb9b3a6);                       // steps up the mound
    for (int i = 0; i < 5; i++) g_rrect(-104 + i * 11, -6 - i * 8, 18, 6, 2, BOTH);
    // white lower storey
    g_stroke(INK, 3.5);
    g_lin(0, -112, 0, -46, 0xfaf7ef, 0xdcd6c8); g_rrect(-46, -112, 92, 68, 2, BOTH);
    g_fill(0x6f6a66); g_rect(-45, -54, 90, 9);
    g_fill(0x2f2a35); g_stroke(INK, 2.5); g_rrect(8, -98, 20, 26, 2, BOTH); g_rrect(-34, -84, 14, 38, 2, BOTH);
    // dark timber upper storey, open to the balcony
    g_stroke(INK, 3.5);
    g_fill(0x4e3d33); g_rrect(-54, -168, 108, 58, 2, BOTH);
    g_fill(0x2f2622); g_rect(-46, -160, 92, 30);
    g_stroke(0x4e3d33, 5); g_path(); g_move(-16, -160); g_line(-16, -130); g_move(18, -160); g_line(18, -130); g_paint(STROKE);
    g_fill(0xd9483b); g_stroke(INK, 2.2);                       // the red parasol, folded
    g_path(); g_move(2, -164); g_quad(9, -146, 5, -128); g_line(-1, -128); g_quad(-5, -146, 2, -164); g_close(); g_paint(BOTH);
    g_stroke(INK, 3); g_fill(0x3d2f28); g_rrect(-60, -132, 120, 24, 3, BOTH);       // balcony
    g_stroke(0x6b574a, 2);
    for (int i = -5; i <= 5; i++) { g_path(); g_move(i * 10.5f - 4, -128); g_line(i * 10.5f + 4, -112); g_move(i * 10.5f + 4, -128); g_line(i * 10.5f - 4, -112); g_paint(STROKE); }
    // roof and chimney
    g_stroke(INK, 3.5);
    g_fill(0xe9e4d8); g_rrect(30, -214, 11, 26, 2, BOTH);
    g_path(); g_move(-68, -166); g_line(-44, -204); g_line(44, -204); g_line(68, -166); g_close();
    g_lin(0, -204, 0, -166, 0xa39a8c, 0x7a7166); g_paint(BOTH);
    g_stroke(0x5f574e, 2); g_path(); g_move(-52, -178); g_line(52, -178); g_move(-46, -190); g_line(46, -190); g_paint(STROKE);
    // the white gate with battlements, at the foot of the mound towards the water
    g_stroke(INK, 3.5);
    g_path(); g_move(58, 3); g_line(72, -52); g_line(72, -64); g_line(84, -64); g_line(84, -56); g_line(94, -56); g_line(94, -64);
    g_line(106, -64); g_line(106, -56); g_line(116, -56); g_line(116, -64); g_line(128, -64); g_line(128, -52); g_line(142, 3); g_close();
    g_lin(0, -64, 0, 0, 0xfaf7ef, 0xd6d0c2); g_paint(BOTH);
    g_fill(0x2f2a35); g_stroke(INK, 3); g_path(); g_move(86, 3); g_line(86, -24); g_quad(100, -44, 114, -24); g_line(114, 3); g_close(); g_paint(BOTH);
    g_stroke(0x8a8f96, 1.8);
    for (int i = 0; i < 4; i++) { g_path(); g_move(91 + i * 6, 1); g_line(91 + i * 6, -28); g_paint(STROKE); }
    g_restore();
}

// Heidestein: a weathered fallen pine.
static void drawLog(float x, float y) {
    g_save(); g_translate(x, y - 2); g_rotate(-0.04f);
    g_stroke(INK, 3);
    g_stroke(INK, 9); g_path(); g_move(-20, -12); g_line(-34, -40); g_move(18, -14); g_line(34, -36); g_move(26, -24); g_line(40, -24); g_paint(STROKE);
    g_stroke(0xb3a696, 4.5); g_paint(STROKE);
    g_stroke(INK, 3); g_lin(0, -20, 0, 0, 0xc2b6a6, 0x8f8272); g_rrect(-62, -20, 118, 20, 9, BOTH);
    g_fill(0x9a8d7c); g_path(); g_move(52, -22); g_line(70, -30); g_line(64, -14); g_line(76, -8); g_line(62, -2); g_line(66, 6); g_line(50, 0); g_close(); g_paint(BOTH);
    g_stroke(0x7a6e60, 2); g_path(); g_move(-48, -12); g_line(-20, -13); g_move(-8, -7); g_line(24, -8); g_move(-40, -5); g_line(-26, -5); g_paint(STROKE);
    g_fill(0xd6ccbc); g_stroke(INK, 2.5); g_ellipse(-61, -10, 5, 9, 0, BOTH);
    g_restore();
}

// Heidestein: a bare, bleached dead pine.
static void drawDeadTree(float x, float y) {
    auto limb = [](float x0, float y0, float cx, float cy, float x1, float y1, float w) {
        g_path(); g_move(x0, y0); g_quad(cx, cy, x1, y1);
        g_stroke(INK, w + 5); g_paint(STROKE); g_stroke(0xc4b9aa, w); g_paint(STROKE);
    };
    limb(x + 12, y - 120, x + 40, y - 150, x + 62, y - 178, 4);
    limb(x - 4, y - 96, x - 36, y - 110, x - 50, y - 146, 4.5);
    limb(x + 8, y - 150, x - 10, y - 178, x - 22, y - 200, 3.5);
    limb(x + 30, y - 144, x + 44, y - 142, x + 56, y - 150, 2.5);
    limb(x - 30, y - 112, x - 46, y - 112, x - 62, y - 120, 2.5);
    limb(x, y + 2, x - 8, y - 90, x + 14, y - 196, 10);
    limb(x + 14, y - 190, x + 16, y - 206, x + 24, y - 216, 3);
}

// Heidestein: a birch by the water, bare as in early spring.
static void drawBirch(float x, float y, float t) {
    float sway = std::sin(t * 1.1f + x) * 2;
    g_stroke(INK, 2.6);
    static const float twigs[][6] = {{4, -150, -40, -190, -78, -196}, {6, -170, 50, -200, 84, -214}, {5, -120, -44, -140, -66, -170},
                                     {8, -196, 20, -236, 12, -262}, {7, -140, 48, -150, 74, -176}, {6, -182, -24, -224, -46, -244},
                                     {-52, -172, -70, -180, -86, -168}, {56, -196, 76, -196, 96, -186}, {-30, -206, -50, -220, -66, -214}};
    for (auto& w : twigs) { g_path(); g_move(x + w[0], y + w[1]); g_quad(x + w[2] + sway * 0.5f, y + w[3], x + w[4] + sway, y + w[5]); g_paint(STROKE); }
    g_path(); g_move(x - 2, y + 2); g_quad(x - 10, y - 100, x + 8, y - 228);
    g_stroke(INK, 17); g_paint(STROKE); g_stroke(0xf3f0e8, 11); g_paint(STROKE);
    g_stroke(INK, 3);
    for (int i = 0; i < 9; i++) {
        float k = (i + 0.6f) / 9.5f, px = x - 2 - 16 * k * (1 - k) * 2 + 10 * k * k, py = y - 226 * k;
        g_path(); g_move(px - 5 + (i % 2) * 4, py); g_line(px + 1 + (i % 2) * 4, py + (i % 3 - 1)); g_paint(STROKE);
    }
    g_fill(INK); g_path(); g_move(x - 12, y + 3); g_quad(x - 4, y - 16, x - 2, y - 24); g_quad(x + 2, y - 12, x + 9, y + 3); g_close(); g_paint(FILL);
}

// Reeds standing in the water. y is the bottom of the top water tile.
static void drawReeds(float x, float y, float t, uint32_t h) {
    float base = y - TILE + WATER_TOP + 8;
    for (int i = 0; i < 8; i++) {
        uint32_t r = hash2(int(h % 911) + i, 17);
        float bx = x - 22 + i * 6.3f, hgt = 26 + r % 34, sway = std::sin(t * 1.5f + i + x) * 3;
        g_path(); g_move(bx, base); g_quad(bx + 1, base - hgt * 0.5f, bx + (int(r >> 6) % 9 - 4) + sway, base - hgt);
        g_stroke(INK, 4.5); g_paint(STROKE); g_stroke(i % 3 ? 0xd9c27a : 0xbfa55e, 2.2); g_paint(STROKE);
        if (i % 3 == 1) { g_fill(0x7a5a3a); g_stroke(INK, 1.8); g_ellipse(bx + (int(r >> 6) % 9 - 4) + sway, base - hgt, 2.8, 7, 0.1, BOTH); }
    }
}

// Vinales: a simple cottage - painted walls, a porch on columns, louvred shutters, a flat slab roof.
static void drawCottage(float x, float y, bool red) {
    int wall = red ? 0xe2605a : 0x79d2c4, wallDark = red ? 0xb9453f : 0x56ab9f, trim = red ? 0xf7efe2 : 0xdff5ef;
    g_save(); g_translate(x, y);
    g_stroke(INK, 3);
    if (!red) {  // low corrugated roof behind, water tank and aerial
        g_fill(0x9c453d); g_path(); g_move(-84, -76); g_line(-66, -100); g_line(30, -100); g_line(48, -76); g_close(); g_paint(BOTH);
        g_stroke(0x73302b, 1.8);
        for (int i = -6; i <= 3; i++) { g_path(); g_move(i * 11.f - 4, -98); g_line(i * 12.f - 8, -78); g_paint(STROKE); }
        g_stroke(INK, 2.5); g_fill(0xb8483c); g_rrect(52, -102, 26, 24, 3, BOTH);
        g_stroke(0x7a2e27, 1.5); g_path(); g_move(52, -94); g_line(78, -94); g_move(52, -86); g_line(78, -86); g_move(61, -102); g_line(61, -78); g_move(69, -102); g_line(69, -78); g_paint(STROKE);
    }
    g_stroke(INK, 2.5); g_path(); g_move(24, -76); g_line(24, -126); g_move(12, -122); g_line(36, -122); g_move(16, -114); g_line(32, -114); g_paint(STROKE);
    // walls and porch
    g_stroke(INK, 3);
    g_lin(0, -72, 0, 0, wall, wallDark); g_rrect(-84, -72, 168, 74, 2, BOTH);
    g_fill(wallDark); g_rect(-80, -66, 70, 66);
    g_fill(trim); g_stroke(INK, 2.5); g_rrect(-62, -56, 20, 56, 2, BOTH);            // door
    g_fill(INK); g_circle(-46, -28, 1.6, FILL);
    auto shutter = [&](float sx) {
        g_fill(0xf4efe2); g_stroke(INK, 2.5); g_rrect(sx, -54, 26, 34, 2, BOTH);
        g_stroke(0xa9a394, 1.5);
        for (int i = 1; i < 7; i++) { g_path(); g_move(sx + 3, -54 + i * 4.8f); g_line(sx + 23, -54 + i * 4.8f); g_paint(STROKE); }
    };
    shutter(-36); shutter(6); shutter(46);
    g_stroke(INK, 2.5); g_fill(trim);
    g_rrect(-82, -68, 8, 70, 2, BOTH); g_rrect(-16, -68, 8, 70, 2, BOTH);            // porch columns
    g_fill(wallDark); g_rect(-83, -8, 166, 8);
    g_stroke(INK, 3); g_fill(red ? 0xc23b36 : 0x6e403b); g_rrect(-92, -80, 184, 11, 3, BOTH);  // flat slab roof
    g_restore();
}

// Vinales: a wooden utility pole. The wires between poles are drawn by drawWires().
static void drawPole(float x, float y) {
    g_stroke(INK, 3); g_fill(0x6e625a);
    g_rrect(x - 4.5f, y - 176, 9, 179, 3, BOTH);
    g_rrect(x - 17, y - 170, 34, 6, 2, BOTH); g_rrect(x - 11, y - 154, 22, 5, 2, BOTH);
    g_fill(0xbfc9c4); g_stroke(INK, 1.8);
    g_circle(x - 13, y - 173, 3, BOTH); g_circle(x + 13, y - 173, 3, BOTH);
}

void drawWires(const Game& g) {
    g_stroke(0x3a3340, 2);
    for (size_t i = 0; i < g.decor.size(); i++) {
        const Decor& a = g.decor[i];
        if (a.kind != 'p') continue;
        if (a.x < g.camX - 700 || a.x > g.camX + VIEW_W + 700) continue;
        for (const Decor& b : g.decor) {
            float dx = b.x - a.x;
            if (b.kind == 'p' && dx > 0 && dx < 12 * TILE && std::fabs(b.y - a.y) < TILE) {       // pole to pole
                for (int w = -1; w <= 1; w += 2) {
                    g_path(); g_move(a.x + w * 13, a.y - 173);
                    g_quad((a.x + b.x) / 2, a.y - 173 + 26 + dx * 0.03f, b.x + w * 13, b.y - 173); g_paint(STROKE);
                }
            } else if ((b.kind == 'c' || b.kind == 'j') && std::fabs(dx) < 5 * TILE && std::fabs(b.y - a.y) < TILE) {  // drop to a house
                g_path(); g_move(a.x, a.y - 152); g_quad((a.x + b.x) / 2, a.y - 96, b.x + (dx > 0 ? -70 : 70), b.y - 80); g_paint(STROKE);
            }
        }
    }
}

// Vinales: the old yellow village bus with its two white stripes, parked on the verge.
static void drawBus(float x, float y) {
    const int YEL = 0xeea524, YEL_D = 0xcf8518, WHITE = 0xf4efe2, GLASS = 0x4c5862;
    g_save(); g_translate(x, y);
    g_stroke(INK, 3.5);
    g_fill(WHITE); g_rrect(-106, -30, 14, 10, 3, BOTH); g_rrect(92, -30, 14, 10, 3, BOTH);     // bumpers
    g_path(); g_move(-98, -24); g_line(-98, -72); g_quad(-98, -102, -68, -102); g_line(64, -102); g_quad(94, -102, 98, -68);
    g_line(98, -24); g_quad(98, -15, 89, -15); g_line(-89, -15); g_quad(-98, -15, -98, -24); g_close();
    g_lin(0, -102, 0, -15, YEL, YEL_D); g_paint(BOTH);
    g_fill(0xf7bd4a); g_rrect(-80, -100, 150, 8, 4, FILL);                                     // roof highlight
    g_fill(WHITE); g_rect(-96.5, -56, 193, 10); g_rect(-96.5, -38, 193, 10);                   // the two white stripes
    g_stroke(INK, 1.8); g_path(); g_move(-97, -56); g_line(97, -56); g_move(-97, -46); g_line(97, -46); g_move(-97, -38); g_line(97, -38); g_move(-97, -28); g_line(97, -28); g_paint(STROKE);
    // windows: rounded at the back, a row along the side, the open door near the front
    g_stroke(INK, 2.8); g_fill(GLASS);
    g_rrect(-91, -93, 27, 28, 10, BOTH);
    for (int i = 0; i < 4; i++) g_rrect(-56 + i * 29.f, -93, 25, 28, 3, BOTH);
    g_rrect(84, -91, 10, 26, 4, BOTH);
    g_fill(0x2f2a35); g_rrect(62, -93, 17, 76, 3, BOTH);
    g_fill(0xd9b02e); g_rect(64, -91, 4, 72);
    g_alpha(0.35); g_fill(0xbfd2dc);
    for (int i = 0; i < 4; i++) { g_path(); g_move(-52 + i * 29.f, -68); g_line(-40 + i * 29.f, -91); g_line(-34 + i * 29.f, -91); g_line(-46 + i * 29.f, -68); g_close(); g_paint(FILL); }
    g_alpha(1);
    g_fill(0xb5332a); g_stroke(INK, 2); g_rrect(-98, -52, 5, 14, 2, BOTH);                      // tail light
    g_alpha(0.3); g_fill(0x8a4a1a);                                                            // a little rust
    g_circle(-30, -20, 4, FILL); g_circle(20, -62, 3, FILL); g_circle(-78, -60, 3.5, FILL); g_circle(44, -22, 5, FILL);
    g_alpha(1);
    for (float wx : {-58.f, 54.f}) {                                                           // wheels
        g_fill(0x2f2a35); g_path(); g_move(wx - 21, -15); g_quad(wx, -42, wx + 21, -15); g_close(); g_paint(FILL);
        g_stroke(INK, 3); g_fill(0x3d3742); g_circle(wx, -13, 15, BOTH);
        g_fill(0xbfc4c9); g_circle(wx, -13, 7, BOTH);
    }
    g_restore();
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
        case 'R': drawStockfishRack(d.x, d.y, t); break;
        case 'D':
            if (d.state == 0) drawDeer(d.x, d.y, t, g.p.x, h);
            else if (d.state == 1) drawDeerRunning(d);
            break;
        case 'x': drawRootPine(d.x, d.y, h); break;
        case 'h': drawTheehuis(d.x, d.y); break;
        case 'l': drawLog(d.x, d.y); break;
        case 'd': drawDeadTree(d.x, d.y); break;
        case 'k': drawBirch(d.x, d.y, t); break;
        case 'r': drawReeds(d.x, d.y, t, h); break;
        case 'c': drawCottage(d.x, d.y, false); break;
        case 'j': drawCottage(d.x, d.y, true); break;
        case 'p': drawPole(d.x, d.y); break;
        case 'U': drawBus(d.x, d.y); break;
        case 'B':  // small boat moored on the water; d.y is the bottom of the top water tile
            drawFishingBoat(d.x, d.y - TILE + WATER_TOP - 34 + std::sin(t * 1.6f + d.x) * 2.5f, 0.6f, false);
            break;
    }
}

// ------------------------------------------------------------ terrain

static bool isTop(const Game& g, int tx, int ty) { return g.solid(tx, ty) && !g.filled(tx, ty - 1); }
// What sits on top of a ground tile: 0 nothing (not a top, or under water), 1 the level's grass / snow, 2 bare sand.
static int capKind(const Game& g, int tx, int ty) {
    if (tx < 0 || tx >= g.W || !isTop(g, tx, ty) || g.at(tx, ty - 1) == 'w') return 0;
    return g.at(tx, ty) == ',' ? 2 : 1;
}
static const int SAND[] = {0xf3e4bb, 0xe7d5a4, 0xd9c48f, 0xcab37c};

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
            const int* soil = sc == ',' ? SAND : th.dirt;
            g_fill(soil[depth]); g_rect(x - 0.5, y - 0.5, TILE + 1, TILE + 1);
            uint32_t h = hash2(tx, ty);
            g_fill(soil[std::min(depth + 1, 3)] - 0x0a0806);
            g_ellipse(x + 8 + h % 30, y + 10 + (h >> 5) % 28, 5 + (h >> 10) % 4, 3.5, 0, FILL);
            if ((h >> 14) % 3 == 0) g_ellipse(x + 6 + (h >> 16) % 34, y + 8 + (h >> 21) % 32, 3, 2.2, 0, FILL);
            if ((h >> 24) % 5 == 0) { g_fill(sc == ',' ? 0xfff6dc : th.pebble); g_circle(x + 10 + (h >> 8) % 28, y + 14 + (h >> 3) % 24, 2.2, FILL); }
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
            if (isTop(g, tx, ty) && g.at(tx, ty - 1) == 'w') { g_move(x, y); g_line(x + TILE, y); }  // lake or river bed
            g_paint(STROKE);
        }
    }
    // grass caps, one path per run of top tiles
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            int kind = capKind(g, tx, ty);
            if (!kind) continue;
            int end = tx;
            while (capKind(g, end + 1, ty) == kind) end++;
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
            if (kind == 2) g_lin(0, y - 9, 0, y + 16, 0xfdf4d6, 0xeedcab); else g_lin(0, y - 9, 0, y + 16, th.grass0, th.grass1);
            g_stroke(INK, 3.5); g_paint(BOTH);
            g_stroke(kind == 2 ? 0xfffbea : th.grassHi, 3);
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
            if (g.at(tx, ty) == ',') {  // sand: only tufts of dry grass and the odd sprig of heather
                if (kind <= 3) {
                    for (int i = -2; i <= 2; i++) {
                        float tipX = x + i * 6 + sway * 0.8f, tipY = y - 12 - ((h >> (10 + i)) % 9);
                        g_path(); g_move(x + i * 2.5f, y + 3); g_quad(x + i * 4, y - 6, tipX, tipY);
                        g_stroke(INK, 4.2); g_paint(STROKE); g_stroke(i % 2 ? 0xd9c27a : 0xc2a85c, 2); g_paint(STROKE);
                    }
                } else if (kind == 4) {
                    g_stroke(INK, 1.8);
                    for (int k = 0; k < 4; k++) { g_fill(k % 2 ? 0xb565d9 : 0xe08ff0); g_circle(x - 6 + k * 4.5f, y - 1 - (k % 2) * 4, 3.4, BOTH); }
                }
                continue;
            }
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
                static const int petals[] = {0xe9705f, 0xf4efe2, 0xe6b84a};
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
                    g_fill(i % 2 ? 0x5a9a48 : 0x7db45c);
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
            float opacity = g.level == 1 ? 0.97f : (g.level == 0 ? 0.92f : 0.8f);  // the river is muddy
            g_alpha(opacity);
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
// Vinales: the narrow old footbridge - one log beam on stakes, with loose, uneven planks nailed across it.
static void drawPlankBridge(const Bridge& b) {
    for (float px : {b.x0 + 16, (b.x0 + b.x1) / 2, b.x1 - 16}) {  // stakes down into the river
        float y = bridgeY(b, px);
        g_path(); g_move(px, y + 4); g_line(px + 3, y + 84);
        g_stroke(INK, 10); g_paint(STROKE); g_stroke(0x5b4a3a, 5.5); g_paint(STROKE);
    }
    g_path(); g_move(b.x0 + 12, b.y - 2); g_line(b.x0 + 20, b.y - 40);  // a broken post still standing at the near end
    g_stroke(INK, 9); g_paint(STROKE); g_stroke(0x5b4a3a, 4.5); g_paint(STROKE);
    g_path(); g_move(b.x0 - 4, b.y + 11);
    for (float x = b.x0; x <= b.x1; x += 8) g_line(x, bridgeY(b, x) + 11);
    g_line(b.x1 + 4, b.y + 11);
    g_stroke(INK, 14); g_paint(STROKE); g_stroke(0x6b5744, 8.5); g_paint(STROKE); g_stroke(0x86705a, 2); g_paint(STROKE);
    static const int wood[] = {0x8d7c68, 0xa08d73, 0x7b6c5b, 0xa89982, 0x93826c};
    int i = 0;
    for (float x = b.x0 - 3; x < b.x1 - 9; x += 12.5f, i++) {
        uint32_t h = hash2(int(x), int(b.y));
        float y = bridgeY(b, x + 6) + (int(h % 5) - 2) * 0.7f, tilt = (int((h >> 4) % 7) - 3) * 0.6f;
        float lip = (h >> 8) % 6 == 0 ? 5.f : 0.f;   // some planks stick out
        g_fill(wood[(h >> 12) % 5]); g_stroke(INK, 2.2);
        g_path(); g_move(x, y - 1 - tilt); g_line(x + 11, y - 1 + tilt); g_line(x + 12.5f, y + 6.5f + tilt + lip); g_line(x + 1.5f, y + 6.5f - tilt + lip); g_close(); g_paint(BOTH);
    }
}

void drawBridge(const Game& g, const Bridge& b) {
    if (g.level == 1) { drawPlankBridge(b); return; }
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
