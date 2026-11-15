// Level 4, Japan: a journey from the ryokan in Izu Nagaoka through Kyoto, Koyasan, Hiroshima and Himeji
// to Tokyo. Background, scenery, the red bridge and the room of lights.
#include "draw.hpp"
#include <algorithm>
#include <cmath>

namespace {

const int VERMILION = 0xf0562a, VERMILION_D = 0xc23f1f, ROOF = 0x7f8a99, ROOF_D = 0x566070;
const int WOOD = 0x7a5a40, WOOD_D = 0x5a4130, PLASTER = 0xfaf6ea;

int mix(int a, int b, float k) {
    k = std::clamp(k, 0.f, 1.f);
    auto ch = [&](int s) { return int(((a >> s) & 255) * (1 - k) + ((b >> s) & 255) * k + 0.5f); };
    return (ch(16) << 16) | (ch(8) << 8) | ch(0);
}

float hillY(float wx, float base, float amp, float f, float ph) {
    return base + amp * std::sin(wx * f + ph) + amp * 0.45f * std::sin(wx * f * 2.3f + ph * 2.1f);
}

// A tiled roof with upturned eaves. y is the eave line, h the height, topHalf half the ridge length.
void roof(float cx, float y, float halfW, float h, float topHalf, int c0 = ROOF, int c1 = ROOF_D) {
    g_path(); g_move(cx - halfW, y - 9); g_quad(cx, y + 6, cx + halfW, y - 9);
    g_quad(cx + halfW * 0.55f, y - h * 0.45f, cx + topHalf, y - h); g_line(cx - topHalf, y - h);
    g_quad(cx - halfW * 0.55f, y - h * 0.45f, cx - halfW, y - 9); g_close();
    g_lin(0, y - h, 0, y, c0, c1); g_stroke(INK, 3); g_paint(BOTH);
    g_alpha(0.35); g_stroke(0xffffff, 2);
    g_path(); g_move(cx - topHalf + 3, y - h + 4); g_line(cx + topHalf - 3, y - h + 4); g_paint(STROKE);
    g_alpha(1); g_stroke(INK, 3);
}

void shoji(float x, float y, float w, float h) {
    g_fill(0xfff6e0); g_stroke(INK, 2.2); g_rrect(x, y, w, h, 2, BOTH);
    g_stroke(0xb89a70, 1.4); g_path();
    g_move(x + w / 2, y + 2); g_line(x + w / 2, y + h - 2);
    for (int i = 1; i < 4; i++) { g_move(x + 2, y + h * i / 4); g_line(x + w - 2, y + h * i / 4); }
    g_paint(STROKE); g_stroke(INK, 3);
}

// ------------------------------------------------------------ scenery

void drawRyokan(float t) {
    for (int i = 0; i < 4; i++) {  // steam from the hot spring behind
        float k = std::fmod(t * 0.22f + i * 0.25f, 1.f);
        g_alpha(0.5 * (1 - k)); g_fill(0xffffff); g_circle(96 + std::sin(k * 6 + i) * 10, -70 - k * 120, 10 + k * 18, FILL);
    }
    g_alpha(1); g_stroke(INK, 3);
    g_fill(0x9a968c); g_rrect(-116, -12, 232, 15, 3, BOTH);
    // upper floor
    g_lin(0, -126, 0, -80, WOOD, WOOD_D); g_rect(-78, -126, 156, 50); g_rrect(-78, -126, 156, 50, 2, STROKE);
    for (int i = 0; i < 4; i++) shoji(-68 + i * 36, -118, 28, 30);
    g_stroke(INK, 2.5); g_path(); g_move(-84, -84); g_line(84, -84); g_paint(STROKE);
    roof(0, -122, 104, 40, 22);
    // ground floor
    g_stroke(INK, 3); g_lin(0, -72, 0, -12, WOOD, WOOD_D); g_rrect(-106, -72, 212, 60, 2, BOTH);
    shoji(-96, -62, 28, 44); shoji(-62, -62, 28, 44); shoji(36, -62, 28, 44); shoji(70, -62, 28, 44);
    g_fill(0x2f2622); g_rrect(-24, -64, 48, 52, 2, BOTH);
    for (int i = 0; i < 3; i++) {  // noren curtain over the door
        float nx = -22 + i * 15, sw = std::sin(t * 2 + i) * 1.2f;
        g_fill(0x2f4f8f); g_stroke(INK, 2); g_path(); g_move(nx, -64); g_line(nx + 14, -64); g_line(nx + 14 + sw, -38); g_line(nx + sw, -38); g_close(); g_paint(BOTH);
        g_fill(0xfaf6ea); g_circle(nx + 7 + sw * 0.5f, -51, 3.4, FILL);
    }
    g_stroke(INK, 3); roof(0, -68, 132, 26, 98);
    // paper lantern by the door
    g_rad(-36, -34, 34, 0xffd98a, 0.5, 0xffd98a, 0); g_circle(-36, -34, 34, FILL);
    g_fill(0xfff1c9); g_stroke(INK, 2.2); g_ellipse(-36, -34, 8, 11, 0, BOTH);
    g_fill(INK); g_rect(-40, -47, 8, 3); g_rect(-40, -24, 8, 3);
}

void drawTempleHall() {
    g_stroke(INK, 3);
    g_fill(0xb9b3a6); g_rrect(-62, -8, 124, 11, 2, BOTH); g_rrect(-52, -17, 104, 11, 2, BOTH);
    g_fill(WOOD_D); g_rrect(-128, -30, 256, 14, 2, BOTH);
    g_lin(0, -96, 0, -30, 0x6a4c36, 0x4f382a); g_rect(-116, -96, 232, 66); g_rrect(-116, -96, 232, 66, 2, STROKE);
    g_fill(PLASTER);
    for (float px : {-108.f, -70.f, 44.f, 82.f}) g_rrect(px, -90, 28, 22, 2, BOTH);
    g_fill(0x8a6a4a);
    for (float px : {-108.f, -70.f, 44.f, 82.f}) g_rrect(px, -64, 28, 32, 2, BOTH);
    // lattice doors
    g_fill(0x3a2a20); g_rrect(-30, -84, 60, 54, 2, BOTH);
    g_stroke(0x8a6a4a, 1.6); g_path();
    for (int i = 1; i < 6; i++) { g_move(-30 + i * 10, -82); g_line(-30 + i * 10, -32); }
    for (int i = 1; i < 5; i++) { g_move(-28, -84 + i * 11); g_line(28, -84 + i * 11); }
    g_paint(STROKE); g_stroke(INK, 3);
    g_fill(0x5a4130);
    for (float px : {-120.f, -40.f, 32.f, 112.f}) g_rrect(px, -100, 8, 72, 2, BOTH);
    roof(0, -94, 156, 74, 76);
    // curved gable over the entrance
    g_path(); g_move(-58, -100); g_quad(-34, -134, 0, -140); g_quad(34, -134, 58, -100); g_quad(0, -116, -58, -100); g_close();
    g_lin(0, -140, 0, -100, 0x8f9aa9, ROOF_D); g_paint(BOTH);
    g_fill(0x4f6a5a); g_stroke(INK, 2); g_circle(0, -124, 6, BOTH);
    g_fill(ROOF_D); g_stroke(INK, 2.5); g_rrect(-84, -176, 12, 10, 3, BOTH); g_rrect(72, -176, 12, 10, 3, BOTH);
}

void spire(float y, float h, int color, int rings) {
    g_stroke(INK, 6); g_path(); g_move(0, y); g_line(0, y - h); g_paint(STROKE);
    g_stroke(color, 2.6); g_paint(STROKE);
    g_stroke(INK, 1.8); g_fill(color);
    for (int i = 0; i < rings; i++) g_ellipse(0, y - 8 - i * (h - 20) / rings, 7 - i * 0.5f, 2.6, 0, BOTH);
    g_circle(0, y - h, 3.5, BOTH);
}

void drawPagoda() {  // three storeys in vermilion, as at Kiyomizu-dera
    g_stroke(INK, 3);
    g_fill(0xa9a69c); g_rrect(-66, -10, 132, 13, 2, BOTH);
    float y = -10;
    for (int i = 0; i < 3; i++) {
        float w = 46 - i * 8;
        g_lin(0, y - 48, 0, y, VERMILION, VERMILION_D); g_rect(-w, y - 48, w * 2, 48); g_rrect(-w, y - 48, w * 2, 48, 1, STROKE);
        g_fill(PLASTER); g_rrect(-w + 5, y - 40, w * 0.5f, 26, 1, BOTH); g_rrect(w - 5 - w * 0.5f, y - 40, w * 0.5f, 26, 1, BOTH);
        g_fill(0x3f8a6a); g_stroke(INK, 2); g_rrect(-7, y - 40, 14, 30, 1, BOTH);
        g_stroke(INK, 2.5); g_path(); g_move(-w - 8, y - 8); g_line(w + 8, y - 8); g_paint(STROKE);   // balcony rail
        g_stroke(INK, 3);
        g_fill(0xff8a4a); g_rrect(-w - 26, y - 58, (w + 26) * 2, 10, 2, BOTH);                        // bracket band under the eaves
        g_stroke(0xffd447, 1.6); g_path(); for (float bx = -w - 20; bx < w + 22; bx += 9) { g_move(bx, y - 56); g_line(bx, y - 50); } g_paint(STROKE);
        g_stroke(INK, 3);
        roof(0, y - 56, w + 44, 26, w * 0.55f, 0x5f6878, 0x3f4654);
        y -= 82;
    }
    spire(y + 4, 70, 0x6f7a6a, 6);
}

void drawTahoto() {  // the two-storeyed pagoda of Koyasan: square below, a white dome, round above
    g_stroke(INK, 3);
    g_fill(0xa8443a);
    for (float px = -72; px <= 72; px += 24) g_rrect(px - 3, -18, 6, 20, 1, BOTH);
    g_fill(0xe6c25a); g_rrect(-82, -22, 164, 6, 2, BOTH);
    g_stroke(INK, 2.5); g_path(); g_move(-80, -34); g_line(80, -34); g_paint(STROKE);
    g_stroke(INK, 3);
    g_fill(PLASTER); g_rrect(-58, -84, 116, 62, 1, BOTH);
    g_fill(0x9c4038);
    for (float px : {-58.f, -22.f, 16.f, 52.f}) g_rect(px, -84, 6, 62);
    g_rect(-58, -60, 116, 5);
    g_fill(0x9fc7a8); g_stroke(INK, 2); g_rrect(-48, -80, 22, 16, 1, BOTH); g_rrect(26, -80, 22, 16, 1, BOTH);
    g_fill(0x7a3a30); g_rrect(-12, -80, 24, 58, 1, BOTH);
    g_stroke(INK, 3);
    g_fill(0xb04a3c); g_rrect(-74, -94, 148, 10, 2, BOTH);
    roof(0, -92, 104, 24, 46, 0x7a6654, 0x55463a);
    g_path(); g_move(-44, -116); g_quad(-38, -148, 0, -150); g_quad(38, -148, 44, -116); g_close();   // the white dome
    g_lin(0, -150, 0, -116, 0xffffff, 0xdcd6c8); g_paint(BOTH);
    g_fill(0xa8443a); g_rrect(-26, -168, 52, 22, 2, BOTH);
    g_stroke(0xe6c25a, 2); g_path(); g_move(-30, -158); g_line(30, -158); g_paint(STROKE);
    g_stroke(INK, 3);
    g_fill(0xb04a3c); g_rrect(-46, -178, 92, 10, 2, BOTH);
    roof(0, -176, 84, 34, 8, 0x7a6654, 0x55463a);
    spire(-208, 72, 0xe6c25a, 8);
    g_stroke(0xe6c25a, 1.4); g_path(); g_move(0, -276); g_line(-70, -188); g_move(0, -276); g_line(70, -188); g_paint(STROKE);
}

void drawCastle() {  // Himeji: the white heron castle
    g_stroke(INK, 3);
    g_path(); g_move(-170, 3); g_quad(-146, -40, -136, -76); g_line(136, -76); g_quad(146, -40, 170, 3); g_close();
    g_lin(0, -76, 0, 0, 0xb9b5aa, 0x8f8b82); g_paint(BOTH);
    g_stroke(0x77736a, 1.6);
    for (int r = 0; r < 4; r++) for (float sx = -138 + (r % 2) * 16; sx < 130; sx += 34) { g_path(); g_move(sx, -64 + r * 17); g_line(sx + 18, -64 + r * 17); g_paint(STROKE); }
    // a smaller tower beside the keep
    g_stroke(INK, 3); g_fill(PLASTER); g_rrect(-122, -124, 52, 50, 1, BOTH);
    g_fill(0x3a3f4a); g_rect(-112, -108, 7, 10);
    roof(-98, -120, 38, 24, 12);
    float y = -76;
    for (int i = 0; i < 4; i++) {
        float w = 100 - i * 22, h = i == 3 ? 34 : 40;
        g_lin(0, y - h, 0, y, 0xffffff, 0xe6e2d6); g_rect(-w, y - h, w * 2, h); g_rrect(-w, y - h, w * 2, h, 1, STROKE);
        g_fill(0x3a3f4a);
        for (float wx = -w + 12; wx < w - 12; wx += 20) g_rect(wx, y - h + 12, 8, 12);
        roof(0, y - h + 4, w + 24, 26, w * 0.72f);
        if (i % 2 == 0) {  // triangular gables
            for (float gx : {-w * 0.45f, w * 0.45f}) {
                g_fill(PLASTER); g_path(); g_move(gx - 20, y - h + 2); g_line(gx, y - h - 20); g_line(gx + 20, y - h + 2); g_close(); g_paint(BOTH);
            }
        } else {
            g_fill(PLASTER); g_path(); g_move(-26, y - h + 2); g_quad(0, y - h - 26, 26, y - h + 2); g_close(); g_paint(BOTH);
        }
        y -= h + 20;
    }
    g_fill(0xe6c25a); g_stroke(INK, 2);
    g_path(); g_move(-22, y + 2); g_quad(-26, y - 10, -16, y - 12); g_line(-14, y + 2); g_close(); g_paint(BOTH);
    g_path(); g_move(22, y + 2); g_quad(26, y - 10, 16, y - 12); g_line(14, y + 2); g_close(); g_paint(BOTH);
}

void drawTokyoTower(float t) {
    const int RED = 0xe8452a, WHITE = 0xf7f4ee;
    auto leg = [](float k, float& x, float& y) {
        x = (1 - k) * (1 - k) * 92 + 2 * (1 - k) * k * 30 + k * k * 11;
        y = -(2 * (1 - k) * k * 190 + k * k * 350);
    };
    // lattice: horizontals and crossed braces between the two legs
    const int N = 11;
    for (int i = 0; i < N; i++) {
        float x0, y0, x1, y1; leg(i / float(N), x0, y0); leg((i + 1) / float(N), x1, y1);
        int col = (i >= 6 && i <= 7) ? WHITE : RED;
        g_path(); g_move(-x0, y0); g_line(x1, y1); g_move(x0, y0); g_line(-x1, y1); g_move(-x1, y1); g_line(x1, y1);
        g_stroke(INK, 5); g_paint(STROKE); g_stroke(col, 2.4); g_paint(STROKE);
    }
    for (int side = -1; side <= 1; side += 2) {
        g_path(); g_move(side * 92, 2); g_quad(side * 30, -190, side * 11, -350);
        g_stroke(INK, 10); g_paint(STROKE); g_stroke(RED, 5.5); g_paint(STROKE);
    }
    g_path(); g_move(-58, 0); g_quad(0, -84, 58, 0); g_stroke(INK, 7); g_paint(STROKE); g_stroke(RED, 3.5); g_paint(STROKE);  // arch at the foot
    // main deck
    g_stroke(INK, 3); g_fill(WHITE); g_rrect(-40, -172, 80, 30, 3, BOTH);
    g_fill(0x5f7fa8); g_rect(-36, -166, 72, 8); g_rect(-36, -154, 72, 6);
    // upper section, top deck and antenna
    g_fill(RED); g_rrect(-10, -400, 20, 52, 2, BOTH);
    g_stroke(0x8f2a1a, 1.6); g_path(); for (int i = 0; i < 5; i++) { g_move(-9, -352 - i * 10); g_line(9, -362 - i * 10); } g_paint(STROKE);
    g_stroke(INK, 3); g_fill(WHITE); g_rrect(-15, -372, 30, 14, 3, BOTH); g_rrect(-11, -414, 22, 16, 3, BOTH);
    for (int i = 0; i < 5; i++) { g_fill(i % 2 ? WHITE : RED); g_stroke(INK, 2); g_rrect(-3.5f + i * 0.4f, -436 - i * 18, 7 - i * 0.8f, 20, 1, BOTH); }
    if (std::fmod(t, 1.6f) < 0.8f) { g_rad(0, -510, 16, 0xff5a4a, 0.9, 0xff5a4a, 0); g_circle(0, -510, 16, FILL); }
}

void drawTorii(float sc, int col, int colDark, bool text) {
    g_save(); g_scale(sc, sc);
    g_stroke(INK, 3 / sc);
    for (int side = -1; side <= 1; side += 2) {
        g_lin(side * 34 - 7, 0, side * 34 + 7, 0, col, colDark); g_rrect(side * 34 - 7, -116, 14, 118, 3, BOTH);
        g_fill(0x2f2a35); g_rrect(side * 34 - 9, -16, 18, 18, 3, BOTH);
    }
    g_fill(col); g_rrect(-46, -98, 92, 10, 2, BOTH);
    g_path(); g_move(-56, -128); g_quad(0, -119, 56, -128); g_line(58, -114); g_quad(0, -106, -58, -114); g_close(); g_paint(BOTH);
    g_fill(0x2f2a35); g_path(); g_move(-60, -137); g_quad(0, -126, 60, -137); g_line(58, -127); g_quad(0, -118, -58, -127); g_close(); g_paint(BOTH);
    if (text) { g_text("奈", 0, 0, 0, 1, INK, INK, 0); g_text("奉", 34, -62, 13, 1, 0x2f2a35, INK, 0); g_text("納", -34, -62, 13, 1, 0x2f2a35, INK, 0); }
    g_restore();
}

void drawLanternRack(float t) {
    static const char* glyphs[] = {"祭", "光", "花", "月", "山", "川", "風", "空", "星"};
    g_stroke(INK, 3); g_fill(0xe3c79a);
    g_rrect(-74, -158, 9, 160, 2, BOTH); g_rrect(65, -158, 9, 160, 2, BOTH);
    for (int r = 0; r < 3; r++) g_rrect(-78, -154 + r * 46, 156, 7, 2, BOTH);
    g_fill(0xc9a878); g_path(); g_move(-90, -158); g_line(-78, -170); g_line(78, -170); g_line(90, -158); g_close(); g_paint(BOTH);
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            float lx = -44 + c * 44, ly = -124 + r * 46 + std::sin(t * 1.5f + c + r) * 0.8f;
            g_rad(lx, ly, 40, 0xfff3c4, 0.42, 0xfff3c4, 0); g_circle(lx, ly, 40, FILL);
            g_lin(0, ly - 19, 0, ly + 19, 0xffffff, 0xf2ead6); g_stroke(INK, 2.2); g_ellipse(lx, ly, 16.5, 19, 0, BOTH);
            g_stroke(0xd9d2c0, 1); g_path();
            for (int k = -2; k <= 2; k++) { float w = 16 * std::sqrt(1 - (k * 6.f / 19) * (k * 6.f / 19)); g_move(lx - w, ly + k * 6); g_line(lx + w, ly + k * 6); }
            g_paint(STROKE);
            g_fill(0x2f2a35); g_rrect(lx - 7, ly - 22, 14, 4, 1.5, FILL); g_rrect(lx - 7, ly + 18, 14, 4, 1.5, FILL);
            g_fill(0xe0452a);  // plum-blossom crest
            for (int k = 0; k < 5; k++) g_circle(lx + 9 + std::cos(k * 1.257f) * 2.6f, ly - 9 + std::sin(k * 1.257f) * 2.6f, 1.5, FILL);
            g_text(glyphs[r * 3 + c], lx - 2, ly + 7, 17, 1, r == 2 ? 0xd93a2a : 0x2f2a35, INK, 0);
        }
    }
}

