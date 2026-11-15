#pragma once
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

constexpr float TILE = 48.f;
constexpr int VIEW_W = 960, VIEW_H = 540;

// Player tuning (pixels, seconds)
constexpr float RUN_SPEED = 290.f;
constexpr float ACCEL_GROUND = 2600.f, ACCEL_AIR = 1700.f;
constexpr float GRAVITY = 2300.f;
constexpr float JUMP_SPEED = 860.f;      // ~3.3 tiles high
constexpr float GLIDE_FALL = 95.f;       // max fall speed while hair-copter gliding
constexpr float BOUNCER_SPEED = 1150.f;  // mushroom / drum: ~6 tiles high
constexpr float CLIMB_SPEED = 175.f;     // on lianas
constexpr float COYOTE_TIME = 0.10f, JUMP_BUFFER = 0.12f;
constexpr float PLAYER_HALF_W = 13.f, PLAYER_H = 58.f;
constexpr float MAX_FALL_TILES = 14.f;   // falling further than this from the last footing counts as a pit
constexpr int MAX_HP = 3;

struct Input {
    bool left = false, right = false, up = false, down = false, jump = false, punch = false;
    bool jumpPressed = false, upPressed = false, punchPressed = false, startPressed = false;
    int requestLevel = -1;  // jump straight to a level (0-based)
};

struct Player {
    float x = 0, y = 0;  // feet centre
    float vx = 0, vy = 0;
    float prevY = 0, lastSafeY = 0;
    int dir = 1;
    bool grounded = false, jumping = false, gliding = false;
    int onMover = -1, onLiana = -1;
    float coyote = 0, jumpBuf = 0, grabCd = 0;
    float invuln = 0, lock = 0;
    int hp = MAX_HP;
    bool dead = false;
    float deadT = 0;
    // animation
    float runPhase = 0, climbPhase = 0, squash = 0, headOff = 0, headVel = 0, punchT = 0;
};

struct Fist { bool active = false, returning = false; float x = 0, y = 0, vx = 0, dist = 0; };
struct Enemy { int type; float x, y, vx, x0, y0, t; bool alive; };  // type 0 walker, 1 flyer
struct Orb { float x, y, phase; bool taken; };
struct Bouncer { float x, y, bounceT; };
// A platform you can ride. It glides back and forth between (ax,ay) and (bx,by); x,y is its top centre.
struct Mover { float ax, ay, bx, by, w, x, y, prevX, prevY, phase; };
struct Liana { float x, y0, y1; };
struct Checkpoint { float x, y; bool active; };
struct Decor { char kind; float x, y; };  // scenery: 'T' tree, 'S' shed, 'b' sheep, 'H' sign
struct Particle { float x, y, vx, vy, grav, life, maxLife, size; int color; int kind; };  // kind 0 dot, 1 star

inline float lianaSway(const Liana& l, float y, float t) {
    float k = std::clamp((y - l.y0) / (l.y1 - l.y0), 0.f, 1.f);
    return std::sin(t * 1.4f + l.x * 0.02f) * 9 * k;
}

struct Game {
    enum State { TITLE, PLAY, WIN } state = TITLE;
    int level = 0;  // 0 Heidestein, 1 Vinales
    std::string name;
    int W = 0, H = 0;
    std::vector<std::string> tiles;
    Player p;
    Fist fist;
    std::vector<Enemy> enemies;
    std::vector<Orb> orbs;
    std::vector<Bouncer> bouncers;
    std::vector<Mover> movers;
    std::vector<Liana> lianas;
    std::vector<Checkpoint> checkpoints;
    std::vector<Decor> decor;
    std::vector<Particle> particles;
    float spawnX = 0, spawnY = 0, goalX = 0, goalY = 0;
    float camX = 0, camY = 0, lookAhead = 0, shake = 0;
    float time = 0, playTime = 0, stateT = 0;
    int orbsTaken = 0;

    void load(int levelIndex);
    void update(const Input& in, float dt);
    bool lastLevel() const;

    char at(int tx, int ty) const;
    bool solid(int tx, int ty) const;

private:
    void stepPlayer(const Input& in, float dt);
    void stepOnLiana(const Input& in, float dt);
    void stepFist(float dt);
    void stepEnemies(float dt);
    void stepPickups();
    void stepCamera(float dt, bool snap);
    void hurt(float fromX);
    void die();
    void respawn();
    void killEnemy(Enemy& e);
    void takeOrb(Orb& o);
    void burst(float x, float y, int n, int color, float speed, int kind, float size = 5);
};

void render(const Game& g);
