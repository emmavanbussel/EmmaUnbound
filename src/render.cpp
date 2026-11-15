// All drawing. Everything is procedural vector art - no image files.
#include "game.hpp"
#include "gfx.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>

namespace {

const int INK = 0x2b1b3f;  // outline colour used for everything in the foreground
const float PI = 3.14159265f;

uint32_t hash2(int x, int y) {
    uint32_t h = uint32_t(x) * 374761393u + uint32_t(y) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

void starPath(float cx, float cy, float R, float r, float rot, int n = 5) {
    g_path();
    for (int i = 0; i < n * 2; i++) {
        float a = rot + i * PI / n - PI / 2, rad = (i % 2 == 0) ? R : r;
        float x = cx + std::cos(a) * rad, y = cy + std::sin(a) * rad;
        if (i == 0) g_move(x, y); else g_line(x, y);
    }
    g_close();
}

void heartPath(float cx, float cy, float s) {
    g_path();
    g_move(cx, cy + s * 0.95f);
    g_cubic(cx - s * 1.7f, cy - s * 0.2f, cx - s * 0.75f, cy - s * 1.4f, cx, cy - s * 0.45f);
    g_cubic(cx + s * 0.75f, cy - s * 1.4f, cx + s * 1.7f, cy - s * 0.2f, cx, cy + s * 0.95f);
    g_close();
}

// ------------------------------------------------------------ background

void drawCloud(float x, float y, float s) {
    g_save(); g_translate(x, y); g_scale(s, s);
    g_fill(0xd4eeff);
    g_rrect(-6, 2, 78, 26, 13, FILL);
    g_fill(0xffffff);
    g_circle(4, 4, 20, FILL); g_circle(30, -10, 27, FILL); g_circle(58, 2, 21, FILL);
    g_rrect(-8, -2, 82, 24, 12, FILL);
    g_restore();
}

float hillY(float wx, float base, float amp, float f, float ph) {
    return base + amp * std::sin(wx * f + ph) + amp * 0.45f * std::sin(wx * f * 2.3f + ph * 2.1f);
}

void drawHills(float off, float base, float amp, float f, float ph, int top, int bottom) {
    g_lin(0, base - amp * 1.5f, 0, VIEW_H, top, bottom);
    g_path(); g_move(0, VIEW_H);
    for (int sx = 0; sx <= VIEW_W + 16; sx += 16) g_line(sx, hillY(sx + off, base, amp, f, ph));
    g_line(VIEW_W, VIEW_H); g_close(); g_paint(FILL);
}

void drawBackground(const Game& g) {
    float t = g.time;
    float vy = (g.H * TILE - VIEW_H - g.camY);  // 0 when camera is at the bottom of the level
    g_lin3(0, 0, 0, VIEW_H, 0x35a2f4, 0x9be0ff, 0xfff0c9);
    g_rect(0, 0, VIEW_W, VIEW_H);

    // sun with slowly turning rays
    float sx = 770 - g.camX * 0.02f, sy = 105 + vy * 0.05f;
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
    g_fill(0xffe873); g_circle(sx, sy, 42, FILL);

    // clouds
    static const float clouds[][3] = {{80, 70, 1.3f}, {420, 120, 0.9f}, {700, 60, 1.1f}, {1050, 140, 1.5f},
                                      {1400, 80, 1.0f}, {1700, 150, 1.2f}};
    for (auto& c : clouds) {
        float span = 2000;
        float x = std::fmod(c[0] - g.camX * (0.06f + c[2] * 0.03f) + t * 5 * c[2], span);
        if (x < 0) x += span;
        g_alpha(0.92); drawCloud(x - 250, c[1] + vy * 0.08f, c[2]); g_alpha(1);
    }

    // far mountains
    drawHills(g.camX * 0.10f, 300 + vy * 0.10f, 60, 0.0042f, 1.0f, 0xa9d3f5, 0xd2ecfb);

    // floating islands
    for (int i = 0; i < 5; i++) {
        float span = 1900;
        float x = std::fmod(i * 410.f + 130 - g.camX * 0.16f, span);
        if (x < 0) x += span;
        x -= 200;
        float y = 190 + (i * 53 % 90) + std::sin(t * 0.6f + i * 2) * 6 + vy * 0.14f, s = 0.8f + (i % 3) * 0.25f;
        g_save(); g_translate(x, y); g_scale(s, s);
        g_fill(0x8fb2de);
        g_path(); g_move(-46, 0); g_quad(-24, 46, -4, 64); g_quad(18, 40, 46, 0); g_close(); g_paint(FILL);
        g_fill(0xa2e3b7); g_ellipse(0, 0, 50, 11, 0, FILL);
        g_fill(0x8a9fcf); g_rect(8, -22, 5, 22);
        g_fill(0x8fdcae); g_circle(10, -30, 15, FILL); g_circle(-2, -22, 10, FILL);
        g_restore();
    }

    // mid hills
    drawHills(g.camX * 0.22f, 372 + vy * 0.2f, 42, 0.006f, 2.0f, 0x7fdcc8, 0xb9f0dc);

    // near hills with lollipop trees
    float off = g.camX * 0.42f, base = 432 + vy * 0.34f;
    for (int k = int(std::floor(off / 150)) - 1; k <= int((off + VIEW_W) / 150) + 1; k++) {
        uint32_t h = hash2(k, 77);
        if (h % 3 == 0) continue;
        float wx = k * 150.f + (h >> 4) % 80, x = wx - off;
        float y = hillY(wx, base, 30, 0.008f, 0.5f) + 6;
        float s = 0.8f + ((h >> 8) % 60) / 100.f;
        bool pink = (h >> 16) % 4 == 0;
        g_fill(0x4a9d86); g_rrect(x - 4 * s, y - 52 * s, 8 * s, 56 * s, 3, FILL);
        g_fill(pink ? 0xf5a3c7 : 0x4dc592);
        g_circle(x, y - 66 * s, 27 * s, FILL); g_circle(x - 20 * s, y - 52 * s, 18 * s, FILL);
        g_circle(x + 20 * s, y - 54 * s, 19 * s, FILL);
        g_fill(pink ? 0xfbc4dc : 0x6edaa5); g_circle(x - 7 * s, y - 74 * s, 12 * s, FILL);
    }
    drawHills(off, base, 30, 0.008f, 0.5f, 0x5fd195, 0x9ee9b4);
}

// ------------------------------------------------------------ terrain

bool isTop(const Game& g, int tx, int ty) { return g.solid(tx, ty) && !g.solid(tx, ty - 1); }

void drawTerrain(const Game& g, int tx0, int tx1, int ty0, int ty1) {
    static const int dirt[] = {0xd08a55, 0xbd7748, 0xa9653c, 0x975632};
    float t = g.time;
    // dirt
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (!g.solid(tx, ty) || tx < 0 || tx >= g.W) continue;
            int depth = 0;
            while (depth < 3 && g.solid(tx, ty - depth - 1)) depth++;
            float x = tx * TILE, y = ty * TILE;
            g_fill(dirt[depth]); g_rect(x - 0.5, y - 0.5, TILE + 1, TILE + 1);
            uint32_t h = hash2(tx, ty);
            g_fill(dirt[std::min(depth + 1, 3)] - 0x0a0806);
            g_ellipse(x + 8 + h % 30, y + 10 + (h >> 5) % 28, 5 + (h >> 10) % 4, 3.5, 0, FILL);
            if ((h >> 14) % 3 == 0) g_ellipse(x + 6 + (h >> 16) % 34, y + 8 + (h >> 21) % 32, 3, 2.2, 0, FILL);
            if ((h >> 24) % 5 == 0) { g_fill(0xe9b889); g_circle(x + 10 + (h >> 8) % 28, y + 14 + (h >> 3) % 24, 2.2, FILL); }
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
            g_lin(0, y - 9, 0, y + 16, 0x8be04e, 0x3fb844);
            g_stroke(INK, 3.5); g_paint(BOTH);
            g_stroke(0xc9f779, 3);
            g_path(); g_move(x0 + 10, y - 4.5); g_line(x1 - 10, y - 4.5); g_paint(STROKE);
            tx = end;
        }
    }
    // tufts and flowers
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (!isTop(g, tx, ty) || g.at(tx, ty - 1) != '.' || tx < 0 || tx >= g.W) continue;
            uint32_t h = hash2(tx * 7 + 3, ty * 13 + 1);
            int kind = h % 9;
            float x = tx * TILE + 10 + (h >> 6) % 28, y = ty * TILE - 7;
            float sway = std::sin(t * 2 + tx * 1.7f) * 3;
            if (kind <= 1) {  // grass tuft
                g_fill(0x6fdc55); g_stroke(INK, 2);
                for (int i = -1; i <= 1; i++) {
                    float tipX = x + i * 10 + sway, tipY = y - 13 - (i == 0 ? 7 : 0);
                    g_path(); g_move(x + i * 5 - 5, y + 3);
                    g_quad(x + i * 6 - 4, y - 7, tipX, tipY);
                    g_quad(x + i * 6 + 5, y - 5, x + i * 5 + 5, y + 3);
                    g_close(); g_paint(BOTH);
                }
            } else if (kind <= 4) {  // flower
                static const int petals[] = {0xff7eb6, 0xffffff, 0xffd447};
                float hgt = 18 + (h >> 12) % 10, fx = x + sway, fy = y - hgt;
                g_stroke(INK, 5); g_path(); g_move(x, y + 2); g_quad(x, y - hgt / 2, fx, fy); g_paint(STROKE);
                g_stroke(0x4cc24a, 2.5); g_paint(STROKE);
                g_fill(petals[kind - 2]); g_stroke(INK, 2);
                for (int i = 0; i < 5; i++) {
                    float a = i * 2 * PI / 5 + t * 0.4f;
                    g_circle(fx + std::cos(a) * 5.5f, fy + std::sin(a) * 5.5f, 4.2, BOTH);
                }
                g_fill(kind == 4 ? 0xff8a3d : 0xffe45c); g_circle(fx, fy, 3.6, BOTH);
            }
        }
    }
}