void drawStoneLantern() {
    g_stroke(INK, 2.8);
    g_fill(0xa9a69c); g_rrect(-15, -8, 30, 10, 2, BOTH);
    g_fill(0xb9b5aa); g_rrect(-6, -40, 12, 34, 2, BOTH);
    g_fill(0xa9a69c); g_rrect(-14, -48, 28, 9, 2, BOTH);
    g_fill(0xb9b5aa); g_rrect(-11, -68, 22, 21, 2, BOTH);
    g_rad(0, -58, 26, 0xffd98a, 0.5, 0xffd98a, 0); g_circle(0, -58, 26, FILL);
    g_fill(0xffe9a8); g_stroke(INK, 2); g_rrect(-5, -64, 10, 12, 1, BOTH);
    g_stroke(INK, 2.8); g_fill(0x9a968c);
    g_path(); g_move(-21, -66); g_quad(0, -62, 21, -66); g_quad(8, -78, 0, -86); g_quad(-8, -78, -21, -66); g_close(); g_paint(BOTH);
    g_circle(0, -89, 4, BOTH);
    g_alpha(0.7); g_fill(0x8fb85a); g_ellipse(-8, -74, 6, 2.5, -0.4, FILL); g_ellipse(9, -9, 6, 2.2, 0, FILL); g_alpha(1);
}

