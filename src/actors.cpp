// Things that move or can be touched: bouncers, platforms, lianas, pickups, enemies, the fist and Emma.
#include "draw.hpp"
#include <algorithm>
#include <cmath>

uint32_t hash2(int x, int y) {
    uint32_t h = uint32_t(x) * 374761393u + uint32_t(y) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

void starPath(float cx, float cy, float R, float r, float rot, int n) {
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

// ------------------------------------------------------------ objects

void drawBouncer(const Game& g, const Bouncer& m) {
    float k = m.bounceT > 0 ? std::sin(m.bounceT / 0.4f * PI) * 0.3f : 0;
    g_save(); g_translate(m.x, m.y); g_scale(1 + k, 1 - k);
    g_stroke(INK, 3);
    if (g.level == 3) {  // Tokyo: a springy rooftop trampoline
        g_fill(0x4a5468); g_rrect(-24, -8, 6, 12, 2, BOTH); g_rrect(18, -8, 6, 12, 2, BOTH);
        g_stroke(INK, 5); g_path(); g_move(-14, -6); g_line(-8, -26); g_line(-2, -6); g_line(4, -26); g_line(10, -6); g_line(16, -26); g_paint(STROKE);
        g_stroke(0xc9d2e0, 2); g_paint(STROKE);
        g_stroke(INK, 3); g_lin(0, -38, 0, -26, 0x5fb4ff, 0x2f6fd0); g_rrect(-28, -38, 56, 13, 6, BOTH);
        g_fill(0xbfe4ff); g_rrect(-20, -36, 40, 3.5, 1.7, FILL);
        g_restore(); return;
    }
    if (g.level == 0) {  // red forest mushroom
        g_fill(0xfff1d6); g_rrect(-10, -24, 20, 28, 7, BOTH);
        g_path(); g_move(-28, -18); g_cubic(-28, -50, 28, -50, 28, -18); g_quad(0, -8, -28, -18); g_close();
        g_lin(0, -44, 0, -12, 0xff6b81, 0xe8334f); g_paint(BOTH);
        g_fill(0xffffff);
        g_circle(-13, -28, 5, FILL); g_circle(4, -34, 6, FILL); g_circle(17, -24, 4, FILL);
    } else {  // conga drum
        g_path(); g_move(-21, -37); g_line(21, -37); g_quad(27, -18, 17, 2); g_line(-17, 2); g_quad(-27, -18, -21, -37); g_close();
        g_lin(-24, 0, 24, 0, 0xc0783e, 0x8a4a26); g_paint(BOTH);
        g_stroke(0xdcc28c, 4);
        g_path(); g_move(-23, -25); g_quad(0, -20, 23, -25); g_move(-21, -8); g_quad(0, -3, 21, -8); g_paint(STROKE);
        g_stroke(INK, 1.6);
        g_path(); g_move(-23, -27.5); g_quad(0, -22.5, 23, -27.5); g_move(-22, -22.5); g_quad(0, -17.5, 22, -22.5);
        g_move(-21, -10.5); g_quad(0, -5.5, 21, -10.5); g_move(-20, -5.5); g_quad(0, -0.5, 20, -5.5); g_paint(STROKE);
        g_stroke(INK, 3); g_fill(0xfff1d6); g_ellipse(0, -38, 22, 6.5, 0, BOTH);
        g_fill(0xf0d9b0); g_ellipse(0, -38, 11, 3, 0, FILL);
    }
    g_restore();
}

// Heidestein: a fallen pine drifting on the lake. You ride on the trunk.
static void drawFloatingPine(const Mover& m, float t) {
    float hw = m.w / 2, bob = std::sin(t * 2) * 0.8f;
    g_save(); g_translate(m.x, m.y);
    // broken branches and the last green tuft at the crown end
    auto stub = [](float x0, float y0, float x1, float y1, float w) {
        g_path(); g_move(x0, y0); g_line(x1, y1);
        g_stroke(INK, w + 4.5f); g_paint(STROKE); g_stroke(0x9a7458, w); g_paint(STROKE);
    };
    stub(hw - 30, 6, hw - 8, -22 + bob, 4.5f); stub(hw - 16, -10, hw + 8, -18 + bob, 3);
    stub(-hw + 52, 6, -hw + 40, -16, 4); stub(8, 6, 18, -12, 3.5f);
    static const float tuft[][3] = {{-10, -30, 13}, {6, -40, 12}, {20, -28, 12}};
    g_fill(INK);
    for (auto& c : tuft) g_circle(hw + c[0], c[1] + bob, c[2] + 3, FILL);
    for (auto& c : tuft) { g_fill(0x3d8f68); g_circle(hw + c[0], c[1] + bob, c[2], FILL); }
    g_fill(0x5fb486); g_circle(hw + 2, -43 + bob, 5, FILL); g_circle(hw - 13, -33 + bob, 5, FILL);
    // the root plate at the other end
    g_stroke(INK, 3); g_fill(0x8d7a66);
    g_path(); g_move(-hw - 4, 4); g_line(-hw - 26, -22); g_line(-hw - 14, -8); g_line(-hw - 30, 2); g_line(-hw - 14, 10); g_line(-hw - 28, 26);
    g_line(-hw - 10, 22); g_line(-hw - 16, 38); g_line(-hw + 6, 30); g_close(); g_paint(BOTH);
    // trunk
    g_lin(0, 0, 0, 36, 0xc99b72, 0x8f6a4e); g_stroke(INK, 3.5);
    g_path(); g_move(-hw - 6, 5); g_quad(-hw - 6, -1, -hw + 6, -1); g_line(hw - 4, 0); g_quad(hw + 6, 4, hw + 6, 14);
    g_quad(hw + 4, 28, hw - 8, 30); g_line(-hw + 4, 36); g_quad(-hw - 8, 34, -hw - 6, 5); g_close(); g_paint(BOTH);
    g_stroke(0xe3bd94, 2.5); g_path(); g_move(-hw + 8, 4.5f); g_line(hw - 10, 5); g_paint(STROKE);
    g_stroke(0x6f5340, 2);
    for (float x = -hw + 16; x < hw - 14; x += 23) { g_path(); g_move(x, 13 + std::fmod(x + 200, 7.f)); g_line(x + 11, 13 + std::fmod(x + 200, 7.f)); g_move(x + 8, 22); g_line(x + 15, 22); g_paint(STROKE); }
    g_restore();
}

// Vinales: a plank platform slung on ropes from a bamboo frame. Lifts run between two masts; the ferry
// hangs from a cable strung between them.
static void drawRopePlatform(const Mover& m, int index) {
    bool lift = std::fabs(m.ax - m.bx) < 1;
    float hw = m.w / 2;
    auto bamboo = [](float x0, float y0, float x1, float y1) {
        g_path(); g_move(x0, y0); g_line(x1, y1);
        g_stroke(INK, 11); g_paint(STROKE); g_stroke(0xa99a62, 6.5); g_paint(STROKE); g_stroke(0xcbbd86, 1.6); g_paint(STROKE);
        g_stroke(0x6f6238, 2);
        float len = std::hypot(x1 - x0, y1 - y0);
        for (float d = 30; d < len - 10; d += 44) {
            float k = d / len, px = x0 + (x1 - x0) * k, py = y0 + (y1 - y0) * k;
            g_path();
            if (std::fabs(x1 - x0) < std::fabs(y1 - y0)) { g_move(px - 3.2f, py); g_line(px + 3.2f, py); } else { g_move(px, py - 3.2f); g_line(px, py + 3.2f); }
            g_paint(STROKE);
        }
    };
    auto rope = [](float x0, float y0, float x1, float y1) {
        g_path(); g_move(x0, y0); g_line(x1, y1);
        g_stroke(INK, 4.5); g_paint(STROKE); g_stroke(0xcdb98a, 2); g_paint(STROKE);
    };
    float topY = std::min(m.ay, m.by) - 118;
    if (lift) {
        float l = m.ax - hw - 20, r = m.ax + hw + 20, bottom = std::max(m.ay, m.by) + 620;
        bamboo(l, topY - 14, l, bottom); bamboo(r, topY - 14, r, bottom);
        bamboo(l - 14, topY, r + 14, topY);
        rope(m.x - hw + 12, topY + 4, m.x - hw + 12, m.y + 4); rope(m.x + hw - 12, topY + 4, m.x + hw - 12, m.y + 4);
    } else {
        float l = std::min(m.ax, m.bx) - hw - 26, r = std::max(m.ax, m.bx) + hw + 26, bottom = m.ay + 620;
        bamboo(l, topY - 14, l, bottom); bamboo(r, topY - 14, r, bottom);
        rope(l, topY, r, topY);
        rope(m.x, topY + 4, m.x - hw + 12, m.y + 4); rope(m.x, topY + 4, m.x + hw - 12, m.y + 4);
        g_fill(0x6e625a); g_stroke(INK, 2.5); g_circle(m.x, topY, 6.5, BOTH);
    }
    // the deck: weathered planks on two cross beams
    g_save(); g_translate(m.x, m.y);
    g_stroke(INK, 3); g_fill(0x6b5744);
    g_rrect(-hw + 14, 9, 12, 13, 3, BOTH); g_rrect(hw - 26, 9, 12, 13, 3, BOTH);
    g_lin(0, 0, 0, 14, 0xb09a7c, 0x86705a); g_stroke(INK, 3.5); g_rrect(-hw, 0, m.w, 14, 4, BOTH);
    g_stroke(0xd3c2a0, 2.2); g_path(); g_move(-hw + 7, 4); g_line(hw - 7, 4); g_paint(STROKE);
    g_stroke(0x5c4c3a, 2);
    for (float x = -hw + 20 + (index % 3) * 3; x < hw - 10; x += 21) { g_path(); g_move(x, 2.5); g_line(x, 11.5); g_paint(STROKE); }
    g_restore();
}

// A small fishing boat. x,y is the middle of the deck (the top edge of the hull).
void drawFishingBoat(float x, float y, float scale, bool cabin) {
    const float hw = 75;
    g_save(); g_translate(x, y); g_scale(scale, scale);
    g_stroke(INK, 3.5);
    // mast and flag
    g_fill(0xb98a5a); g_rrect(-hw + 54, -92, 6, 94, 2, BOTH);
    g_fill(0xd9483b); g_stroke(INK, 2.5); g_rrect(-hw + 60, -92, 30, 20, 2, BOTH);
    g_fill(0xffffff); g_rect(-hw + 68, -91, 7, 18); g_rect(-hw + 61, -85.5, 28, 7);
    g_fill(0x2f4f9f); g_rect(-hw + 70, -91, 3, 18); g_rect(-hw + 61, -83.5, 28, 3);
    g_stroke(INK, 3.5);
    if (cabin) {  // wheelhouse at the stern
        g_fill(0xfdf8ee); g_rrect(-hw + 4, -36, 42, 38, 4, BOTH);
        g_fill(0x8fd6f0); g_stroke(INK, 2.5); g_rrect(-hw + 12, -28, 16, 13, 3, BOTH);
        g_stroke(INK, 3.5); g_fill(0x3f6fae); g_rrect(-hw, -43, 50, 10, 4, BOTH);
        g_fill(0xffffff); g_rrect(-hw + 4, -46, 42, 5, 2.5, FILL);
    }
    // hull
    g_lin(0, 0, 0, 84, 0xe2503f, 0xb5332a);
    g_path(); g_move(-hw - 10, 0); g_line(hw + 16, 0); g_quad(hw + 6, 48, hw - 28, 84); g_line(-hw + 14, 84); g_quad(-hw - 6, 48, -hw - 10, 0); g_close(); g_paint(BOTH);
    g_fill(0xfdf8ee); g_stroke(INK, 2.5);
    g_path(); g_move(-hw - 9, 1); g_line(hw + 15, 1); g_line(hw + 11, 16); g_line(-hw - 7, 16); g_close(); g_paint(BOTH);
    g_fill(0xf2c94c); g_circle(hw - 12, 30, 5, BOTH);
    g_text("EMMA", 4, 42, 15, 1, 0xfdf8ee, INK, 0);
    g_restore();
}

// Japan: a flat wooden boat.
static void drawWoodenBoat(const Mover& m, float t) {
    float hw = m.w / 2, bob = std::sin(t * 1.8f) * 1.2f;
    g_save(); g_translate(m.x, m.y + bob * 0.3f);
    g_stroke(INK, 3.5);
    g_lin(0, 0, 0, 44, 0xb98a5a, 0x7a5638);
    g_path(); g_move(-hw - 16, -8); g_quad(-hw - 2, 2, -hw + 8, 2); g_line(hw - 8, 2); g_quad(hw + 2, 2, hw + 18, -10);
    g_quad(hw + 6, 34, hw - 22, 44); g_line(-hw + 22, 44); g_quad(-hw - 6, 34, -hw - 16, -8); g_close(); g_paint(BOTH);
    g_lin(0, 0, 0, 12, 0xe6c49a, 0xc29a6c); g_rrect(-hw, 0, m.w, 11, 4, BOTH);
    g_stroke(0x8a6440, 2);
    for (float x = -hw + 18; x < hw - 8; x += 22) { g_path(); g_move(x, 2.5); g_line(x, 8.5); g_paint(STROKE); }
    g_stroke(0x5a4028, 2); g_path(); g_move(-hw + 6, 22); g_line(hw - 6, 22); g_paint(STROKE);
    g_restore();
}

void drawMover(const Game& g, const Mover& m, int index) {
    if (g.level == 0) drawFloatingPine(m, g.time);
    else if (g.level == 1) drawRopePlatform(m, index);
    else if (g.level == 3) drawWoodenBoat(m, g.time);
    else drawFishingBoat(m.x, m.y, 1, true);
}

void drawLiana(const Game& g, const Liana& l) {
    float t = g.time;
    if (g.level == 3) {  // Tokyo: a strand of lights to climb
        g_path(); g_move(l.x, l.y0 - 30);
        for (float y = l.y0 - 14; y < l.y1; y += 16) g_line(l.x + lianaSway(l, y, t), y);
        g_stroke(INK, 6); g_paint(STROKE); g_stroke(0x9fd0ff, 2.5); g_paint(STROKE);
        int i = 0;
        for (float y = l.y0; y < l.y1; y += 20, i++) {
            float x = l.x + lianaSway(l, y, t), b = 0.5f + 0.5f * std::sin(t * 4 - i * 0.7f);
            g_rad(x, y, 16, 0x9fe0ff, 0.5 * b, 0x9fe0ff, 0); g_circle(x, y, 16, FILL);
            g_fill(b > 0.6f ? 0xffffff : 0x7fc0ff); g_stroke(INK, 2); g_circle(x, y, 4.5, BOTH);
        }
        return;
    }
    auto trace = [&]() {
        g_path(); g_move(l.x, l.y0 - 30);
        for (float y = l.y0 - 14; y < l.y1; y += 16) g_line(l.x + lianaSway(l, y, t), y);
        g_line(l.x + lianaSway(l, l.y1, t), l.y1);
    };
    g_stroke(INK, 9); trace(); g_paint(STROKE);
    g_stroke(0x4fae3c, 5); g_paint(STROKE);
    g_stroke(0x9be86a, 1.6); g_paint(STROKE);
    int i = 0;
    for (float y = l.y0 + 8; y < l.y1 - 10; y += 38, i++) {
        float x = l.x + lianaSway(l, y, t), side = i % 2 ? 1.f : -1.f;
        g_save(); g_translate(x, y); g_rotate(side * (0.9f + std::sin(t * 2 + i) * 0.12f));
        g_fill(i % 3 ? 0x6fdc55 : 0x43c463); g_stroke(INK, 2.2);
        g_path(); g_move(0, 0); g_quad(8, -9, 0, -22); g_quad(-8, -9, 0, 0); g_close(); g_paint(BOTH);
        g_restore();
    }
    float ex = l.x + lianaSway(l, l.y1, t);
    g_fill(0x6fdc55); g_stroke(INK, 2.5); g_circle(ex, l.y1 + 3, 5, BOTH);
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

static void angryFace(float ex, float ey, float s) {
    g_fill(0xffffff); g_stroke(INK, 2);
    g_ellipse(ex, ey, 5 * s, 6 * s, 0, BOTH); g_ellipse(ex + 10 * s, ey, 4.5 * s, 5.5 * s, 0, BOTH);
    g_fill(INK); g_circle(ex + 1.8 * s, ey + 1, 2.3 * s, FILL); g_circle(ex + 11.5 * s, ey + 1, 2.1 * s, FILL);
    g_stroke(INK, 3.5 * s);
    g_path(); g_move(ex - 6 * s, ey - 9 * s); g_line(ex + 4 * s, ey - 4.5 * s);
    g_move(ex + 16 * s, ey - 8.5 * s); g_line(ex + 7.5 * s, ey - 4.5 * s); g_paint(STROKE);
}


static void drawWolf(const Enemy& e) {
    float w = std::sin(e.t * 9), bobY = -std::fabs(w) * 1.5f;
    const int FUR = 0x9aa0b4, DARK = 0x6d7389, LIGHT = 0xe4e7f0;
    g_translate(-4, 0);
    g_stroke(INK, 3);
    // tail
    g_fill(FUR);
    g_path(); g_move(-16, -25 + bobY); g_quad(-38, -38 + w * 3, -37, -17 + w * 2); g_quad(-28, -11, -15, -17 + bobY); g_close(); g_paint(BOTH);
    // far legs
    g_fill(DARK);
    g_rrect(-14 + w * 5, -13, 7, 15, 3, BOTH); g_rrect(8 - w * 5, -13, 7, 15, 3, BOTH);
    // body
    g_lin(0, -36, 0, -10, 0xb4bacb, 0x7f869b); g_ellipse(0, -23 + bobY, 20, 12.5, 0, BOTH);
    g_fill(LIGHT); g_ellipse(3, -17 + bobY, 12, 4.5, 0, FILL);
    // near legs
    g_fill(FUR);
    g_rrect(-9 - w * 5, -13, 7, 15, 3, BOTH); g_rrect(13 + w * 5, -13, 7, 15, 3, BOTH);
    // head
    float hy = bobY * 1.5f;
    g_fill(FUR);
    g_path(); g_move(9, -42 + hy); g_line(11, -57 + hy); g_line(19, -45 + hy); g_close(); g_paint(BOTH);
    g_path(); g_move(18, -44 + hy); g_line(24, -57 + hy); g_line(28, -42 + hy); g_close(); g_paint(BOTH);
    g_circle(19, -35 + hy, 11.5, BOTH);
    g_fill(LIGHT);
    g_path(); g_move(24, -40 + hy); g_line(41, -35 + hy); g_quad(43, -29 + hy, 37, -27 + hy); g_line(21, -25 + hy); g_close(); g_paint(BOTH);
    g_fill(INK); g_circle(40.5, -33.5 + hy, 2.8, FILL);
    g_fill(0xffffff); g_stroke(INK, 1.6);
    g_path(); g_move(29, -27 + hy); g_line(31, -22.5 + hy); g_line(33, -27 + hy); g_close(); g_paint(BOTH);
    g_fill(0xffe45c); g_stroke(INK, 1.8); g_ellipse(21, -37 + hy, 3.4, 3.8, 0, BOTH);
    g_fill(INK); g_circle(22, -36.5 + hy, 1.6, FILL);
    g_stroke(INK, 3); g_path(); g_move(15, -43 + hy); g_line(25, -39.5 + hy); g_paint(STROKE);
}

static void drawCrab(const Enemy& e) {
    float w = std::sin(e.t * 12), look = e.vx > 0 ? 1.4f : -1.4f;
    g_stroke(INK, 3);
    // legs
    for (int side = -1; side <= 1; side += 2)
        for (int i = 0; i < 3; i++) {
            float k = (i % 2 ? w : -w) * 2.5f;
            g_stroke(INK, 6); g_path(); g_move(side * (10 + i * 3), -10); g_quad(side * (20 + i * 3), -12, side * (19 + i * 4) + k, -1); g_paint(STROKE);
            g_stroke(0xf0503c, 2.6); g_paint(STROKE);
        }
    g_stroke(INK, 3);
    // claws
    for (int side = -1; side <= 1; side += 2) {
        float cy = -33 + (side > 0 ? w : -w) * 2.5f;
        g_stroke(INK, 7); g_path(); g_move(side * 15, -18); g_line(side * 24, cy + 6); g_paint(STROKE);
        g_stroke(0xf0503c, 3); g_paint(STROKE);
        g_stroke(INK, 3); g_lin(0, cy - 8, 0, cy + 8, 0xff8a6a, 0xe0402e); g_circle(side * 26, cy, 8.5, BOTH);
        g_stroke(INK, 2.5); g_path(); g_move(side * 26, cy + 1); g_line(side * 30, cy - 8); g_paint(STROKE);
    }
    // shell
    g_stroke(INK, 3);
    g_lin(0, -28, 0, -4, 0xff8a6a, 0xd93a2b); g_ellipse(0, -15 + std::fabs(w), 19, 12.5, 0, BOTH);
    g_alpha(0.35); g_fill(0xffffff); g_ellipse(-8, -21, 6, 3, -0.4, FILL); g_alpha(1);
    // eyes on stalks
    for (int side = -1; side <= 1; side += 2) {
        g_stroke(INK, 6); g_path(); g_move(side * 6, -25); g_line(side * 7, -33); g_paint(STROKE);
        g_stroke(0xf0503c, 2.6); g_paint(STROKE);
        g_stroke(INK, 2); g_fill(0xffffff); g_circle(side * 7, -36, 5.2, BOTH);
        g_fill(INK); g_circle(side * 7 + look, -35.5, 2.2, FILL);
    }
    g_stroke(INK, 3); g_path(); g_move(-13, -43); g_line(-3, -39); g_move(13, -43); g_line(3, -39); g_paint(STROKE);
    g_stroke(INK, 2.2); g_path(); g_move(-5, -10); g_quad(0, -13, 5, -10); g_paint(STROKE);
}

static void drawPolarBear(const Enemy& e) {
    bool angry = e.hp < 2;
    float w = std::sin(e.t * (angry ? 12 : 7)), bobY = -std::fabs(w) * 1.8f;
    const int FUR = 0xf7fafd, SHADE = 0xd3deeb;
    g_translate(-9, 0);
    g_stroke(INK, 3);
    g_fill(FUR); g_circle(-31, -32 + bobY, 6, BOTH);  // tail
    g_fill(SHADE);
    g_rrect(-23 + w * 6, -17, 12, 19, 5, BOTH); g_rrect(11 - w * 6, -17, 12, 19, 5, BOTH);
    g_lin(0, -48, 0, -10, FUR, SHADE); g_ellipse(-1, -29 + bobY, 31, 19, 0, BOTH);
    g_fill(FUR);
    g_rrect(-15 - w * 6, -17, 12, 19, 5, BOTH); g_rrect(19 + w * 6, -17, 12, 19, 5, BOTH);
    // head
    float hy = bobY * 1.4f;
    g_fill(FUR); g_circle(20, -51 + hy, 6, BOTH); g_circle(35, -52 + hy, 6, BOTH);
    g_fill(0xffc2cf); g_circle(20, -51 + hy, 2.6, FILL); g_circle(35, -52 + hy, 2.6, FILL);
    g_fill(FUR); g_circle(29, -38 + hy, 15, BOTH);
    g_ellipse(41, -33 + hy, 10, 7.5, 0, BOTH);
    g_fill(INK); g_ellipse(49, -35.5f + hy, 3.6, 3, 0, FILL);
    g_circle(32, -41 + hy, 2.4, FILL);
    if (angry) {  // one knock in: furious
        g_alpha(0.55); g_fill(0xff6b6b); g_circle(26, -33 + hy, 5, FILL); g_alpha(1);
        g_stroke(INK, 3.2); g_path(); g_move(26, -48 + hy); g_line(37, -43.5f + hy); g_paint(STROKE);
        g_fill(0xb3304f); g_stroke(INK, 2); g_path(); g_move(36, -31 + hy); g_quad(43, -22 + hy, 49, -30 + hy); g_close(); g_paint(BOTH);
        g_fill(0xffffff); g_path(); g_move(39, -30 + hy); g_line(41, -26 + hy); g_line(43, -30 + hy); g_close(); g_paint(FILL);
    } else {
        g_stroke(INK, 2.6); g_path(); g_move(27, -46.5f + hy); g_line(36, -44 + hy); g_paint(STROKE);
        g_stroke(INK, 2); g_path(); g_move(38, -29 + hy); g_quad(43, -26 + hy, 48, -30 + hy); g_paint(STROKE);
    }
}

static void drawGull(const Enemy& e) {
    float f = std::sin(e.t * 13);
    g_stroke(INK, 2.5);
    g_fill(0xf7fafd); g_path(); g_move(-13, 0); g_line(-25, -5); g_line(-23, 5); g_close(); g_paint(BOTH);  // tail
    g_lin(0, -10, 0, 10, 0xffffff, 0xd3deeb); g_ellipse(0, 0, 16, 10.5, 0, BOTH);
    g_fill(0xf2a81d); g_path(); g_move(17, -8); g_line(29, -4); g_line(17, -1); g_close(); g_paint(BOTH);
    g_fill(0xffffff); g_circle(12, -6, 7.5, BOTH);
    g_fill(INK); g_circle(14.5f, -7, 1.9f, FILL);
    g_stroke(INK, 2.6); g_path(); g_move(9, -12); g_line(17, -9.5f); g_paint(STROKE);
    // wing
    g_stroke(INK, 2.5); g_fill(0xb9c4d6);
    g_path(); g_move(-10, -2); g_quad(-8, -8 - 24 * f, 6, -6 - 26 * f); g_quad(4, -4, 7, -1); g_close(); g_paint(BOTH);
    g_fill(INK); g_circle(5, -6 - 24 * f, 2.6, FILL);
}

static void drawBoar(const Enemy& e) {
    float w = std::sin(e.t * 10), bobY = -std::fabs(w) * 1.5f;
    const int FUR = 0x8a6a4a, DARK = 0x5f4632;
    g_translate(-3, 0);
    g_stroke(INK, 3);
    g_fill(DARK); g_rrect(-15 + w * 5, -12, 7, 14, 3, BOTH); g_rrect(8 - w * 5, -12, 7, 14, 3, BOTH);
    g_path(); g_move(-21, -24 + bobY); g_quad(-30, -30 + bobY, -27, -20 + bobY); g_stroke(INK, 5); g_paint(STROKE); g_stroke(DARK, 2); g_paint(STROKE);
    g_stroke(INK, 3);
    g_lin(0, -38, 0, -8, 0xa07c58, 0x6f5238); g_ellipse(-1, -22 + bobY, 22, 14.5, 0, BOTH);
    g_fill(DARK);                                                       // bristles along the back
    g_path(); g_move(-16, -33 + bobY);
    for (int i = 0; i < 6; i++) { g_line(-12 + i * 6, -41 + bobY - (i % 2) * 2); g_line(-9 + i * 6, -35 + bobY); }
    g_close(); g_paint(BOTH);
    g_fill(FUR); g_rrect(-10 - w * 5, -12, 7, 14, 3, BOTH); g_rrect(13 + w * 5, -12, 7, 14, 3, BOTH);
    float hy = bobY * 1.3f;
    g_fill(FUR); g_path(); g_move(12, -36 + hy); g_line(16, -46 + hy); g_line(21, -37 + hy); g_close(); g_paint(BOTH);   // ear
    g_circle(19, -26 + hy, 11.5, BOTH);
    g_fill(0xc9a184); g_ellipse(30, -22 + hy, 7.5, 6, 0, BOTH);         // snout
    g_fill(INK); g_circle(33, -23 + hy, 1.4, FILL); g_circle(33, -20 + hy, 1.4, FILL);
    g_fill(0xfffaf0); g_stroke(INK, 1.6); g_path(); g_move(24, -18 + hy); g_quad(27, -22 + hy, 28, -27 + hy); g_line(29, -18 + hy); g_close(); g_paint(BOTH);  // tusk
    g_fill(INK); g_circle(21, -29 + hy, 2, FILL);
    g_stroke(INK, 2.6); g_path(); g_move(16, -34 + hy); g_line(25, -31 + hy); g_paint(STROKE);
}

static void drawCrow(const Enemy& e) {
    float f = std::sin(e.t * 13);
    g_stroke(INK, 2.5);
    g_fill(0x3a3a4a); g_path(); g_move(-13, 0); g_line(-27, -4); g_line(-25, 6); g_close(); g_paint(BOTH);
    g_lin(0, -10, 0, 10, 0x55556a, 0x2f2f3d); g_ellipse(0, 0, 16, 10.5, 0, BOTH);
    g_fill(0x4a4a5c); g_circle(12, -6, 7.5, BOTH);
    g_fill(0x2a2a34); g_path(); g_move(17, -9); g_line(30, -4); g_line(17, -1); g_close(); g_paint(BOTH);
    g_fill(0xffffff); g_circle(14, -7, 2.4, FILL); g_fill(INK); g_circle(14.6f, -7, 1.3f, FILL);
    g_fill(0x2f2f3d);
    g_path(); g_move(-10, -2); g_quad(-8, -8 - 24 * f, 6, -6 - 26 * f); g_quad(4, -4, 7, -1); g_close(); g_paint(BOTH);
}

void drawEnemy(const Game& g, const Enemy& e) {
    if (e.hitT > 0 && std::fmod(e.hitT, 0.14f) < 0.07f) g_alpha(0.45);
    g_save(); g_translate(e.x, e.y);
    if (e.type == 0) {
        if (g.level == 0) { g_scale(e.vx > 0 ? 1 : -1, 1); drawWolf(e); }
        else if (g.level == 1) drawCrab(e);
        else if (g.level == 3) { g_scale(e.vx > 0 ? 1 : -1, 1); drawBoar(e); }
        else { g_scale(e.vx > 0 ? 1 : -1, 1); if (e.hitT > 0.35f) g_rotate(-0.12f); drawPolarBear(e); }
    } else if (g.level >= 2) {
        g_scale(std::cos(e.t * 1.1f) > 0 ? 1 : -1, 1);
        if (g.level == 2) drawGull(e); else drawCrow(e);
    } else {  // a wasp in Heidestein, an emerald beetle in Vinales
        bool wasp = g.level == 0;
        g_scale(std::cos(e.t * 1.1f) > 0 ? 1 : -1, 1);
        float flap = std::fabs(std::sin(e.t * 26));
        g_stroke(INK, 2.5);
        g_alpha(0.85); g_fill(0xe9f8ff);
        g_ellipse(-8, -13, 6, 4 + 11 * flap, -0.5, BOTH); g_ellipse(2, -14, 6, 4 + 11 * flap, 0.3, BOTH);
        g_alpha(1);
        g_fill(INK); g_path(); g_move(-13, -3); g_line(-24, 2); g_line(-12, 5); g_close(); g_paint(BOTH);  // stinger
        g_stroke(INK, 3);
        if (wasp) g_lin(0, -14, 0, 14, 0xffe05a, 0xf2a81d); else g_lin(0, -14, 0, 14, 0x5fe8b8, 0x1fa37c);
        g_ellipse(0, 0, 16, 14, 0, BOTH);
        g_stroke(wasp ? 0x3d2a1a : 0x0d6b55, wasp ? 4 : 3);
        g_path(); g_move(-7, -11); g_quad(-11, 0, -7, 11);
        if (wasp) { g_move(-12.5, -6); g_quad(-15, 0, -12.5, 6); }
        g_paint(STROKE);
        angryFace(1, -2, 0.85f);
    }
    g_restore();
    g_alpha(1);
}

static void drawGlove(float x, float y, float r) {
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
    const int HAIR = 0xb98a5a, HAIR_LIGHT = 0xd6ae80, SKIN = 0xffdcb8, TUNIC = 0x1ec8b6, TUNIC_DARK = 0x129c94;
    const int SHOE = 0xffb21f, SCARF = 0xffd447;

    float alpha = 1;
    if (p.dead) alpha = std::max(0.f, 1 - p.deadT / 1.1f);
    else if (p.invuln > 0 && std::fmod(p.invuln, 0.16f) < 0.07f) alpha = 0.35f;
    g_save();
    g_alpha(alpha);
    bool climb = p.onLiana >= 0;
    float swayX = climb ? lianaSway(g.lianas[p.onLiana], p.y - 90, t) : 0;
    g_translate(p.x + swayX, p.y);
    g_scale(p.dir, 1);
    g_scale(1 + p.squash * 0.22f, 1 - p.squash * 0.22f);

    bool air = !p.grounded && p.onLiana < 0, running = p.grounded && std::fabs(p.vx) > 30, win = g.state == Game::WIN;
    float ph = p.runPhase, s = std::sin(ph), c = std::cos(ph);
    float bob = running ? -std::fabs(s) * 3 : (air || p.onLiana >= 0 ? 0 : std::sin(t * 3) * 1.3f);
    float by = -35 + bob;  // body centre

    float f1x, f1y, f2x, f2y, h1x, h1y, h2x, h2y;  // 1 = far side, 2 = near side
    if (climb) {  // hands reach up and grip the liana above her head
        float a = std::sin(p.climbPhase) * 5;
        f1x = -6; f1y = -5 + a; f2x = 5; f2y = -9 - a;
        h1x = 0; h1y = -108 + a; h2x = 1; h2y = -99 - a;
    } else if (running) {
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
    if (p.punchT > 0 && !climb) { h1x = -16; h1y = by; }

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

    // body: a fitted dress with a waist and a scarf, and nothing connecting it to head, hands or feet
    begin(0, by, -0.2f, -0.5f);
    {
        float flutter = std::sin(t * 9) * 3, back = std::min(14.f, std::fabs(p.vx) * 0.04f);
        g_fill(SCARF); g_stroke(INK, 2.5);
        g_path(); g_move(-7, by - 14); g_quad(-15 - back, by - 16 + flutter, -21 - back, by - 8 + flutter);
        g_quad(-13 - back * 0.5f, by - 8, -7, by - 9); g_close(); g_paint(BOTH);
        g_path();
        g_move(-9.5, by - 13); g_quad(0, by - 17, 9.5, by - 13);
        g_cubic(11.5, by - 6, 7, by - 2, 7.5, by + 2);
        g_cubic(9, by + 8, 15.5, by + 13, 15, by + 21);
        g_quad(0, by + 26, -15, by + 21);
        g_cubic(-15.5, by + 13, -9, by + 8, -7.5, by + 2);
        g_cubic(-7, by - 2, -11.5, by - 6, -9.5, by - 13);
        g_close();
        g_lin(0, by - 16, 0, by + 24, TUNIC, TUNIC_DARK); g_stroke(INK, 3); g_paint(BOTH);
        g_alpha(alpha * 0.3f); g_fill(0xffffff); g_ellipse(-5, by - 5, 2.6, 6, 0.2, FILL); g_alpha(alpha);
        g_path(); g_move(-7.5, by + 2); g_quad(0, by + 4.5, 7.5, by + 2);          // belt
        g_stroke(INK, 5); g_paint(STROKE); g_stroke(SCARF, 2.4); g_paint(STROKE);
        g_stroke(TUNIC_DARK, 1.6); g_path(); g_move(-5, by + 10); g_quad(-7, by + 16, -8, by + 21); g_move(5, by + 10); g_quad(7, by + 16, 8, by + 21); g_paint(STROKE);
        g_fill(SCARF); g_stroke(INK, 2.5);  // scarf wrapped round the shoulders
        g_path(); g_move(-11, by - 13); g_quad(0, by - 20, 11, by - 13); g_line(11.5, by - 9);
        g_quad(0, by - 4, -11.5, by - 9); g_close(); g_paint(BOTH);
    }
    g_restore();

    begin(f2x, f2y, 0.7f, -0.3f); shoe(f2x, f2y); g_restore();

    // head: floats above the body with a clear gap, and lags behind when she jumps or falls
    float hx = 2, hy = -82 + bob * 1.4f + p.headOff;
    begin(hx, hy, 0.15f, -1.5f);
    g_translate(hx, hy);
    g_rotate(std::clamp(std::fabs(p.vx) * 0.0003f + (air ? p.vy * 0.00012f : 0.f), -0.15f, 0.2f));
    {
        g_stroke(INK, 2.8);
        if (p.gliding) {  // her ponytail spins like a helicopter blade
            float sp = t * 34;
            for (int i = 0; i < 2; i++) {
                float w = std::fabs(std::sin(sp + i * PI / 2));
                g_fill(i ? HAIR : HAIR_LIGHT);
                g_ellipse(-2, -21, 7 + 25 * w, 5.5, 0, BOTH);
            }
            g_alpha(alpha * 0.35f); g_fill(0xffffff); g_ellipse(-2, -21, 36, 7, 0, FILL); g_alpha(alpha);
        } else {  // one long ponytail
            float a = 0.45f + std::fabs(p.vx) / RUN_SPEED * 0.8f + (air ? p.vy * 0.0009f : 0.f) + std::sin(t * 4) * 0.06f;
            a = std::clamp(a, 0.1f, 2.5f);
            g_save(); g_translate(-12, -9); g_rotate(a);
            g_fill(HAIR);
            g_path(); g_move(0, -2); g_cubic(10, 6, 11, 28, 5, 40); g_quad(2, 47, -3, 49); g_cubic(-2, 36, -11, 22, -7, 4); g_quad(-5, -2, 0, -2); g_close(); g_paint(BOTH);
            g_stroke(HAIR_LIGHT, 2.2); g_path(); g_move(1, 6); g_quad(5, 22, 1, 38); g_paint(STROKE);
            g_restore();
            g_stroke(INK, 2.8);
        }
        g_fill(HAIR); g_ellipse(-1.5, -2, 15.5, 16.5, 0, BOTH);
        g_fill(SKIN); g_ellipse(1.8, 1.5, 12, 13.6, 0, BOTH);
        // side-swept fringe
        g_fill(HAIR);
        g_path(); g_move(-11.5, -1);
        g_quad(-8, -19, 6, -16.5f); g_quad(14.5f, -14, 14.5f, -4);
        g_quad(9, -11, 0, -9.5f); g_quad(-7, -8, -11.5, -1);
        g_close(); g_paint(BOTH);
        g_alpha(alpha * 0.6f); g_stroke(HAIR_LIGHT, 2.2);
        g_path(); g_move(-7, -13.5f); g_quad(-1, -17, 6, -14.5f); g_paint(STROKE); g_alpha(alpha);
        g_fill(SCARF); g_stroke(INK, 2); g_circle(-12.5f, -9, 3.2, BOTH);   // hair tie
        // face: almond eyes with lashes, fine brows, lips
        bool blink = std::fmod(t, 3.3f) < 0.11f, hurt = p.invuln > 0.9f || p.dead;
        if (hurt) {
            g_stroke(INK, 2.2); g_path();
            g_move(2, -1); g_line(7, 3); g_move(7, -1); g_line(2, 3);
            g_move(9.5, -1); g_line(14, 3); g_move(14, -1); g_line(9.5, 3); g_paint(STROKE);
        } else if (blink) {
            g_stroke(INK, 2.2); g_path(); g_move(1.5, 1.5); g_quad(4.5, 3, 7.8, 1.5); g_move(9.3, 1.5); g_quad(11.8, 3, 14.4, 1.5); g_paint(STROKE);
        } else {
            g_stroke(INK, 1.4);
            g_fill(0xffffff); g_ellipse(4.6, 1, 3.3, 3.5, 0, BOTH); g_ellipse(11.8, 1, 2.7, 3.3, 0, BOTH);
            g_fill(0x5a8a6a); g_ellipse(5.3, 1.3, 2, 2.5, 0, FILL); g_ellipse(12.3, 1.3, 1.7, 2.3, 0, FILL);
            g_fill(INK); g_circle(5.5, 1.4, 1.2, FILL); g_circle(12.4, 1.4, 1.1, FILL);
            g_fill(0xffffff); g_circle(6.1, 0.4, 0.7, FILL); g_circle(12.9, 0.4, 0.6, FILL);
            g_stroke(INK, 2.2); g_path();                                   // upper lashes with a little flick
            g_move(0.2, 0.2); g_quad(3.6, -4.2, 8.2, -1.2); g_move(9.2, -1.2); g_quad(12, -4, 15.2, -0.6); g_paint(STROKE);
        }
        g_stroke(INK, 1.5); g_path(); g_move(1.5, -6.3f); g_quad(4.8, -8.6f, 8.2, -6.8f); g_move(9.8, -7); g_quad(12.4, -8.6f, 14.8, -6.6f); g_paint(STROKE);
        g_stroke(0xd9a184, 1.5); g_path(); g_move(11.2, 4.2); g_quad(12.6, 6.2, 10.8, 6.8); g_paint(STROKE);  // nose
        g_alpha(alpha * 0.28f); g_fill(0xff8fa8); g_ellipse(0.5, 6.5, 3.2, 2.2, 0, FILL); g_alpha(alpha);
        g_stroke(INK, 1.6);
        if ((air && !climb) || win) { g_fill(0xb3304f); g_ellipse(8.8, 10, 2.8, 3.2, 0, BOTH); }
        else { g_fill(0xd9606e); g_path(); g_move(5.2, 9); g_quad(8.8, 7.8, 12.2, 8.8); g_quad(8.8, 12.8, 5.2, 9); g_close(); g_paint(BOTH); }
    }
    g_restore();

    if (!g.fist.active) { begin(h2x, h2y, 1.0f, -0.6f); drawGlove(h2x, h2y, 7.2); g_restore(); }
    g_restore();
    g_alpha(1);
}

