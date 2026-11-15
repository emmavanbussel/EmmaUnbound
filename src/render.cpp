// Puts a frame together: background, world, HUD.
#include "draw.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>

// ------------------------------------------------------------ HUD

static void drawHud(const Game& g) {
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
    if (g.level == 3 && g.state == Game::PLAY && g.playTime > 3.2f)   // where on the journey we are
        g_text(japanPlace(g.p.x), VIEW_W / 2, 44, 24, 1, 0xffffff, INK, 6);
    const char* keys = "Arrows / A D: move      Space: jump, hold to glide      X: punch";
    if (const char* tip = g.hint()) g_text(tip, VIEW_W / 2, VIEW_H - 22, 19, 1, 0xffffff, INK, 5);
    if (g.state == Game::PLAY && g.playTime < 3.2f) {  // level name card
        float k = std::min(1.f, g.playTime * 3) * std::min(1.f, (3.2f - g.playTime) * 2);
        g_alpha(k);
        std::snprintf(buf, sizeof buf, "Level %d", g.level + 1);
        g_text(buf, VIEW_W / 2, 150 - (1 - k) * 20, 30, 1, 0xffffff, INK, 6);
        g_text(g.name.c_str(), VIEW_W / 2, 215 - (1 - k) * 20, 70, 1, INK, INK, 18);
        g_text(g.name.c_str(), VIEW_W / 2, 211 - (1 - k) * 20, 70, 1, g.level == 0 ? 0xd68fe8 : (g.level == 1 ? 0xffd447 : (g.level == 2 ? 0x9fe8ff : 0xff6a4a)), INK, 10);
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
        g_text("Press 1, 2, 3 or 4 to pick a level", VIEW_W / 2, 378, 18, 1, 0xffffff, INK, 5);
        g_text(keys, VIEW_W / 2, VIEW_H - 22, 19, 1, 0xffffff, INK, 5);
    }
    if (g.state == Game::WIN) {
        float k = std::min(1.f, g.stateT * 2);
        g_alpha(0.35 * k); g_fill(INK); g_rect(0, 0, VIEW_W, VIEW_H); g_alpha(1);
        g_save(); g_translate(VIEW_W / 2, 200 - (1 - k) * 60); g_rotate(std::sin(t * 2) * 0.02f);
        const char* head = g.lastLevel() ? "You did it!" : "Level complete!";
        g_text(head, 0, 5, 78, 1, INK, INK, 20);
        g_text(head, 0, 0, 78, 1, 0xffd447, INK, 11);
        g_restore();
        int secs = int(g.playTime);
        std::snprintf(buf, sizeof buf, "Orbs %d / %d      Time %d:%02d", g.orbsTaken, int(g.orbs.size()), secs / 60, secs % 60);
        g_text(buf, VIEW_W / 2, 280, 32, 1, 0xffffff, INK, 6);
        if (g.stateT > 1.2f && std::fmod(t, 1.2f) < 0.8f)
            g_text(g.lastLevel() ? "Press ENTER or tap to play again" : "Press ENTER or tap for the next level",
                   VIEW_W / 2, 350, 26, 1, 0xffffff, INK, 6);
    }
}


void render(const Game& g) {
    g_begin();
    float t = g.time;
    drawBackground(g);
    drawFog(g, false);

    float shx = 0, shy = 0;
    if (g.shake > 0) { shx = std::sin(t * 90) * g.shake * 18; shy = std::cos(t * 70) * g.shake * 14; }
    g_save();
    g_translate(-g.camX + shx, -g.camY + shy);

    int tx0 = int(g.camX / TILE) - 2, tx1 = int((g.camX + VIEW_W) / TILE) + 2;
    int ty0 = std::max(0, int(g.camY / TILE) - 2), ty1 = std::min(g.H - 1, int((g.camY + VIEW_H) / TILE) + 1);
    auto visible = [&](float x, float y, float margin = 160) {
        return x > g.camX - margin && x < g.camX + VIEW_W + margin && y > g.camY - margin && y < g.camY + VIEW_H + 2.5f * margin;
    };

    drawWires(g);
    for (auto& d : g.decor) if (visible(d.x, d.y, 200)) drawDecor(g, d);
    for (auto& c : g.checkpoints) if (visible(c.x, c.y)) drawCheckpoint(c, t);
    if (visible(g.goalX, g.goalY)) drawGoal(g);
    drawThorns(g, tx0, tx1, ty0, ty1);
    drawTerrain(g, tx0, tx1, ty0, ty1);
    for (auto& b : g.bouncers) if (visible(b.x, b.y)) drawBouncer(g, b);
    drawPlanks(g, tx0, tx1, ty0, ty1);
    for (auto& l : g.lianas) if (visible(l.x, l.y0, 500)) drawLiana(g, l);
    for (auto& b : g.bridges) if (visible((b.x0 + b.x1) / 2, b.y, 500)) drawBridge(g, b);
    for (size_t i = 0; i < g.movers.size(); i++) if (visible(g.movers[i].x, g.movers[i].y, 260)) drawMover(g, g.movers[i], int(i));
    for (auto& o : g.orbs) if (!o.taken && visible(o.x, o.y)) drawOrb(o.x, o.y, t, o.phase);
    for (auto& e : g.enemies) if (e.alive && visible(e.x, e.y)) drawEnemy(g, e);
    drawEmma(g);
    drawFist(g);
    drawWater(g, tx0, tx1, ty0, ty1);

    for (auto& q : g.particles) {
        float k = q.life / q.maxLife;
        g_alpha(std::min(1.f, k * 1.6f)); g_fill(q.color);
        if (q.kind == 1) { starPath(q.x, q.y, q.size * (0.5f + k), q.size * 0.4f * (0.5f + k), q.life * 6, 4); g_paint(FILL); }
        else g_circle(q.x, q.y, q.size * k, FILL);
    }
    g_alpha(1);
    g_restore();

    drawFog(g, true);
    drawHud(g);
}