void drawCedar(uint32_t h) {
    float hgt = 330 + h % 70;
    g_stroke(INK, 3);
    g_lin(-9, 0, 9, 0, 0xa9806a, 0x77584a); g_path(); g_move(-10, 3); g_line(-5, -hgt + 30); g_line(5, -hgt + 30); g_line(10, 3); g_close(); g_paint(BOTH);
    g_stroke(0x5a4238, 1.6); g_path(); for (int i = 0; i < 6; i++) { g_move(-3 + (i % 2) * 5, -20 - i * 26); g_line(-2 + (i % 2) * 5, -36 - i * 26); } g_paint(STROKE);
    const int OUT = 0x1f4034;
    for (int i = 0; i < 7; i++) {
        float yb = -110 - i * (hgt - 130) / 7.f, w = 46 - i * 4.5f, dh = 60;
        g_path(); g_move(-w, yb); g_quad(-w * 0.4f, yb + 12, 0, yb + 4); g_quad(w * 0.4f, yb + 12, w, yb);
        g_quad(w * 0.3f, yb - dh * 0.6f, 0, yb - dh); g_quad(-w * 0.3f, yb - dh * 0.6f, -w, yb); g_close();
        g_lin(0, yb - dh, 0, yb + 8, 0x4a8a66, 0x2c6048); g_stroke(OUT, 3); g_paint(BOTH);
    }
}

