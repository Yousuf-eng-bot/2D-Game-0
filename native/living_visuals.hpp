#pragma once
// Included after the UI primitives, before presentation. Original raster art.
namespace av {
void drawLifeProp(const Prop &p) {
  int x = sx(p.x), y = syAt(p.x, p.y);
  if (x < -60 || x > W + 60 || y < -30 || y > H + 70)
    return;
  if (p.kind == 20) {
    C leaf = p.biome == 2                   ? 0xffa3c2bb
             : p.biome == 1 || p.biome == 4 ? 0xff939454
                                            : 0xff447348;
    rect(x - 2, y - 11, 4, 13, 0xff735643);
    ellipse(x, y - 9, 12, 8, leaf);
    ellipse(x - 6, y - 16, 7, 6, mix(leaf, WHITE, 45));
    ellipse(x + 6, y - 16, 7, 6, leaf);
    if (plantReady(p))
      for (int j = 0; j < 5; j++)
        rect(x - 8 + j * 4, y - 13 + (j % 2) * 5, 3, 3,
             p.biome == 2 ? 0xff8078bd : 0xffe7a368);
  } else if (p.kind == 21) {
    ellipse(x, y + 2, 16, 7, 0xff647d7c);
    box(x - 14, y - 17, 28, 19, 0xff9aaca3, 0xff485f61);
    ellipse(x, y - 16, 14, 6, 0xffd7d3ac);
    ellipse(x, y - 16, 10, 4, 0xff416d78);
    rect(x - 18, y - 40, 3, 38, 0xff715343);
    rect(x + 15, y - 40, 3, 38, 0xff715343);
    line(x - 19, y - 40, x + 19, y - 40, GOLD);
    line(x, y - 40, x, y - 17, 0xffc9b07d);
    rect(x - 5, y - 16, 10, 7, 0xff8da3a1);
  } else if (p.kind == 22) {
    ellipse(x, y + 1, 19, 8, 0xff584b45);
    for (int j = 0; j < 7; j++) {
      float a = j * PI * 2 / 7;
      ellipse(x + int(std::cos(a) * 16), y + int(std::sin(a) * 6), 4, 3,
              0xff9b9984);
    }
    line(x - 9, y, x + 9, y - 3, 0xffb98b4d);
    line(x - 8, y - 3, x + 8, y, 0xff72543a);
    int flick = int(std::sin(g.time * 11 + x) * 3);
    poly({{x - 8, y - 3},
          {x - 5, y - 17 - flick},
          {x, y - 10},
          {x + 4, y - 23 + flick},
          {x + 9, y - 3}},
         0xffd87f3f);
    poly({{x - 4, y - 3}, {x, y - 16}, {x + 5, y - 3}}, 0xffffd382);
    line(x - 20, y, x - 10, y - 29, INK);
    line(x + 20, y, x + 10, y - 29, INK);
    line(x - 10, y - 29, x + 10, y - 29, INK);
    line(x, y - 29, x, y - 19, INK);
    ellipse(x, y - 10, 10, 6, 0xff263a3d);
    rect(x - 10, y - 16, 20, 6, 0xff334749);
    ellipse(x, y - 16, 10, 3, 0xff91896c);
    int steam = int(g.time * 6) % 16;
    rect(x - 3, y - 26 - steam, 2, 4, 0xffc6c9ab);
  } else if (p.kind == 23) {
    rect(x - 19, y - 12, 4, 16, 0xff604c3b);
    rect(x + 15, y - 12, 4, 16, 0xff604c3b);
    box(x - 23, y - 22, 46, 12, 0xffa48255, 0xff5b4939);
    rect(x - 21, y - 21, 42, 2, 0xffd1b67b);
    line(x - 13, y - 28, x - 4, y - 18, 0xffd3c9a7);
    rect(x - 15, y - 30, 10, 5, 0xff778f8b);
    box(x + 5, y - 26, 12, 8, 0xff536b6c);
    rect(x - 7, y - 9, 15, 8, 0xff91764f);
  } else if (p.kind == 24) {
    box(x - 24, y - 15, 48, 22, 0xff745a46, 0xff4d433c);
    rect(x - 20, y - 12, 40, 17, 0xff738675);
    rect(x - 18, y - 10, 11, 13, 0xffd0cbb0);
    rect(x - 3, y - 12, 23, 17, 0xff435f64);
    rect(x + 13, y - 11, 4, 15, 0xffc4ad79);
  } else if (p.kind == 25) {
    ellipse(x, y, 29, 13, 0xff756548);
    for (int j = -2; j <= 2; j++) {
      line(x + j * 10, y + 7, x + j * 10, y - 6, 0xff5c7544);
      if (plantReady(p)) {
        rect(x + j * 10 - 4, y - 9, 9, 5, 0xffa8af5d);
        rect(x + j * 10 - 2, y - 13, 4, 8, GOLD);
      }
    }
  } else if (p.kind == 26) {
    rect(x - 20, y - 7, 4, 14, 0xff614f3e);
    rect(x + 16, y - 7, 4, 14, 0xff614f3e);
    box(x - 25, y - 14, 50, 10, 0xffa28961, 0xff534b3e);
    for (int j = -1; j <= 1; j++) {
      ellipse(x + j * 16, y - 15, 6, 3, 0xffe0d4ad);
      ellipse(x + j * 16, y - 15, 4, 2, 0xff998b50);
    }
  }
  stationDetail(p);
}
void drawLifeAnimal(const Animal &a) {
  int x = sx(a.x), y = syAt(a.x, a.y), d = a.dx < 0 ? -1 : 1, s = a.species;
  if (x < -50 || x > W + 50 || y < -30 || y > H + 55)
    return;
  if (!a.alive) {
    if (a.meat <= 0) {
      line(x - 9, y - 2, x + 8, y - 2, 0xffd8cfab);
      for (int j = -1; j <= 1; j++)
        line(x + j * 4, y - 5, x + j * 4, y + 1, 0xffd8cfab);
    } else {
      ellipse(x, y - 3, s == CROCODILE ? 22 : 12, 5, 0xff796754);
      rect(x + 8, y - 5, 5, 4, 0xffb3a282);
    }
    return;
  }
  float wave = std::sin(a.step);
  int gait = int(wave * 3);
  bool low = a.behaviour == SLEEP || a.behaviour == DRINK ||
             a.behaviour == GRAZE || a.behaviour == FEED;
  C fur = s == WOLF       ? 0xff788580
          : s == SNOWWOLF ? 0xffd7e2dc
          : s == FOX      ? 0xffb86f40
          : s == JACKAL   ? 0xffa9905d
          : s == BOAR     ? 0xff756458
          : s == SNOWHARE ? 0xffeef0e2
          : s == IBEX     ? 0xffad9577
          : s == GOAT     ? 0xffc7b89b
          : s == DEER     ? 0xffb98b61
                          : 0xffd6c39d;
  if (a.flash > 0)
    fur = WHITE;
  if (s == INSECT) {
    int flap = int(std::sin(g.time * 28 + a.x) * 2);
    rect(x - 3, y - 9 - flap, 3, 2, 0xffd2e8bb);
    rect(x + 1, y - 8 + flap, 3, 2, 0xffd2e8bb);
    rect(x, y - 8, 2, 3, 0xff514f37);
    return;
  }
  if (s == VULTURE || s == RAVEN) {
    int up = low ? 2 : 14, flap = int(std::sin(a.step + g.time * 3) * 5);
    C c = s == RAVEN ? 0xff2d4149 : 0xff756657;
    ellipse(x, y - up - 4, 7, 5, c);
    poly({{x, y - up - 5}, {x - 17, y - up - 7 + flap}, {x - 10, y - up + 2}},
         c);
    poly({{x, y - up - 5}, {x + 17, y - up - 7 + flap}, {x + 10, y - up + 2}},
         c);
    circle(x + d * 7, y - up - 6, 3, s == VULTURE ? 0xffc4a27a : c, true);
    line(x + d * 9, y - up - 6, x + d * 13, y - up - 5, GOLD);
    point(x + d * 8, y - up - 7, WHITE);
  } else if (s == FROG) {
    int hop = low ? 0 : int(std::abs(wave) * 4);
    ellipse(x, y - 4 - hop, 7, 4, 0xff60987c);
    ellipse(x - 7, y - 1 - hop, 4, 2, 0xff466f55);
    ellipse(x + 7, y - 1 - hop, 4, 2, 0xff466f55);
    circle(x + d * 5, y - 9 - hop, 3, 0xffa0b979, true);
    point(x + d * 6, y - 10 - hop, INK);
  } else if (s == LIZARD || s == CROCODILE) {
    bool big = s == CROCODILE;
    int z = big ? 2 : 1;
    C c = a.flash > 0 ? WHITE : big ? 0xff526e58 : 0xff918e52;
    poly({{x - d * 8 * z, y - 5 * z},
          {x - d * 29 * z, y + 1},
          {x - d * 8 * z, y}},
         c);
    ellipse(x, y - 4 * z, 12 * z, 4 * z, c);
    ellipse(x + d * 15 * z, y - 4 * z, 9 * z, 3 * z, c);
    for (int j = -1; j <= 1; j += 2) {
      line(x + j * 6 * z, y - 4 * z, x + j * 9 * z + gait, y + 2 * z, c);
      rect(x + j * 8 * z + gait, y + 2 * z, 4, 2, c);
    }
    for (int j = -1; j <= 2; j++)
      rect(x + j * 6 * z, y - 8 * z, 3 * z, 2 * z, 0xffa1a46e);
    point(x + d * 15 * z, y - 7 * z, GOLD);
  } else if (s == HARE || s == SNOWHARE) {
    int hop = low ? 0 : int(std::abs(wave) * 4);
    ellipse(x, y - 6 - hop, 8, 5, fur);
    ellipse(x + d * 7, y - 10 - hop, 5, 5, fur);
    rect(x + d * 6 - 1, y - 22 - hop, 3, 10, fur);
    rect(x + d * 10 - 1, y - 20 - hop, 3, 9, fur);
    rect(x - 5 + gait, y - 1 - hop, 6, 2, 0xff9b9680);
    point(x + d * 9, y - 12 - hop, INK);
    circle(x - d * 8, y - 7 - hop, 3, WHITE, true);
  } else {
    int size = s == BOAR ? 15 : 12,
        height = s == DEER || s == IBEX || s == GOAT ? 18 : 12;
    if (low)
      height -= 5;
    for (int leg = 0; leg < 4; leg++) {
      int lx = x + (leg % 2 ? 8 : -8),
          z = int(std::sin(a.step + (leg % 2) * PI + (leg / 2) * .8f) *
                  (low ? 0 : 4));
      line(lx, y - height + 3, lx + z, y - 1, fur);
      rect(lx + z - 1, y - 2, 3, 3, 0xff4c4e43);
    }
    ellipse(x, y - height, size, s == BOAR ? 9 : 6, fur);
    ellipse(x + d * 14, y - height - (low ? -1 : 5), 7, s == BOAR ? 6 : 5, fur);
    rect(x + d * 17 - 2, y - height - (low ? -2 : 3), 7, 3, fur);
    point(x + d * 17, y - height - (low ? 1 : 6), INK);
    if (s == WOLF || s == SNOWWOLF || s == FOX || s == JACKAL) {
      poly({{x + d * 9, y - height - 7},
            {x + d * 11, y - height - 14},
            {x + d * 15, y - height - 7}},
           fur);
      poly({{x - d * 8, y - height},
            {x - d * 27, y - height - 5 + gait},
            {x - d * 21, y - height + 3 + gait}},
           fur);
      if (s == FOX)
        rect(x - d * 26 - 2, y - height - 5 + gait, 5, 4, WHITE);
      rect(x + d * 6, y - height, 8, 4, mix(fur, WHITE, 90));
    }
    if (s == DEER || s == IBEX || s == GOAT) {
      line(x + d * 13, y - height - 8, x + d * 11, y - height - 22, 0xff725b43);
      line(x + d * 18, y - height - 8, x + d * 20, y - height - 22, 0xff725b43);
      if (s == DEER) {
        line(x + d * 12, y - height - 16, x + d * 6, y - height - 20,
             0xff725b43);
        line(x + d * 19, y - height - 17, x + d * 25, y - height - 21,
             0xff725b43);
      }
    }
    if (s == BOAR) {
      line(x + d * 19, y - height, x + d * 20, y - height - 7, WHITE);
      for (int j = -2; j <= 2; j++)
        line(x + j * 4, y - height - 6, x + j * 4 - 2, y - height - 11,
             0xff4e5246);
    }
  }
  if (a.behaviour == ATTACKING) {
    circle(x, y, 22, RED);
    if (a.cooldown < .4f)
      circle(x, y, 25, GOLD);
  }
  if (a.behaviour == SLEEP && int(g.time) % 3 == 0)
    text(x + 7, y - 27, "Z", WHITE);
  if (len(a.x - g.px, a.y - g.py) < 210 && s != INSECT && a.id != 0) {
    center(x, y + 7, speciesName(s), a.behaviour == FLEE ? GOLD : DIM);
    if (a.hp < a.maxhp)
      bar(x - 15, y + 17, 30, 3, a.hp / a.maxhp, RED);
  }
}
bool drawRoutineActor(const Enemy &e) {
  if (e.routine < 0 || e.boss() || e.kind == 2 || e.routine == DUTY ||
      e.routine == WATCHING)
    return false;
  int x = sx(e.x), y = syAt(e.x, e.y);
  if (x < -50 || x > W + 50 || y < -30 || y > H + 80)
    return true;
  C coat = e.kind == 1   ? 0xff778b62
           : e.kind == 3 ? 0xff7c8583
                         : 0xff6e716e,
    skin = 0xff9bab85;
  if (e.routine == SLEEPING) {
    rect(x - 19, y - 8, 34, 12, coat);
    circle(x - 21, y - 4, 6, skin, true);
    rect(x - 12, y - 7, 21, 10, 0xff45656a);
    if (int(g.time) % 3 < 2)
      text(x - 4, y - 22, "Z Z", DIM);
    return true;
  }
  int gait = int(std::sin(e.step) * 3),
      arm = int(std::sin(e.routineAnim * 4) * 4);
  rect(x - 7 + gait, y - 13, 5, 14, 0xff4e5854);
  rect(x + 3 - gait, y - 13, 5, 14, 0xff4e5854);
  rect(x - 8 + gait, y - 2, 7, 3, INK);
  rect(x + 3 - gait, y - 2, 7, 3, INK);
  box(x - 10, y - 31, 21, 21, coat, 0xff495957);
  rect(x - 9, y - 13, 18, 3, 0xffae9470);
  circle(x, y - 38, 7, skin, true);
  rect(x - 7, y - 46, 14, 5, 0xff5c6960);
  point(x - 3, y - 39, 0xffefd39a);
  point(x + 3, y - 39, 0xffefd39a);
  line(x - 10, y - 28, x - 15, y - 16, skin);
  line(x + 10, y - 28, x + 16, y - 19 + arm, skin);
  if (e.routine == COOKING) {
    line(x + 16, y - 19 + arm, x + 21 + arm, y - 4, 0xffccb286);
    ellipse(x + 10, y - 11, 7, 3, 0xff9f9d7d);
  }
  if (e.routine == EATING) {
    ellipse(x + 9, y - 21, 7, 3, 0xffd2c7a3);
    line(x + 16, y - 20, x + 4, y - 35 + arm, 0xffc4b896);
  }
  if (e.routine == WORKING) {
    line(x + 16, y - 19 + arm, x + 24, y - 31 + arm, 0xffc3a36d);
    rect(x + 18, y - 36 + arm, 13, 6, 0xffabc1b3);
    if (arm > 2)
      rect(x + 23, y - 9, 2, 3, GOLD);
  }
  if (e.routine == GATHERING) {
    box(x - 7, y - 18, 20, 13, 0xff8d744c, 0xffccb37e);
    rect(x - 4, y - 21, 14, 4, 0xff88a16c);
  }
  if (e.routine == FETCHING) {
    box(x + 12, y - 14, 11, 13, 0xff8b9f9a);
    arc(x + 17, y - 15, 6, PI, PI * 2, 0xffd6b982, 1);
  }
  return true;
}
void drawRoutine(const Enemy &e) {
  if (!e.alive || e.alertTime > 0 || e.routine < 0)
    return;
  int x = sx(e.x), y = syAt(e.x, e.y);
  if (x < 20 || x > 620 || y < 0 || y > 360)
    return;
  for (auto &other : g.enemies)
    if (other.alive && other.alertTime <= 0 && other.entityId < e.entityId &&
        len(other.x - e.x, other.y - e.y) < 36)
      return;
  if (len(e.x - g.px, e.y - g.py) < 340)
    center(x, y + 10, routineName(e.routine),
           e.routine == SLEEPING ? DIM : 0xffdbd6ab);
}
void drawBodyMarks(int x, int y) {
  const int xx[] = {0, 0, -9, 9, -5, 5}, yy[] = {-39, -24, -20, -20, -6, -6};
  for (int i = 0; i < 6; i++)
    if (v.injury[i] > 18 || v.bleeding[i] > 0) {
      C c = v.bleeding[i] > 0 ? RED : 0xffd5bc99;
      rect(x + xx[i] - 2, y + yy[i], 4, 3, c);
      if (v.lastPart == i && v.hitMark > 0 && int(g.time * 7) % 2)
        circle(x + xx[i], y + yy[i] + 1, 5, GOLD);
    }
}
struct SunProjection {
  float dx, dy, length;
  int alpha;
};
SunProjection sunProjection() {
  float h = worldHour();
  bool night = h < 6 || h > 18;
  float phase = ((night ? std::fmod(h + 12, 24.f) : h) - 6) / 12 * PI;
  float height = std::max(.12f, std::sin(phase));
  float length =
      clamp(std::sqrt(std::max(0.f, 1 - height * height)) / height, .16f, 2.5f);
  float dx = -std::cos(phase), dy = .4f;
  float d = len(dx, dy);
  return {dx / d, dy / d * .55f, length, night ? 38 : 70};
}
void drawSunShadows() {
  static std::array<uint8_t, W * H> mask;
  mask.fill(0);
  auto sun = sunProjection();
  auto stamp = [&](int x, int y, int rx, int ry, int alpha) {
    rx = std::max(1, rx);
    ry = std::max(1, ry);
    for (int j = std::max(0, y - ry); j < std::min(H, y + ry + 1); j++)
      for (int i = std::max(0, x - rx); i < std::min(W, x + rx + 1); i++) {
        float dx = float(i - x) / rx, dy = float(j - y) / ry;
        if (dx * dx + dy * dy <= 1)
          mask[j * W + i] = std::max(mask[j * W + i], uint8_t(alpha));
      }
  };
  auto cast = [&](float xx, float yy, float height, int width, int depth) {
    int x = sx(xx), y = sy(yy);
    if (x < -240 || x > W + 240 || y < -120 || y > H + 120)
      return;
    float dx = sun.dx * height * sun.length, dy = sun.dy * height * sun.length;
    stamp(x, y, width / 2, depth, sun.alpha);
    for (int i = 1; i <= 9; i++) {
      float t = i / 9.f;
      stamp(x + int(dx * t), y + int(dy * t),
            std::max(2, int(width * (.4f + .6f * t))), depth, sun.alpha);
    }
  };
  for (auto &p : g.props) {
    if (p.kind == 0)
      cast(p.x, p.y,
           treeCut(p)            ? 4
           : treeSpecies(p) == 2 ? 93
                                 : 74,
           treeCut(p) ? 9 : 24, treeCut(p) ? 3 : 9);
    else if (p.kind == 1)
      cast(p.x, p.y, 19, 15, 6);
    else if (p.kind == 21)
      cast(p.x, p.y, 36, 13, 5);
    else if (p.kind == 20)
      cast(p.x, p.y, 17, 10, 4);
    else if (p.kind == 23 || p.kind == 26)
      cast(p.x, p.y, 17, 20, 4);
    else if (p.kind == 12)
      cast(p.x, p.y, 12, 17, 4);
  }
  for (auto &a : g.animals)
    if (a.alive && a.species != INSECT)
      cast(a.x, a.y,
           a.behaviour == SLEEP ? 7
           : flying(a.species)  ? 22
                                : 18,
           10, 3);
  for (auto &e : g.enemies)
    if (e.alive)
      cast(e.x, e.y,
           e.routine == SLEEPING && e.alertTime <= 0 ? 9
           : e.boss()                                ? 68
                                                     : 37,
           e.boss() ? 22 : 9, 4);
  for (auto &f : w.falling)
    cast(f.x + f.dir * f.time * 40, f.y, 70 * std::max(0.f, 1 - f.time), 18, 5);
  cast(g.px, g.py, 49, 10, 4);
  for (int i = 0; i < W * H; i++)
    if (mask[i])
      pix[i] = mix(pix[i], 0xff1e3942, mask[i]);
}
void drawAtmosphere() {
  float hour = worldHour(), alt = std::sin((hour - 6) / 12 * PI);
  int dark = int(clamp((.20f - alt) * 95, 0, 86));
  if (dark > 0)
    shade(0, 0, W, H, 0xff1d345c, dark);
  if (dark > 15)
    for (auto &p : g.props)
      if (p.kind == 12 || p.kind == 22) {
        int x = sx(p.x), y = syAt(p.x, p.y);
        if (x < -80 || x > W + 80 || y < -70 || y > H + 70)
          continue;
        for (int yy = std::max(0, y - 45); yy < std::min(H, y + 46); yy++)
          for (int xx = std::max(0, x - 70); xx < std::min(W, x + 71); xx++) {
            float d = std::hypot(float(xx - x) / 70, float(yy - y) / 45);
            if (d < 1)
              pix[yy * W + xx] =
                  mix(pix[yy * W + xx], 0xffdeb877,
                      int((1 - d) * (23 + 3 * std::sin(g.time * 9))));
          }
      }
}
void survivalHud() {
  panel(10, 83, 180, 36);
  const char *n[] = {"STAM", "FOOD", "WATER"};
  float values[] = {v.stamina, v.food, v.water};
  C c[] = {0xffdfc875, 0xffb2c980, 0xff7cbacf};
  for (int i = 0; i < 3; i++) {
    text(15, 86 + i * 11, n[i], DIM);
    bar(49, 87 + i * 11, 108, 6, values[i] / 100, values[i] < 20 ? RED : c[i]);
    text(164, 86 + i * 11, num(int(values[i])), WHITE);
  }
  button(10, 123, 86, 22, "BODY");
  button(102, 123, 88, 22, v.hunting ? "HUNT ON" : "HUNT OFF");
  button(435, 43, 92, 23, "WOODS / FX");
  panel(217, 64, 190, 15);
  int h = int(worldHour()), m = int((worldHour() - h) * 60);
  center(312, 68,
         "DAY " + num(int((o.seconds + w.clockOffset) / DAY_SECONDS) + 1) +
             " / " + (h < 10 ? "0" : "") + num(h) + ":" + (m < 10 ? "0" : "") +
             num(m) + " / " + std::string(dayPhase()),
         WHITE);
  if (v.task) {
    panel(221, 179, 195, 30);
    center(318, 184,
           v.task == 1   ? "COOKING"
           : v.task == 2 ? "BOILING WATER"
                         : "RESTING",
           GOLD);
    bar(231, 198, 175, 5, 1 - v.taskTime / v.taskTotal, TEAL);
  }
  float bleed = 0;
  for (float b : v.bleeding)
    bleed += b;
  if (bleed > 0) {
    panel(9, 178, 182, 18);
    text(15, 183, "BLEEDING - OPEN BODY", RED);
  }
}
C injuryColor(int i) { return mix(0xff77a996, RED, int(v.injury[i] * 2.3f)); }
void survivalScreen() {
  dim();
  box(24, 16, 592, 330, PANEL, GOLD);
  text(44, 31, "BODY / SURVIVAL", GOLD, 2);
  button(448, 27, 97, 26, "FOOD WEBS");
  button(566, 27, 30, 26, "X");
  box(42, 66, 239, 224, 0xff182f35, EDGE);
  circle(164, 95, 18, injuryColor(0), true);
  box(141, 117, 46, 67, injuryColor(1), GOLD);
  box(113, 122, 21, 70, injuryColor(2));
  box(194, 122, 21, 70, injuryColor(3));
  box(141, 190, 21, 68, injuryColor(4));
  box(167, 190, 21, 68, injuryColor(5));
  C bone = 0xffd1d2b5;
  line(164, 128, 164, 171, bone);
  for (int j = 0; j < 5; j++) {
    line(151, 132 + j * 7, 177, 132 + j * 7, bone);
  }
  line(145, 178, 183, 178, bone);
  line(123, 130, 123, 180, bone);
  line(204, 130, 204, 180, bone);
  line(151, 201, 151, 248, bone);
  line(177, 201, 177, 248, bone);
  circle(164, 95, 10, bone);
  point(160, 93, INK);
  point(168, 93, INK);
  int cx[] = {164, 164, 123, 204, 151, 177},
      cy[] = {95, 150, 156, 156, 224, 224};
  for (int i = 0; i < 6; i++) {
    if (v.bleeding[i] > 0)
      circle(cx[i], cy[i], 4, int(g.time * 5) % 2 ? RED : GOLD, true);
    if (i == v.selectedPart)
      circle(cx[i], cy[i], i == 1 ? 25 : 15, WHITE);
  }
  text(51, 76, "HEAD", DIM);
  text(224, 124, "R", DIM);
  text(88, 124, "L", DIM);
  center(162, 272, "TAP A BODY PART", GOLD);
  int i = v.selectedPart;
  text(309, 69, partName(i), WHITE, 2);
  bar(310, 92, 278, 7, 1 - v.injury[i] / 100, injuryColor(i));
  text(310, 106,
       "INJURY " + num(int(v.injury[i])) + "% / " +
           (v.bleeding[i] > 0  ? "BLEEDING"
            : v.injury[i] > 60 ? "SEVERE"
            : v.injury[i] > 0  ? "WOUNDED"
                               : "HEALTHY"),
       v.bleeding[i] > 0 ? RED : DIM);
  text(310, 121, "LIMBS AFFECT SPEED / STAMINA / ATTACKS", DIM);
  button(310, 140, 133, 28,
         "EAT " + num(v.meals) + " / FRUIT " + num(v.berries));
  button(451, 140, 137, 28, "DRINK " + num(v.cleanWater));
  button(310, 178, 133, 28, "BANDAGE " + num(v.bandages));
  button(451, 178, 137, 28, "SPLINT " + num(v.splints));
  button(310, 216, 133, 28, "COOK / RAW " + num(v.rawMeat));
  button(451, 216, 137, 28,
         nearFire() && v.dirtyWater > 0 ? "BOIL RAW " + num(v.dirtyWater)
                                        : "REFILL WATER");
  button(310, 254, 133, 28, "FORAGE");
  button(451, 254, 137, 28, "REST AT FIRE");
  text(310, 292, "FIBER " + num(v.fiber) + " / 3 FIBER = 1 BANDAGE", DIM);
  text(310, 308,
       nearFire() ? "FIRE NEARBY / REST COST: MEAL + WATER"
                  : "FIND A FIRE TO COOK, BOIL OR REST",
       nearFire() ? TEAL : DIM);
  bool origin = std::abs(globalX()) < 5 && std::abs(globalY()) < 5;
  button(51, 302, 221, 26,
         origin ? "FORGE: " + num(20 + g.upgrade * 15) + " GOLD"
                : "BACK TO EXPLORE");
  if (g.toastTime > 0)
    center(320, 333, g.toast.substr(0, 94), GOLD);
}
void ecologyScreen() {
  dim();
  box(24, 16, 592, 330, PANEL, GOLD);
  text(44, 31, "LIVING FOOD WEBS", GOLD, 2);
  button(566, 27, 30, 26, "X");
  const char *names[] = {"FOREST", "DESERT", "SNOW", "SWAMP", "VOLCANIC"};
  for (int b = 0; b < 5; b++)
    button(45 + b * 111, 65, 104, 27, names[b], b == v.journalBiome);
  int chains[5][5] = {{-1, DEER, WOLF, RAVEN, -2},
                      {-1, INSECT, LIZARD, JACKAL, VULTURE},
                      {-1, IBEX, SNOWWOLF, RAVEN, -2},
                      {-1, INSECT, FROG, CROCODILE, RAVEN},
                      {-1, INSECT, LIZARD, JACKAL, VULTURE}};
  int b = v.journalBiome;
  for (int j = 0; j < 5; j++) {
    int x = 82 + j * 117, s = chains[b][j];
    if (s == -2)
      continue;
    if (s == -1) {
      rect(x - 2, 134, 4, 22, 0xff8b9b62);
      ellipse(x - 8, 135, 10, 5, 0xff89b07a);
      ellipse(x + 8, 125, 10, 5, 0xffaabd76);
      center(x, 174, "PLANTS", WHITE);
    } else {
      Animal a{};
      a.x = g.camx + x;
      a.y = g.camy + 150;
      a.species = s;
      a.behaviour = WANDER;
      a.hp = a.maxhp = 100;
      a.dx = 1;
      a.step = g.time;
      a.hunger = 90;
      drawLifeAnimal(a);
      center(x, 174, speciesName(s), WHITE);
    }
    if (j < 4 && chains[b][j + 1] != -2)
      text(x + 51, 134, ">", GOLD, 2);
  }
  text(47, 201, "ARROWS MEAN FOOD FLOW; SCAVENGERS EAT CARRION.", DIM);
  text(47, 218, "PREY KEEPS ITS DISTANCE. PREDATORS HUNT WHEN HUNGRY.", WHITE);
  text(47, 235, "INSECTS FEED FROGS/LIZARDS. CARRION RETURNS NUTRIENTS.", DIM);
  text(47, 252, "FORMER HUMANS KEEP COOKING, WORK AND SLEEP ROUTINES.", WHITE);
  text(47, 272,
       "YOUR HUNTS " + num(v.hunts) + " / PREDATION " + num(v.predatorKills) +
           " / GRAZING " + num(v.grazingEvents),
       TEAL);
  text(47, 288,
       "SCAVENGING " + num(v.scavenges) + " / CAMP CHORES " +
           num(v.routinesDone),
       TEAL);
  text(47, 321, "LOCAL SIMULATION ONLY - NOT THE WHOLE INFINITE WORLD", DIM);
}
void regionAtlas() {
  int scale = o.generator == 2 ? 32 : 4;
  int64_t px = int64_t(globalX()), py = int64_t(globalY());
  for (int y = 0; y < 65; y++)
    for (int x = 0; x < 90; x++)
      rect(69 + x * 4, 65 + y * 4, 4, 4,
           biomeColor(biomeAt(px + (x - 45) * scale, py + (y - 32) * scale)));
  for (auto &[key, mask] : o.slain) {
    int x = 249 + int((key.first * CELL + CELL / 2 - px) * 4 / scale),
        y = 193 + int((key.second * CELL + CELL / 2 - py) * 4 / scale);
    if (x > 70 && x < 426 && y > 66 && y < 322)
      rect(x - 1, y - 1, 3, 3, GOLD);
  }
  if (o.waypoint) {
    int x = 249 + int((o.waypointX - px) * 4 / scale),
        y = 193 + int((o.waypointY - py) * 4 / scale);
    if (x > 74 && x < 421 && y > 71 && y < 319)
      circle(x, y, 5, GOLD);
  }
  circle(249, 193, 5, INK, true);
  circle(249, 193, 3, WHITE, true);
  panel(75, 305, 235, 15);
  text(81, 309, num(scale) + " TILES/CELL / YOU ARE THE WHITE DOT", WHITE);
}
bool survivalTouch(float x, float y) {
  if (!g.openWorld)
    return false;
  auto in = [&](int a, int b, int w, int h) {
    return x >= a && x < a + w && y >= b && y < b + h;
  };
  if (g.overlay == 3) {
    if (in(120, 298, 195, 28)) {
      g.overlay = 0;
      return true;
    }
    if (in(325, 298, 195, 28)) {
      v.journalBiome = g.world;
      g.overlay = 8;
      return true;
    }
  }
  if (g.overlay == 8) {
    if (in(566, 27, 30, 26))
      g.overlay = 0;
    for (int b = 0; b < 5; b++)
      if (in(45 + b * 111, 65, 104, 27))
        v.journalBiome = b;
    return true;
  }
  if (g.overlay == 7) {
    if (in(566, 27, 30, 26)) {
      g.overlay = 0;
      return true;
    }
    if (in(448, 27, 97, 26)) {
      v.journalBiome = g.world;
      g.overlay = 8;
      return true;
    }
    const int bx[] = {141, 141, 113, 194, 141, 167},
              by[] = {76, 117, 122, 122, 190, 190},
              bw[] = {46, 46, 21, 21, 21, 21}, bh[] = {38, 67, 70, 70, 68, 68};
    for (int i = 0; i < 6; i++)
      if (in(bx[i], by[i], bw[i], bh[i])) {
        v.selectedPart = i;
        return true;
      }
    if (in(310, 140, 133, 28))
      eatMeal();
    if (in(451, 140, 137, 28))
      drinkWater();
    if (in(310, 178, 133, 28))
      bandagePart();
    if (in(451, 178, 137, 28))
      splintPart();
    if (in(310, 216, 133, 28))
      beginTask(1);
    if (in(451, 216, 137, 28)) {
      if (nearFire() && v.dirtyWater > 0)
        beginTask(2);
      else
        fillWater();
    }
    if (in(310, 254, 133, 28))
      forage();
    if (in(451, 254, 137, 28))
      beginTask(3);
    if (in(51, 302, 221, 26)) {
      if (std::abs(globalX()) < 5 && std::abs(globalY()) < 5) {
        int cost = 20 + g.upgrade * 15;
        if (g.gold >= cost && g.upgrade < 50) {
          g.gold -= cost;
          g.upgrade++;
          notify("FORGED - ALL WEAPONS GAIN +3 ATTACK");
          o.dirty = true;
          sfx(12);
        } else
          notify("FORGE NEEDS " + num(cost) + " GOLD");
      } else
        g.overlay = 0;
    }
    return true;
  }
  if (g.scene == PLAY && !g.overlay) {
    if (in(10, 123, 86, 22)) {
      clearInput();
      g.overlay = 7;
      return true;
    }
    if (in(102, 123, 88, 22)) {
      v.hunting = !v.hunting;
      notify(v.hunting ? "HUNT ON - WEAPONS CAN TARGET WILDLIFE"
                       : "HUNT OFF - TARGET MONSTERS ONLY");
      return true;
    }
  }
  return false;
}
} // namespace av
