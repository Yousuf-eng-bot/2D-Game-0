#pragma once
#include "combat.hpp"
namespace av {
bool mediumEnabled();
bool graphicsCoolingBudget();
void drawMediumPlayer(int,int,float,bool);
void drawMediumEnemy(const Enemy &);
void drawMediumTree(const Prop &);
int sx(float x) { return int(std::round(x - g.camx)); }
int sy(float y) { return int(std::round(y - g.camy)); }
int heightLocal(float x, float y) {
  int xx = int(std::floor(x / T)), yy = int(std::floor(y / T));
  return g.openWorld && xx >= 0 && xx < MW && yy >= 0 && yy < MH
             ? j.heights[yy * MW + xx]
             : 0;
}
int vistaElevation(float x, float y) { return heightLocal(x, y) * 6; }
int syAt(float x, float y) { return sy(y) - vistaElevation(x, y); }
void drawVistaTerrain();
void vistaUnderstory(const Prop &p);

C mix(C a, C b, int t) {
  int r = ((a >> 16 & 255) * (255 - t) + (b >> 16 & 255) * t) / 255,
      g = ((a >> 8 & 255) * (255 - t) + (b >> 8 & 255) * t) / 255,
      bl = ((a & 255) * (255 - t) + (b & 255) * t) / 255;
  return 0xff000000 | (r << 16) | (g << 8) | bl;
}
void shade(int x, int y, int w, int h, C c, int alpha) {
  for (int j = std::max(0, y); j < std::min(H, y + h); j++)
    for (int i = std::max(0, x); i < std::min(W, x + w); i++)
      {pix[j * W + i] = mix(pix[j * W + i], c, alpha);
       if(uiOverlayTarget)trackUI(i,j,c,alpha); }
}
void thickLine(int x, int y, int xx, int yy, C c, int r = 1) {
  int dx = std::abs(xx - x), ss = x < xx ? 1 : -1, dy = -std::abs(yy - y),
      tt = y < yy ? 1 : -1, e = dx + dy;
  for (;;) {
    rect(x - r, y - r, r * 2 + 1, r * 2 + 1, c);
    if (x == xx && y == yy)
      break;
    int ee = e * 2;
    if (ee >= dy) {
      e += dy;
      x += ss;
    }
    if (ee <= dx) {
      e += dx;
      y += tt;
    }
  }
}
void poly(std::initializer_list<std::pair<int, int>> ps, C c) {
  std::vector<std::pair<int, int>> v(ps);
  int ymin = H, ymax = 0;
  for (auto [x, y] : v) {
    ymin = std::min(ymin, y);
    ymax = std::max(ymax, y);
  }
  for (int y = std::max(0, ymin); y <= std::min(H - 1, ymax); y++) {
    std::array<int, 32> cross{};
    int n = 0;
    for (size_t i = 0; i < v.size(); i++) {
      auto [x1, y1] = v[i];
      auto [x2, y2] = v[(i + 1) % v.size()];
      if ((y1 <= y && y2 > y) || (y2 <= y && y1 > y))
        cross[n++] = int(x1 + (y - y1) * float(x2 - x1) / (y2 - y1));
    }
    std::sort(cross.begin(), cross.begin() + n);
    for (int i = 0; i + 1 < n; i += 2)
      rect(cross[i], y, cross[i + 1] - cross[i] + 1, 1, c);
  }
}
void arc(int x, int y, int r, float a, float b, C c, int width = 2) {
  for (float t = a; t <= b; t += .022f) {
    int xx = x + int(std::cos(t) * r), yy = y + int(std::sin(t) * r);
    rect(xx - width / 2, yy - width / 2, width, width, c);
  }
}
void ring(float x, float y, int r, C c) { circle(sx(x), syAt(x, y), r, c); }
void groundShadow(int x, int y, int rx, int ry) {
  if (g.openWorld && (g.scene == PLAY || g.scene == DEAD))
    return;
  for (int j = -ry; j <= ry; j++)
    for (int i = -rx; i <= rx; i++)
      if (i * i * ry * ry + j * j * rx * rx <= rx * rx * ry * ry &&
          x + i >= 0 && x + i < W && y + j >= 0 && y + j < H)
        pix[(y + j) * W + x + i] =
            mix(pix[(y + j) * W + x + i], 0xff224c3c, 85);
}
void leafCluster(int x, int y, int r, C c, bool fade) {
  for (int j = -r; j <= r; j++)
    for (int i = -r; i <= r; i++)
      if (i * i + j * j < r * r && (!fade || (i + j) % 2 == 0)) {
        int xx = x + i, yy = y + j;
        if (xx >= 0 && xx < W && yy >= 0 && yy < H)
          pix[yy * W + xx] = c;
      }
}
void treeArt(int x, int y, int variant, bool fade = false, int region = -1) {
  if (region < 0)
    region = g.scene == HUB ? 0 : g.world;
  int sway = int(std::sin(g.time * 1.3f + x * .02f) * 2);
  groundShadow(x + 7, y + 4, 29, 9);
  if (region == 4) {
    thickLine(x, y, x - 2, y - 51, 0xff493d48, 5);
    thickLine(x - 1, y - 29, x - 19, y - 48, 0xff493d48, 3);
    thickLine(x, y - 38, x + 17, y - 62, 0xff493d48, 3);
    line(x - 3, y - 40, x - 1, y - 8, 0xffb67962);
    rect(x - 18, y - 49, 3, 3, 0xffec9c69);
    return;
  }
  if (region == 2) {
    rect(x - 4, y - 48, 8, 48, 0xff69686b);
    for (int k = 0; k < 4; k++) {
      int yy = y - 15 - k * 14, rr = 29 - k * 5;
      poly({{x + sway, yy - 32},
            {x - rr + sway, yy + 5},
            {x + rr + sway, yy + 5}},
           0xff487d83);
      poly({{x + sway, yy - 33},
            {x - rr + sway + 3, yy - 2},
            {x + rr + sway - 4, yy - 2}},
           0xffdcecee);
    }
    return;
  }
  if (region == 1 && variant == 0) {
    C dark = 0xff447758, light = 0xff88a569;
    box(x - 5, y - 45, 10, 46, dark, 0xff315942);
    rect(x - 2, y - 43, 3, 39, light);
    thickLine(x - 4, y - 17, x - 18, y - 17, dark, 4);
    thickLine(x - 18, y - 17, x - 18, y - 33, dark, 4);
    line(x - 20, y - 32, x - 20, y - 18, light);
    thickLine(x + 4, y - 28, x + 17, y - 28, dark, 3);
    thickLine(x + 17, y - 28, x + 17, y - 40, dark, 3);
    rect(x - 4, y - 47, 8, 4, 0xffa8b675);
    return;
  }
  rect(x - 6, y - 48, 12, 47, 0xff6a5138);
  rect(x - 3, y - 47, 4, 45, 0xffac8250);
  rect(x + 4, y - 41, 2, 38, 0xff493e32);
  thickLine(x, y - 31, x - 18 + sway, y - 55, 0xff785a3a, 2);
  thickLine(x, y - 36, x + 15 + sway, y - 62, 0xff785a3a, 2);
  poly({{x - 9, y + 1}, {x - 5, y - 11}, {x + 5, y - 9}, {x + 11, y + 1}},
       0xff78613e);
  C deep = region ? 0xff596b3b : 0xff28694e,
    mid = region ? 0xff8e9a48 : 0xff4b9456,
    light = region ? 0xffc7bc64 : 0xff8dbb68,
    highlight = region ? 0xffe4cd83 : 0xffb6d581;
  if (region == 3) {
    deep = 0xff294e4e;
    mid = 0xff487c70;
    light = 0xff83a898;
    highlight = 0xffb5c4a0;
  }
  if (variant == 2 && region == 0) {
    for (int k = 0; k < 4; k++) {
      int yy = y - 30 - k * 13, rr = 29 - k * 5;
      poly({{x + sway, yy - 35},
            {x + sway - rr, yy + 6},
            {x + sway + rr, yy + 6}},
           deep);
      poly({{x + sway, yy - 33},
            {x + sway - rr + 4, yy + 1},
            {x + sway + rr - 8, yy + 1}},
           mid);
      line(x + sway - rr + 7, yy - 1, x + sway - 3, yy - 1, light);
    }
    return;
  }
  leafCluster(x + sway, y - 54, 31, deep, fade);
  leafCluster(x - 21 + sway, y - 48, 20, deep, fade);
  leafCluster(x + 24 + sway, y - 49, 21, deep, fade);
  leafCluster(x - 13 + sway, y - 64, 22, mid, fade);
  leafCluster(x + 15 + sway, y - 63, 24, mid, fade);
  leafCluster(x - 24 + sway, y - 51, 14, mid, fade);
  leafCluster(x - 15 + sway, y - 70, 15, light, fade);
  leafCluster(x + 8 + sway, y - 75, 15, light, fade);
  leafCluster(x + 26 + sway, y - 59, 12, light, fade);
  for (int k = 0; k < 20; k++) {
    int xx = x - 27 + hash(k + x) % 53 + sway, yy = y - 78 + hash(k + y) % 31;
    if (!fade || k % 2 == 0) {
      rect(xx, yy, 4, 2, k % 4 ? light : highlight);
      rect(xx + 1, yy - 2, 3, 2, k % 4 ? mid : light);
    }
  }
}
void rockArt(int x, int y, int v, bool desert) {
  groundShadow(x + 6, y + 2, 17, 6);
  C a = desert ? 0xffa87553 : 0xff737f79, b = desert ? 0xffd7aa72 : 0xffa9b4a1,
    c = desert ? 0xffe9c48b : 0xffd0d7b5;
  int h = 17 + v * 7;
  poly({{x - 17, y - 2},
        {x - 13, y - h + 5},
        {x - 5, y - h - 5},
        {x + 11, y - h + 1},
        {x + 17, y - 2}},
       a);
  poly({{x - 13, y - h + 5},
        {x - 5, y - h - 5},
        {x + 11, y - h + 1},
        {x + 4, y - 4},
        {x - 14, y - 4}},
       b);
  line(x - 12, y - h + 5, x - 5, y - h - 5, c);
  line(x - 5, y - h - 5, x + 11, y - h + 1, c);
  line(x + 5, y - h, x + 1, y - 8, a);
  rect(x - 14, y, 31, 3, a);
}
void flame(int x, int y, C col = GOLD) {
  int t = int(g.time * 10) % 3;
  poly({{x - 5, y},
        {x - 6, y - 8},
        {x - 1, y - 15 - t},
        {x + 2, y - 8},
        {x + 5, y - 10},
        {x + 6, y},
        {x, y + 3}},
       col);
  rect(x - 2, y - 7, 4, 8, 0xffffecc1);
}
void propArt(const Prop &p) {
  if (g.openWorld && (p.kind == 20 || p.kind == 25)) {
    drawNewPlants(p);
    return;
  }
  if (g.openWorld && p.kind == 0) {
    if (mediumEnabled()) drawMediumTree(p); else drawNewTree(p);
    return;
  }
  if (g.openWorld && p.kind == 1) {
    drawNewRock(p);
    return;
  }
  if (g.openWorld && p.kind == 3) {
    drawUnderstory(p);
    return;
  }
  if (p.kind >= 20) {
    drawLifeProp(p);
    return;
  }
  int x = sx(p.x), y = syAt(p.x, p.y);
  if (x < -90 || x > W + 90 || y < -15 || y > H + 115)
    return;
  int biome = p.biome >= 0 ? p.biome : (g.scene == HUB ? 0 : g.world);
  bool desert = biome == 1;
  if (p.kind == 0) {
    bool fade = g.py < p.y + 5 && g.py > p.y - 90 && std::abs(g.px - p.x) < 45;
    treeArt(x, y, p.variant, fade, biome);
  }
  if (p.kind == 1) {
    rockArt(x, y, p.variant, desert);
    if (biome == 2) {
      line(x - 10, y - 22, x + 8, y - 22, WHITE);
      rect(x - 6, y - 24, 9, 3, 0xffdfedf0);
    }
    if (biome == 4) {
      line(x - 6, y - 23, x + 1, y - 12, 0xffeaa078);
      line(x + 1, y - 12, x - 3, y - 5, 0xffd27558);
    }
  }
  if (p.kind == 10) {
    box(x - 12, y - 12, 25, 15, 0xff795846, GOLD);
    rect(x - 10, y - 15, 21, 5, p.open ? 0xff423b36 : 0xffcaa76f);
    rect(x - 2, y - 8, 4, 5, p.open ? EDGE : GOLD);
  }
  if (p.kind == 11) {
    rect(x - 2, y - 27, 4, 28, 0xff5c4e47);
    poly({{x + 2, y - 26}, {x + 25, y - 22}, {x + 2, y - 13}}, p.open ? TEAL
                                                               : p.variant
                                                                   ? RED
                                                                   : GOLD);
  }
  if (p.kind == 12) {
    groundShadow(x, y, 25, 9);
    for (int k = 0; k < 6; k++) {
      float a = k * PI / 3;
      circle(x + int(std::cos(a) * 15), y + int(std::sin(a) * 6), 4, 0xff909a92,
             true);
    }
    flame(x, y - 5);
    center(x, y + 18, "ORIGIN CAMP", GOLD);
  }

  if (p.kind == 3) {
    C c = biome == 4   ? 0xff9d7c69
          : biome == 2 ? 0xffa5bab4
          : desert     ? 0xffa19e57
                       : 0xff568c4c;
    for (int k = 0; k < 5; k++) {
      int d = k * 3 - 6, s = int(std::sin(g.time * 2 + p.x + k));
      line(x + d, y, x + d + s - 2, y - 5 - k % 3, c);
      line(x + d, y, x + d + s + 3, y - 8 + k % 3, c);
    }
    if (p.variant == 1) {
      rect(x - 3, y - 7, 3, 3, desert ? 0xffe28e74 : 0xffe9cf92);
      rect(x + 4, y - 8, 3, 3, 0xffd5a7cd);
    }
  }
  if (p.kind == 4) {
    rect(x - 12, y - 5, 25, 6, 0xff886947);
    rect(x - 10, y - 7, 22, 3, 0xffb5996b);
    ellipse(x - 12, y - 3, 3, 4, 0xffceb586);
    line(x - 7, y - 5, x + 5, y - 5, 0xff715e41);
  }
  if (p.kind == 5) {
    groundShadow(x, y, 21, 6);
    box(x - 18, y - 13, 36, 14, desert ? 0xffb49875 : 0xff9aab89, 0xff627864);
    box(x - 12, y - 35, 24, 23, 0xffc3c4a1, 0xff7d917c);
    rect(x - 15, y - 39, 30, 5, 0xffe6d8ac);
    bool lit = g.rooms[p.variant];
    circle(x, y - 49, 10, lit ? TEAL : 0xff6d7e71);
    if (lit) {
      flame(x, y - 40, 0xff9dde9a);
      circle(x, y - 49, 14, 0xffcaf7b3);
    } else {
      poly({{x, y - 59}, {x + 7, y - 49}, {x, y - 41}, {x - 7, y - 49}},
           0xff89a49a);
    }
    return;
  }
  if (p.kind == 6) {
    groundShadow(x, y + 2, 16, 5);
    box(x - 13, y - 14, 27, 16, 0xff977044, 0xff594d37);
    rect(x - 13, y - 14, 27, 4, 0xffd4b367);
    rect(x - 10, y - 11, 3, 12, 0xffd4b367);
    rect(x + 7, y - 11, 3, 12, 0xffd4b367);
    rect(x - 2, y - 9, 5, 5, GOLD);
    if (p.open) {
      box(x - 13, y - 24, 27, 10, 0xffb3975d, 0xff594d37);
      rect(x - 10, y - 3, 21, 4, INK);
    } else if (g.rooms[p.variant]) {
      int b = int(std::sin(g.time * 3) * 2);
      text(x - 2, y - 28 + b, "!", GOLD);
    }
    return;
  }
  if (p.kind == 7 || p.kind == 8) {
    groundShadow(x + 5, y + 2, 19, 6);
    box(x - 13, y - 64, 26, 65, desert ? 0xffbb9064 : 0xff899b85, 0xff536e62);
    rect(x - 9, y - 60, 5, 54, desert ? 0xffe4bd83 : 0xffb6c4a4);
    rect(x + 6, y - 60, 4, 56, desert ? 0xff8e674d : 0xff687f6f);
    rect(x - 18, y - 68, 36, 6, desert ? 0xffe7c898 : 0xffd4d7b5);
    rect(x - 17, y - 3, 34, 6, desert ? 0xffcfad7c : 0xffa5b697);
    if (p.kind == 8) {
      circle(x, y - 34, 11, p.variant ? GOLD : TEAL);
      text(x - 2, y - 38, p.variant ? "2" : "1", WHITE);
    }
  }
  if (p.kind == 9) {
    groundShadow(x, y, 24, 7);
    box(x - 22, y - 18, 44, 18, 0xff795840, 0xff523f34);
    rect(x - 24, y - 23, 48, 8, 0xffbdb694);
    rect(x - 13, y - 30, 29, 7, 0xff607877);
    flame(x + 15, y - 20, 0xffefb259);
  }
}
void weaponArt(int x, int y, int weapon, float a, float stretch = 0,
               C tint = 0) {
  C steel = tint ? tint : 0xffd7e8d7, edge = tint ? tint : 0xfff5f5d5;
  float ux = std::cos(a), uy = std::sin(a), vx = -uy, vy = ux;
  if (weapon == BOW) {
    int bx = x + int(ux * 8), by = y + int(uy * 8);
    for (float t = -1.15f; t < 1.16f; t += .08f) {
      int xx = bx + int(std::cos(a + t) * 13),
          yy = by + int(std::sin(a + t) * 13);
      rect(xx, yy, 2, 2, 0xffd6ab65);
    }
    int x1 = bx + int(std::cos(a - 1.15f) * 13),
        y1 = by + int(std::sin(a - 1.15f) * 13),
        x2 = bx + int(std::cos(a + 1.15f) * 13),
        y2 = by + int(std::sin(a + 1.15f) * 13);
    line(x1, y1, x - int(ux * stretch * 8), y - int(uy * stretch * 8),
         0xffe9e5bb);
    line(x2, y2, x - int(ux * stretch * 8), y - int(uy * stretch * 8),
         0xffe9e5bb);
    line(x - int(ux * 12), y - int(uy * 12), x + int(ux * 25), y + int(uy * 25),
         steel);
    return;
  }
  thickLine(x - int(ux * 6), y - int(uy * 6),
            x + int(ux * (weapon == AXE ? 26 : 10)),
            y + int(uy * (weapon == AXE ? 26 : 10)), 0xff825d3b, 1);
  if (weapon == SWORD) {
    thickLine(x + int(ux * 7), y + int(uy * 7), x + int(ux * 30),
              y + int(uy * 30), steel, 1);
    line(x + int(ux * 8 + vx), y + int(uy * 8 + vy), x + int(ux * 32 + vx),
         y + int(uy * 32 + vy), edge);
    thickLine(x + int(ux * 5 - vx * 6), y + int(uy * 5 - vy * 6),
              x + int(ux * 5 + vx * 6), y + int(uy * 5 + vy * 6), GOLD, 1);
  } else {
    int bx = x + int(ux * 23), by = y + int(uy * 23);
    poly({{bx + int(vx * 13 - ux * 5), by + int(vy * 13 - uy * 5)},
          {bx + int(vx * 14 + ux * 7), by + int(vy * 14 + uy * 7)},
          {bx + int(ux * 8), by + int(uy * 8)},
          {bx - int(vx * 11) + int(ux * 5), by - int(vy * 11) + int(uy * 5)},
          {bx - int(vx * 11) - int(ux * 4), by - int(vy * 11) - int(uy * 4)}},
         steel);
    line(bx + int(vx * 14 + ux * 7), by + int(vy * 14 + uy * 7),
         bx + int(ux * 8), by + int(uy * 8), edge);
  }
}
void heroArt(int x, int y, float phase, bool moving, bool ghost = false) {
  if (g.openWorld && !ghost && mediumEnabled()) { drawMediumPlayer(x,y,phase,moving); return; }
  if (g.openWorld && !ghost) {
    drawNewRanger(x, y, phase, moving);
    return;
  }
  float cycle = moving ? std::sin(phase) : 0;
  int face = g.fx < -.25f ? -1 : 1;
  bool back = g.fy < -.45f;
  int bob = moving ? int(std::abs(cycle) * 2) : int(std::sin(g.time * 2) * .7f);
  int lean = 0;
  if (g.attackTime > 0)
    lean = int(g.aimx *
               std::sin((g.attackLength - g.attackTime) / g.attackLength * PI) *
               4);
  if (g.hurtTime > 0)
    lean += int(g.hurtx * .035f);
  bool flash = g.hurtTime > 0 && int(g.time * 28) % 2;
  C cloak = ghost   ? 0xff9be0ce
            : flash ? 0xffffc8b8
                    : 0xff276678,
    clight = ghost   ? 0xffa9e4d5
             : flash ? WHITE
                     : 0xff4a94a0,
    skin = flash ? WHITE : 0xffe6b17f, shadeSkin = 0xffa87854,
    pants = ghost ? TEAL : 0xff334a55, boots = 0xff644e37,
    hair = flash ? WHITE : 0xff4a352d;
  groundShadow(x + 3, y + 2, 15, 5);
  if (g.scene == DEAD) {
    poly({{x - 17, y - 7}, {x + 14, y - 3}, {x + 10, y + 6}, {x - 22, y + 4}},
         cloak);
    rect(x + 12, y - 5, 10, 9, skin);
    rect(x + 13, y - 7, 10, 4, hair);
    weaponArt(x - 12, y + 4, g.weapon, .2f);
    return;
  }
  int bodyy = y - 24 - bob, lx = x - 5 + int(cycle * 2),
      rx = x + 5 - int(cycle * 2), ly = y - 2 + int(cycle * 3),
      ry = y - 2 - int(cycle * 3);
  // Eight-phase limb travel comes from real distance moved, not a bobbing
  // static sprite.
  thickLine(x - 4, y - 16, lx, ly, pants, 2);
  thickLine(x + 4, y - 16, rx, ry, pants, 2);
  rect(lx - 3, ly - 2, 7, 5, boots);
  rect(rx - 3, ry - 2, 7, 5, boots);
  rect(lx - 3, ly - 2, 5, 1, 0xffa88959);
  rect(rx - 3, ry - 2, 5, 1, 0xffa88959);
  x += lean;
  int trail = int(std::sin(phase * .65f) * 3);
  poly({{x - 7, bodyy},
        {x + 7, bodyy},
        {x + 11 - face * 3, y - 8},
        {x - 10 - face * 5, y - 7}},
       cloak);
  rect(x - 6, bodyy, 12, 16, cloak);
  rect(x - 5, bodyy + 2, 4, 12, clight);
  rect(x + 5, bodyy + 3, 3, 10, 0xff1e495d);
  rect(x - 7, y - 14, 15, 3, 0xffba915c);
  rect(x + 1, y - 14, 3, 3, GOLD);
  poly({{x - 4, bodyy - 2},
        {x + 6, bodyy - 2},
        {x + 6 - face * 13, bodyy + 5 + trail},
        {x - face * 18, bodyy + 2 + trail},
        {x - face * 9, bodyy + 1}},
       flash ? WHITE : 0xffb94f40);
  rect(x - 6, bodyy - 3, 12, 4, 0xffda7760);
  int hy = bodyy - 13;
  rect(x - 6, hy + 2, 12, 11, shadeSkin);
  rect(x - 5, hy + 2, 11, 8, skin);
  rect(x - 6, hy, 13, 4, hair);
  rect(x - 7, hy + 2, 3, 6, hair);
  rect(x + 3, hy - 2, 4, 4, 0xff6d4b32);
  rect(x - 3, hy - 2, 6, 3, 0xff856044);
  if (back) {
    rect(x - 6, hy + 3, 12, 7, hair);
    rect(x - 4, hy + 9, 8, 2, 0xff9b724a);
  } else {
    rect(x + (face < 0 ? -5 : 2), hy + 5, 2, 2, INK);
    rect(x + (face < 0 ? -7 : 5), hy + 6, 2, 3, skin);
    rect(x - 2, hy + 10, 6, 2, shadeSkin);
  }
  float a = g.attackTime > 0 ? std::atan2(g.aimy, g.aimx)
            : face > 0       ? -1.0f
                             : -2.14f;
  float p =
      g.attackTime > 0 ? (g.attackLength - g.attackTime) / g.attackLength : 0;
  if (g.attackTime > 0 && g.attackWeapon != BOW)
    a += g.attackWeapon == AXE ? (-1.65f + std::min(1.f, p / .70f) * 3.5f)
                               : ((g.combo % 2 ? -1 : 1) * (-1.6f + p * 3.5f));
  int ax = x + face * 7, ay = bodyy + 8;
  int hx = ax + int(std::cos(a) * 8), hy2 = ay + int(std::sin(a) * 6);
  thickLine(ax, bodyy + 3, hx, hy2, clight, 2);
  rect(hx - 2, hy2 - 2, 4, 4, skin);
  int swing = int(cycle * 4);
  thickLine(x - face * 7, bodyy + 4, x - face * 10, bodyy + 12 + swing, cloak,
            2);
  rect(x - face * 10 - 2, bodyy + 11 + swing, 4, 4, skin);
  if (!ghost)
    weaponArt(hx, hy2, g.weapon, a,
              g.weapon == BOW && g.attackTime > 0 ? std::sin(p * PI) : 0,
              flash ? WHITE : 0);
  if (!ghost && g.openWorld)
    drawBodyMarks(x, y);
}
void animalArt(const Animal &a) {
  if (g.openWorld && a.species >= 0) {
    drawLifeAnimal(a);
    return;
  }
  int x = sx(a.x), y = syAt(a.x, a.y), dir = a.dx < 0 ? -1 : 1;
  float wave = std::sin(a.step);
  if (x < -30 || x > W + 30 || y < -20 || y > H + 30)
    return;
  groundShadow(x, y + 1, 12, 3);
  if (a.kind == 1) {
    int hop = int(std::abs(wave) * 3);
    ellipse(x, y - 5 - hop, 7, 5, 0xffd8cdb2);
    rect(x + dir * 5 - 2, y - 10 - hop, 6, 6, 0xffece2c7);
    rect(x + dir * 6 - 2, y - 17 - hop, 2, 7, 0xffece2c7);
    rect(x + dir * 8 - 2, y - 16 - hop, 2, 7, 0xffc8b6a0);
    point(x + dir * 9, y - 9 - hop, INK);
    rect(x - 4, y - 1, 9, 2, 0xffa19077);
    return;
  }
  if (a.kind == 2) {
    ellipse(x, y - 3, 10, 4, 0xff849862);
    thickLine(x - dir * 5, y - 3, x - dir * 20, y - 7, 0xff697b50, 1);
    rect(x + dir * 8 - 3, y - 8, 7, 5, 0xffb3b66d);
    line(x - 4, y - 3, x - 8, y + 3, 0xff697b50);
    line(x + 4, y - 3, x + 7, y + 2, 0xff697b50);
    point(x + dir * 11, y - 6, INK);
    return;
  }
  C fur = 0xffb88e57;
  ellipse(x, y - 12, 15, 8, fur);
  ellipse(x + dir * 12, y - 21, 6, 7, 0xffceab70);
  thickLine(x + dir * 9, y - 12, x + dir * 12, y - 24, fur, 3);
  for (int j = 0; j < 4; j++) {
    int bx = x + (j % 2 ? 9 : -8), by = y - 8,
        step = int(std::sin(a.step + (j % 2 ? PI : 0)) * 3);
    line(bx, by, bx + step, y, 0xff765e40);
  }
  line(x + dir * 13, y - 27, x + dir * 17, y - 36, 0xff765e40);
  line(x + dir * 16, y - 33, x + dir * 21, y - 34, 0xff765e40);
  line(x + dir * 12, y - 27, x + dir * 7, y - 34, 0xff765e40);
  point(x + dir * 16, y - 22, INK);
  rect(x - dir * 16, y - 14, 4, 4, 0xffe7d6ad);
}
void enemyArt(const Enemy &e) {
  if (g.openWorld && mediumEnabled() && e.kind != 2) { drawMediumEnemy(e); return; }
  if (g.openWorld) {
    drawNewEnemy(e);
    return;
  }
  if (g.openWorld && e.alive && e.alertTime <= 0 && drawRoutineActor(e))
    return;
  int x = sx(e.x), y = syAt(e.x, e.y);
  if (x < -100 || x > W + 100 || y < -10 || y > H + 120)
    return;
  int face = g.px < e.x ? -1 : 1;
  bool flash = e.flash > 0;
  if (!e.alive) {
    if (e.death <= 0)
      return;
    int sink = int((1.5f - e.death) * 6);
    groundShadow(x, y, 19, 5);
    if (e.boss()) {
      rockArt(x - 14, y + sink, 0, g.world);
      rockArt(x + 13, y + sink, 0, g.world);
    } else {
      ellipse(x, y, 15, 4, 0xff556b50);
      rect(x + 9, y - 3, 6, 4, 0xffbcaa79);
    }
    return;
  }
  if (e.boss()) {
    groundShadow(x + 8, y + 3, 37, 11);
    int z = e.state == 2 && e.pattern == 4
                ? int(std::sin(clamp(1 - e.stateTime / .52f, 0, 1) * PI) * 65)
                : 0;
    y -= z;
    int lift = e.wind > 0 ? int((1 - e.wind / e.windMax) * 12) : 0;
    if (e.kind == 8) {
      C bark = flash ? WHITE : 0xff655a3d, light = flash ? WHITE : 0xffa3915c,
        moss = flash ? WHITE : 0xff4e8c53;
      for (int j = 0; j < 4; j++) {
        int xx = x + (j < 2 ? -16 : 16),
            step = int(std::sin(e.step + (j % 2 ? PI : 0)) * 4);
        thickLine(xx, y - 26, xx + (j % 2 ? 5 : -4), y - 3 + step, bark, 4);
        rect(xx - 5, y - 3 + step, 11, 5, 0xff3d4535);
      }
      ellipse(x, y - 38, 28, 22, bark);
      ellipse(x - 5, y - 43, 23, 18, moss);
      poly({{x - 19, y - 55},
            {x + 21, y - 60},
            {x + 30, y - 28},
            {x + 13, y - 22},
            {x - 6, y - 31},
            {x - 28, y - 24}},
           moss);
      for (int j = 0; j < 10; j++)
        rect(x - 20 + hash(j) % 40, y - 51 + hash(j + 7) % 27, 5, 3,
             flash ? WHITE : 0xff86ac66);
      int heady = y - 65 - lift / 3;
      ellipse(x + face * 4, heady, 13, 18, bark);
      rect(x - 7, heady - 9, 13, 14, light);
      rect(x + face * 10 - 5, heady + 1, 11, 10, bark);
      rect(x - 9, heady - 6, 4, 3, 0xfff5d570);
      rect(x + 6, heady - 6, 4, 3, 0xfff5d570);
      for (int s : {-1, 1}) {
        thickLine(x + s * 8, heady - 11, x + s * 24, heady - 37, light, 2);
        thickLine(x + s * 24, heady - 37, x + s * 29, heady - 56, light, 2);
        thickLine(x + s * 21, heady - 31, x + s * 38, heady - 36, light, 2);
        thickLine(x + s * 38, heady - 36, x + s * 41, heady - 48, light, 1);
        thickLine(x + s * 14, heady - 23, x + s * 12, heady - 43, light, 1);
      }
    } else {
      C stone = flash ? WHITE : 0xff947c5a, edge = flash ? WHITE : 0xffd8bd84,
        dark = 0xff63584a;
      if (g.openWorld && !flash) {
        if (e.biome == 2) {
          stone = 0xff7596ac;
          edge = 0xffcee9ee;
          dark = 0xff3f5a71;
        }
        if (e.biome == 3) {
          stone = 0xff526f66;
          edge = 0xffa1b99a;
          dark = 0xff324b48;
        }
        if (e.biome == 4) {
          stone = 0xff5c4d5d;
          edge = 0xffc29480;
          dark = 0xff322d40;
        }
      }
      int stride = int(std::sin(e.step) * 4);
      box(x - 23, y - 25, 17, 27 + stride, stone, dark);
      box(x + 7, y - 25, 17, 27 - stride, stone, dark);
      rect(x - 28, y - 4 + stride, 23, 7, edge);
      rect(x + 6, y - 4 - stride, 24, 7, edge);
      poly({{x - 30, y - 73},
            {x + 29, y - 73},
            {x + 33, y - 34},
            {x + 16, y - 23},
            {x - 20, y - 23},
            {x - 35, y - 42}},
           stone);
      poly({{x - 30, y - 73},
            {x - 6, y - 80},
            {x + 29, y - 73},
            {x + 19, y - 62},
            {x - 22, y - 62}},
           edge);
      box(x - 12, y - 96, 27, 25, edge, dark);
      rect(x - 15, y - 101, 32, 6, stone);
      rect(x - 7, y - 88, 6, 4, 0xffffc760);
      rect(x + 6, y - 88, 6, 4, 0xffffc760);
      for (int s : {-1, 1}) {
        box(x + s * 37 - 10, y - 66 - lift, 21, 38, stone, dark);
        rect(x + s * 37 - 7, y - 63 - lift, 6, 28, edge);
        box(x + s * 39 - 12, y - 33 - lift, 24, 16, edge, dark);
      }
      poly({{x, y - 62}, {x + 11, y - 49}, {x, y - 33}, {x - 10, y - 49}},
           0xffd67c45);
      flame(x, y - 41, 0xffffc66e);
      line(x - 20, y - 58, x - 11, y - 42, dark);
      line(x + 15, y - 33, x + 23, y - 45, dark);
    }
    if (!g.gate) {
      circle(x, y - 30, 45, TEAL);
      center(x, y + 14, "SHRINE-SEALED", WHITE);
    }
    if (e.state == 4) {
      for (int k = 0; k < 3; k++) {
        float a = g.time * 4 + k * 2.1f;
        rect(x + int(std::cos(a) * 25), y - 104 + int(std::sin(a) * 7), 4, 4,
             GOLD);
      }
    }
    return;
  }
  groundShadow(x + 2, y + 2, e.kind == 3 ? 19 : 13, 5);
  float step = std::sin(e.step);
  int lift = e.wind > 0 ? int((1 - e.wind / e.windMax) * 8) : 0;
  if (e.kind == 2) {
    C body = flash ? WHITE : g.world ? 0xff916048 : 0xff405c68;
    ellipse(x, y - 12, 17, 9, body);
    ellipse(x + face * 14, y - 17, 9, 7, body);
    poly({{x + face * 12, y - 20},
          {x + face * 9, y - 30},
          {x + face * 18, y - 22}},
         body);
    thickLine(x - face * 14, y - 13, x - face * 26, y - 23 + int(step * 3),
              body, 2);
    for (int j = 0; j < 4; j++) {
      int xx = x + (j % 2 ? 9 : -9);
      thickLine(xx, y - 10, xx + int(std::sin(e.step + j) * 5), y, body, 1);
    }
    rect(x + face * 18 - 1, y - 19, 3, 2, GOLD);
    rect(x + face * 21 - 2, y - 13, 5, 3, 0xffddd7b6);
  } else {
    int bodyw = e.kind == 3 ? 14 : 9, bob = int(std::abs(step) * 2);
    C robe = flash         ? WHITE
             : e.kind == 1 ? 0xff735c92
             : e.kind == 4 ? 0xff427c78
             : e.kind == 3 ? 0xff737760
                           : 0xff637c54;
    C hi = flash         ? WHITE
           : e.kind == 1 ? 0xffb29ab5
           : e.kind == 4 ? 0xff8fb995
           : e.kind == 3 ? 0xffb9ac7c
                         : 0xffa6b776;
    int yy = y - bob;
    thickLine(x - 5, yy - 12, x - 7 + int(step * 3), y - 1, 0xff425449, 2);
    thickLine(x + 5, yy - 12, x + 7 - int(step * 3), y - 1, 0xff425449, 2);
    rect(x - 11, y - 3, 8, 4, 0xff3c4137);
    rect(x + 3, y - 3, 8, 4, 0xff3c4137);
    poly({{x - bodyw, yy - 29},
          {x + bodyw, yy - 29},
          {x + bodyw + 3, yy - 7},
          {x - bodyw - 4, yy - 7}},
         robe);
    rect(x - bodyw + 3, yy - 25, 4, 14, hi);
    rect(x - bodyw, yy - 11, bodyw * 2, 3, 0xffa78856);
    ellipse(x, yy - 35, e.kind == 3 ? 11 : 8, 9, robe);
    rect(x - 5, yy - 36, 11, 8, flash ? WHITE : 0xffc1ba8b);
    rect(x - 5, yy - 35, 3, 2, 0xffe6ab68);
    rect(x + 3, yy - 35, 3, 2, 0xffe6ab68);
    thickLine(x + face * bodyw, yy - 25, x + face * (bodyw + 6), yy - 13 - lift,
              robe, 2);
    if (e.kind == 3) {
      box(x - face * 19 - 8, yy - 27, 17, 25, 0xffbdad7d, 0xff596259);
      rect(x - face * 19 - 1, yy - 24, 3, 19, GOLD);
      weaponArt(x + face * 18, yy - 20 - lift, AXE, -1.4f + lift * .1f);
    } else if (e.kind == 1 || e.kind == 4) {
      thickLine(x + face * 15, yy - 4, x + face * 15, yy - 42 - lift,
                0xff8b704e, 1);
      circle(x + face * 15, yy - 44 - lift, 5, e.kind == 1 ? 0xffdca0dc : TEAL,
             true);
      circle(x + face * 15, yy - 44 - lift, 8,
             e.kind == 1 ? 0xff9f78a5 : 0xff5b9b91);
    } else {
      thickLine(x + face * 14, yy - 17 - lift, x + face * 20, yy - 30 - lift,
                0xffa0b2a4, 2);
      rect(x + face * 14 - 2, yy - 17 - lift, 4, 4, 0xffd2bc8b);
    }
  }
  if (e.elite) {
    poly({{x, y - 57}, {x + 5, y - 51}, {x, y - 45}, {x - 5, y - 51}}, GOLD);
  }
  if (e.hp < e.maxhp) {
    box(x - 17, y - 49, 34, 5, INK, EDGE);
    rect(x - 16, y - 48, int(32 * e.hp / e.maxhp), 3, RED);
  }
  if (e.wind > 0) {
    center(x, y - (e.kind == 3 ? 61 : 58), "!", GOLD, 2);
  }
}
} // namespace av
