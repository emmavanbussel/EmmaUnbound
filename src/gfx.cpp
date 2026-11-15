#include "gfx.hpp"
#include <emscripten.h>

EM_JS(void, g_init, (int w, int h), {
  var c = document.getElementById("canvas");
  var G = c.getContext("2d");
  var E = { c: c, G: G, w: w, h: h, cols: {} };
  E.col = function(v) {
    var s = E.cols[v];
    if (!s) { s = "#" + ("000000" + (v >>> 0).toString(16)).slice(-6); E.cols[v] = s; }
    return s;
  };
  E.rgba = function(v, a) {
    return "rgba(" + ((v >> 16) & 255) + "," + ((v >> 8) & 255) + "," + (v & 255) + "," + a + ")";
  };
  E.paint = function(m) { if (m & 1) G.fill(); if (m & 2) G.stroke(); };
  globalThis.EG = E;
});

EM_JS(void, g_begin, (), {
  var c = EG.c; var G = EG.G; var dpr = window.devicePixelRatio || 1;
  var r = c.getBoundingClientRect();
  var pw = Math.max(1, Math.round(r.width * dpr)); var ph = Math.max(1, Math.round(r.height * dpr));
  if (c.width !== pw || c.height !== ph) { c.width = pw; c.height = ph; }
  G.setTransform(pw / EG.w, 0, 0, ph / EG.h, 0, 0);
  G.lineJoin = "round"; G.lineCap = "round"; G.globalAlpha = 1;
});

EM_JS(void, g_fill, (int c), { EG.G.fillStyle = EG.col(c); });
EM_JS(void, g_stroke, (int c, double w), { EG.G.strokeStyle = EG.col(c); EG.G.lineWidth = w; });
EM_JS(void, g_alpha, (double a), { EG.G.globalAlpha = a; });

EM_JS(void, g_lin, (double x0, double y0, double x1, double y1, int c0, int c1), {
  var g = EG.G.createLinearGradient(x0, y0, x1, y1);
  g.addColorStop(0, EG.col(c0)); g.addColorStop(1, EG.col(c1));
  EG.G.fillStyle = g;
});
EM_JS(void, g_lin3, (double x0, double y0, double x1, double y1, int c0, int c1, int c2), {
  var g = EG.G.createLinearGradient(x0, y0, x1, y1);
  g.addColorStop(0, EG.col(c0)); g.addColorStop(0.55, EG.col(c1)); g.addColorStop(1, EG.col(c2));
  EG.G.fillStyle = g;
});
EM_JS(void, g_lin3a, (double x0, double y0, double x1, double y1, int c0, double a0, int c1, double a1, int c2, double a2), {
  var g = EG.G.createLinearGradient(x0, y0, x1, y1);
  g.addColorStop(0, EG.rgba(c0, a0)); g.addColorStop(0.45, EG.rgba(c1, a1)); g.addColorStop(1, EG.rgba(c2, a2));
  EG.G.fillStyle = g;
});
EM_JS(void, g_rad, (double x, double y, double r, int c0, double a0, int c1, double a1), {
  var g = EG.G.createRadialGradient(x, y, 0, x, y, r);
  g.addColorStop(0, EG.rgba(c0, a0)); g.addColorStop(1, EG.rgba(c1, a1));
  EG.G.fillStyle = g;
});

EM_JS(void, g_rect, (double x, double y, double w, double h), { EG.G.fillRect(x, y, w, h); });
EM_JS(void, g_rrect, (double x, double y, double w, double h, double r, int mode), {
  var G = EG.G; r = Math.min(r, w / 2, h / 2);
  G.beginPath();
  G.moveTo(x + r, y);
  G.arcTo(x + w, y, x + w, y + h, r); G.arcTo(x + w, y + h, x, y + h, r);
  G.arcTo(x, y + h, x, y, r); G.arcTo(x, y, x + w, y, r);
  G.closePath(); EG.paint(mode);
});
EM_JS(void, g_circle, (double x, double y, double r, int mode), {
  EG.G.beginPath(); EG.G.arc(x, y, Math.max(0, r), 0, 6.2831853); EG.paint(mode);
});
EM_JS(void, g_ellipse, (double x, double y, double rx, double ry, double rot, int mode), {
  EG.G.beginPath(); EG.G.ellipse(x, y, Math.max(0, rx), Math.max(0, ry), rot, 0, 6.2831853); EG.paint(mode);
});

EM_JS(void, g_path, (), { EG.G.beginPath(); });
EM_JS(void, g_move, (double x, double y), { EG.G.moveTo(x, y); });
EM_JS(void, g_line, (double x, double y), { EG.G.lineTo(x, y); });
EM_JS(void, g_quad, (double cx, double cy, double x, double y), { EG.G.quadraticCurveTo(cx, cy, x, y); });
EM_JS(void, g_cubic, (double a, double b, double c, double d, double x, double y), {
  EG.G.bezierCurveTo(a, b, c, d, x, y);
});
EM_JS(void, g_close, (), { EG.G.closePath(); });
EM_JS(void, g_paint, (int mode), { EG.paint(mode); });

EM_JS(void, g_save, (), { EG.G.save(); });
EM_JS(void, g_restore, (), { EG.G.restore(); });
EM_JS(void, g_translate, (double x, double y), { EG.G.translate(x, y); });
EM_JS(void, g_rotate, (double a), { EG.G.rotate(a); });
EM_JS(void, g_scale, (double x, double y), { EG.G.scale(x, y); });

EM_JS(void, g_text, (const char* s, double x, double y, double size, int align, int fill, int stroke, double sw), {
  var G = EG.G; var t = UTF8ToString(s);
  G.font = "900 " + size + "px ui-rounded, \"Arial Rounded MT Bold\", \"Baloo 2\", \"Nunito\", \"Trebuchet MS\", \"Verdana\", sans-serif";
  G.textAlign = align === 1 ? "center" : (align === 2 ? "right" : "left");
  if (sw > 0) { G.strokeStyle = EG.col(stroke); G.lineWidth = sw; G.strokeText(t, x, y); }
  G.fillStyle = EG.col(fill); G.fillText(t, x, y);
});
