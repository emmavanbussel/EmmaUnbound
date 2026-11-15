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
static bool pendJump, pendPunch, pendStart;            // presses waiting for the next update
static bool keyL, keyR, keyJ, keyP, btn[4];
static double lastTime, accumulator;
static const double STEP = 1.0 / 120.0;                // fixed physics step

static void refreshHeld() {
    bool jump = keyJ || btn[2], punch = keyP || btn[3];
    if (jump && !held.jump) pendJump = true;
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
    else if (is("Space") || is("ArrowUp") || is("KeyW") || is("KeyZ")) keyJ = down;
    else if (is("KeyX") || is("KeyJ") || is("KeyK")) keyP = down;
    else if (is("Enter")) { if (down && !e->repeat) pendStart = true; }
    else if (is("ArrowDown")) return EM_TRUE;  // just stop the page scrolling
    else return EM_FALSE;
    refreshHeld();
    return EM_TRUE;
}

static EM_BOOL onBlur(int, const EmscriptenFocusEvent*, void*) {
    keyL = keyR = keyJ = keyP = false;
    std::fill(btn, btn + 4, false);
    refreshHeld();
    return EM_FALSE;
}

extern "C" {
// On-screen touch buttons call this: 0 left, 1 right, 2 jump, 3 punch, 4 start.
EMSCRIPTEN_KEEPALIVE void emma_button(int id, int down) {
    if (id == 4) { if (down) pendStart = true; return; }
    if (id < 0 || id > 3) return;
    btn[id] = down != 0;
    refreshHeld();
}
// Read-only peek at the game, handy for automated tests: 0 x, 1 y (in tiles), 2 state, 3 hp, 4 orbs, 5 on ground.
EMSCRIPTEN_KEEPALIVE double emma_state(int i) {
    switch (i) {
        case 0: return game.p.x / TILE;
        case 1: return game.p.y / TILE;
        case 2: return game.state;
        case 3: return game.p.hp;
        case 4: return game.orbsTaken;
        case 5: return game.p.grounded;
        default: return 0;
    }
}
}

static void frame() {
    double now = emscripten_get_now() / 1000.0;
    accumulator += std::min(now - lastTime, 0.1);
    lastTime = now;
    while (accumulator >= STEP) {
        Input in = held;
        in.jumpPressed = pendJump; in.punchPressed = pendPunch; in.startPressed = pendStart;
        pendJump = pendPunch = pendStart = false;
        game.update(in, float(STEP));
        accumulator -= STEP;
    }
    render(game);
}

int main() {
    g_init(VIEW_W, VIEW_H);
    game.load();
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, onKey);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, onKey);
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, onBlur);
    lastTime = emscripten_get_now() / 1000.0;
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}
