#include "game.hpp"
#include "level.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>

static float approach(float v, float target, float d) {
    return v < target ? std::min(v + d, target) : std::max(v - d, target);
}
static float frand() {  // 0..1
    static uint32_t s = 0x2545F491u;
    s ^= s << 13; s ^= s >> 17; s ^= s << 5;
    return (s & 0xFFFFFF) / float(0x1000000);
}
static float frand(float a, float b) { return a + (b - a) * frand(); }

char Game::at(int tx, int ty) const {
    if (tx < 0 || tx >= W) return '#';  // level sides are walls
    if (ty < 0 || ty >= H) return '.';
    return tiles[ty][tx];
}
bool Game::solid(int tx, int ty) const { char c = at(tx, ty); return c == '#' || c == ','; }
float Game::fogAt(float x) const {
    float f = 0;
    for (float fx : fogs) f = std::max(f, 1 - std::fabs(x - fx) / (15 * TILE));
    return std::clamp(f * 1.6f, 0.f, 1.f);
}
bool Game::filled(int tx, int ty) const { char c = at(tx, ty); return c == '#' || c == ',' || c == '/' || c == '\\'; }
bool Game::lastLevel() const { return level >= LEVEL_COUNT - 1; }

void Game::load(int levelIndex) {
    level = std::clamp(levelIndex, 0, LEVEL_COUNT - 1);
    const LevelDef& def = LEVELS[level];
    name = def.name;
    tiles.clear(); enemies.clear(); orbs.clear(); bouncers.clear(); movers.clear();
    lianas.clear(); bridges.clear(); checkpoints.clear(); decor.clear(); particles.clear(); fogs.clear();
    H = def.height;
    for (int y = 0; y < H; y++) tiles.emplace_back(def.rows[y]);
    W = int(tiles[0].size());
    const std::vector<std::string> src = tiles;  // untouched copy, for looking at neighbours
    auto is = [&](int tx, int ty, const char* set) {
        if (tx < 0 || tx >= W || ty < 0 || ty >= H) return false;
        for (const char* c = set; *c; c++) if (src[ty][tx] == *c) return true;
        return false;
    };
    static const float moverWidths[] = {150.f, 110.f, 150.f};  // floating pine, rope platform, fishing boat
    float moverW = moverWidths[level];
    int walkerHp = level == 2 ? 2 : 1;                         // polar bears take two hits
    float walkerSpeed = level == 2 ? 50.f : 60.f;
    for (int ty = 0; ty < H; ty++) {
        for (int tx = 0; tx < W; tx++) {
            char& c = tiles[ty][tx];
            float cx = tx * TILE + TILE / 2, bottom = (ty + 1) * TILE, mid = ty * TILE + TILE / 2;
            switch (c) {
                case 'P': spawnX = cx; spawnY = bottom; break;
                case 'G': goalX = cx; goalY = bottom; break;
                case 'o': orbs.push_back({cx, mid, tx * 0.7f, false}); break;
                case 'E': enemies.push_back({0, cx, bottom, -walkerSpeed, cx, bottom, tx * 1.3f, true, walkerHp, 0}); break;
                case 'F': enemies.push_back({1, cx, mid, 0, cx, mid, tx * 1.3f, true, 1, 0}); break;
                case 'M': bouncers.push_back({cx, bottom, 0}); break;
                case 'C': checkpoints.push_back({cx, bottom, false}); break;
                case 'T': case 'S': case 'b': case 'H': case 't': case 'R': case 'D': case 'l': case 'd':
                case 'k': case 'c': case 'j': case 'p': case 'U': case 'x': case 'h':
                    decor.push_back({c, cx, bottom}); break;
                case 'B': case 'r': decor.push_back({c, cx, bottom}); c = 'w'; continue;  // these sit in water
                case 'f': fogs.push_back(cx); break;
                case '~':  // sideways platform: a run of ~ marks how far its centre travels
                    if (!is(tx - 1, ty, "~")) {
                        int n = 1;
                        while (is(tx + n, ty, "~")) n++;
                        float y = ty * TILE;
                        if (n == 1) movers.push_back({cx - 74, y, cx + 74, y, moverW, cx, y, cx, y, tx * 1.f});
                        else movers.push_back({cx, y, cx + (n - 1) * TILE, y, moverW, cx, y, cx, y, 0});
                    }
                    if (is(tx, ty + 1, "w")) { c = 'w'; continue; }  // it floats: the water carries on underneath
                    break;
                case ':': case ';':  // lift: rides up and down its column
                    if (c == ';') orbs.push_back({cx, mid, tx * 0.7f, false});
                    if (!is(tx, ty - 1, ":;")) {
                        int n = 1;
                        while (is(tx, ty + n, ":;")) n++;
                        float y0 = ty * TILE, y1 = (ty + n - 1) * TILE;
                        movers.push_back({cx, y1, cx, y0, moverW, cx, y1, cx, y1, 0});
                    }
                    break;
                case '|': case 'i':  // liana
                    if (c == 'i') orbs.push_back({cx, mid, tx * 0.7f, false});
                    if (!is(tx, ty - 1, "|i")) {
                        int n = 1;
                        while (is(tx, ty + n, "|i")) n++;
                        lianas.push_back({cx, ty * TILE, (ty + n) * TILE});
                    }
                    break;
                case 'n': case 'u': {  // curved bridge across a run of cells
                    const char* same = c == 'n' ? "n" : "u";
                    if (!is(tx - 1, ty, same)) {
                        int n = 1;
                        while (is(tx + n, ty, same)) n++;
                        float span = n * TILE;
                        float arch = c == 'n' ? std::min(46.f, span * 0.13f) : -std::min(30.f, span * 0.08f);
                        if (level == 1) arch = -6;  // the old plank bridge barely sags
                        bridges.push_back({tx * TILE, (tx + n) * TILE, ty * TILE, arch});
                    }
                    if (is(tx, ty + 1, "w")) { c = 'w'; continue; }  // water carries on underneath a low bridge
                    break;
                }
                default: continue;  // keep '#', '=', '^', '/', '\', 'w', '.'
            }
            c = '.';
        }
    }
    orbsTaken = 0; playTime = 0; shake = 0;
    fist = Fist{};
    respawn();
    p.invuln = 0;
    stepCamera(0, true);
}