void drawPlanks(const Game& g, int tx0, int tx1, int ty0, int ty1) {
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = std::max(0, tx0); tx <= std::min(g.W - 1, tx1); tx++) {
            if (g.at(tx, ty) != '=') continue;
            int end = tx;
            while (g.at(end + 1, ty) == '=' && end + 1 < g.W) end++;
            float x0 = tx * TILE - 4, x1 = (end + 1) * TILE + 4, y = ty * TILE;
            // hanging leaves
            for (int i = tx; i <= end; i++) {
                uint32_t h = hash2(i, ty * 5);
                if (h % 2) continue;
                float lx = i * TILE + 12 + (h >> 4) % 24, len = 12 + (h >> 9) % 14;
                float sw = std::sin(g.time * 2 + i) * 2;
                g_stroke(INK, 5); g_path(); g_move(lx, y + 12); g_quad(lx, y + 12 + len / 2, lx + sw, y + 12 + len); g_paint(STROKE);
                g_stroke(0x4cc24a, 2.5); g_paint(STROKE);
                g_fill(0x6fdc55); g_stroke(INK, 2); g_ellipse(lx + sw, y + 14 + len, 4.5, 6, 0.3, BOTH);
            }
            g_lin(0, y, 0, y + 18, 0xf2bf78, 0xc98845);
            g_stroke(INK, 3.5); g_rrect(x0, y, x1 - x0, 18, 8, BOTH);
            g_stroke(0xffe0a8, 2.5); g_path(); g_move(x0 + 9, y + 5); g_line(x1 - 9, y + 5); g_paint(STROKE);
            g_stroke(0xa56a35, 2);
            for (float x = x0 + 30; x < x1 - 14; x += 34) { g_path(); g_move(x, y + 9); g_line(x + 12, y + 9); g_move(x - 8, y + 13); g_line(x, y + 13); g_paint(STROKE); }
            tx = end;
        }
    }
}

