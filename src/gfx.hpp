#pragma once
// Thin drawing layer: C++ calls into the browser's Canvas 2D API.
// Colours are 0xRRGGBB integers. "mode" is FILL, STROKE or BOTH.

enum { FILL = 1, STROKE = 2, BOTH = 3 };

extern "C" {
void g_init(int viewW, int viewH);
void g_begin();  // start of frame: fit canvas to window, reset state

void g_fill(int color);
void g_stroke(int color, double width);
void g_alpha(double a);
void g_lin(double x0, double y0, double x1, double y1, int c0, int c1);            // fill = linear gradient
void g_lin3(double x0, double y0, double x1, double y1, int c0, int c1, int c2);  // 3-stop gradient
// vertical-ish gradient with three stops that each have their own opacity (used for the northern lights)
void g_lin3a(double x0, double y0, double x1, double y1, int c0, double a0, int c1, double a1, int c2, double a2);
void g_rad(double x, double y, double r, int c0, double a0, int c1, double a1);   // fill = radial glow

void g_rect(double x, double y, double w, double h);
void g_rrect(double x, double y, double w, double h, double r, int mode);
void g_circle(double x, double y, double r, int mode);
void g_ellipse(double x, double y, double rx, double ry, double rot, int mode);

void g_path();
void g_move(double x, double y);
void g_line(double x, double y);
void g_quad(double cx, double cy, double x, double y);
void g_cubic(double c1x, double c1y, double c2x, double c2y, double x, double y);
void g_close();
void g_paint(int mode);

void g_save();
void g_restore();
void g_translate(double x, double y);
void g_rotate(double a);
void g_scale(double sx, double sy);

// align: 0 left, 1 centre, 2 right. strokeW 0 = no outline.
void g_text(const char* s, double x, double y, double size, int align, int fill, int stroke, double strokeW);
}