void drawRock(uint32_t h) {
    int kind = h % 3;
    float w = 26 + (h >> 4) % 14, hg = kind == 0 ? 70 + (h >> 8) % 24 : 30 + (h >> 8) % 16;
    g_stroke(INK, 3);
    g_path(); g_move(-w, 2); g_quad(-w - 4, -hg * 0.5f, -w * 0.5f, -hg * 0.9f); g_quad(-w * 0.1f, -hg * 1.08f, w * 0.4f, -hg * 0.85f);
    g_quad(w + 6, -hg * 0.45f, w, 2); g_close();
    g_lin(0, -hg, 0, 0, 0x6f6a72, 0x45414a); g_paint(BOTH);
    g_alpha(0.4); g_fill(0xcfcad6); g_ellipse(-w * 0.3f, -hg * 0.62f, w * 0.3f, hg * 0.12f, -0.5, FILL);
    g_alpha(0.75); g_fill(0x8fb85a); g_ellipse(w * 0.25f, -hg * 0.82f, w * 0.3f, 3.5, 0.3, FILL); g_alpha(1);
}

void drawMaple(float t, uint32_t h) {
    float lean = (int(h % 20) - 10);
    auto limb = [](float x0, float y0, float cx, float cy, float x1, float y1, float w) {
        g_path(); g_move(x0, y0); g_quad(cx, cy, x1, y1);
        g_stroke(INK, w + 5); g_paint(STROKE); g_stroke(0x9a8f88, w); g_paint(STROKE);
    };
    limb(lean * 0.4f, -80, -40, -110, -62 + lean, -140, 4.5f);
    limb(lean * 0.5f, -90, 40, -120, 66 + lean, -130, 4.5f);
    limb(0, 3, -lean, -70, lean * 0.6f, -150, 9);
    static const float blobs[][3] = {{-62, -150, 30}, {-24, -182, 36}, {26, -186, 34}, {66, -150, 30}, {0, -146, 34}, {-88, -124, 20}, {92, -120, 20}, {44, -124, 24}, {-42, -120, 24}};
    static const int reds[] = {0xe6332a, 0xff5a3c, 0xd42a2a, 0xf24a34};
    float sway = std::sin(t * 1.2f + h % 7) * 2;
    g_fill(0x6a1420);
    for (auto& b : blobs) g_circle(b[0] + lean + sway, b[1], b[2] + 3.5f, FILL);
    int i = 0;
    for (auto& b : blobs) { g_fill(reds[i++ % 4]); g_circle(b[0] + lean + sway, b[1], b[2], FILL); }
    for (int k = 0; k < 22; k++) {  // individual leaves catching the light
        uint32_t r = hash2(int(h % 977) + k, 44);
        const float* b = blobs[r % 9];
        float a = (r >> 5) % 628 / 100.f, d = b[2] * (0.3f + (r >> 15) % 70 / 100.f);
        g_fill((r >> 22) % 2 ? 0xff7a4a : 0xffa05a);
        starPath(b[0] + lean + sway + std::cos(a) * d, b[1] + std::sin(a) * d, 7, 2.6f, a, 5); g_paint(FILL);
    }
    for (int k = 0; k < 3; k++) {  // falling leaves
        float f = std::fmod(t * 0.16f + k / 3.f + (h % 10) * 0.1f, 1.f);
        g_alpha(std::min(1.f, (1 - f) * 4)); g_fill(0xf24a34);
        starPath(-60 + k * 55 + lean + std::sin(f * 9 + k) * 16, -120 + f * 122, 6, 2.3f, f * 8, 5); g_paint(FILL);
    }
    g_alpha(1);
}

}  // namespace