void drawThorns(const Game& g, int tx0, int tx1, int ty0, int ty1) {
    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = std::max(0, tx0); tx <= std::min(g.W - 1, tx1); tx++) {
            if (g.at(tx, ty) != '^') continue;
            float x = tx * TILE, base = (ty + 1) * TILE - 4;
            for (int i = 0; i < 3; i++) {
                float cx = x + 8 + i * 16, hgt = 24 + hash2(tx * 3 + i, ty) % 10;
                float lean = (int(hash2(tx + i, 9) % 7) - 3);
                g_path(); g_move(cx - 9, base + 6); g_line(cx + lean, base - hgt); g_line(cx + 9, base + 6); g_close();
                g_lin(cx - 9, 0, cx + 9, 0, 0xe8457e, 0x9c1f4f); g_stroke(INK, 3); g_paint(BOTH);
                g_stroke(0xffa3c4, 2); g_path(); g_move(cx - 4, base); g_line(cx + lean - 1, base - hgt + 9); g_paint(STROKE);
            }
        }
    }
}

// ------------------------------------------------------------ objects

void drawMushroom(const Mushroom& m) {
    float k = m.bounceT > 0 ? std::sin(m.bounceT / 0.4f * PI) * 0.3f : 0;
    g_save(); g_translate(m.x, m.y); g_scale(1 + k, 1 - k);
    g_stroke(INK, 3);
    g_fill(0xfff1d6); g_rrect(-10, -24, 20, 28, 7, BOTH);
    g_path(); g_move(-28, -18); g_cubic(-28, -50, 28, -50, 28, -18); g_quad(0, -8, -28, -18); g_close();
    g_lin(0, -44, 0, -12, 0xff6b81, 0xe8334f); g_paint(BOTH);
    g_fill(0xffffff);
    g_circle(-13, -28, 5, FILL); g_circle(4, -34, 6, FILL); g_circle(17, -24, 4, FILL);
    g_restore();
}

void drawMover(const Mover& m, float t) {
    g_save(); g_translate(m.x, m.y + std::sin(t * 2.2f) * 1.5f);
    float hw = m.w / 2;
    g_stroke(INK, 3.5);
    g_fill(0xcfe6ff);
    g_path();
    g_move(-hw + 10, 0); g_line(hw - 10, 0);
    g_quad(hw + 6, 2, hw, 14);
    g_quad(hw - 4, 30, hw - 26, 24);
    g_quad(hw - 44, 40, 0 + 8, 28);
    g_quad(-14, 42, -hw + 30, 26);
    g_quad(-hw - 2, 30, -hw, 14);
    g_quad(-hw - 6, 2, -hw + 10, 0);
    g_close(); g_paint(BOTH);
    g_fill(0xffffff); g_rrect(-hw + 6, 3, m.w - 12, 11, 5.5, FILL);
    // sleepy face
    g_stroke(INK, 2.5);
    g_path(); g_move(-16, 17); g_quad(-11, 21, -6, 17); g_move(6, 17); g_quad(11, 21, 16, 17); g_paint(STROKE);
    g_alpha(0.5); g_fill(0xff9dbb); g_circle(-22, 21, 4, FILL); g_circle(22, 21, 4, FILL); g_alpha(1);
    g_restore();
}