void Game::respawn() {
    p = Player{};
    p.x = spawnX; p.y = p.prevY = p.lastSafeY = spawnY;
    p.invuln = 1.5f;
    fist.active = false;
}

void Game::burst(float x, float y, int n, int color, float speed, int kind, float size) {
    for (int i = 0; i < n; i++) {
        float a = frand(0, 6.2832f), s = speed * frand(0.35f, 1.f);
        float life = frand(0.35f, 0.75f);
        particles.push_back({x, y, std::cos(a) * s, std::sin(a) * s - speed * 0.3f,
                             kind == 1 ? 500.f : 300.f, life, life, size * frand(0.6f, 1.2f), color, kind});
    }
}

void Game::hurt(float fromX) {
    if (p.invuln > 0 || p.dead || state == WIN) return;
    p.hp--;
    shake = 0.3f;
    burst(p.x, p.y - 30, 10, 0xffffff, 220, 1);
    if (p.hp <= 0) { die(); return; }
    p.invuln = 1.4f; p.lock = 0.28f;
    p.vy = -560; p.vx = (p.x < fromX ? -1.f : 1.f) * 280;
    p.grounded = false; p.jumping = false; p.onMover = -1; p.onSlope = 0;
    if (p.onLiana >= 0) { p.onLiana = -1; p.grabCd = 0.5f; }
}

void Game::die() {
    if (p.dead) return;
    p.dead = true; p.deadT = 0; p.hp = 0;
    shake = 0.4f;
    fist.active = false;
    burst(p.x, p.y - 30, 18, 0xff4f7b, 300, 1, 6);
}

void Game::killEnemy(Enemy& e) {
    static const int walkerColor[] = {0x9aa0b4, 0xf0503c, 0xf4f8fc}, flyerColor[] = {0xffd23f, 0x3fd6a8, 0xf4f8fc};
    e.alive = false;
    shake = std::max(shake, 0.15f);
    float cy = e.type == 0 ? e.y - 18 : e.y;
    burst(e.x, cy, 14, e.type == 0 ? walkerColor[level] : flyerColor[level], 260, 0, 7);
    burst(e.x, cy, 8, 0xffffff, 300, 1, 5);
}