// Koyasan: the monk who looks after the rock garden. He rakes quietly, and turns to wave Emma off
// when she comes near the gravel.
void drawMonk(const Game& g, const Decor& d) {
    float t = g.time;
    bool warn = std::fabs(g.p.x - d.x) < 520 && g.p.x > d.x - 60, onGravel = g.p.invuln > 0.9f && warn;
    float dir = g.p.x < d.x - 30 ? -1.f : 1.f;
    g_scale(dir, 1);
    g_stroke(INK, 3);
    // rake (held still while he warns)
    float sweep = warn ? 0 : std::sin(t * 2.2f) * 7;
    g_path(); g_move(-6, -44); g_line(30 + sweep, -2);
    g_stroke(INK, 6); g_paint(STROKE); g_stroke(0xc9a878, 2.6); g_paint(STROKE);
    g_stroke(INK, 3); g_fill(0xc9a878); g_rrect(20 + sweep, -5, 22, 6, 2, BOTH);
    // robe: saffron under a dark outer robe
    g_fill(0xfaf3e3); g_ellipse(-7, -3, 8, 4.5, 0, BOTH); g_ellipse(7, -3, 8, 4.5, 0, BOTH);
    g_path(); g_move(-13, -62); g_quad(0, -68, 13, -62); g_quad(21, -30, 19, -6); g_quad(0, -1, -19, -6); g_quad(-21, -30, -13, -62); g_close();
    g_lin(0, -66, 0, -4, 0xf2a03a, 0xcf7a1f); g_paint(BOTH);
    g_fill(0x5a4636); g_path(); g_move(-13, -62); g_quad(-2, -66, 6, -63); g_quad(-8, -36, -19, -8); g_quad(-21, -32, -13, -62); g_close(); g_paint(BOTH);
    // head
    g_fill(0xf2cfa6); g_circle(1, -79, 13, BOTH);
    g_fill(INK); g_circle(5, -80, 1.6f, FILL); g_circle(11, -80, 1.5f, FILL);
    g_stroke(INK, 1.8); g_path();
    if (onGravel) { g_move(2, -85); g_line(8, -83.5f); g_move(13, -85); g_line(9, -83.5f); g_move(5, -72); g_quad(8, -74, 11, -72); }
    else { g_move(5, -73); g_quad(8, -70.5f, 11, -73); }
    g_paint(STROKE);
    // hands: one on the rake, the other raised when warning
    g_stroke(INK, 2.5); g_fill(0xf2cfa6); g_circle(-5, -44, 5, BOTH);
    if (warn) g_circle(18, -58 + std::sin(t * 9) * 3, 5.5f, BOTH); else g_circle(8, -36, 5, BOTH);
    if (warn) {  // speech bubble
        g_scale(dir, 1);
        g_fill(0xffffff); g_stroke(INK, 2.5); g_rrect(-24, -128, 48, 28, 10, BOTH);
        g_path(); g_move(-4, -101); g_line(dir * 4, -93); g_line(6, -101); g_paint(BOTH);
        g_fill(0xffffff); g_rect(-5, -104, 12, 4);
        g_text(onGravel ? "!!" : "!", 0, -106, 22, 1, 0xd9483b, INK, 0);
    }
}

bool drawJapanDecor(const Game& g, const Decor& d) {
    uint32_t h = hash2(int(d.x), int(d.y));
    float t = g.time;
    g_save(); g_translate(d.x, d.y);
    bool known = true;
    switch (d.kind) {
        case 'S': drawRyokan(t); break;
        case 'T': drawMaple(t, h); break;
        case 'm': drawTempleHall(); break;
        case 'a': drawPagoda(); break;
        case 'y': drawTahoto(); break;
        case 'W': drawCastle(); break;
        case 'V': drawTokyoTower(t); break;
        case 'J': drawLanternRack(t); break;
        case 'L': drawStoneLantern(); break;
        case 'K': drawCedar(h); break;
        case 'q': drawRock(h); break;
        case 'X': drawMonk(g, d); break;
        case 'g':  // a gate with a second one just behind it, so a row of them reads as a tunnel
            g_save(); g_translate(22, 0); drawTorii(0.93f, 0xd9481f, 0xa83418, false); g_restore();
            drawTorii(1, VERMILION, VERMILION_D, true);
            break;
        case 'O': {  // the great gate standing in the sea; d.y is the bottom of a water tile one row down
            g_translate(0, -2 * TILE + WATER_TOP + 16);
            for (int side = -1; side <= 1; side += 2)
                for (int k = -1; k <= 1; k += 2) {
                    g_fill(VERMILION_D); g_stroke(INK, 3); g_rrect(side * 61 + k * 22 - 6, -84, 12, 86, 3, BOTH);
                    g_rrect(side * 61 + (k < 0 ? -22 : 0), -62, 22, 8, 2, BOTH);
                }
            drawTorii(1.8f, VERMILION, VERMILION_D, false);
            g_fill(ROOF_D); g_stroke(INK, 3);
            g_path(); g_move(-112, -246); g_quad(0, -228, 112, -246); g_line(106, -256); g_quad(0, -240, -106, -256); g_close(); g_paint(BOTH);
            break;
        }
        default: known = false;
    }
    g_restore();
    return known;
}