void drawCheckpoint(const Checkpoint& c, float t) {
    g_save(); g_translate(c.x, c.y);
    if (c.active) { g_rad(20, -62, 60, 0xffa3c4, 0.6, 0xffa3c4, 0); g_circle(20, -62, 60, FILL); }
    g_stroke(INK, 3);
    g_fill(0xc98845); g_rrect(-4, -80, 8, 82, 4, BOTH);
    g_fill(0xffd447); g_circle(0, -84, 6.5, BOTH);
    float w1 = c.active ? std::sin(t * 6) * 4 : 0, w2 = c.active ? std::sin(t * 6 + 1.5f) * 4 : 6;
    g_path(); g_move(4, -76);
    g_quad(22, -80 + w1, 46, -66 + w2);
    g_quad(22, -58 + w1, 4, -50);
    g_close();
    g_fill(c.active ? 0xff5d8f : 0xb9c0cc); g_paint(BOTH);
    if (c.active) { g_fill(0xfff27a); starPath(19, -65 + w1 * 0.5f, 7.5, 3.4, 0); g_paint(FILL); }
    g_restore();
}

void drawGoal(const Game& g) {
    float t = g.time, x = g.goalX, y = g.goalY;
    g_stroke(INK, 3.5);
    g_fill(0xa9a4c4); g_rrect(x - 34, y - 14, 68, 18, 6, BOTH);
    g_fill(0xc4c0dc); g_rrect(x - 23, y - 28, 46, 16, 6, BOTH);
    float sy = y - 86 + std::sin(t * 2.4f) * 6;
    g_rad(x, sy, 110, 0xfff27a, 0.75, 0xfff27a, 0); g_circle(x, sy, 110, FILL);
    g_save(); g_translate(x, sy); g_rotate(t * 0.5f); g_alpha(0.35); g_fill(0xffffff);
    for (int i = 0; i < 8; i++) { g_rotate(PI / 4); g_path(); g_move(0, 0); g_line(-9, -78); g_line(9, -78); g_close(); g_paint(FILL); }
    g_alpha(1); g_restore();
    float rot = std::sin(t * 1.6f) * 0.18f, pulse = 1 + std::sin(t * 4) * 0.05f;
    starPath(x, sy, 34 * pulse, 16.5f * pulse, rot);
    g_lin(0, sy - 34, 0, sy + 30, 0xfff9a8, 0xffb21f); g_stroke(INK, 4); g_paint(BOTH);
    g_fill(INK); g_ellipse(x - 6, sy + 1, 2.6, 4, 0, FILL); g_ellipse(x + 6, sy + 1, 2.6, 4, 0, FILL);
    g_stroke(INK, 2.5); g_path(); g_move(x - 5, sy + 9); g_quad(x, sy + 14, x + 5, sy + 9); g_paint(STROKE);
}

void drawOrb(float x, float y, float t, float phase) {
    y += std::sin(t * 3 + phase) * 4;
    g_rad(x, y, 24, 0xfff2a0, 0.7, 0xfff2a0, 0); g_circle(x, y, 24, FILL);
    g_lin(0, y - 10, 0, y + 10, 0xfffbd0, 0xffb21f);
    g_stroke(0xa85b00, 2.5); g_circle(x, y, 9.5, BOTH);
    g_fill(0xffffff); g_circle(x - 3.2, y - 3.5, 2.8, FILL);
    float tw = 0.5f + 0.5f * std::sin(t * 5 + phase * 3);
    g_alpha(tw); starPath(x + 11, y - 11, 6 * tw + 2, 1.6f, t, 4); g_paint(FILL); g_alpha(1);
}

void angryFace(float ex, float ey, float s) {
    g_fill(0xffffff); g_stroke(INK, 2);
    g_ellipse(ex, ey, 5 * s, 6 * s, 0, BOTH); g_ellipse(ex + 10 * s, ey, 4.5 * s, 5.5 * s, 0, BOTH);
    g_fill(INK); g_circle(ex + 1.8 * s, ey + 1, 2.3 * s, FILL); g_circle(ex + 11.5 * s, ey + 1, 2.1 * s, FILL);
    g_stroke(INK, 3.5 * s);
    g_path(); g_move(ex - 6 * s, ey - 9 * s); g_line(ex + 4 * s, ey - 4.5 * s);
    g_move(ex + 16 * s, ey - 8.5 * s); g_line(ex + 7.5 * s, ey - 4.5 * s); g_paint(STROKE);
}