// One punch or stomp. Returns false if the enemy is still reeling from the last hit.
bool Game::hitEnemy(Enemy& e) {
    if (e.hitT > 0) return false;
    if (--e.hp <= 0) { killEnemy(e); return true; }
    e.hitT = 0.7f;                                  // stunned for a moment, then angrier and faster
    e.vx = (e.vx > 0 ? 1.f : -1.f) * 95;
    shake = std::max(shake, 0.12f);
    burst(e.x, e.y - 30, 8, 0xffffff, 240, 1, 5);
    return true;
}

void Game::takeOrb(Orb& o) {
    o.taken = true; orbsTaken++;
    burst(o.x, o.y, 7, 0xffe45c, 170, 1, 4);
}

// ---------------------------------------------------------------------------

// Is there a slope under this point? Gives the height of its surface at x and which way it leans.
bool Game::slopeSurface(float x, float y, float& surf, int& dir) const {
    int cx = int(std::floor(x / TILE));
    float fx = x - cx * TILE;
    int r0 = int(std::floor((y - 1) / TILE));
    static const int order[] = {0, 1, -1};
    for (int d : order) {
        int r = r0 + d;
        char c = at(cx, r);
        if (c != '/' && c != '\\') continue;
        float s = c == '/' ? (r + 1) * TILE - fx : r * TILE + fx;
        if (d == -1 && y - s > 20) continue;  // that one is overhead, not underfoot
        surf = s; dir = c == '/' ? 1 : -1;
        return true;
    }
    return false;
}