// ------------------------------------------------------------ the red bridge

void drawRedBridge(const Bridge& b) {
    auto curve = [&](float dy, bool reverse) {
        if (!reverse) for (float x = b.x0; x <= b.x1; x += 8) g_line(x, bridgeY(b, x) + dy);
        else for (float x = b.x1; x >= b.x0; x -= 8) g_line(x, bridgeY(b, x) + dy);
    };
    // dark plank fascia under the deck
    g_path(); g_move(b.x0, b.y + 10); curve(10, false); curve(34, true); g_close();
    g_fill(0x5a5560); g_stroke(INK, 3); g_paint(BOTH);
    g_stroke(0x3a3640, 1.6);
    for (float x = b.x0 + 12; x < b.x1 - 4; x += 13) { float y = bridgeY(b, x); g_path(); g_move(x, y + 12); g_line(x, y + 32); g_paint(STROKE); }
    // railing: posts and two rails
    for (float x = b.x0 + 6; x <= b.x1 - 4; x += 38) {
        float y = bridgeY(b, x);
        g_path(); g_move(x, y + 2); g_line(x, y - 34);
        g_stroke(INK, 9); g_paint(STROKE); g_stroke(VERMILION, 5); g_paint(STROKE);
    }
    for (float dy : {-17.f, -31.f}) {
        g_path(); g_move(b.x0, b.y + dy); curve(dy, false);
        g_stroke(INK, 8); g_paint(STROKE); g_stroke(VERMILION, 4.5); g_paint(STROKE);
        if (dy < -20) { g_stroke(0xff9a6a, 1.4); g_paint(STROKE); }
    }
    g_fill(VERMILION_D); g_stroke(INK, 3);
    g_rrect(b.x0 - 7, b.y - 46, 14, 50, 3, BOTH); g_rrect(b.x1 - 7, b.y - 46, 14, 50, 3, BOTH);
    g_fill(0x2f2a35); g_rrect(b.x0 - 9, b.y - 52, 18, 8, 3, BOTH); g_rrect(b.x1 - 9, b.y - 52, 18, 8, 3, BOTH);
    // deck
    g_path(); g_move(b.x0, b.y); curve(0, false); curve(11, true); g_close();
    g_lin(0, b.y - b.arch, 0, b.y + 11, 0xff6a3a, VERMILION_D); g_stroke(INK, 3.5); g_paint(BOTH);
}

// ------------------------------------------------------------ the room of lights (Tokyo)

// Strands of tiny lights hanging in a dark, mirrored room, slowly changing colour.
void drawLightRoom(const Game& g, bool front) {
    float f = g.fogAt(g.camX + VIEW_W / 2.f), t = g.time;
    if (f <= 0) return;
    static const int hues[] = {0x4f7dff, 0x7fe0ff, 0xb48cff, 0xff8ad6};
    float cyc = t * 0.12f;
    int base = mix(hues[int(cyc) % 4], hues[(int(cyc) + 1) % 4], cyc - std::floor(cyc));
    if (!front) { g_alpha(0.9 * f); g_fill(0x0a1250); g_rect(0, 0, VIEW_W, VIEW_H); }
    else { g_alpha(0.3 * f); g_fill(0x0a1a8a); g_rect(0, 0, VIEW_W, VIEW_H); }
    float par = front ? 1.15f : 0.6f, gap = front ? 92.f : 22.f, r = front ? 2.6f : 1.5f;
    float off = g.camX * par;
    for (int k = int(std::floor(off / gap)) - 1; k <= int((off + VIEW_W) / gap) + 1; k++) {
        uint32_t h = hash2(k, front ? 71 : 37);
        float x = k * gap + h % 12 - off;
        g_alpha((front ? 0.12 : 0.1) * f); g_fill(0xbfd6ff); g_rect(x - 0.5f, 0, 1, VIEW_H);
        for (float y = (h >> 4) % 14; y < VIEW_H; y += front ? 15 : 11) {
            uint32_t d = hash2(k * 131 + int(y), 5);
            float tw = 0.5f + 0.5f * std::sin(t * (2 + d % 5) + d % 100 + y * 0.02f - t * 1.5f);
            if (tw < 0.25f) continue;
            g_alpha(tw * f * (front ? 0.9f : 0.75f));
            g_fill(d % 7 == 0 ? 0xffffff : mix(base, 0xffffff, (d >> 8) % 40 / 100.f));
            g_circle(x, y, r * (0.7f + tw * 0.5f), FILL);
        }
    }
    g_alpha(1);
}

// ------------------------------------------------------------ background

static void drawShinkansen(float x, float y, int stripe) {
    const float len = 560;
    g_fill(0xf7f9fb);
    g_path(); g_move(x, y - 21); g_line(x + len - 120, y - 21);
    g_cubic(x + len - 70, y - 21, x + len - 30, y - 9, x + len, y - 3);   // the long nose
    g_quad(x + len - 2, y, x + len - 14, y); g_line(x, y); g_close(); g_paint(FILL);
    g_fill(stripe); g_rect(x, y - 9, len - 46, 2.5f);
    g_fill(0x2f3f58);
    for (float wx = x + 14; wx < x + len - 150; wx += 13) { if (int((wx - x) / 13) % 9 == 8) continue; g_rect(wx, y - 17, 8, 5); }
    g_path(); g_move(x + len - 112, y - 19); g_quad(x + len - 84, y - 18, x + len - 66, y - 11); g_line(x + len - 108, y - 11); g_close(); g_paint(FILL);
    g_fill(0xffe07a); g_ellipse(x + len - 24, y - 4.5f, 5, 1.8f, 0.1f, FILL);
    g_fill(0xc9d2dc); g_rect(x, y - 2, len - 16, 2);
}