void drawEnemy(const Enemy& e) {
    g_save(); g_translate(e.x, e.y);
    if (e.type == 0) {
        g_scale(e.vx > 0 ? 1 : -1, 1);
        float w = std::sin(e.t * 10), hop = -std::fabs(w) * 2.5f;
        g_stroke(INK, 3);
        g_fill(0x3d2466);
        g_ellipse(-9 + w * 5, -4, 8, 5, 0, BOTH); g_ellipse(9 - w * 5, -4, 8, 5, 0, BOTH);
        // horns
        g_fill(0xfff1c9);
        g_path(); g_move(-13, -30 + hop); g_line(-17, -45 + hop); g_line(-4, -36 + hop); g_close(); g_paint(BOTH);
        g_path(); g_move(13, -30 + hop); g_line(17, -45 + hop); g_line(4, -36 + hop); g_close(); g_paint(BOTH);
        g_lin(0, -38 + hop, 0, -4 + hop, 0xa873f5, 0x6c3bbf);
        g_circle(0, -21 + hop, 17.5, BOTH);
        g_alpha(0.3); g_fill(0xffffff); g_ellipse(-7, -30 + hop, 6, 3.5, -0.5, FILL); g_alpha(1);
        angryFace(0, -24 + hop, 1);
        g_stroke(INK, 2.5); g_path();
        g_move(-1, -12 + hop); g_line(3, -9 + hop); g_line(7, -12 + hop); g_line(11, -9 + hop); g_paint(STROKE);
    } else {
        g_scale(std::cos(e.t * 1.1f) > 0 ? 1 : -1, 1);
        float flap = std::fabs(std::sin(e.t * 26));
        g_stroke(INK, 2.5);
        g_alpha(0.85); g_fill(0xe9f8ff);
        g_ellipse(-8, -13, 6, 4 + 11 * flap, -0.5, BOTH); g_ellipse(2, -14, 6, 4 + 11 * flap, 0.3, BOTH);
        g_alpha(1);
        g_fill(INK); g_path(); g_move(-13, -3); g_line(-24, 2); g_line(-12, 5); g_close(); g_paint(BOTH);  // stinger
        g_stroke(INK, 3);
        g_lin(0, -14, 0, 14, 0xffa052, 0xf2542d); g_ellipse(0, 0, 16, 14, 0, BOTH);
        g_stroke(0xb5371b, 3); g_path(); g_move(-7, -11); g_quad(-11, 0, -7, 11); g_paint(STROKE);
        angryFace(1, -2, 0.85f);
    }
    g_restore();
}

void drawGlove(float x, float y, float r) {
    g_fill(0xffffff); g_stroke(INK, 2.5); g_circle(x, y, r, BOTH);
}

void drawFist(const Game& g) {
    const Fist& f = g.fist;
    if (!f.active) return;
    float d = f.returning ? (g.p.x > f.x ? 1.f : -1.f) : (f.vx > 0 ? 1.f : -1.f);
    for (int i = 3; i >= 1; i--) {
        g_alpha(0.16 * (4 - i)); g_fill(0xffffff);
        g_circle(f.x - d * i * 11, f.y, 9 - i * 1.5f, FILL);
    }
    g_alpha(1);
    g_save(); g_translate(f.x, f.y); g_scale(f.vx > 0 ? 1 : -1, 1);
    g_fill(0xffd447); g_stroke(INK, 2.5); g_rrect(-15, -6, 9, 12, 3, BOTH);
    drawGlove(0, 0, 10);
    g_stroke(INK, 2); g_path(); g_move(3, -5); g_quad(6, -2, 3, 0); g_move(4, 0); g_quad(7, 3, 4, 5); g_paint(STROKE);
    g_restore();
}

// ------------------------------------------------------------ Emma