void Game::stepPlayer(const Input& in, float dt) {
    if (p.dead) {
        p.deadT += dt;
        if (p.deadT > 1.2f) respawn();
        return;
    }
    p.invuln = std::max(0.f, p.invuln - dt);
    p.lock = std::max(0.f, p.lock - dt);
    p.punchT = std::max(0.f, p.punchT - dt);
    p.grabCd = std::max(0.f, p.grabCd - dt);
    p.squash = approach(p.squash, 0, dt * 4);

    if (p.onLiana >= 0) { stepOnLiana(in, dt); return; }
    bool jumpHeld = in.jump || in.up, jumpPressed = in.jumpPressed || in.upPressed;
    bool ice = level == 2;

    // ride moving platforms (sideways and up / down)
    if (p.onMover >= 0) { const Mover& m = movers[p.onMover]; p.x += m.x - m.prevX; p.y = m.y; }

    // horizontal control
    float ax = p.lock > 0 ? 0 : float(in.right) - float(in.left);
    if (ax != 0) p.dir = ax > 0 ? 1 : -1;
    if (p.lock <= 0) {
        float accel = !p.grounded ? ACCEL_AIR : (ice && p.onMover < 0 ? (ax != 0 ? ACCEL_ICE : FRICTION_ICE) : ACCEL_GROUND);
        bool sliding = ice && p.grounded && p.onSlope != 0;
        if (!(sliding && ax == 0)) p.vx = approach(p.vx, ax * RUN_SPEED, accel * dt);
        if (sliding) p.vx = std::clamp(p.vx - p.onSlope * SLOPE_SLIDE * dt, -430.f, 430.f);  // icy slopes pull you downhill
    }

    // jump: buffered press + coyote time, variable height
    p.jumpBuf = jumpPressed ? JUMP_BUFFER : p.jumpBuf - dt;
    p.coyote = p.grounded ? COYOTE_TIME : p.coyote - dt;
    if (p.jumpBuf > 0 && p.coyote > 0 && p.lock <= 0) {
        p.vy = -JUMP_SPEED; p.jumpBuf = 0; p.coyote = 0;
        p.grounded = false; p.jumping = true; p.onMover = -1; p.onSlope = 0;
        p.squash = -0.5f;
        burst(p.x, p.y, 5, 0xffffff, 90, 0, 5);
    }
    if (p.jumping && !jumpHeld && p.vy < -260) { p.vy *= 0.5f; p.jumping = false; }
    if (p.vy >= 0) p.jumping = false;

    // gravity + hair-copter glide (hold jump while falling)
    p.vy += GRAVITY * dt;
    p.gliding = !p.grounded && jumpHeld && p.vy > 0 && p.lock <= 0;
    if (p.gliding && p.vy > GLIDE_FALL) p.vy = std::max(GLIDE_FALL, p.vy - 5000 * dt);
    p.vy = std::min(p.vy, 1100.f);

    // --- move X, collide with solid tiles. The lowest 14px of the body is ignored so that slopes and
    //     small lips do not stop her.
    p.prevY = p.y;
    p.x += p.vx * dt;
    {
        int ty0 = int(std::floor((p.y - PLAYER_H) / TILE)), ty1 = int(std::floor((p.y - 15) / TILE));
        int txr = int(std::floor((p.x + PLAYER_HALF_W) / TILE)), txl = int(std::floor((p.x - PLAYER_HALF_W) / TILE));
        for (int ty = ty0; ty <= ty1; ty++) {
            if (solid(txr, ty)) { p.x = txr * TILE - PLAYER_HALF_W - 0.01f; if (p.vx > 0) p.vx = 0; }
            if (solid(txl, ty)) { p.x = (txl + 1) * TILE + PLAYER_HALF_W + 0.01f; if (p.vx < 0) p.vx = 0; }
        }
    }

    // --- move Y
    bool wasGrounded = p.grounded;
    float fallSpeed = p.vy;
    p.y += p.vy * dt;
    p.grounded = false; p.onMover = -1; p.onSlope = 0;
    int tx0 = int(std::floor((p.x - PLAYER_HALF_W) / TILE)), tx1 = int(std::floor((p.x + PLAYER_HALF_W) / TILE));
    if (p.vy >= 0) {
        float surf; int dir;
        if (slopeSurface(p.x, p.y, surf, dir)) {
            // slopes: follow the surface; when walking downhill stay glued to it
            if (p.y >= surf || (wasGrounded && surf - p.y <= 16)) { p.y = surf; p.vy = 0; p.grounded = true; p.onSlope = dir; }
        } else {
            int ty = int(std::floor(p.y / TILE));
            for (int tx = tx0; tx <= tx1; tx++) {
                char c = at(tx, ty);
                bool plank = c == '=' && p.prevY <= ty * TILE + 0.5f;  // one-way: only when coming from above
                if (solid(tx, ty) || plank) { p.y = ty * TILE; p.vy = 0; p.grounded = true; }
            }
        }
        for (const Bridge& b : bridges) {  // curved bridges: one-way, and glued like slopes
            if (p.x < b.x0 || p.x > b.x1) continue;
            float s = bridgeY(b, p.x);
            bool landing = p.y >= s && p.prevY <= s + 14, glued = wasGrounded && p.y < s && s - p.y <= 14;
            if (landing || glued) { p.y = s; p.vy = 0; p.grounded = true; }
        }
        for (size_t i = 0; i < movers.size(); i++) {
            const Mover& m = movers[i];
            if (p.prevY <= std::max(m.y, m.prevY) + 1 && p.y >= m.y && std::fabs(p.x - m.x) < m.w / 2 + PLAYER_HALF_W - 6) {
                p.y = m.y; p.vy = 0; p.grounded = true; p.onMover = int(i);
            }
        }
    } else {
        int ty = int(std::floor((p.y - PLAYER_H) / TILE));
        for (int tx = tx0; tx <= tx1; tx++)
            if (solid(tx, ty)) { p.y = (ty + 1) * TILE + PLAYER_H; p.vy = 0; p.jumping = false; }
    }
    if (p.grounded && !wasGrounded && fallSpeed > 250) {
        p.squash = std::min(1.f, fallSpeed / 900.f);
        burst(p.x, p.y, 6, 0xffffff, 110, 0, 5);
    }

    // animation state
    if (p.grounded && std::fabs(p.vx) > 30) p.runPhase += std::fabs(p.vx) * dt * 0.052f;
    else p.runPhase = 0;
    float headTarget = std::clamp(-p.vy * 0.01f, -9.f, 6.f);  // the detached head lags behind the body
    p.headVel += ((headTarget - p.headOff) * 170 - p.headVel * 13) * dt;
    p.headOff += p.headVel * dt;

    // bouncers (mushrooms / drums)
    for (auto& m : bouncers) {
        if (p.vy >= 0 && std::fabs(p.x - m.x) < 32 && p.y > m.y - 44 && p.y <= m.y + 1) {  // land on it or just walk into it
            p.y = m.y - 38; p.vy = -BOUNCER_SPEED;
            p.jumping = false; p.grounded = false; p.squash = -0.7f;
            m.bounceT = 0.4f;
            burst(m.x, m.y - 36, 8, 0xffffff, 160, 1, 4);
        }
    }

    // spikes
    {
        int ty0 = int(std::floor((p.y - 30) / TILE)), ty1 = int(std::floor((p.y - 2) / TILE));
        int ax0 = int(std::floor((p.x - 9) / TILE)), ax1 = int(std::floor((p.x + 9) / TILE));
        for (int ty = ty0; ty <= ty1; ty++)
            for (int tx = ax0; tx <= ax1; tx++)
                if (at(tx, ty) == '^' && p.y > (ty + 1) * TILE - 28) hurt(p.x + p.dir * 10);
    }

    // water: Emma cannot swim
    {
        int tx = int(std::floor(p.x / TILE)), ty = int(std::floor((p.y - 6) / TILE));
        if (at(tx, ty) == 'w' && (at(tx, ty - 1) == 'w' || p.y > ty * TILE + WATER_TOP + 22)) {
            burst(p.x, ty * TILE + WATER_TOP, 16, level == 2 ? 0xbfe6ff : (level == 1 ? 0xd9b47a : 0xcfe2f0), 280, 0, 7);
            die();
        }
    }

    // pits: off the bottom of the map, or a very long fall
    if (p.grounded) p.lastSafeY = p.y;
    if (p.y > H * TILE + 140 || p.y > p.lastSafeY + MAX_FALL_TILES * TILE) die();

    // grab a liana when touching one in mid-air
    if (!p.grounded && !p.dead && p.grabCd <= 0 && p.lock <= 0) {
        float cy = p.y - 30;
        for (size_t i = 0; i < lianas.size(); i++) {
            const Liana& l = lianas[i];
            if (std::fabs(p.x - l.x) < 16 && cy > l.y0 + 6 && cy < l.y1) {
                p.onLiana = int(i); p.x = l.x; p.vx = p.vy = 0;
                p.jumping = p.gliding = false; p.onMover = -1;
                burst(l.x, cy - 40, 4, 0x8be04e, 90, 0, 4);
                break;
            }
        }
    }

    // punch: launch the flying fist
    if (in.punchPressed && !fist.active && p.lock <= 0 && !p.dead) {
        fist = Fist{true, false, p.x + p.dir * 16, p.y - 31, p.dir * 900.f, 0};
        p.punchT = 0.18f;
    }
}