void drawJapanBackground(const Game& g) {
    float t = g.time;
    float maxCam = g.W * TILE - VIEW_W, prog = g.camX / maxCam;
    float dusk = std::clamp((prog - 0.70f) / 0.2f, 0.f, 1.f);       // evening falls on the way into Tokyo
    float vy = g.H * TILE - VIEW_H - g.camY;
    float farUp = std::min(vy * 0.10f, 110.f), midUp = std::min(vy * 0.20f, 200.f), nearUp = std::min(vy * 0.34f, 330.f);
    g_lin3(0, 0, 0, VIEW_H, mix(0x2f7fe0, 0x6f8fd0, dusk), mix(0x86c4f7, 0xf0b4c0, dusk), mix(0xe3f2ff, 0xfbe0b4, dusk));
    g_rect(0, 0, VIEW_W, VIEW_H);
    for (int i = 0; i < 5; i++) {  // a few small clouds
        uint32_t h = hash2(i, 808);
        float x = std::fmod(float(h % 1400) - g.camX * 0.05f + t * 4, 1400.f); if (x < 0) x += 1400;
        float y = 50 + (h >> 11) % 120, s = 0.7f + (h >> 20) % 50 / 100.f;
        g_alpha(0.9); g_fill(mix(0xffffff, 0xffd9c9, dusk));
        g_ellipse(x - 200, y, 46 * s, 13 * s, 0, FILL); g_ellipse(x - 170, y - 9 * s, 30 * s, 13 * s, 0, FILL); g_alpha(1);
    }

    // Mount Fuji, seen from the Izu side at the start of the journey
    float fx = 640 - g.camX * 0.11f, fb = 404 + farUp;
    if (fx > -420) {
        g_path(); g_move(fx - 400, fb); g_quad(fx - 130, fb - 110, fx - 26, fb - 262); g_quad(fx, fb - 270, fx + 26, fb - 262);
        g_quad(fx + 130, fb - 110, fx + 400, fb); g_close();
        g_lin(0, fb - 270, 0, fb, 0x5f79ad, 0x9bb6dc); g_paint(FILL);
        g_fill(0xffffff); g_path(); g_move(fx - 26, fb - 262); g_quad(fx, fb - 270, fx + 26, fb - 262);
        g_line(fx + 84, fb - 176); g_line(fx + 62, fb - 190); g_line(fx + 50, fb - 160); g_line(fx + 30, fb - 186); g_line(fx + 14, fb - 150);
        g_line(fx - 4, fb - 184); g_line(fx - 24, fb - 156); g_line(fx - 40, fb - 188); g_line(fx - 60, fb - 166); g_line(fx - 80, fb - 180);
        g_close(); g_paint(FILL);
    }
    // far hills
    g_lin(0, 330 + farUp, 0, VIEW_H, mix(0x7fa6c9, 0x8f8fb8, dusk), mix(0xb4d0e3, 0xd9b9c6, dusk));
    g_path(); g_move(0, VIEW_H);
    for (int sx = 0; sx <= VIEW_W + 8; sx += 8) g_line(sx, hillY(sx + g.camX * 0.1f, 372 + farUp, 26, 0.0046f, 1.3f));
    g_line(VIEW_W, VIEW_H); g_close(); g_paint(FILL);

    // Tokyo's towers slide into view at the end of the journey
    {
        static const float bld[][3] = {{40, 250, 74}, {130, 330, 86}, {236, 290, 70}, {318, 360, 84}, {420, 240, 60}, {500, 310, 92},
                                       {610, 270, 64}, {690, 350, 78}, {790, 230, 70}, {872, 300, 80}, {950, 262, 66}};
        float shift = (maxCam - g.camX) * 0.26f, by = 470 + midUp;
        int n = 0;
        for (auto& b : bld) {
            float x = b[0] + shift - 40, w = b[2], hgt = b[1];
            n++;
            if (x > VIEW_W + 20 || x + w < -20) continue;
            int wall = mix(n % 3 == 0 ? 0x5f6f8a : (n % 3 == 1 ? 0xb9c4d6 : 0x8794ab), 0x6f6f98, dusk * 0.5f);
            if (n == 9) {  // the cocoon-shaped tower
                g_fill(wall); g_ellipse(x + w / 2, by - hgt * 0.5f, w * 0.42f, hgt * 0.52f, 0, FILL);
                g_stroke(0xf2f5fa, 1.6);
                for (int k = -3; k <= 3; k++) { g_path(); g_move(x + w / 2 + k * 9, by - hgt); g_quad(x + w / 2 + k * 16 + 20, by - hgt * 0.5f, x + w / 2 + k * 9, by); g_move(x + w / 2 + k * 9, by - hgt); g_quad(x + w / 2 + k * 16 - 20, by - hgt * 0.5f, x + w / 2 + k * 9, by); g_paint(STROKE); }
                continue;
            }
            g_fill(wall); g_rect(x, by - hgt, w, hgt + 80);
            g_fill(mix(wall, 0x1f2a44, 0.35f)); g_rect(x + w * 0.72f, by - hgt, w * 0.28f, hgt + 80);
            g_fill(mix(wall, 0xffffff, 0.25f)); g_rect(x + w * 0.2f, by - hgt - 14, w * 0.5f, 14);
            g_stroke(0xe0452a, 2); g_path(); g_move(x + w * 0.3f, by - hgt - 14); g_line(x + w * 0.3f, by - hgt - 40); g_paint(STROKE);
            for (float wy = by - hgt + 10; wy < by + 40; wy += 11)
                for (float wx = x + 6; wx < x + w * 0.7f - 4; wx += 9) {
                    uint32_t h = hash2(int(wx - shift) + n * 97, int(wy - midUp));
                    bool lit = h % 100 < dusk * 46;
                    g_fill(lit ? 0xffe9a0 : mix(wall, 0x1f2a44, 0.3f)); g_rect(wx, wy, 5, 6);
                }
        }
    }

    // mid: forested hills in autumn colours
    float off = g.camX * 0.2f, base = 400 + midUp;
    float cityK = std::clamp((prog - 0.80f) / 0.08f, 0.f, 1.f);
    if (cityK < 1) {
        g_alpha(1 - cityK);
        g_lin(0, base - 60, 0, VIEW_H, 0x3f7a58, 0x6fa878);
        g_path(); g_move(0, VIEW_H);
        for (int sx = 0; sx <= VIEW_W + 8; sx += 8) g_line(sx, hillY(sx + off, base, 34, 0.0062f, 2.0f));
        g_line(VIEW_W, VIEW_H); g_close(); g_paint(FILL);
        static const int patch[] = {0xd9772e, 0xc9402e, 0xe0a83a, 0x5a9a5a, 0x2f6a4c};
        for (int k = int(std::floor(off / 26)) - 1; k <= int((off + VIEW_W) / 26) + 1; k++) {
            uint32_t h = hash2(k, 23);
            float wx = k * 26.f + h % 18, y = hillY(wx, base, 34, 0.0062f, 2.0f) + 6 + (h >> 5) % 40;
            g_fill(patch[(h >> 12) % 5]); g_circle(wx - off, y, 9 + (h >> 16) % 8, FILL);
        }
        g_alpha(1);
    }

    // the Shinkansen line: a viaduct with a train passing every so often
    {
        float o2 = g.camX * 0.3f, y = 316 + midUp * 1.2f;  // high enough to clear the sea in the foreground
        g_fill(mix(0xaab2bd, 0x8f8fa8, dusk));
        for (int k = int(std::floor(o2 / 110)) - 1; k <= int((o2 + VIEW_W) / 110) + 1; k++) g_rect(k * 110.f - o2 - 5, y, 10, VIEW_H - y);
        g_rect(0, y - 2, VIEW_W, 9);
        g_stroke(mix(0x8a929d, 0x77778f, dusk), 1.4);
        for (int k = int(std::floor(o2 / 160)) - 1; k <= int((o2 + VIEW_W) / 160) + 1; k++) { float px = k * 160.f - o2; g_path(); g_move(px, y - 2); g_line(px, y - 44); g_line(px + 14, y - 44); g_paint(STROKE); }
        g_path(); g_move(0, y - 40); g_line(VIEW_W, y - 40); g_paint(STROKE);
        float tx = std::fmod(t * 520, 3600.f) - 900 - std::fmod(o2, 3600.f) * 0.0f;
        drawShinkansen(tx - (o2 - std::floor(o2 / 3600) * 3600) * 0.25f, y - 2, 0x2f5fb8);
    }

    // near: rice fields, farmhouses and pampas grass - giving way to city blocks in Tokyo
    off = g.camX * 0.42f; base = 452 + nearUp;
    for (int k = int(std::floor(off / 170)) - 1; k <= int((off + VIEW_W) / 170) + 1; k++) {
        uint32_t h = hash2(k, 61);
        float wx = k * 170.f + (h >> 4) % 90, x = wx - off, y = hillY(wx, base, 8, 0.008f, 0.5f) + 6;
        bool city = wx > g.W * TILE * 0.42f * 0.80f;
        if (city) {
            float w = 70 + (h >> 8) % 50, hg = 60 + (h >> 14) % 90;
            g_fill(mix(0x7f8aa3, 0x5f5f88, dusk)); g_rect(x - w / 2, y - hg, w, hg + 10);
            for (float wy = y - hg + 8; wy < y - 4; wy += 12) for (float wxx = x - w / 2 + 6; wxx < x + w / 2 - 8; wxx += 11) {
                g_fill(hash2(int(wxx + off), int(wy - nearUp)) % 100 < 20 + dusk * 50 ? 0xffe9a0 : 0x4a5674); g_rect(wxx, wy, 6, 7);
            }
        } else if (h % 3 != 0) {  // farmhouse with a tiled roof
            g_fill(0xf2eee2); g_rect(x - 26, y - 22, 52, 22);
            g_fill(0x4a5262); g_path(); g_move(x - 34, y - 20); g_quad(x - 20, y - 38, x - 10, y - 40); g_line(x + 10, y - 40); g_quad(x + 20, y - 38, x + 34, y - 20); g_close(); g_paint(FILL);
            g_fill(0x6a5444); g_rect(x - 6, y - 14, 12, 14);
        }
    }
    g_lin(0, base - 10, 0, VIEW_H, mix(0xd3dc55, 0xa9a85a, dusk), mix(0xa4c23c, 0x808a4a, dusk));
    g_path(); g_move(0, VIEW_H);
    for (int sx = 0; sx <= VIEW_W + 8; sx += 8) g_line(sx, hillY(sx + off, base, 8, 0.008f, 0.5f));
    g_line(VIEW_W, VIEW_H); g_close(); g_paint(FILL);
    g_stroke(mix(0x8fae32, 0x6f7a3a, dusk), 2);  // rows in the rice
    for (int row = 0; row < 5; row++) {
        g_path(); g_move(0, hillY(off, base, 8, 0.008f, 0.5f) + 14 + row * 17);
        for (int sx = 16; sx <= VIEW_W + 16; sx += 16) g_line(sx, hillY(sx + off, base, 8, 0.008f, 0.5f) + 14 + row * 17);
        g_paint(STROKE);
    }
    for (int k = int(std::floor(off / 58)) - 1; k <= int((off + VIEW_W) / 58) + 1; k++) {  // pampas grass
        uint32_t h = hash2(k, 303);
        if (h % 3) continue;
        float wx = k * 58.f + h % 30, x = wx - off, y = hillY(wx, base, 8, 0.008f, 0.5f) + 4, sw = std::sin(t * 1.4f + k) * 3;
        for (int i = -1; i <= 1; i++) {
            g_stroke(0xb9b06a, 1.6); g_path(); g_move(x + i * 3, y); g_quad(x + i * 5, y - 16, x + i * 9 + sw, y - 30); g_paint(STROKE);
            g_fill(0xf4ecd0); g_ellipse(x + i * 9 + sw, y - 34, 2.6f, 7, i * 0.3f, FILL);
        }
    }
}

const char* japanPlace(float x) {
    float tx = x / TILE;
    if (tx < 35) return "Izu Nagaoka";
    if (tx < 96) return "Kyoto";
    if (tx < 133) return "Kōyasan";
    if (tx < 159) return "Hiroshima";
    if (tx < 181) return "Himeji";
    return "Tokyo";
}
