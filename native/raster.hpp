#pragma once
// Death World 0.3 -- original C++20 mobile action RPG.
// No external runtime or asset dependencies. Android bridge is at the end.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace av {
constexpr int W = 640, H = 360, T = 24, MW = 72, MH = 52;
constexpr float PI = 3.14159265f;
using C = uint32_t;
constexpr C INK = 0xff0b141c, PANEL = 0xff16241f, EDGE = 0xff495b4d,
            GOLD = 0xffcdb886, WHITE = 0xffe7e3d6, TEAL = 0xff91b39b,
            RED = 0xffd16a65, DIM = 0xff9aac9d;
C *pix = nullptr;
// Optional software G-buffer normal target; null on the frozen Low path.
C *surfaceNormals = nullptr;
C *uiOverlayTarget = nullptr;
inline void trackUI(int x,int y,C color,int alpha=255) {
  if(!uiOverlayTarget||x<0||y<0||x>=W||y>=H||alpha<=0)return;
  alpha=std::min(255,alpha);
  if(alpha==255){uiOverlayTarget[y*W+x]=color|0xff000000;return;}
  C p=uiOverlayTarget[y*W+x];int inv=255-alpha;
  unsigned a=alpha+((p>>24)*inv)/255;
  unsigned r=(((color>>16)&255)*alpha+((p>>16)&255)*inv)/255;
  unsigned g=(((color>>8)&255)*alpha+((p>>8)&255)*inv)/255;
  unsigned b=((color&255)*alpha+(p&255)*inv)/255;
  uiOverlayTarget[y*W+x]=(a<<24)|(r<<16)|(g<<8)|b;
}
void rect(int x, int y, int w, int h, C c) {
  int a = std::max(0, x), b = std::max(0, y), e = std::min(W, x + w),
      f = std::min(H, y + h);
  if (e <= a)
    return;
  for (int j = b; j < f; j++)
    {
    std::fill(pix + j * W + a, pix + j * W + e, c);
    if(uiOverlayTarget)std::fill(uiOverlayTarget+j*W+a,uiOverlayTarget+j*W+e,c|0xff000000);
    if (surfaceNormals) std::fill(surfaceNormals + j * W + a, surfaceNormals + j * W + e, 0x008080ff);
    }
}
void point(int x, int y, C c) {
  if (x >= 0 && x < W && y >= 0 && y < H) {
    pix[y * W + x] = c;
    if(uiOverlayTarget)trackUI(x,y,c);
    if(surfaceNormals)surfaceNormals[y*W+x]=0x008080ff;
  }
}
void line(int x, int y, int xx, int yy, C c) {
  int dx = std::abs(xx - x), sx = x < xx ? 1 : -1, dy = -std::abs(yy - y),
      sy = y < yy ? 1 : -1, er = dx + dy;
  for (;;) {
    point(x, y, c);
    if (x == xx && y == yy)
      break;
    int e = 2 * er;
    if (e >= dy) {
      er += dy;
      x += sx;
    }
    if (e <= dx) {
      er += dx;
      y += sy;
    }
  }
}
void box(int x, int y, int w, int h, C fill, C border = EDGE) {
  rect(x, y, w, h, fill);
  rect(x, y, w, 1, border);
  rect(x, y + h - 1, w, 1, border);
  rect(x, y, 1, h, border);
  rect(x + w - 1, y, 1, h, border);
}
void circle(int x, int y, int r, C c, bool fill = false) {
  for (int j = -r; j <= r; j++)
    for (int i = -r; i <= r; i++) {
      int d = i * i + j * j;
      if (d <= r * r && (fill || d >= (r - 1) * (r - 1)))
        point(x + i, y + j, c);
    }
}
void ellipse(int x, int y, int rx, int ry, C c) {
  for (int j = -ry; j <= ry; j++)
    for (int i = -rx; i <= rx; i++)
      if (i * i * ry * ry + j * j * rx * rx <= rx * rx * ry * ry)
        point(x + i, y + j, c);
}
void dim() {
  for (int i = 0; i < W * H; i++) {
    C p = pix[i];
    pix[i] = 0xff000000 | ((p & 0x00fefefe) >> 1);
  }
}
const char *letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+:/!.?<>%()=#";
const uint8_t glyphs[][7] = {
    {14, 17, 17, 31, 17, 17, 17}, {30, 17, 17, 30, 17, 17, 30},
    {14, 17, 16, 16, 16, 17, 14}, {30, 17, 17, 17, 17, 17, 30},
    {31, 16, 16, 30, 16, 16, 31}, {31, 16, 16, 30, 16, 16, 16},
    {14, 17, 16, 23, 17, 17, 15}, {17, 17, 17, 31, 17, 17, 17},
    {31, 4, 4, 4, 4, 4, 31},      {7, 2, 2, 2, 18, 18, 12},
    {17, 18, 20, 24, 20, 18, 17}, {16, 16, 16, 16, 16, 16, 31},
    {17, 27, 21, 21, 17, 17, 17}, {17, 25, 21, 19, 17, 17, 17},
    {14, 17, 17, 17, 17, 17, 14}, {30, 17, 17, 30, 16, 16, 16},
    {14, 17, 17, 17, 21, 18, 13}, {30, 17, 17, 30, 20, 18, 17},
    {15, 16, 16, 14, 1, 1, 30},   {31, 4, 4, 4, 4, 4, 4},
    {17, 17, 17, 17, 17, 17, 14}, {17, 17, 17, 17, 17, 10, 4},
    {17, 17, 17, 21, 21, 21, 10}, {17, 17, 10, 4, 10, 17, 17},
    {17, 17, 10, 4, 4, 4, 4},     {31, 1, 2, 4, 8, 16, 31},
    {14, 17, 19, 21, 25, 17, 14}, {4, 12, 4, 4, 4, 4, 14},
    {14, 17, 1, 2, 4, 8, 31},     {30, 1, 1, 14, 1, 1, 30},
    {2, 6, 10, 18, 31, 2, 2},     {31, 16, 16, 30, 1, 1, 30},
    {14, 16, 16, 30, 17, 17, 14}, {31, 1, 2, 4, 8, 8, 8},
    {14, 17, 17, 14, 17, 17, 14}, {14, 17, 17, 15, 1, 1, 14},
    {0, 0, 0, 31, 0, 0, 0},       {0, 4, 4, 31, 4, 4, 0},
    {0, 4, 4, 0, 4, 4, 0},        {1, 2, 2, 4, 8, 8, 16},
    {4, 4, 4, 4, 4, 0, 4},        {0, 0, 0, 0, 0, 6, 6},
    {14, 17, 1, 2, 4, 0, 4},      {2, 4, 8, 16, 8, 4, 2},
    {8, 4, 2, 1, 2, 4, 8},        {25, 25, 2, 4, 8, 19, 19},
    {2, 4, 8, 8, 8, 4, 2},        {8, 4, 2, 2, 2, 4, 8},
    {0, 0, 31, 0, 31, 0, 0},      {10, 10, 31, 10, 31, 10, 10}};