// Hanging on a liana: up / down climb, left / right turn, jump leaps off.
void Game::stepOnLiana(const Input& in, float dt) {
    const Liana& l = lianas[p.onLiana];
    if (in.left != in.right) p.dir = in.right ? 1 : -1;
    float move = float(in.down) - float(in.up);
    float cy = p.y - 30 + move * CLIMB_SPEED * dt;
    float top = l.y0 + 14, bottom = l.y1 - 4;
    bool letGo = in.down && cy > bottom;
    cy = std::clamp(cy, top, bottom);
    if (move != 0) p.climbPhase += dt * 11;
    p.x = l.x; p.y = p.prevY = p.lastSafeY = cy + 30;
    p.vx = p.vy = 0; p.grounded = false; p.gliding = false; p.runPhase = 0;
    p.headVel += ((0 - p.headOff) * 170 - p.headVel * 13) * dt;
    p.headOff += p.headVel * dt;
    if (in.jumpPressed) {
        p.onLiana = -1; p.grabCd = 0.3f;
        p.vy = -JUMP_SPEED * 0.9f; p.vx = (float(in.right) - float(in.left)) * RUN_SPEED;
        p.jumping = true; p.squash = -0.5f;
    } else if (letGo) {
        p.onLiana = -1; p.grabCd = 0.4f;
    }
    if (in.punchPressed && !fist.active) {
        fist = Fist{true, false, p.x + p.dir * 16, p.y - 31, p.dir * 900.f, 0};
        p.punchT = 0.18f;
    }
}