void drawEmma(const Game& g) {
    const Player& p = g.p;
    float t = g.time;
    const int HAIR = 0xff4f7b, HAIR_LIGHT = 0xff86a6, SKIN = 0xffdcb8, TUNIC = 0x1ec8b6, TUNIC_DARK = 0x129c94;
    const int SHOE = 0xffb21f, SCARF = 0xffd447;

    float alpha = 1;
    if (p.dead) alpha = std::max(0.f, 1 - p.deadT / 1.1f);
    else if (p.invuln > 0 && std::fmod(p.invuln, 0.16f) < 0.07f) alpha = 0.35f;
    g_save();
    g_alpha(alpha);
    g_translate(p.x, p.y);
    g_scale(p.dir, 1);
    g_scale(1 + p.squash * 0.22f, 1 - p.squash * 0.22f);

    bool air = !p.grounded, running = p.grounded && std::fabs(p.vx) > 30, win = g.state == Game::WIN;
    float ph = p.runPhase, s = std::sin(ph), c = std::cos(ph);
    float bob = running ? -std::fabs(s) * 3 : (air ? 0 : std::sin(t * 3) * 1.3f);
    float by = -31 + bob;  // body centre

    float f1x, f1y, f2x, f2y, h1x, h1y, h2x, h2y;  // 1 = far side, 2 = near side
    if (running) {
        f1x = -s * 14; f1y = -5 - std::max(0.f, -c) * 9; f2x = s * 14; f2y = -5 - std::max(0.f, c) * 9;
        h1x = -12 + s * 12; h1y = by + 4; h2x = 12 - s * 12; h2y = by + 4;
    } else if (air) {
        if (p.vy < 0) { f1x = -9; f1y = -3; f2x = 6; f2y = -11; h1x = -18; h1y = by - 10; h2x = 19; h2y = by - 15; }
        else {
            float wig = p.gliding ? std::sin(t * 16) * 3 : 0;
            f1x = -8 + wig; f1y = -2; f2x = 9 - wig; f2y = -5;
            h1x = -22; h1y = by - 5 - wig; h2x = 22; h2y = by - 7 + wig;
        }
    } else {
        f1x = -8; f1y = -5; f2x = 8; f2y = -5;
        h1x = -17; h1y = by + 5 + std::sin(t * 3) * 1.5f; h2x = 17; h2y = by + 5 + std::sin(t * 3 + 1) * 1.5f;
    }
    if (win) { h1x = -19; h1y = by - 22 + std::sin(t * 12) * 4; h2x = 19; h2y = by - 22 + std::cos(t * 12) * 4; }
    if (p.punchT > 0) { h1x = -16; h1y = by; }

    // When Emma is knocked out her parts fly apart (she is "unbound" after all).
    auto begin = [&](float ox, float oy, float kx, float ky) {
        g_save();
        if (p.dead) {
            float d = p.deadT;
            g_translate(kx * 230 * d, ky * 230 * d + 520 * d * d);
            g_translate(ox, oy); g_rotate(d * 7 * (kx >= 0 ? 1 : -1)); g_translate(-ox, -oy);
        }
    };
    auto shoe = [&](float x, float y) {
        g_fill(SHOE); g_stroke(INK, 2.8); g_ellipse(x + 3, y, 10, 6.2, 0, BOTH);
        g_fill(0xffe08a); g_ellipse(x + 6, y - 2, 3.5, 1.8, -0.2, FILL);
    };

    begin(h1x, h1y, -1.0f, -0.7f); drawGlove(h1x, h1y, 6.8); g_restore();
    begin(f1x, f1y, -0.7f, -0.2f); shoe(f1x, f1y); g_restore();

    // body: a little bell-shaped tunic with a scarf, and nothing connecting it to head, hands or feet
    begin(0, by, -0.2f, -0.5f);
    {
        float flutter = std::sin(t * 9) * 3, back = std::min(14.f, std::fabs(p.vx) * 0.04f);
        g_fill(SCARF); g_stroke(INK, 2.5);
        g_path(); g_move(-8, by - 14); g_quad(-16 - back, by - 16 + flutter, -22 - back, by - 8 + flutter);
        g_quad(-14 - back * 0.5f, by - 8, -8, by - 9); g_close(); g_paint(BOTH);
        g_path();
        g_move(-10.5, by - 12); g_quad(0, by - 17, 10.5, by - 12);
        g_cubic(13, by - 2, 17, by + 6, 16, by + 13);
        g_quad(0, by + 19, -16, by + 13);
        g_cubic(-17, by + 6, -13, by - 2, -10.5, by - 12);
        g_close();
        g_lin(0, by - 16, 0, by + 18, TUNIC, TUNIC_DARK); g_stroke(INK, 3); g_paint(BOTH);
        g_alpha(alpha * 0.35f); g_fill(0xffffff); g_ellipse(-6, by - 3, 3.5, 7, 0.25, FILL); g_alpha(alpha);
        g_fill(0xfff27a); g_stroke(INK, 1.8); starPath(3, by + 3, 6.5, 3, 0); g_paint(BOTH);
        g_fill(SCARF); g_stroke(INK, 2.5);  // scarf wrapped round the top of the tunic
        g_path(); g_move(-12, by - 13); g_quad(0, by - 20, 12, by - 13); g_line(12.5, by - 9);
        g_quad(0, by - 4, -12.5, by - 9); g_close(); g_paint(BOTH);
    }
    g_restore();

    begin(f2x, f2y, 0.7f, -0.3f); shoe(f2x, f2y); g_restore();

    // head: floats above the body with a clear gap, and lags behind when she jumps or falls
    float hx = 2, hy = -72 + bob * 1.4f + p.headOff;
    begin(hx, hy, 0.15f, -1.5f);
    g_translate(hx, hy);
    g_rotate(std::clamp(std::fabs(p.vx) * 0.0003f + (air ? p.vy * 0.00012f : 0.f), -0.15f, 0.2f));
    {
        g_stroke(INK, 2.8);
        if (p.gliding) {  // pigtails spin like helicopter blades
            float sp = t * 34;
            for (int i = 0; i < 2; i++) {
                float w = std::fabs(std::sin(sp + i * PI / 2));
                g_fill(i ? HAIR : HAIR_LIGHT);
                g_ellipse(-2, -21, 7 + 25 * w, 5.5, 0, BOTH);
            }
            g_alpha(alpha * 0.35f); g_fill(0xffffff); g_ellipse(-2, -21, 36, 7, 0, FILL); g_alpha(alpha);
        } else {
            float a = 0.55f + std::fabs(p.vx) / RUN_SPEED * 0.75f + (air ? p.vy * 0.0009f : 0.f) + std::sin(t * 4) * 0.07f;
            a = std::clamp(a, 0.1f, 2.5f);
            for (int i = 0; i < 2; i++) {
                g_save(); g_translate(i ? -12 : -8, i ? -4 : -11); g_rotate(a + i * 0.25f);
                g_fill(i ? HAIR : HAIR_LIGHT);
                g_path(); g_move(0, 0); g_cubic(10, 6, 9, 25, 0, 28); g_cubic(-9, 25, -10, 6, 0, 0); g_close(); g_paint(BOTH);
                g_restore();
            }
        }
        g_fill(HAIR); g_circle(-1.5, -1.5, 17, BOTH);
        g_fill(SKIN); g_circle(1.5, 1.5, 13.8, BOTH);
        // fringe
        g_fill(HAIR);
        g_path(); g_move(-12.5, -3);
        g_quad(-6, -18, 8, -15); g_quad(15, -12, 16, -3);
        g_quad(11, -9, 6, -5); g_quad(2, -10, -3, -5); g_quad(-7, -9, -12.5, -3);
        g_close(); g_paint(BOTH);
        g_alpha(alpha * 0.6f); g_stroke(HAIR_LIGHT, 2.5);
        g_path(); g_move(-8, -13); g_quad(-2, -17, 5, -15); g_paint(STROKE); g_alpha(alpha);
        // bobble holding the pigtails
        g_fill(SCARF); g_stroke(INK, 2); g_circle(-12, -10, 3.6, BOTH);
        // face
        bool blink = std::fmod(t, 3.3f) < 0.11f, hurt = p.invuln > 0.9f || p.dead;
        g_stroke(INK, 1.8);
        if (hurt) {
            g_stroke(INK, 2.5); g_path();
            g_move(1, -1); g_line(6, 3); g_move(6, -1); g_line(1, 3);
            g_move(9, -1); g_line(14, 3); g_move(14, -1); g_line(9, 3); g_paint(STROKE);
        } else if (blink) {
            g_stroke(INK, 2.5); g_path(); g_move(0, 1); g_line(7, 1); g_move(9, 1); g_line(15, 1); g_paint(STROKE);
        } else {
            g_fill(0xffffff); g_ellipse(4, 1, 4.3, 5.6, 0, BOTH); g_ellipse(11.5, 1, 3.6, 5.2, 0, BOTH);
            g_fill(INK); g_ellipse(5.4, 1.8, 2.3, 3.2, 0, FILL); g_ellipse(12.3, 1.8, 2, 3, 0, FILL);
            g_fill(0xffffff); g_circle(6.2, 0.4, 1, FILL); g_circle(12.9, 0.4, 0.9, FILL);
        }
        g_alpha(alpha * 0.55f); g_fill(0xff8fa8); g_circle(-1, 7.5, 3.4, FILL); g_alpha(alpha);
        g_stroke(INK, 2.2);
        if (air || win) { g_fill(0xb3304f); g_ellipse(8.5, 9.5, 3.2, 3.6, 0, BOTH); }  // "whee!"
        else { g_path(); g_move(4.5, 8.5); g_quad(8.5, 12.5, 12, 8); g_paint(STROKE); }
    }
    g_restore();

    if (!g.fist.active) { begin(h2x, h2y, 1.0f, -0.6f); drawGlove(h2x, h2y, 7.2); g_restore(); }
    g_restore();
    g_alpha(1);
}

