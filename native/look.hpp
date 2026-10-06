#pragma once
// Wild Earth: original procedural material/sprite renderer, not third-party
// art.
namespace av {
void blendPixel(int x, int y, C color, int alpha) {
  if (x < 0 || y < 0 || x >= W || y >= H || alpha <= 0)
    return;
  pix[y * W + x] = mix(pix[y * W + x], color, std::min(255, alpha));
}
struct TreeSprite {
  static constexpr int width = 128, height = 144;
  std::vector<C> pixels;
};
void foliage(int x, int y, int rx, int ry, C dark, C light, uint32_t seed) {
  C ramp[5] = {mix(dark, 0xff183d39, 65), dark, mix(dark, light, 115),
               mix(dark, light, 205), mix(light, 0xffd3d391, 42)};
  for (int yy = -ry; yy <= ry; yy++)
    for (int xx = -rx; xx <= rx; xx++) {
      uint32_t h = hash(seed + uint32_t((xx + rx) / 5) * 991 +
                        uint32_t((yy + ry) / 4) * 7109);
      float d = float(xx * xx) / (rx * rx) + float(yy * yy) / (ry * ry);
      if (d > 1.f - float(h % 9) * .016f)
        continue;
      float volume = .58f - float(yy) / (ry * 2.1f) - float(xx) / (rx * 4.f);
      int shade = std::clamp(int(volume * 4) + int((h >> 8) % 3) - 1, 0, 4);
      if ((yy + ry) % 4 == 0 && (xx + rx) % 5 < 3 && shade > 1)
        shade = std::min(4, shade + 1);
      point(x + xx, y + yy, ramp[shade]);
    }
}
TreeSprite &treeSprite(int species, int biome, int variant) {
  static std::map<int, TreeSprite> cache;
  int key = species * 256 + biome * 32 + variant;
  auto it = cache.find(key);
  if (it != cache.end())
    return it->second;
  static std::vector<C> canvas(W * H);
  std::fill(canvas.begin(), canvas.end(), 0);
  C *old = pix;
  pix = canvas.data();
  int x = 180, y = 170;
  uint32_t seed = hash(key + 9977);
  C bark = biome == 4 ? 0xff484440 : 0xff655745,
    hi = biome == 4 ? 0xff80796a : 0xffab9270;
  C dark = biome == 1   ? 0xff485545
           : biome == 2 ? 0xff294e47
           : biome == 3 ? 0xff29413a
                        : 0xff244738;
  C light = biome == 1   ? 0xff94a070
            : biome == 2 ? 0xff879e85
            : biome == 3 ? 0xff889371
                         : 0xffa1b56a;
  int h = species == 2 || species == 6   ? 106
          : species == 1 || species == 8 ? 97
          : species == 5                 ? 78
                                         : 92;
  int lean = (variant % 5) * 3 - 6;
  if (species == 1) {
    bark = 0xffbfc1b0;
    hi = 0xffe4e1c8;
  }
  for (int j = 0; j < 5; j++) {
    int dx = (j - 2) * 5;
    thickLine(x, y - 14, x + dx, y + (j % 2) * 2, bark, 2);
    line(x + 1, y - 12, x + dx + 1, y, hi);
  }
  thickLine(x, y - 2, x + lean, y - h + 20, bark,
            species == 0   ? 6
            : species == 1 ? 3
                           : 4);
  line(x - 2, y - 4, x + lean - 2, y - h + 20, hi);
  line(x + 3, y - 3, x + lean + 3, y - h + 22, mix(bark, INK, 90));
  for (int j = 0; j < 20; j++) {
    int yy = y - j * 4 - 5;
    uint32_t z = hash(seed + j);
    if (species == 1)
      rect(x - 3 + int(z % 3), yy, 3 + int(z % 3), 1, 0xff505951);
    else
      line(x + int(z % 7) - 3, yy, x + int(z % 7) - 3, yy - 3,
           mix(bark, hi, 75));
  }
  auto crown = [&](int dx, int dy, int rx, int ry, int n) {
    foliage(x + dx + int(hash(seed + n) % 9) - 4,
            y + dy + int(hash(seed + n * 19) % 7) - 3, rx, ry, dark, light,
            seed + n * 827);
  };
  if (species == 7) {
    for (int j = 0; j < 5; j++) {
      int dir = j % 2 ? 1 : -1, yy = y - 27 - j * 11;
      thickLine(x, yy, x + dir * (16 + j * 2), yy - 19, bark, 2);
      line(x + dir * (16 + j * 2), yy - 19, x + dir * (21 + j * 2), yy - 34,
           bark);
      line(x + dir * (16 + j * 2), yy - 19, x + dir * (28 + j * 2), yy - 21,
           hi);
    }
    line(x, y - 55, x + 8, y - 98, bark);
    if (biome == 4) {
      line(x - 2, y - 16, x + 1, y - 34, 0xffb88964);
      point(x + 2, y - 30, 0xffd3a577);
    }
  } else if (species == 2 || species == 6) {
    for (int tier = 0; tier < 7; tier++) {
      int yy = y - 22 - tier * 12, wide = 34 - tier * 4;
      poly({{x + lean, yy - 26}, {x - wide, yy + 5}, {x + wide, yy + 5}}, dark);
      poly({{x + lean - 1, yy - 25}, {x - wide + 3, yy + 1}, {x - 1, yy + 2}},
           mix(dark, light, 140));
      for (int n = -2; n <= 2; n++) {
        int dx = n * wide / 3, dy = std::abs(n) * 3;
        foliage(x + dx + lean / 2, yy - 2 + dy, std::max(4, wide / 3 + 2),
                5 + (tier % 2), dark, light, seed + tier * 91 + n * 711);
      }
      line(x + lean - 1, yy - 22, x + lean - 3, yy - 17, mix(light, WHITE, 35));
      if (biome == 2) {
        for (int n = -2; n <= 2; n++) {
          int dx = n * wide / 3;
          line(x + dx - 3, yy + std::abs(n) * 3 - 5, x + dx + 4,
               yy + std::abs(n) * 3 - 5, 0xffc1d2cb);
          point(x + dx - 3, yy + std::abs(n) * 3 - 6, 0xffe2e6d6);
        }
      }
    }
  } else if (species == 4) {
    for (int j = 0; j < 10; j++) {
      float a = j * 2 * PI / 10;
      int ex = int(std::cos(a) * 48), ey = int(std::sin(a) * 17) + 14;
      int px = x + lean, py = y - 82;
      for (int k = 1; k <= 14; k++) {
        float t = k / 14.f;
        int xx = x + lean + int(ex * t),
            yy = y - 82 + int(ey * t - 19 * std::sin(t * PI));
        thickLine(px, py, xx, yy, k < 9 ? light : dark, 1);
        for (int side : {-1, 1})
          line(xx, yy, xx - int(ex * .14f) + side * 3, yy + 8, dark);
        px = xx;
        py = yy;
      }
    }
    ellipse(x + lean - 4, y - 79, 4, 5, 0xff8c7851);
  } else if (species == 5) {
    thickLine(x, y - 31, x - 22, y - 58, bark, 3);
    thickLine(x, y - 36, x + 24, y - 61, bark, 3);
    crown(-31, -66, 23, 11, 1);
    crown(3, -76, 34, 14, 2);
    crown(32, -68, 25, 12, 3);
    crown(-12, -76, 26, 11, 4);
  } else if (species == 1) {
    for (int j = 0; j < 7; j++) {
      int dx = (j % 2 ? 1 : -1) * (12 + (j % 3) * 5), dy = -34 - j * 9;
      line(x, y + dy + 12, x + dx, y + dy, bark);
      crown(dx, dy - 9, 13 + (j % 3) * 3, 17, 1 + j);
    }
    crown(lean, -103, 15, 19, 9);
  } else if (species == 3) {
    thickLine(x, y - 36, x - 25, y - 63, bark, 3);
    thickLine(x, y - 36, x + 21, y - 64, bark, 3);
    crown(-22, -66, 26, 24, 1);
    crown(14, -73, 29, 25, 2);
    crown(0, -87, 28, 22, 3);
    for (int j = 0; j < 25; j++) {
      int dx = j * 4 - 48, top = y - 64 + std::abs(dx) / 4,
          drop = 20 + int(hash(seed + j) % 24);
      C c = mix(dark, light, 80 + int(hash(seed + j * 17) % 100));
      for (int k = 0; k < drop; k++) {
        int xx = x + dx + int(std::sin(k * .14f + j) * 2);
        if (k % 3 != 2)
          rect(xx, top + k, 2, 2, c);
      }
    }
  } else if (species == 8) {
    for (int j = 0; j < 7; j++) {
      int yy = -35 - j * 11, dx = j % 2 ? 8 : -7;
      line(x, y + yy, x + dx * 3, y + yy - 7, bark);
      crown(dx, yy - 8, 28 - j * 2, 14, 1 + j);
    }
  } else {
    for (int j = 0; j < 5; j++)
      thickLine(x, y - 32, x + (j - 2) * 14, y - 66 - (j % 2) * 9, bark, 2);
    crown(-28, -53, 23, 22, 1);
    crown(28, -56, 25, 23, 2);
    crown(0, -61, 36, 31, 3);
    crown(-20, -79, 26, 23, 4);
    crown(14, -87, 28, 25, 5);
    crown(37, -70, 18, 17, 6);
    crown(-40, -67, 16, 17, 7);
  }
  TreeSprite s;
  s.pixels.resize(TreeSprite::width * TreeSprite::height);
  for (int yy = 0; yy < TreeSprite::height; yy++)
    for (int xx = 0; xx < TreeSprite::width; xx++)
      s.pixels[yy * TreeSprite::width + xx] =
          canvas[(y - 135 + yy) * W + x - 64 + xx];
  pix = old;
  return cache.emplace(key, std::move(s)).first->second;
}
void blitTree(const TreeSprite &s, float x, float y, float angle, int alpha,
              bool fade, float wind) {
  float co = std::cos(angle), si = std::sin(angle);
  for (int yy = 0; yy < TreeSprite::height; yy++) {
    float sway =
        yy < 100 ? std::sin(wind + yy * .025f) * (100 - yy) * .016f : 0;
    for (int xx = 0; xx < TreeSprite::width; xx++) {
      C c = s.pixels[yy * TreeSprite::width + xx];
      if (!c)
        continue;
      float dx = xx - 64 + sway, dy = yy - 135;
      int px = int(std::round(x + dx * co - dy * si)),
          py = int(std::round(y + dx * si + dy * co));
      int a = fade && yy < 107 ? alpha * 110 / 255 : alpha;
      blendPixel(px, py, c, a);
    }
  }
}
void drawNewTree(const Prop &p) {
  int x = sx(p.x), y = syAt(p.x, p.y);
  if (x < -150 || x > W + 150 || y < -15 || y > H + 150)
    return;
  auto key = treeKey(p);
  int species = treeSpecies(p);
  if (treeCut(p)) {
    ellipse(x, y + 2, 13, 5, 0xff3e4432);
    box(x - 8, y - 8, 17, 10, 0xff685641, 0xff3c3f31);
    ellipse(x, y - 8, 8, 4, 0xffb5a078);
    ellipse(x, y - 8, 5, 2, 0xff82714f);
    point(x - 1, y - 8, 0xffd3bf94);
    for (int i = 0; i < 7; i++) {
      uint32_t h = hash(uint32_t(key.first) + i * 883);
      rect(x + int(h % 28) - 14, y + int((h >> 8) % 9) - 3, 3, 1, 0xff968360);
    }
    return;
  }
  bool fade = g.py < p.y + 8 && g.py > p.y - 125 && std::abs(g.px - p.x) < 57;
  blitTree(treeSprite(species, p.biome,
                      int(coordinateHash(key.first, key.second, 9187) % 8)),
           float(x), float(y), 0, 255, fade,
           g.time * 1.1f + float(coordinateHash(key.first, key.second) % 99));
  auto it = w.trees.find(key);
  if (it != w.trees.end() && it->second > 0) {
    rect(x - 5, y - 17, 7, 3, 0xffc2a375);
    if (len(p.x - g.px, p.y - g.py) < 100) {
      panel(x - 29, y + 9, 58, 14);
      center(x, y + 13, std::string(treeName(species)) + " " + num(it->second),
             GOLD);
    }
  }
}
void drawFallingTree(const FallingTree &f) {
  float t = clamp(f.time / .95f, 0, 1), angle = f.dir * t * t * 1.53f;
  int alpha = int(clamp((1.35f - f.time) / .3f, 0, 1) * 255);
  blitTree(treeSprite(f.species, f.biome,
                      int(coordinateHash(
                              o.originX + int64_t(std::floor(f.x / T)),
                              o.originY + int64_t(std::floor(f.y / T)), 9187) %
                          8)),
           float(sx(f.x)), float(syAt(f.x, f.y)), angle, alpha, false, 0);
}
void drawNewRock(const Prop &p) {
  int x = sx(p.x), y = syAt(p.x, p.y);
  if (x < -35 || x > W + 35 || y < -15 || y > H + 45)
    return;
  C stone = p.biome == 1 ? 0xffad9270 : p.biome == 4 ? 0xff645f59 : 0xff7e847a;
  poly({{x - 19, y - 3},
        {x - 17, y - 20},
        {x - 7, y - 30},
        {x + 11, y - 26},
        {x + 20, y - 7},
        {x + 14, y + 3},
        {x - 13, y + 3}},
       mix(stone, INK, 60));
  poly({{x - 17, y - 17},
        {x - 7, y - 30},
        {x + 11, y - 26},
        {x + 5, y - 13},
        {x - 8, y - 9}},
       mix(stone, WHITE, 48));
  poly({{x - 8, y - 9},
        {x + 5, y - 13},
        {x + 11, y - 26},
        {x + 17, y - 9},
        {x + 12, y},
        {x - 10, y}},
       stone);
  line(x + 5, y - 24, x + 1, y - 15, mix(stone, INK, 80));
  line(x + 1, y - 15, x + 4, y - 5, mix(stone, INK, 80));
  uint32_t h = hash(uint32_t(treeKey(p).first) ^ uint32_t(treeKey(p).second));
  for (int j = 0; j < 27; j++) {
    h = hash(h + j);
    int dx = int(h % 27) - 13, dy = int((h >> 12) % 18) - 19;
    rect(x + dx, y + dy, 1 + (j % 3), 1,
         mix(stone, j % 3 ? WHITE : INK, j % 3 ? 28 : 50));
  }
  if (p.biome == 0 || p.biome == 3) {
    ellipse(x - 11, y - 3, 9, 4, 0xff536344);
    for (int j = 0; j < 8; j++)
      point(x - 17 + j * 2, y - 4 + (j % 2), 0xff94a277);
  }
  if (p.biome == 2) {
    poly({{x - 17, y - 21},
          {x - 7, y - 31},
          {x + 11, y - 27},
          {x + 8, y - 22},
          {x - 4, y - 23},
          {x - 9, y - 17}},
         0xffc6d1c9);
  }
  auto oreKey = treeKey(p);
  int oreKind = int(coordinateHash(oreKey.first, oreKey.second, 7301) % 7);
  if (oreKind <= 2) {
    C ore = oreKind == 0 ? 0xffc29b7c : 0xff333c36;
    for (int n = 0; n < 8; n++) {
      auto h = hash(uint32_t(p.x + p.y) + n * 137);
      rect(x - 10 + int(h % 21), y - 9 - int((h >> 8) % 16), 3, 2, ore);
    }
    if (len(p.x - g.px, p.y - g.py) < 75)
      center(x, y - 38, oreKind == 0 ? "IRON ORE" : "COAL", GOLD);
  }
}
void drawUnderstory(const Prop &p) { vistaUnderstory(p); }
C earthColor(int b) {
  const C c[] = {0xff75805a, 0xffb5a27c, 0xffbac8c2, 0xff637765, 0xff726a5b};
  return c[std::clamp(b, 0, 4)];
}
void drawNewTerrain() { drawVistaTerrain(); }
void drawNewRanger(int x, int y, float phase, bool moving) {
  drawJourneyRanger(x, y, phase, moving);
  return;
  float stride = moving ? std::sin(phase * .65f) : 0,
        bob = moving ? std::abs(std::sin(phase * .65f)) * 1.7f
                     : std::sin(g.time * 2) * .6f;
  int face = g.attackTime > 0 ? (g.aimx < 0 ? -1 : 1) : (g.fx < 0 ? -1 : 1),
      by = y - int(bob), leg = int(stride * 5);
  bool back = g.fy < -.45f && g.attackTime <= 0;
  float attack =
      g.attackTime > 0 ? clamp(1 - g.attackTime / g.attackLength, 0, 1) : 0;
  float swing = std::sin(attack * PI);
  int lean = g.attackTime > 0 ? int(face * swing * 3) : int(g.vx / 100);
  C cloak = 0xff647263, edge = 0xffb7b99b, leather = 0xff806547,
    skin = 0xffc6a37d, dark = 0xff29342e;
  if (g.hurtTime > .18f) {
    cloak = mix(cloak, WHITE, 90);
    leather = mix(leather, WHITE, 110);
  }
  // Grounded boots, weight shift and overlapping cloth silhouette.
  ellipse(x, y + 2, 13, 4, 0xff424b39);
  thickLine(x - 5, by - 15, x - 6 + leg, y - 3, 0xff494f43, 3);
  thickLine(x + 5, by - 15, x + 6 - leg, y - 3, 0xff494f43, 3);
  box(x - 9 + leg, y - 4, 9, 5, 0xff443e32, dark);
  box(x + 2 - leg, y - 4, 9, 5, 0xff443e32, dark);
  line(x - 8 + leg, y - 4, x - 2 + leg, y - 4, 0xffa08c68);
  int cape = int(std::sin(phase * .45f + g.time * .5f) * (moving ? 4 : 1));
  poly({{x - 10 + lean, by - 36},
        {x + 9 + lean, by - 36},
        {x + 13 + cape - lean, by - 8},
        {x - 3 + cape - lean, by - 12},
        {x - 13 + cape - lean, by - 6}},
       dark);
  poly({{x - 8 + lean, by - 35},
        {x + 7 + lean, by - 35},
        {x + 10 + cape - lean, by - 10},
        {x - 3 + cape - lean, by - 14},
        {x - 10 + cape - lean, by - 8}},
       cloak);
  line(x - 8 + lean, by - 31, x - 9 + cape - lean, by - 10, edge);
  line(x + 3 + lean, by - 32, x + 4 + cape - lean, by - 14, 0xff455447);
  box(x - 8 + lean, by - 34, 17, 20, leather, dark);
  rect(x - 5 + lean, by - 31, 7, 13, 0xff9b8059);
  rect(x + 3 + lean, by - 31, 4, 16, 0xff5f513d);
  for (int j = 0; j < 3; j++)
    line(x - 6 + lean, by - 28 + j * 5, x + 6 + lean, by - 29 + j * 5,
         0xffb6a17a);
  rect(x - 10 + lean, by - 17, 21, 4, 0xff403e31);
  box(x - 2 + lean, by - 18, 5, 5, 0xffbaa36a);
  box(x - 9 + lean, by - 22, 5, 7, 0xff786342, dark);
  ellipse(x - 9 + lean, by - 33, 5, 4, 0xffa3aaa0);
  ellipse(x + 9 + lean, by - 33, 5, 4, 0xff6e7d72);
  line(x - 12 + lean, by - 35, x - 7 + lean, by - 36, 0xffd9d5b7);
  box(x - 6 + lean, by - 47, 13, 14, dark);
  rect(x - 5 + lean, by - 45, 11, 10, back ? 0xff756f53 : skin);
  rect(x - 6 + lean, by - 49, 12, 5, 0xff4f4a37);
  rect(x - 7 + lean, by - 45, 4, 7, 0xff615842);
  if (!back) {
    rect(x + face * 3 + lean, by - 42, 2, 1, 0xff282f28);
    rect(x + face * 5 + lean, by - 40, 2, 3, 0xffdec49a);
    rect(x - 3 + lean, by - 37, 8, 3, 0xff76664a);
  } else
    rect(x - 4 + lean, by - 43, 9, 7, 0xff73745b);
  line(x - 5 + lean, by - 49, x + 4 + lean, by - 49, 0xffc1b797);
  rect(x - 5 + lean, by - 34, 10, 3, 0xffd0c4a3);
  float aim =
      g.attackTime > 0 ? std::atan2(g.aimy, g.aimx) : std::atan2(g.fy, g.fx);
  float a = aim;
  if (g.weapon != BOW) {
    float wind = attack < .28f ? attack / .28f : (attack - .28f) / .72f;
    a = aim + (g.attackTime > 0 ? (-1.5f + wind * 2.6f) : -.85f);
  }
  int hx = x + lean + face * 13 + int(std::cos(a) * swing * 8),
      hy = by - 24 + int(std::sin(a) * swing * 6);
  thickLine(x + face * 9 + lean, by - 32, hx, hy, dark, 3);
  thickLine(x + face * 9 + lean, by - 32, hx, hy, leather, 2);
  box(hx - 2, hy - 2, 5, 5, skin, dark);
  int off = x - face * 11 + lean;
  thickLine(x - face * 9 + lean, by - 31, off, by - 19 - int(stride * 2),
            leather, 2);
  rect(off - 2, by - 21 - int(stride * 2), 4, 5, skin);
  weaponArt(hx, hy, g.weapon, a,
            g.weapon == BOW && g.attackTime > 0 ? swing : 0, 0);
  drawBodyMarks(x + lean, by);
  if (v.task) {
    float a = g.time * 3;
    line(hx, hy, hx + int(std::sin(a) * 7), hy + 10, 0xffc8b48d);
  }
}
void softArc(int x, int y, float r, float a, float b, C color, int opacity) {
  for (float t = a; t < b; t += .025f) {
    float f = (t - a) / std::max(.01f, b - a);
    int xx = x + int(std::cos(t) * r), yy = y + int(std::sin(t) * r);
    for (int j = -1; j <= 1; j++)
      blendPixel(xx, yy + j, color,
                 int(opacity * (.25f + .75f * f)) / (std::abs(j) + 1));
  }
}
void drawWeaponRibbon() {
  float a = std::atan2(g.aimy, g.aimx), p = clamp(1 - g.slash / .2f, 0, 1),
        r = g.attackWeapon == AXE ? 70 : 57;
  int opacity = int((1 - p) * 205);
  for (int k = 0; k < 4; k++)
    softArc(sx(g.px), syAt(g.px, g.py) - 14, r - k * 2, a - 1.4f + p * 1.3f,
            a + .8f + p * 1.3f, k < 2 ? 0xffe5dbb4 : 0xffa99978,
            opacity / (1 + k));
}
void drawNewPlants(const Prop &p) {
  int x = sx(p.x), y = syAt(p.x, p.y);
  if (x < -35 || x > W + 35 || y < -15 || y > H + 40)
    return;
  if (p.kind == 25) {
    ellipse(x, y, 27, 12, 0xff615b43);
    for (int j = -2; j <= 2; j++) {
      line(x + j * 9, y + 8, x + j * 9, y - 8, 0xff465637);
      if (plantReady(p)) {
        line(x + j * 9, y - 3, x + j * 9 - 5, y - 9, 0xffa2a36c);
        line(x + j * 9, y - 4, x + j * 9 + 5, y - 10, 0xff9a9a60);
        rect(x + j * 9 - 1, y - 15, 2, 10, 0xffb2a16c);
      }
    }
    return;
  }
  C dark = p.biome == 2 ? 0xff6c8174 : 0xff3c5536,
    light = p.biome == 1 || p.biome == 4 ? 0xffa2a06c : 0xff819663;
  line(x, y, x - 4, y - 20, 0xff746342);
  line(x, y, x + 6, y - 18, 0xff746342);
  foliage(x - 7, y - 13, 10, 9, dark, light, 281 + p.variant * 17);
  foliage(x + 6, y - 16, 11, 10, dark, light, 782 + p.variant * 77);
  if (plantReady(p))
    for (int j = 0; j < 6; j++) {
      int dx = j * 4 - 10, dy = -12 - (j % 3) * 3;
      rect(x + dx, y + dy, 2, 2, p.biome == 2 ? 0xff778a9e : 0xffb79456);
      point(x + dx, y + dy, 0xffe1c28d);
    }
}
void stationDetail(const Prop &p) {
  int x = sx(p.x), y = syAt(p.x, p.y);
  if (x < -35 || x > W + 35 || y < -15 || y > H + 50)
    return;
  if (p.kind == 23 || p.kind == 26) {
    int top = p.kind == 23 ? -20 : -13;
    for (int j = 0; j < 4; j++) {
      int xx = x - 19 + j * 12;
      line(xx, y + top, xx + 8, y + top + 2, 0xff786747);
      point(xx, y + top + 3, 0xffccb589);
    }
    point(x - 19, y + top, 0xff454f44);
    point(x + 18, y + top, 0xff454f44);
  }
  if (p.kind == 21) {
    for (int j = 0; j < 3; j++) {
      line(x - 13, y - 12 + j * 4, x + 12, y - 12 + j * 4, 0xff7e8980);
      line(x - 8 + j * 7, y - 13, x - 8 + j * 7, y - 6, 0xff6e7c72);
    }
    line(x - 17, y - 36, x - 17, y - 10, 0xffb7a078);
  }
  if (p.kind == 24) {
    line(x - 1, y - 10, x + 16, y - 10, 0xff859383);
    line(x + 4, y - 9, x + 4, y + 3, 0xff344e49);
  }
}
void drawNewEnemy(const Enemy &e) {
  int x = sx(e.x), y = syAt(e.x, e.y);
  if (x < -90 || x > W + 90 || y < -30 || y > H + 140)
    return;
  if (!e.alive) {
    ellipse(x, y - 3, e.boss() ? 28 : 14, 5, 0xff4c503e);
    line(x - 8, y - 5, x + 11, y - 3, 0xffb8b299);
    return;
  }
  bool idle = e.alertTime <= 0 && e.routine >= 0;
  int face = (idle && len(e.x - e.previousX, e.y - e.previousY) > .05f
                  ? e.x - e.previousX
                  : g.px - e.x) < 0
                 ? -1
                 : 1;
  float step = std::sin(e.step),
        swing =
            e.wind > 0 ? std::sin(clamp(1 - e.wind / e.windMax, 0, 1) * PI) : 0;
  if (e.kind == 2) {
    Animal a{};
    a.x = e.x;
    a.y = e.y;
    a.species = WOLF;
    a.step = e.step;
    a.dx = face;
    a.flash = e.flash;
    a.hp = a.maxhp = 1;
    a.behaviour = idle && e.routine == SLEEPING ? SLEEP : WANDER;
    drawLifeAnimal(a);
    for (int j = 0; j < 4; j++)
      line(x - 8 + j * 5, y - 17, x - 10 + j * 5, y - 24, 0xffb0b29c);
    point(x + face * 18, y - 19, 0xffdfb779);
    return;
  }
  C skin = e.flash > 0 ? WHITE : 0xffa6a88b, dark = 0xff303b32;
  if (e.boss()) {
    int bob = int(std::abs(step) * 3), yy = y - bob;
    C rock = e.biome == 4 ? 0xff6d5b49 : e.biome == 2 ? 0xff768b82 : 0xff656d51;
    thickLine(x - 14, yy - 29, x - 18 + int(step * 5), y - 3, 0xff454b3c, 8);
    thickLine(x + 14, yy - 29, x + 18 - int(step * 5), y - 3, 0xff454b3c, 8);
    rect(x - 29 + int(step * 5), y - 5, 20, 8, dark);
    rect(x + 8 - int(step * 5), y - 5, 20, 8, dark);
    poly({{x - 23, yy - 72},
          {x + 25, yy - 70},
          {x + 32, yy - 40},
          {x + 17, yy - 16},
          {x - 20, yy - 19},
          {x - 31, yy - 45}},
         dark);
    poly({{x - 20, yy - 71},
          {x + 22, yy - 68},
          {x + 28, yy - 40},
          {x + 14, yy - 19},
          {x - 17, yy - 22},
          {x - 28, yy - 45}},
         e.flash > 0 ? WHITE : rock);
    for (int j = 0; j < 9; j++) {
      int dx = (j % 3 - 1) * 14, dy = -62 + (j / 3) * 15;
      poly({{x + dx - 7, yy + dy - 7},
            {x + dx + 7, yy + dy - 9},
            {x + dx + 9, yy + dy + 4},
            {x + dx - 5, yy + dy + 6}},
           mix(rock, j % 2 ? WHITE : INK, j % 2 ? 35 : 50));
      line(x + dx - 6, yy + dy - 5, x + dx + 4, yy + dy - 7, 0xffa7aa82);
    }
    for (int side : {-1, 1}) {
      int armx = x + side * (36 + int(swing * 4)),
          army = yy - 27 - int(swing * 22);
      thickLine(x + side * 25, yy - 62, armx, army, dark, 8);
      thickLine(x + side * 25, yy - 62, armx, army, rock, 6);
      ellipse(armx, army + 3, 8, 9, 0xff7f8060);
      line(armx - 4, army, armx + 4, army - 2, 0xffc1b38a);
    }
    box(x - 12, yy - 94, 26, 28, dark);
    poly({{x - 10, yy - 91},
          {x + 10, yy - 91},
          {x + 13, yy - 73},
          {x + 2, yy - 66},
          {x - 11, yy - 74}},
         0xff8b9070);
    rect(x - 8, yy - 84, 6, 3, 0xffddb77a);
    rect(x + 4, yy - 84, 6, 3, 0xffddb77a);
    line(x - 6, yy - 74, x + 8, yy - 76, dark);
    for (int side : {-1, 1}) {
      thickLine(x + side * 8, yy - 92, x + side * 21, yy - 117, 0xff968765, 3);
      line(x + side * 18, yy - 110, x + side * 33, yy - 123, 0xffc5b58b);
      line(x + side * 17, yy - 108, x + side * 13, yy - 124, 0xffc5b58b);
    }
    if (e.biome == 0 || e.biome == 3)
      for (int j = 0; j < 12; j++)
        rect(x - 21 + int(hash(j + 31) % 44), yy - 62 + int(hash(j + 211) % 35),
             3, 2, 0xff82925b);
    return;
  }
  C coat = e.kind == 1   ? 0xff655d61
           : e.kind == 3 ? 0xff6d756b
           : e.kind == 4 ? 0xff4c6960
                         : 0xff687258;
  if (idle && e.routine == SLEEPING) {
    box(x - 20, y - 7, 39, 12, 0xff586653, dark);
    circle(x - 22, y - 2, 6, skin, true);
    rect(x - 11, y - 6, 24, 9, 0xff485e53);
    line(x + 5, y - 5, x + 5, y + 3, 0xffa6a88b);
    if (int(g.time) % 3 < 2)
      text(x - 3, y - 23, "Z Z", DIM);
    return;
  }
  int gait = int(step * 3), yy = y - int(std::abs(step) * 1.5f);
  float action = idle ? std::sin(e.routineAnim * 4) : swing * 3;
  thickLine(x - 5, yy - 15, x - 6 + gait, y - 2, dark, 3);
  thickLine(x + 5, yy - 15, x + 6 - gait, y - 2, dark, 3);
  rect(x - 10 + gait, y - 3, 8, 5, 0xff4b4637);
  rect(x + 3 - gait, y - 3, 8, 5, 0xff4b4637);
  poly({{x - 11, yy - 36},
        {x + 10, yy - 36},
        {x + 14, yy - 11},
        {x + 5, yy - 8},
        {x - 3, yy - 12},
        {x - 13, yy - 9}},
       dark);
  poly({{x - 9, yy - 34},
        {x + 8, yy - 34},
        {x + 11, yy - 13},
        {x + 4, yy - 11},
        {x - 9, yy - 12}},
       e.flash > 0 ? WHITE : coat);
  for (int j = 0; j < 4; j++)
    line(x - 6 + j * 4, yy - 30, x - 8 + j * 4, yy - 15,
         mix(coat, j % 2 ? WHITE : INK, 40));
  rect(x - 11, yy - 17, 23, 3, 0xff615339);
  box(x - 2, yy - 18, 5, 4, 0xffb19f72);
  rect(x - 8, yy - 31, 3, 15, 0xff92836a);
  ellipse(x - 10, yy - 34, 5, 4, 0xff879183);
  ellipse(x + 10, yy - 34, 5, 4, 0xff5b6c60);
  box(x - 7, yy - 49, 15, 15, dark);
  rect(x - 6, yy - 46, 13, 9, skin);
  rect(x - 7, yy - 49, 15, 5, 0xff5f6654);
  rect(x - 5, yy - 37, 12, 3, 0xff78806a);
  point(x - 3, yy - 42, 0xffe4b76b);
  point(x + 4, yy - 42, 0xffe4b76b);
  line(x - 6, yy - 48, x + 3, yy - 48, 0xff9da58c);
  float bladeAngle = std::atan2(g.py - e.y, g.px - e.x) - .65f;
  if (!idle && e.kind == 0) {
    float aim = std::atan2(e.wind > 0 ? e.ty - e.y : g.py - e.y,
                           e.wind > 0 ? e.tx - e.x : g.px - e.x);
    if (e.wind > .12f) {
      float p = clamp((e.windMax - e.wind) / (e.windMax - .12f), 0, 1);
      bladeAngle = aim - .65f - .8f * (p * p * (3 - 2 * p));
    } else if (e.wind > 0) {
      float p = 1 - e.wind / .12f;
      bladeAngle = aim - 1.45f + 1.45f * (p * p * (3 - 2 * p));
    } else if (e.stateTime > 0) {
      float p = clamp(1 - e.stateTime / .18f, 0, 1);
      bladeAngle = aim + (p < .4f ? 1.15f * std::sin(p / .4f * PI * .5f)
                                  : 1.15f - 1.8f * ((p - .4f) / .6f));
    }
  }
  int hx = x + face * 16, hy = yy - 22 - int(action * 4);
  if (!idle && e.kind == 0) {
    hx = x + int(std::cos(bladeAngle) * 17);
    hy = yy - 28 + int(std::sin(bladeAngle) * 12);
  }
  thickLine(x + face * 10, yy - 32, hx, hy, 0xff887658, 2);
  rect(hx - 2, hy - 2, 5, 5, skin);
  thickLine(x - face * 10, yy - 32, x - face * 14, yy - 19, coat, 2);
  if (idle && e.routine == COOKING) {
    line(hx, hy, hx + int(action * 4), hy + 17, 0xffbda677);
    ellipse(x + 5, yy - 17, 7, 3, 0xff9b9879);
  } else if (idle && e.routine == WORKING) {
    line(hx, hy, hx + 4, hy - 14, 0xffbba16e);
    box(hx - 2, hy - 20, 13, 6, 0xffa0aba0);
  } else if (idle && e.routine == EATING) {
    ellipse(x + 6, yy - 22, 7, 3, 0xffb9b494);
    line(hx, hy, x + 4, yy - 40 + int(action * 3), 0xffd4c297);
  } else if (idle && e.routine == FETCHING) {
    box(hx - 4, hy + 3, 12, 13, 0xff7e8f81, dark);
    line(hx - 3, hy + 7, hx + 6, hy + 7, 0xffb2ad89);
  } else if (idle && e.routine == GATHERING) {
    box(x - 8, yy - 22, 20, 15, 0xff8a7955, dark);
    for (int j = 0; j < 4; j++)
      line(x - 6 + j * 5, yy - 21, x - 6 + j * 5, yy - 9, 0xffc4b087);
  } else if (e.kind == 3) {
    poly({{hx - 9, hy - 12},
          {hx + 9, hy - 12},
          {hx + 10, hy + 6},
          {hx, hy + 14},
          {hx - 10, hy + 6}},
         0xffb2b396);
    poly({{hx - 7, hy - 10},
          {hx + 7, hy - 10},
          {hx + 7, hy + 5},
          {hx, hy + 11},
          {hx - 7, hy + 5}},
         0xff607364);
    line(hx, hy - 8, hx, hy + 7, 0xffb0a775);
  } else if (e.kind == 1) {
    arc(hx, hy, 18, -1.3f, 1.3f, 0xffb2a077, 2);
    line(hx + 5, hy - 17, hx + 5, hy + 17, 0xffd1c5a6);
  } else if (e.kind == 4) {
    line(hx, hy + 13, hx, hy - 28, 0xffa6956d);
    circle(hx, hy - 29, 4, 0xff78988b, true);
    point(hx - 1, hy - 30, 0xffdbd5a7);
  } else {
    float a = idle ? -.95f : bladeAngle;
    int tipx = hx + int(std::cos(a) * 25), tipy = hy + int(std::sin(a) * 25);
    thickLine(hx, hy, tipx, tipy, 0xff3b4841, 2);
    line(hx, hy, tipx, tipy, 0xffc4c7b1);
    line(hx - int(std::sin(a) * 5), hy + int(std::cos(a) * 5),
         hx + int(std::sin(a) * 5), hy - int(std::cos(a) * 5), 0xffbdab78);
  }
  if (e.elite)
    circle(x, yy - 58, 4, GOLD);
  if (e.hp < e.maxhp)
    bar(x - 17, yy - 56, 34, 5, e.hp / e.maxhp, RED);
  if (e.wind > 0)
    center(x, yy - 69, "!", GOLD, 2);
}
void drawDayLight() {
  struct Key {
    float h, r, g, b;
  };
  static const Key keys[] = {{0, .48f, .56f, .72f},   {4, .40f, .48f, .64f},
                             {5, .57f, .60f, .70f},   {6.5f, .98f, .84f, .70f},
                             {8, 1.03f, 1.02f, .94f}, {12, 1.08f, 1.07f, 1.01f},
                             {16, 1.04f, .98f, .86f}, {18, 1.01f, .78f, .59f},
                             {19, .74f, .63f, .68f},  {20.5f, .48f, .56f, .72f},
                             {24, .48f, .56f, .72f}};
  float h = worldHour();
  int k = 0;
  while (k < 9 && h > keys[k + 1].h)
    k++;
  float t = clamp((h - keys[k].h) / (keys[k + 1].h - keys[k].h), 0, 1);
  t = t * t * (3 - 2 * t);
  float rr = keys[k].r + (keys[k + 1].r - keys[k].r) * t,
        gg = keys[k].g + (keys[k + 1].g - keys[k].g) * t,
        bb = keys[k].b + (keys[k + 1].b - keys[k].b) * t;
  for (int i = 0; i < W * H; i++) {
    C c = pix[i];
    int r = std::min(255, int((c >> 16 & 255) * rr)),
        g = std::min(255, int((c >> 8 & 255) * gg)),
        b = std::min(255, int((c & 255) * bb));
    pix[i] = 0xff000000 | (r << 16) | (g << 8) | b;
  }
  bool night = h < 7 || h > 18;
  if ((h < 6 || h > 19) && (g.world == 0 || g.world == 3))
    for (int j = 0; j < 12; j++) {
      int x = int(hash(j + 681) % W + std::sin(g.time * .7f + j) * 9),
          y = int(hash(j + 997) % H + std::cos(g.time * .4f + j) * 5);
      int a = int(60 + 60 * std::sin(g.time * 1.8f + j));
      blendPixel(x, y, 0xffc6d795, a);
      blendPixel(x + 1, y, 0xffc6d795, a / 2);
    }
  if (h > 4.5f && h < 8 && g.world != 1 && g.world != 4) {
    float strength = 1 - std::abs(h - 6.25f) / 1.75f;
    for (int j = 0; j < 4; j++) {
      int y = int((hash(j + 771) % 360) + std::sin(g.time * .08f + j) * 18);
      for (int yy = std::max(0, y - 10); yy < std::min(H, y + 11); yy++)
        for (int x = 0; x < W; x++) {
          float fade = 1 - std::abs(yy - y) / 11.f;
          int alpha = int(9 * strength * fade);
          if (std::abs(x - sx(g.px)) < 22 &&
              std::abs(yy - syAt(g.px, g.py) + 25) < 35)
            alpha /= 4;
          blendPixel(x, yy, 0xffbdc8b9, alpha);
        }
    }
  }
}
void drawMotionBlur() {
  static std::array<C, W * H> previous;
  static bool valid = false;
  static double lastX = 0, lastY = 0;
  static uint64_t seed = 0;
  static std::string id;
  double x = o.originX * double(T) + g.camx, y = o.originY * double(T) + g.camy;
  double dx = x - lastX, dy = y - lastY;
  bool use = valid && motionBlur && !g.lowPower && !g.overlay &&
             g.scene == PLAY && renderDt > 0 && renderDt < .08f &&
             seed == o.seed && id == o.id && std::abs(dx) < 20 &&
             std::abs(dy) < 20;
  static std::array<C, W * H> current;
  std::copy(pix, pix + W * H, current.begin());
  if (use && (len(g.vx, g.vy) > 70 || g.attackTime > 0 || g.dash > 0)) {
    int ix = int(std::round(dx)), iy = int(std::round(dy));
    int alpha = g.dash > 0 ? 32 : 19;
    for (int yy = std::max(0, -iy); yy < std::min(H, H - iy); yy++)
      for (int xx = std::max(0, -ix); xx < std::min(W, W - ix); xx++) {
        C a = previous[(yy + iy) * W + xx + ix];
        pix[yy * W + xx] = mix(current[yy * W + xx], a, alpha);
      }
  }
  previous = current;
  lastX = x;
  lastY = y;
  seed = o.seed;
  id = o.id;
  valid = g.scene == PLAY && !g.overlay && renderDt > 0;
}
void drawExploreAtlas() {
  static std::array<C, 360 * 260> cache;
  static int64_t lastX = INT64_MAX, lastY = INT64_MAX;
  static int lastZoom = -1;
  static std::string lastId;
  static uint64_t lastSeed = 0;
  int scale = 1 << w.mapZoom;
  int64_t px = int64_t(globalX()), py = int64_t(globalY());
  if (w.atlasDirty || lastX != px || lastY != py || lastZoom != w.mapZoom ||
      lastId != o.id || lastSeed != o.seed) {
    cache.fill(0xff030708);
    for (auto &[key, bits] : w.explored) {
      if (key.first * 32 < px - 181 * scale - 32 ||
          key.first * 32 > px + 181 * scale ||
          key.second * 32 < py - 131 * scale - 32 ||
          key.second * 32 > py + 131 * scale)
        continue;
      for (int i = 0; i < 16; i++) {
        uint64_t z = bits[i];
        while (z) {
          int bit = std::countr_zero(z), n = i * 64 + bit;
          z &= z - 1;
          int64_t x = key.first * 32 + n % 32, y = key.second * 32 + n / 32;
          int xx = 180 + int(floorDiv(x - px, scale)),
              yy = 130 + int(floorDiv(y - py, scale));
          if (xx >= 0 && xx < 360 && yy >= 0 && yy < 260)
            cache[yy * 360 + xx] = earthColor(biomeAt(x, y));
        }
      }
    }
    lastX = px;
    lastY = py;
    lastZoom = w.mapZoom;
    lastId = o.id;
    lastSeed = o.seed;
    w.atlasDirty = false;
  }
  for (int y = 0; y < 260; y++)
    for (int x = 0; x < 360; x++)
      point(69 + x, 65 + y, cache[y * 360 + x]);
  circle(249, 195, 4, WHITE, true);
  circle(249, 195, 6, INK);
  if (o.waypoint && exploredAt(o.waypointX, o.waypointY)) {
    int x = 249 + int((o.waypointX - px) / scale),
        y = 195 + int((o.waypointY - py) / scale);
    if (x > 74 && x < 423 && y > 71 && y < 318)
      circle(x, y, 5, GOLD);
  }
  button(77, 290, 87, 23, "ZOOM +");
  button(170, 290, 87, 23, "ZOOM -");
  panel(75, 314, 341, 13);
  text(80, 317, num(scale) + " TILES/PIXEL / BLACK = UNEXPLORED", WHITE);
}
void woodsScreen() {
  dim();
  box(44, 20, 552, 321, PANEL, GOLD);
  text(66, 38, "WILD EARTH / FIELD KIT", GOLD, 2);
  button(552, 32, 28, 25, "X");
  text(68, 81, "WOOD " + num(w.wood) + " / TREES FELLED " + num(w.felled),
       WHITE, 2);
  text(68, 109, "EQUIP THE AXE. FACE A TREE AND HOLD ATTACK.", DIM);
  text(68, 125, "CUT TRUNKS BECOME WALKABLE. STUMPS STAY SAVED.", DIM);
  button(69, 153, 239, 32, "CRAFT SPLINT: 4 WOOD + 1 FIBER");
  button(321, 153, 249, 32, "BUILD CAMPFIRE: 8 WOOD");
  button(69, 199, 239, 32,
         motionBlur ? "MOTION BLUR: SUBTLE" : "MOTION BLUR: OFF");
  button(321, 199, 249, 32,
         g.lowPower ? "QUALITY: BATTERY / 30 TARGET"
                    : "QUALITY: FULL / 60 TARGET");
  text(68, 250, "DAY LENGTH: 24 MINUTES OF ACTIVE SIMULATION", GOLD);
  text(68, 269, "DAWN / MORNING / NOON / AFTERNOON / DUSK / NIGHT", DIM);
  text(68, 288, "MOTION BLUR IS DISABLED IN BATTERY MODE.", DIM);
  if (g.toastTime > 0)
    center(320, 321, g.toast.substr(0, 87), GOLD);
}
bool woodsTouch(float x, float y) {
  auto in = [&](int a, int b, int ww, int hh) {
    return x >= a && x < a + ww && y >= b && y < b + hh;
  };
  if (!g.openWorld)
    return false;
  if (g.overlay == 6 && o.atlasLarge) {
    if (in(77, 290, 87, 23)) {
      w.mapZoom = std::max(0, w.mapZoom - 1);
      return true;
    }
    if (in(170, 290, 87, 23)) {
      w.mapZoom = std::min(9, w.mapZoom + 1);
      return true;
    }
  }
  if (g.overlay == 9) {
    if (in(552, 32, 28, 25)) {
      g.overlay = 0;
      return true;
    }
    if (in(69, 153, 239, 32))
      craftSplint();
    if (in(321, 153, 249, 32))
      buildCampfire();
    if (in(69, 199, 239, 32)) {
      motionBlur = !motionBlur;
      saveSettings();
    }
    if (in(321, 199, 249, 32)) {
      g.lowPower = !g.lowPower;
      saveSettings();
    }
    return true;
  }
  if (g.scene == PLAY && !g.overlay && in(435, 43, 92, 23)) {
    clearInput();
    g.overlay = 9;
    return true;
  }
  return false;
}
} // namespace av
