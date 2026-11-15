#pragma once
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
constexpr float MUSHROOM_SPEED = 1150.f; // ~6 tiles high
constexpr float COYOTE_TIME = 0.10f, JUMP_BUFFER = 0.12f;
constexpr float PLAYER_HALF_W = 13.f, PLAYER_H = 58.f;
constexpr int MAX_HP = 3;

struct Input {
    bool left = false, right = false, jump = false, punch = false;
    bool jumpPressed = false, punchPressed = false, startPressed = false;
};

struct Player {
    float x = 0, y = 0;  // feet centre
    float vx = 0, vy = 0;
    float prevY = 0;
    int dir = 1;
    bool grounded = false, jumping = false, gliding = false;
    int onMover = -1;
    float coyote = 0, jumpBuf = 0;
    float invuln = 0, lock = 0;
    int hp = MAX_HP;
    bool dead = false;
    float deadT = 0;
    // animation
    float runPhase = 0, squash = 0, headOff = 0, headVel = 0, punchT = 0;
};

struct Fist { bool active = false, returning = false; float x = 0, y = 0, vx = 0, dist = 0; };
struct Enemy { int type; float x, y, vx, x0, y0, t; bool alive; };  // type 0 walker, 1 flyer
struct Orb { float x, y, phase; bool taken; };
struct Mushroom { float x, y, bounceT; };
struct Mover { float x0, y, w, range, x, prevX, phase; };
struct Checkpoint { float x, y; bool active; };
struct Particle { float x, y, vx, vy, grav, life, maxLife, size; int color; int kind; };  // kind 0 dot, 1 star

struct Game {
    enum State { TITLE, PLAY, WIN } state = TITLE;
    int W = 0, H = 0;
    std::vector<std::string> tiles;
    Player p;
    Fist fist;
    std::vector<Enemy> enemies;
    std::vector<Orb> orbs;
    std::vector<Mushroom> mushrooms;
    std::vector<Mover> movers;
    std::vector<Checkpoint> checkpoints;
    std::vector<Particle> particles;
    float spawnX = 0, spawnY = 0, goalX = 0, goalY = 0;
    float camX = 0, camY = 0, lookAhead = 0, shake = 0;
    float time = 0, playTime = 0, stateT = 0;
    int orbsTaken = 0;

    void load();
    void update(const Input& in, float dt);

    char at(int tx, int ty) const;
    bool solid(int tx, int ty) const;

private:
    void stepPlayer(const Input& in, float dt);
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