// ------------------------------------------------------------ HUD

void drawHud(const Game& g) {
    char buf[96];
    float t = g.time;
    if (g.state != Game::TITLE) {
        drawOrb(36, 36, 0, 0);
        std::snprintf(buf, sizeof buf, "%d / %d", g.orbsTaken, int(g.orbs.size()));
        g_text(buf, 58, 46, 28, 0, 0xffffff, INK, 6);
        for (int i = 0; i < MAX_HP; i++) {
            float x = VIEW_W - 38 - i * 40, y = 34;
            bool full = i < g.p.hp;
            float s = full ? 12 + std::sin(t * 4 + i) * 0.6f : 11;
            heartPath(x, y, s);
            g_fill(full ? 0xff4d6d : 0x6a5a80); g_stroke(INK, 3.5); g_alpha(full ? 1 : 0.55); g_paint(BOTH); g_alpha(1);
            if (full) { g_fill(0xffffff); g_alpha(0.7); g_circle(x - 5, y - 5, 2.6, FILL); g_alpha(1); }
        }
    }
    if (g.state == Game::PLAY && g.playTime < 9) {
        g_alpha(std::min(1.f, 9 - g.playTime));
        g_text("Arrows / A D: move      Space: jump, hold to glide      X: punch", VIEW_W / 2, VIEW_H - 22, 19, 1, 0xffffff, INK, 5);
        g_alpha(1);
    }
    if (g.state == Game::TITLE) {
        g_save(); g_translate(VIEW_W / 2, 170 + std::sin(t * 2) * 6); g_rotate(-0.04f + std::sin(t * 1.3f) * 0.015f);
        g_text("Emma", 0, 6, 132, 1, INK, INK, 26);
        g_text("Emma", 0, 0, 132, 1, 0xff4f7b, INK, 14);
        g_text("UNBOUND", 0, 84, 76, 1, INK, INK, 20);
        g_text("UNBOUND", 0, 79, 76, 1, 0xffd447, INK, 11);
        g_restore();
        if (std::fmod(t, 1.2f) < 0.8f)
            g_text("Press SPACE or tap to start", VIEW_W / 2, 340, 28, 1, 0xffffff, INK, 6);
        g_text("Arrows / A D: move      Space: jump, hold to glide      X: punch", VIEW_W / 2, VIEW_H - 22, 19, 1, 0xffffff, INK, 5);
    }
    if (g.state == Game::WIN) {
        float k = std::min(1.f, g.stateT * 2);
        g_alpha(0.35 * k); g_fill(INK); g_rect(0, 0, VIEW_W, VIEW_H); g_alpha(1);
        g_save(); g_translate(VIEW_W / 2, 200 - (1 - k) * 60); g_rotate(std::sin(t * 2) * 0.02f);
        g_text("Level complete!", 0, 5, 78, 1, INK, INK, 20);
        g_text("Level complete!", 0, 0, 78, 1, 0xffd447, INK, 11);
        g_restore();
        int secs = int(g.playTime);
        std::snprintf(buf, sizeof buf, "Orbs %d / %d      Time %d:%02d", g.orbsTaken, int(g.orbs.size()), secs / 60, secs % 60);
        g_text(buf, VIEW_W / 2, 280, 32, 1, 0xffffff, INK, 6);
        if (g.stateT > 1.2f && std::fmod(t, 1.2f) < 0.8f)
            g_text("Press ENTER or tap to play again", VIEW_W / 2, 350, 26, 1, 0xffffff, INK, 6);
    }
}

}  // namespace