bool ui7Active();
int ui7TextWidth(const std::string&,int);
void ui7WorldText(int,int,const std::string&,C,int);
int tw(const std::string &s, int sc = 1) { if(ui7Active())return ui7TextWidth(s,sc);return int(s.size()) * 6 * sc - sc; }
void text(int x, int y, const std::string &s, C c = WHITE, int sc = 1) {
  if(ui7Active()){ui7WorldText(x,y,s,c,sc);return;}
  for (char ch : s) {
    if (ch >= 'a' && ch <= 'z')
      ch -= 32;
    const char *p = strchr(letters, ch);
    if (p)
      for (int r = 0; r < 7; r++)
        for (int b = 0; b < 5; b++)
          if (glyphs[p - letters][r] & (16 >> b))
            rect(x + b * sc, y + r * sc, sc, sc, c);
    x += 6 * sc;
  }
}
void center(int x, int y, const std::string &s, C c = WHITE, int sc = 1) {
  text(x - tw(s, sc) / 2, y, s, c, sc);
}
std::string num(int n) { return std::to_string(n); }
float len(float x, float y) { return std::sqrt(x * x + y * y); }
float clamp(float a, float l, float h) { return std::max(l, std::min(h, a)); }
uint32_t hash(uint32_t a) {
  a ^= a >> 16;
  a *= 0x7feb352d;
  a ^= a >> 15;
  a *= 0x846ca68b;
  return a ^ (a >> 16);
}
uint32_t rng = 76143;
int rnd(int n) {
  rng = hash(rng + 0x9e3779b9);
  return n ? int(rng % uint32_t(n)) : 0;
}

} // namespace av