void Game::stepFist(float dt) {
    if (!fist.active) return;
    if (!fist.returning) {
        float nx = fist.x + fist.vx * dt;
        fist.dist += std::fabs(fist.vx) * dt;
        int tx = int(std::floor((nx + (fist.vx > 0 ? 9 : -9)) / TILE)), ty = int(std::floor(fist.y / TILE));
        if (solid(tx, ty)) {
            fist.returning = true;
            burst(fist.x, fist.y, 4, 0xffffff, 120, 0, 4);
        } else {
            fist.x = nx;
            if (fist.dist > 250) fist.returning = true;
        }
    } else {
        float tx = p.x + p.dir * 14, ty = p.y - 31;
        float dx = tx - fist.x, dy = ty - fist.y, d = std::sqrt(dx * dx + dy * dy);
        float step = 1150 * dt;
        if (d < step + 16) { fist.active = false; return; }
        fist.x += dx / d * step; fist.y += dy / d * step;
    }
    for (auto& e : enemies) {
        if (!e.alive) continue;
        float cy = e.type == 0 ? e.y - 20 : e.y, reach = e.type == 0 && level == 2 ? 36.f : 28.f;
        if (std::fabs(e.x - fist.x) < reach && std::fabs(cy - fist.y) < reach) {
            if (hitEnemy(e)) fist.returning = true;
        }
    }
    for (auto& o : orbs)
        if (!o.taken && std::fabs(o.x - fist.x) < 24 && std::fabs(o.y - fist.y) < 24) takeOrb(o);
}

void Game::stepEnemies(float dt) {
    bool bear = level == 2;
    for (auto& e : enemies) {
        if (!e.alive) continue;
        e.t += dt;
        e.hitT = std::max(0.f, e.hitT - dt);
        float cy, hw, hh;
        if (e.type == 0) {  // walker: patrol, turn at walls, ledges, slopes and water
            if (e.hitT < 0.35f) e.x += e.vx * dt;
            float sign = e.vx > 0 ? 1.f : -1.f;
            int tx = int(std::floor((e.x + sign * (bear ? 30 : 20)) / TILE)), ty = int(std::floor(e.y / TILE));
            bool floorAhead = solid(tx, ty) || at(tx, ty) == '=';
            char ahead = at(tx, ty - 1);
            if (solid(tx, ty - 1) || !floorAhead || ahead == '^' || ahead == '/' || ahead == '\\') e.vx = -e.vx;
            cy = e.y - (bear ? 22 : 18); hw = bear ? 25 : 19; hh = bear ? 22 : 17;
        } else {  // flyer: hover back and forth
            e.x = e.x0 + std::sin(e.t * 1.1f) * 110;
            e.y = e.y0 + std::sin(e.t * 2.6f) * 16;
            cy = e.y; hw = 15; hh = 13;
        }
        if (p.dead || e.hitT > 0) continue;  // a reeling enemy cannot hurt you
        bool overlap = std::fabs(p.x - e.x) < PLAYER_HALF_W + hw && p.y > cy - hh && p.y - PLAYER_H < cy + hh;
        if (!overlap) continue;
        if (p.vy > 100 && p.prevY <= cy - hh + 18) {  // stomp
            hitEnemy(e);
            p.vy = -620; p.jumping = true; p.squash = -0.5f;
        } else {
            hurt(e.x);
        }
    }
}

void Game::stepPickups() {
    if (p.dead) return;
    float pcx = p.x, pcy = p.y - 30;
    for (auto& o : orbs)
        if (!o.taken && std::fabs(o.x - pcx) < 28 && std::fabs(o.y - pcy) < 40) takeOrb(o);
    for (auto& c : checkpoints) {
        if (!c.active && std::fabs(c.x - p.x) < 34 && std::fabs(c.y - p.y) < 70) {
            for (auto& o : checkpoints) o.active = false;
            c.active = true; spawnX = c.x; spawnY = c.y;
            p.hp = MAX_HP;
            burst(c.x + 18, c.y - 62, 16, 0xff5d8f, 240, 1, 5);
        }
    }
    if (state == PLAY && std::fabs(goalX - p.x) < 42 && std::fabs(goalY - 60 - pcy) < 70) {
        state = WIN; stateT = 0;
        burst(goalX, goalY - 80, 40, 0xffe45c, 420, 1, 7);
    }
}