void render(const Game& g) {
    g_begin();
    float t = g.time;
    drawBackground(g);

    float shx = 0, shy = 0;
    if (g.shake > 0) { shx = std::sin(t * 90) * g.shake * 18; shy = std::cos(t * 70) * g.shake * 14; }
    g_save();
    g_translate(-g.camX + shx, -g.camY + shy);

    int tx0 = int(g.camX / TILE) - 2, tx1 = int((g.camX + VIEW_W) / TILE) + 2;
    int ty0 = std::max(0, int(g.camY / TILE) - 2), ty1 = std::min(g.H - 1, int((g.camY + VIEW_H) / TILE) + 1);
    auto visible = [&](float x) { return x > g.camX - 150 && x < g.camX + VIEW_W + 150; };

    for (auto& c : g.checkpoints) if (visible(c.x)) drawCheckpoint(c, t);
    if (visible(g.goalX)) drawGoal(g);
    drawThorns(g, tx0, tx1, ty0, ty1);
    drawTerrain(g, tx0, tx1, ty0, ty1);
    for (auto& m : g.mushrooms) if (visible(m.x)) drawMushroom(m);
    drawPlanks(g, tx0, tx1, ty0, ty1);
    for (auto& m : g.movers) if (visible(m.x)) drawMover(m, t);
    for (auto& o : g.orbs) if (!o.taken && visible(o.x)) drawOrb(o.x, o.y, t, o.phase);
    for (auto& e : g.enemies) if (e.alive && visible(e.x)) drawEnemy(e);
    drawEmma(g);
    drawFist(g);

    for (auto& q : g.particles) {
        float k = q.life / q.maxLife;
        g_alpha(std::min(1.f, k * 1.6f)); g_fill(q.color);
        if (q.kind == 1) { starPath(q.x, q.y, q.size * (0.5f + k), q.size * 0.4f * (0.5f + k), q.life * 6, 4); g_paint(FILL); }
        else g_circle(q.x, q.y, q.size * k, FILL);
    }
    g_alpha(1);
    g_restore();

    drawHud(g);
}
