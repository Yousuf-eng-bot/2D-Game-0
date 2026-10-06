#pragma once
// Vista 0.8: original CPU pixel renderer. Cosmetic data never enters world
// saves.
namespace av {
int vistaQuality = 1;
bool vistaFocus = true;
int vistaTier() { return g.lowPower || graphicsCoolingBudget() ? 0 : mediumEnabled() ? 2 : vistaQuality; }
void vistaLoad() {
  vistaQuality = 1;
  vistaFocus = true;
  std::string raw, magic;
  int q, f;
  if ((readChecked(g.path + "/visual8.cfg", raw) ||
       readChecked(g.path + "/visual8.cfg.bak", raw))) {
    std::istringstream s(raw);
    if (s >> magic >> q >> f && magic == "DWVIS1" && q >= 0 && q <= 2 &&
        f >= 0 && f <= 1) {
      vistaQuality = q;
      vistaFocus = f;
    }
  }
}
bool vistaSave() {
  return atomicWorld(g.path + "/visual8.cfg", "DWVIS1 " + num(vistaQuality) +
                                                  " " + num(vistaFocus) + "\n");
}
struct VistaDecor {
  float x, y;
  uint32_t seed;
  int biome, kind;
};
std::vector<VistaDecor> vistaDecor;
std::array<uint8_t, W * H> vistaWater{};
struct VistaLight {
  float x, y, r;
};
std::vector<VistaLight> vistaLights;
void vistaGatherLights() {
  vistaLights.clear();
  auto add = [&](float x, float y, float r) {
    if (x > g.camx - r && x < g.camx + W + r && y > g.camy - r &&
        y < g.camy + H + r)
      vistaLights.push_back({x, y, r});
  };
  for (auto &p : g.props)
    if (p.kind == 12 || p.kind == 22)
      add(p.x, p.y, p.kind == 12 ? 120 : 96);
  for (auto &[k, b] : j.built)
    if (b.kind == TORCH || (b.kind == FURNACE && b.progress > 0))
      add(float(k.first - o.originX) * T + 12,
          float(k.second - o.originY) * T + 12, 106);
  std::stable_sort(
      vistaLights.begin(), vistaLights.end(), [](auto &a, auto &b) {
        return len(a.x - g.px, a.y - g.py) < len(b.x - g.px, b.y - g.py);
      });
  if (vistaLights.size() > size_t(vistaTier() == 2 ? 6 : 3))
    vistaLights.resize(vistaTier() == 2 ? 6 : 3);
}
// Coarse shape first, coherent clusters second, tiny highlights last: no
// white-noise carpet.
C vistaGround(int b, int ground, int64_t wx, int64_t wy, int xx, int yy) {
  static const C lo[] = {0xff435842, 0xffa78b61, 0xff99b3bc, 0xff304d49,
                         0xff44434a};
  static const C hi[] = {0xff91a15e, 0xffd3b784, 0xffe5e6d8, 0xff72876a,
                         0xff8d7b6d};
  static int64_t lastX = 0, lastY = 0;
  static uint64_t lastSeed = 0, h = 0;
  static int macro = 0;
  static bool valid = false;
  if (!valid || lastX != wx || lastY != wy || lastSeed != o.seed) {
    lastX = wx;
    lastY = wy;
    lastSeed = o.seed;
    h = coordinateHash(wx, wy, 9011);
    macro = int(noiseAt(wx, wy, 9, 9013) * 45);
    valid = true;
  }
  int cluster = int(
      hash(uint32_t(h) + uint32_t(xx / 4) * 917 + uint32_t(yy / 3) * 719) % 5);
  int level = 100 + cluster * 14 + macro;
  C base = mix(lo[b], hi[b], level);
  if (ground == 2 || ground == 4) {
    C soil = b == 2 ? 0xff9eaba7 : b == 4 ? 0xff78645c : 0xffa29670;
    base = mix(base, soil, ground == 2 ? 230 : 185);
  }
  return base;
}
void drawVistaTerrain() {
  vistaDecor.clear();
  vistaWater.fill(0);
  int ix = std::max(0, int(g.camx / T) - 2),
      iy = std::max(0, int(g.camy / T) - 2);
  for (int y = iy; y < std::min(MH, iy + H / T + 9); y++)
    for (int x = ix; x < std::min(MW, ix + W / T + 5); x++) {
      int i = y * MW + x, b = std::clamp(int(g.biomes[i]), 0, 4), m = g.map[i],
          ground = g.ground[i];
      int64_t wx = o.originX + x, wy = o.originY + y;
      uint64_t h = coordinateHash(wx, wy, 9071);
      int xx = sx(x * T), yy = syAt(x * T + 12, y * T + 12) - 12;
      if (m == 4) {
        C deep = b == 4   ? 0xff502d29
                 : b == 3 ? 0xff244947
                 : b == 2 ? 0xff355b70
                          : 0xff245663;
        C shall = b == 4 ? 0xffdb7d46 : b == 3 ? 0xff698976 : 0xff74a99b;
        rect(xx, yy, T, T, deep);
        for (int cy = std::max(0, yy); cy < std::min(H, yy + T); cy++)
          for (int cx = std::max(0, xx); cx < std::min(W, xx + T); cx++)
            vistaWater[cy * W + cx] = 1;
        for (int n = 0; n < 7; n++) {
          int py = yy + n * 4 + int(h % 3),
              shift =
                  int(std::sin(g.time * .8f + float(wy % 100) * .6f + n) * 3);
          int px = xx + int((h >> (n * 5)) % 16) + shift;
          line(px, py, px + 5 + int(h % 8), py,
               mix(deep, shall, 35 + int((h >> (n * 3)) % 80)));
        }
        int sides[] = {
            x > 0 && g.map[i - 1] != 4, x < MW - 1 && g.map[i + 1] != 4,
            y > 0 && g.map[i - MW] != 4, y < MH - 1 && g.map[i + MW] != 4};
        for (int side = 0; side < 4; side++)
          if (sides[side])
            for (int q = 0; q < T; q++) {
              int edge = 2 + int(hash(uint32_t(h) + q / 3) % 3);
              for (int d = 0; d < edge; d++) {
                int a = side == 0   ? d
                        : side == 1 ? T - 1 - d
                                    : q,
                    c = side == 2   ? d
                        : side == 3 ? T - 1 - d
                                    : q;
                point(xx + a, yy + c, mix(shall, deep, d * 45));
              }
              if (q % 7 == 0) {
                int a = side == 0   ? edge
                        : side == 1 ? T - 1 - edge
                                    : q,
                    c = side == 2   ? edge
                        : side == 3 ? T - 1 - edge
                                    : q;
                point(xx + a, yy + c, mix(shall, WHITE, 70));
              }
            }
        if (b == 3 && h % 9 == 0) {
          ellipse(xx + 11, yy + 13, 5, 2, 0xff648765);
          point(xx + 10, yy + 12, 0xffcfb6a1);
        }
        continue;
      }
      for (int cy = std::max(0, yy); cy < std::min(H, yy + T); cy++)
        for (int cx = std::max(0, xx); cx < std::min(W, xx + T); cx++)
          vistaWater[cy * W + cx] = 0;
      // Quantized 4x3 material clusters; coordinate-anchored and stable under
      // camera movement.
      for (int dy = 0; dy < T; dy += 3)
        for (int dx = 0; dx < T; dx += 4)
          rect(xx + dx, yy + dy, 4, 3, vistaGround(b, ground, wx, wy, dx, dy));
      C base = vistaGround(b, ground, wx, wy, 12, 12);
      if (ground == 3) {
        rect(xx, yy, T, T, 0xff403d34);
        for (int q = 0; q < T; q += 6) {
          rect(xx + 1, yy + q, 22, 5, 0xff8c795b);
          line(xx + 2, yy + q, xx + 21, yy + q, 0xffc1ae7d);
          line(xx + 4, yy + q + 3, xx + 16, yy + q + 3, 0xff6c5c46);
          point(xx + 3, yy + q + 2, INK);
          point(xx + 20, yy + q + 2, INK);
        }
      } else {
        for (int n = 0; n < 9; n++) {
          uint32_t z = hash(uint32_t(h) + n * 691);
          int dx = int(z % 21), dy = int((z >> 8) % 22);
          C c = mix(base, n % 3 ? 0xffd0bf8d : 0xff243e35, n % 3 ? 35 : 55);
          rect(xx + dx, yy + dy, 2 + int((z >> 14) % 4), 1, c);
          if (!ground && b != 1 && b != 4 && n % 3 == 0) {
            line(xx + dx, yy + dy, xx + dx - 1, yy + dy - 3,
                 mix(base, 0xff263d35, 100));
            point(xx + dx - 1, yy + dy - 3, mix(base, 0xffc6cf91, 90));
          }
        }
        if (ground == 2 || ground == 4) {
          for (int side = 0; side < 4; side++) {
            int near = side == 0   ? i - 1
                       : side == 1 ? i + 1
                       : side == 2 ? i - MW
                                   : i + MW;
            if (near < 0 || near >= MW * MH || g.ground[near] != 0 ||
                g.map[near] == 4)
              continue;
            for (int q = 0; q < T; q++) {
              int reach = 1 + int(hash(uint32_t(h) + q / 3) % 5);
              for (int d = 0; d < reach; d++)
                point(xx + (side == 0   ? d
                            : side == 1 ? T - 1 - d
                                        : q),
                      yy + (side == 2   ? d
                            : side == 3 ? T - 1 - d
                                        : q),
                      vistaGround(b, 0, wx, wy, q, d));
            }
          }
        }
        if (b == 1 && h % 3 == 0) {
          for (int q = 0; q < 3; q++) {
            line(xx + 2, yy + 7 + q * 5, xx + 10, yy + 5 + q * 5, 0xffb39a71);
            line(xx + 10, yy + 5 + q * 5, xx + 21, yy + 8 + q * 5, 0xffc7ae80);
          }
        }
        if (b == 4 && h % 4 == 0) {
          line(xx + 4, yy + 3, xx + 12, yy + 12, 0xff302f34);
          line(xx + 12, yy + 12, xx + 9, yy + 23, 0xff3a3439);
          point(xx + 11, yy + 14, 0xffb27651);
        }
      }
      // Height belongs to the existing save. Only its projection/material
      // treatment changes.
      int level = j.heights[i],
          down = y < MH - 1 ? level - int(j.heights[i + MW]) : 0;
      if (down > 0 && y < MH - 1) {
        int dep = down * 6;
        C rock = b == 1 ? 0xff8c6d50 : b == 2 ? 0xff566c78 : 0xff4b514a;
        rect(xx, yy + T, T, dep, rock);
        for (int cy = std::max(0, yy + T); cy < std::min(H, yy + T + dep); cy++)
          for (int cx = std::max(0, xx); cx < std::min(W, xx + T); cx++)
            vistaWater[cy * W + cx] = 0;
        for (int r = 0; r < dep; r += 3) {
          int shift = int(hash(uint32_t(h) + r) % 5);
          line(xx, yy + T + r, xx + 23, yy + T + r, mix(rock, INK, 40 + r * 3));
          for (int k = 0; k < 3; k++) {
            int dx = k * 9 + shift;
            line(xx + dx, yy + T + r, xx + dx - 1,
                 yy + T + std::min(dep - 1, r + 2), mix(rock, WHITE, 25));
          }
        }
        line(xx, yy + T - 1, xx + 23, yy + T - 1, mix(base, 0xffc9cca4, 95));
        shade(xx, yy + T + dep, T, 3, 0xff233632, 60);
        if (terrainLink(i, i + MW))
          for (int k = 0; k < down * 2; k++) {
            rect(xx + 7, yy + T + k * 3, 10, 3, 0xff77826e);
            line(xx + 7, yy + T + k * 3, xx + 16, yy + T + k * 3, 0xffc0bea0);
          }
      }
      if (x < MW - 1 && level > j.heights[i + 1]) {
        shade(xx + T - 3, yy, 3, T, 0xff20392f, 75);
        line(xx + T - 4, yy + 1, xx + T - 4, yy + T - 2, mix(base, WHITE, 35));
      }
      auto dug = j.mined.find({wx, wy});
      if (dug != j.mined.end() && dug->second == 2) {
        shade(xx + 1, yy + 2, 22, 20, 0xff594933, 110);
        line(xx + 2, yy + 3, xx + 21, yy + 3, 0xff382f2c);
        line(xx + 3, yy + 19, xx + 20, yy + 19, 0xffb09a6d);
      }
      if (!ground && m != 5 && h % (vistaTier() ? 3 : 5) == 0) {
        float px = x * T + 4 + float(h % 17),
              py = y * T + 5 + float((h >> 12) % 15);
        int kind = int((h >> 19) % 12);
        vistaDecor.push_back({px, py, uint32_t(h), b, kind});
      }
    }
}
void vistaReflections() {
  if (vistaTier() == 0)
    return;
  int step = vistaTier() == 2 ? 2 : 3;
  for (auto &p : g.props)
    if (p.kind == 0 && !treeCut(p)) {
      int x = sx(p.x), y = syAt(p.x, p.y);
      if (x < -70 || x > W + 70 || y < -45 || y > H)
        continue;
      auto k = treeKey(p);
      auto &s = treeSprite(treeSpecies(p), p.biome,
                           int(coordinateHash(k.first, k.second, 9187) % 8));
      for (int yy = 0; yy < 135; yy += step)
        for (int xx = 0; xx < 128; xx += step) {
          C c = s.pixels[yy * 128 + xx];
          if (!c)
            continue;
          int ox = x + xx - 64 + int(std::sin(g.time * 1.3f + yy * .3f) * 2),
              oy = y + int((135 - yy) * .34f);
          for (int dy = 0; dy < 1; dy++)
            for (int dx = 0; dx < step; dx++) {
              int a = ox + dx, b = oy + dy;
              if (a >= 0 && a < W && b >= 0 && b < H && vistaWater[b * W + a])
                blendPixel(a, b, c, 45 - (135 - yy) / 4);
            }
        }
    }
}
void vistaDecorArt(const VistaDecor &d) {
  int x = sx(d.x), y = syAt(d.x, d.y), b = d.biome;
  uint32_t h = d.seed;
  int sway = int(std::sin(g.time * 1.5f + float(h % 73)) * 1.4f);
  if (x < -35 || x > W + 35 || y < -8 || y > H + 45)
    return;
  C dark = b == 1   ? 0xff697052
           : b == 2 ? 0xff486960
           : b == 4 ? 0xff4a4940
                    : 0xff294f3d;
  C mid = b == 1   ? 0xff9e9a62
          : b == 2 ? 0xff7f9b83
          : b == 4 ? 0xff81745c
                   : 0xff63834b;
  C light = b == 1   ? 0xffc8ba7f
            : b == 2 ? 0xffc4d2bc
            : b == 4 ? 0xffac956b
                     : 0xffb0b876;
  if (d.kind < 4) { // ferns, meadow blades, reeds and alpine tufts
    for (int k = -3; k <= 3; k++) {
      int ht = 5 + int(hash(h + k) % 12), ex = x + k * 3 + sway, ey = y - ht;
      line(x + k, y, ex, ey, dark);
      line(x + k, y - 1, ex, ey - 1, mid);
      if (d.kind == 0 && b != 1 && b != 4) {
        for (int n = 2; n < ht; n += 3) {
          int px = x + k + (ex - x - k) * n / ht, py = y - n;
          line(px, py, px - 3, py - 2, mid);
          line(px, py, px + 3, py - 1, light);
        }
      } else
        point(ex, ey, light);
      if (d.kind == 2 && b != 4) {
        rect(ex - 1, ey - 2, 3, 2, b == 2 ? 0xffe7e5d5 : 0xffc7b79a);
      }
    }
  } else if (d.kind <
             7) { // clustered bushes, not additional harvest/collision objects
    ellipse(x, y + 1, 14, 3, 0xff354734);
    for (int k = 0; k < 4; k++) {
      int dx = int(hash(h + k) % 23) - 11, dy = -6 - int(hash(h + 9 + k) % 9);
      foliage(x + dx + sway, y + dy, 7 + int(h % 4), 6, dark, light,
              h + k * 91);
    }
    if (b == 2) {
      line(x - 8, y - 14, x + 4, y - 14, 0xffd5ddd0);
      line(x - 2, y - 19, x + 9, y - 19, 0xffd5ddd0);
    }
  } else if (d.kind == 7) { // fallen timber with lit cut end and moss
    thickLine(x - 9, y - 2, x + 12, y - 6, 0xff403b32, 3);
    thickLine(x - 9, y - 4, x + 11, y - 8, 0xff857251, 2);
    line(x - 7, y - 5, x + 10, y - 9, 0xffb5a073);
    ellipse(x + 12, y - 6, 3, 4, 0xffbaaa7b);
    line(x - 3, y - 5, x - 5, y - 13, 0xff625d42);
    if (b == 0 || b == 3)
      line(x - 9, y - 6, x + 2, y - 8, mid);
  } else if (d.kind == 8) {
    for (int k = 0; k < 3; k++) {
      int dx = k * 7 - 7;
      poly({{x + dx - 4, y},
            {x + dx - 3, y - 4 - k},
            {x + dx + 1, y - 7 - k},
            {x + dx + 5, y - 2},
            {x + dx + 3, y + 1}},
           b == 1 ? 0xff9b835f : 0xff626f68);
      line(x + dx - 3, y - 4 - k, x + dx + 1, y - 7 - k,
           b == 2 ? 0xffd3dfd8 : 0xffb5b7a0);
    }
  } else if (d.kind == 9 && (b == 0 || b == 3)) {
    for (int k = 0; k < 3; k++) {
      int dx = k * 5 - 5;
      line(x + dx, y, x + dx, y - 5 - k, 0xffc1b291);
      ellipse(x + dx, y - 6 - k, 3, 2, k % 2 ? 0xffb89471 : 0xffa8755f);
      point(x + dx - 1, y - 7 - k, 0xffddc89d);
    }
  } else {
    for (int k = 0; k < 5; k++) {
      int dx = int(hash(h + k) % 22) - 11, dy = int(hash(h + k * 37) % 9) - 4;
      line(x + dx, y + dy, x + dx + 3, y + dy - 1,
           b == 2 ? 0xffdce3d6 : 0xffa9966a);
      point(x + dx + 2, y + dy - 2, mid);
    }
  }
}
void vistaUnderstory(const Prop &p) {
  auto k = treeKey(p);
  uint32_t h = uint32_t(coordinateHash(k.first, k.second, 9181));
  vistaDecorArt({p.x, p.y, h, p.biome, int(h % 7)});
}
constexpr int VSW = W / 2, VSH = H / 2;
std::array<uint8_t, VSW * VSH> vistaShadow{}, vistaTemp{};
void vistaStamp(float x, float y, int rx, int ry, int alpha) {
  int cx = int(x / 2), cy = int(y / 2);
  rx = std::max(1, rx / 2);
  ry = std::max(1, ry / 2);
  for (int yy = std::max(0, cy - ry); yy < std::min(VSH, cy + ry + 1); yy++)
    for (int xx = std::max(0, cx - rx); xx < std::min(VSW, cx + rx + 1); xx++) {
      float dx = float(xx - cx) / rx, dy = float(yy - cy) / ry,
            d = dx * dx + dy * dy;
      if (d <= 1)
        vistaShadow[yy * VSW + xx] =
            std::max(vistaShadow[yy * VSW + xx],
                     uint8_t(alpha * std::min(1.f, (1 - d) * 4)));
    }
}
void drawVistaShadows() {
  vistaGatherLights();
  vistaShadow.fill(0);
  auto sun = sunProjection();
  auto cast = [&](float px, float py, float ht, int width, int depth, bool tree,
                  int variant, int species, int biome) {
    int x = sx(px), y = syAt(px, py);
    if (x < -300 || x > W + 300 || y < -90 || y > H + 220)
      return;
    if (tree && vistaTier() > 0) {
      auto &s = treeSprite(species, biome, variant);
      for (int yy = 0; yy < TreeSprite::height; yy += 3)
        for (int xx = 0; xx < TreeSprite::width; xx += 3)
          if (s.pixels[yy * TreeSprite::width + xx]) {
            float tall = 135 - yy;
            vistaStamp(x + (xx - 64) * .83f + sun.dx * tall * sun.length,
                       y + sun.dy * tall * sun.length, 4, 3, sun.alpha + 30);
          }
    } else
      for (int n = 0; n < 7; n++) {
        float t = n / 6.f;
        vistaStamp(x + sun.dx * ht * sun.length * t,
                   y + sun.dy * ht * sun.length * t,
                   int(width * (.5f + .5f * t)), depth, sun.alpha + 25);
      }
    if (worldHour() < 7 || worldHour() > 18) {
      for (auto &l : vistaLights) {
        float dx = px - l.x, dy = py - l.y, d = len(dx, dy);
        if (d < 12 || d > l.r)
          continue;
        float length = std::min(90.f, ht * d / 55);
        for (int n = 1; n <= 6; n++) {
          float t = n / 6.f;
          vistaStamp(x + dx / d * length * t, y + dy / d * length * .55f * t,
                     int(width * (.4f + .35f * t)), depth,
                     int(65 * (1 - d / l.r)));
        }
        break;
      }
    }
  };
  for (auto &p : g.props) {
    if (p.kind == 0)
      cast(p.x, p.y, treeCut(p) ? 5 : 100, treeCut(p) ? 8 : 28,
           treeCut(p) ? 3 : 8, !treeCut(p),
           int(coordinateHash(treeKey(p).first, treeKey(p).second, 9187) % 8),
           treeSpecies(p), p.biome);
    else if (p.kind == 1 || p.kind == 20 || p.kind == 21 || p.kind == 23 ||
             p.kind == 26 || p.kind == 12)
      cast(p.x, p.y, p.kind == 1 ? 27 : 22, 15, 5, false, 0, 0, 0);
  }
  for (auto &e : g.enemies)
    if (e.alive)
      cast(e.x, e.y, e.boss() ? 65 : 41, e.boss() ? 19 : 10, 4, false, 0, 0, 0);
  for (auto &a : g.animals)
    if (a.alive && a.species != INSECT)
      cast(a.x, a.y, flying(a.species) ? 25 : 16, 9, 3, false, 0, 0, 0);
  for (auto &[k, b] : j.built) {
    float x = float(k.first - o.originX) * T + 12,
          y = float(k.second - o.originY) * T + 12;
    cast(x, y, b.kind == WALL || b.kind == DOOR ? 32 : 17, 12, 4, false, 0, 0,
         0);
  }
  for (auto &f : w.falling)
    cast(f.x + f.dir * f.time * 40, f.y, 100 * std::max(0.f, 1 - f.time), 22, 5,
         false, 0, 0, 0);
  cast(g.px, g.py, 49 + int(j.z), 10, 4, false, 0, 0, 0);
  if (vistaTier() > 0) {
    for (int y = 0; y < VSH; y++)
      for (int x = 0; x < VSW; x++) {
        int sum = 0;
        for (int d = -2; d <= 2; d++)
          sum += vistaShadow[y * VSW + std::clamp(x + d, 0, VSW - 1)] *
                 (3 - std::abs(d));
        vistaTemp[y * VSW + x] = sum / 9;
      }
    for (int y = 0; y < VSH; y++)
      for (int x = 0; x < VSW; x++) {
        int sum = 0;
        for (int d = -2; d <= 2; d++)
          sum += vistaTemp[std::clamp(y + d, 0, VSH - 1) * VSW + x] *
                 (3 - std::abs(d));
        vistaShadow[y * VSW + x] = sum / 9;
      }
  }
  // Contact shadows stay tight; broad penumbras do not make feet float.
  for (auto &p : g.props)
    if (p.kind == 0 || p.kind == 1)
      vistaStamp(sx(p.x), syAt(p.x, p.y) + 1, p.kind == 0 ? 15 : 20, 5, 115);
  vistaStamp(sx(g.px), syAt(g.px, g.py) + 1, 11, 4,
             int(110 / (1 + j.z * .025f)));
  for (int y = 0; y < H; y++)
    for (int x = 0; x < W; x++)
      if (auto a = vistaShadow[(y / 2) * VSW + x / 2])
        pix[y * W + x] = mix(pix[y * W + x], 0xff182d35, a);
}
void vistaGroundFocus() {
  if (!vistaFocus || vistaTier() == 0)
    return;
  static std::array<C, W * H> source;
  std::copy(pix, pix + W * H, source.begin());
  int hx = sx(g.px), hy = syAt(g.px, g.py);
  for (int y = 2; y < H - 2; y++)
    for (int x = 2; x < W - 2; x++) {
      int dx = x - hx, dy = y - hy;
      if (dx * dx + dy * dy < 150 * 150)
        continue;
      int edge = std::max(0, 50 - y) + std::max(0, y - 300);
      int a = std::min(vistaTier() == 2 ? 72 : 45, edge);
      if (!a)
        continue;
      C c =
          mix(mix(source[y * W + x - 2], source[y * W + x + 2], 128),
              mix(source[(y - 2) * W + x], source[(y + 2) * W + x], 128), 128);
      pix[y * W + x] = mix(source[y * W + x], c, a);
    }
}
// Local-light blocker test: terrain occlusion, not a 3D shadow-map claim.
bool vistaLightBlocked(float ax, float ay, float bx, float by) {
  int steps = std::min(24, std::max(1, int(len(bx - ax, by - ay) / 12)));
  int base = heightLocal(ax, ay);
  for (int n = 1; n < steps; n++) {
    float t = float(n) / steps;
    int x = int((ax + (bx - ax) * t) / T), y = int((ay + (by - ay) * t) / T);
    if (x < 0 || x >= MW || y < 0 || y >= MH)
      continue;
    int i = y * MW + x;
    if (g.map[i] == 5 || g.map[i] == 3 || int(j.heights[i]) > base + 1)
      return true;
  }
  return false;
}
void drawVistaLight() {
  // Keep proven day-cycle interpolation, replacing only its extra glow below
  // via stronger occluded emitters.
  drawDayLight();
  float hour = worldHour();
  bool night = hour < 7 || hour > 18;
  if (night) {
    for (auto &l : vistaLights) {
      int cx = sx(l.x), cy = syAt(l.x, l.y) - 7, r = int(l.r);
      float flicker = .94f + .06f * std::sin(g.time * 8 + l.x);
      int step = vistaTier() == 0 ? 8 : 4;
      for (int y = std::max(0, cy - r * 2 / 3); y < std::min(H, cy + r * 2 / 3);
           y += step)
        for (int x = std::max(0, cx - r); x < std::min(W, cx + r); x += step) {
          float dx = float(x - cx) / r, dy = float(y - cy) / (r * .68f),
                d = dx * dx + dy * dy;
          if (d >= 1)
            continue;
          float wx = x + g.camx, wy = y + g.camy + vistaElevation(l.x, l.y);
          bool blocked = vistaTier() > 0 && vistaLightBlocked(l.x, l.y, wx, wy);
          int a = int((1 - d) * (1 - d) * 95 * flicker * (blocked ? .14f : 1));
          shade(x, y, std::min(step, W - x), std::min(step, H - y), 0xffffbd72,
                a);
        }
    }
  }
  if (vistaTier() > 0 && g.world != 1 &&
      g.world != 4) { // slow, broad cloud shadows; tiny airborne seeds. No
                      // gameplay RNG.
    float drift = g.time * .055f;
    for (int y = 0; y < H; y += 4)
      for (int x = 0; x < W; x += 4) {
        float wx = float(o.originX * T + g.camx + x),
              wy = float(o.originY * T + g.camy + y);
        float z =
            std::sin(wx * .009f + drift) + std::sin(wy * .013f - drift * .6f);
        int a = int(std::max(0.f, z - .4f) * (night ? 3 : 9));
        if (a)
          shade(x, y, 4, 4, 0xff294351, a);
      }
    for (int n = 0; n < (vistaTier() == 2 ? 18 : 8); n++) {
      uint32_t h = hash(n + 931);
      float wx =
          std::fmod(float(h % 3000) + g.time * (1 + n % 3), float(MW * T));
      float wy =
          std::fmod(float((h >> 12) % 2000) + g.time * .4f, float(MH * T));
      int x = sx(wx), y = sy(wy);
      if (x > 0 && x < W && y > 0 && y < H)
        blendPixel(x, y, night ? 0xffb0d0a1 : 0xffe4d5a3, night ? 70 : 80);
    }
  }
}
} // namespace av