// Deer are shy: when Emma comes close they bolt off to the right in long bounds, clearing whatever
// is in the way, and are gone once they leave the screen.
void Game::stepDeer(float dt) {
    for (Decor& d : decor) {
        if (d.kind != 'D' || d.state == 2) continue;
        if (d.state == 0) {
            if (!p.dead && std::fabs(p.x - d.x) < 210 && std::fabs(p.y - d.y) < 4 * TILE) { d.state = 1; d.t = 0; }
            continue;
        }
        d.t += dt;
        d.x += std::min(470.f, 150 + d.t * 520) * dt;
        int tx = int(std::floor(d.x / TILE)), r = 0;
        while (r < H && !solid(tx, r)) r++;
        bool footing = r < H && at(tx, r - 1) != 'w' && at(tx, r - 1) != '^';
        if (footing) d.y += (r * TILE - d.y) * std::min(1.f, 10 * dt);   // otherwise it is in mid-leap over a gap, water or thorns
        d.lift += ((footing ? 0.f : 1.f) - d.lift) * std::min(1.f, 8 * dt);
        if (d.x > camX + VIEW_W + 120 || d.x > W * TILE) d.state = 2;
    }
}

void Game::stepCamera(float dt, bool snap) {
    lookAhead = snap ? 80.f * p.dir : approach(lookAhead, p.dir * 80.f, 220 * dt);
    float tx = p.x + lookAhead - VIEW_W / 2.f;
    float ty = p.y - VIEW_H * 0.63f;
    float maxX = W * TILE - VIEW_W, maxY = H * TILE - VIEW_H;
    tx = std::clamp(tx, 0.f, maxX);
    ty = std::clamp(ty, 0.f, maxY);
    if (snap) { camX = tx; camY = ty; return; }
    if (p.dead) return;
    camX += (tx - camX) * std::min(1.f, 7 * dt);
    camY += (ty - camY) * std::min(1.f, 5 * dt);
}

// A short tip that depends on where the player is.
const char* Game::hint() const {
    if (state != PLAY || p.dead) return nullptr;
    if (p.onLiana >= 0) return "Up / Down: climb      Space + direction: leap off";
    for (const Liana& l : lianas)
        if (std::fabs(l.x - p.x) < 260 && p.y > l.y0 && p.y < l.y1 + 140) return "Jump onto the liana to grab it";
    if (playTime < 9) {
        if (level == 0) return "Arrows / A D: move      Space: jump, hold to glide      X: punch";
        if (level == 1) return "Cross the old plank bridge and mind the gaps - Emma cannot swim";
        if (level == 2) return "Careful, the ice is slippery!      Polar bears need two hits";
    }
    return nullptr;
}

void Game::update(const Input& in, float dt) {
    time += dt; stateT += dt;
    shake = std::max(0.f, shake - dt);

    if (in.requestLevel >= 0) { load(in.requestLevel); state = PLAY; stateT = 0; return; }

    for (auto& q : particles) {
        q.life -= dt; q.vy += q.grav * dt;
        q.x += q.vx * dt; q.y += q.vy * dt;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
                                   [](const Particle& q) { return q.life <= 0; }), particles.end());

    for (Mover& m : movers) {
        m.prevX = m.x; m.prevY = m.y; m.phase += dt;
        float k = 0.5f - 0.5f * std::cos(m.phase * 0.95f);  // ease in and out at both ends
        m.x = m.ax + (m.bx - m.ax) * k;
        m.y = m.ay + (m.by - m.ay) * k;
    }
    for (auto& b : bouncers) b.bounceT = std::max(0.f, b.bounceT - dt);

    bool anyPress = in.startPressed || in.jumpPressed || in.upPressed;
    Input play = in;
    if (state == TITLE) {
        play = Input{};
        if (anyPress) { state = PLAY; stateT = 0; }
    } else if (state == WIN) {
        play = Input{};
        if (p.grounded) { play.jump = play.jumpPressed = true; }  // victory hops
        if (std::fmod(stateT, 0.35f) < dt)
            burst(goalX + frand(-200, 200), goalY - frand(80, 260), 8,
                  frand() < 0.5f ? 0xff5d8f : (frand() < 0.5f ? 0xffe45c : 0x4fd6ff), 200, 1, 6);
        if (stateT > 1.2f && anyPress) { load(lastLevel() ? 0 : level + 1); state = PLAY; stateT = 0; return; }
    } else {
        playTime += dt;
    }

    stepPlayer(play, dt);
    stepFist(dt);
    stepEnemies(dt);
    stepPickups();
    stepDeer(dt);
    stepCamera(dt, false);
}
