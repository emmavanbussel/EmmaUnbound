// EmmaUnbound - a small cartoon platformer.
// C++ compiled to WebAssembly with Emscripten; drawing goes through Canvas 2D (see gfx.cpp).
#include "game.hpp"
#include "gfx.hpp"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <algorithm>
#include <cstring>

static Game game;
static Input held;                                     // current state of the controls
static bool pendJump, pendUp, pendPunch, pendStart;    // presses waiting for the next update
static int pendLevel = -1;
static bool keyL, keyR, keyU, keyD, keyJ, keyP, btn[7];
static double lastTime, accumulator;
static const double STEP = 1.0 / 120.0;                // fixed physics step

static void refreshHeld() {
    bool jump = keyJ || btn[2], punch = keyP || btn[3], up = keyU || btn[5];
    if (jump && !held.jump) pendJump = true;
    if (up && !held.up) pendUp = true;
    held.up = up; held.down = keyD || btn[6];
    if (punch && !held.punch) pendPunch = true;
    held.left = keyL || btn[0]; held.right = keyR || btn[1];
    held.jump = jump; held.punch = punch;
}

static EM_BOOL onKey(int type, const EmscriptenKeyboardEvent* e, void*) {
    bool down = type == EMSCRIPTEN_EVENT_KEYDOWN;
    if (e->ctrlKey || e->metaKey || e->altKey) return EM_FALSE;
    auto is = [&](const char* c) { return std::strcmp(e->code, c) == 0; };
    if (is("ArrowLeft") || is("KeyA")) keyL = down;
    else if (is("ArrowRight") || is("KeyD")) keyR = down;
    else if (is("ArrowUp") || is("KeyW")) keyU = down;      // jump, or climb up a liana
    else if (is("ArrowDown") || is("KeyS")) keyD = down;    // climb down
    else if (is("Space") || is("KeyZ")) keyJ = down;
    else if (is("KeyX") || is("KeyJ") || is("KeyK")) keyP = down;
    else if (is("Enter")) { if (down && !e->repeat) pendStart = true; }
    else if (is("Digit1") || is("Digit2") || is("Digit3") || is("Digit4")) { if (down && !e->repeat) pendLevel = e->code[5] - '1'; }
    else return EM_FALSE;
    refreshHeld();
    return EM_TRUE;
}

static EM_BOOL onBlur(int, const EmscriptenFocusEvent*, void*) {
    keyL = keyR = keyU = keyD = keyJ = keyP = false;
    std::fill(btn, btn + 7, false);
    refreshHeld();
    return EM_FALSE;
}

extern "C" {
// On-screen touch buttons call this: 0 left, 1 right, 2 jump, 3 punch, 4 start, 5 up, 6 down.
EMSCRIPTEN_KEEPALIVE void emma_button(int id, int down) {
    if (id == 4) { if (down) pendStart = true; return; }
    if (id < 0 || id > 6) return;
    btn[id] = down != 0;
    refreshHeld();
}
// Jump straight to a level (1-based). The page uses this for links like index.html#level2.
EMSCRIPTEN_KEEPALIVE void emma_goto_level(int n) { pendLevel = n - 1; }
// Read-only peek at the game, handy for automated tests: 0 x, 1 y (in tiles), 2 state, 3 hp, 4 orbs, 5 on ground, 6 liana, 7 level.
EMSCRIPTEN_KEEPALIVE double emma_state(int i) {
    switch (i) {
        case 0: return game.p.x / TILE;
        case 1: return game.p.y / TILE;
        case 2: return game.state;
        case 3: return game.p.hp;
        case 4: return game.orbsTaken;
        case 5: return game.p.grounded;
        case 6: return game.p.onLiana;
        case 7: return game.level + 1;
        default:  // 10 + 2n, 11 + 2n: position of platform n
            if (i >= 10 && (i - 10) / 2 < int(game.movers.size())) {
                const Mover& m = game.movers[(i - 10) / 2];
                return (i % 2 ? m.y : m.x) / TILE;
            }
            return 0;
    }
}
}

static void frame() {
    double now = emscripten_get_now() / 1000.0;
    accumulator += std::min(now - lastTime, 0.1);
    lastTime = now;
    while (accumulator >= STEP) {
        Input in = held;
        in.jumpPressed = pendJump; in.upPressed = pendUp; in.punchPressed = pendPunch;
        in.startPressed = pendStart; in.requestLevel = pendLevel;
        pendJump = pendUp = pendPunch = pendStart = false; pendLevel = -1;
        game.update(in, float(STEP));
        accumulator -= STEP;
    }
    render(game);
}

int main() {
    g_init(VIEW_W, VIEW_H);
    game.load(0);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, onKey);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, onKey);
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, onBlur);
    lastTime = emscripten_get_now() / 1000.0;
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}
