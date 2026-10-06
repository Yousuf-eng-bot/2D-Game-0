#pragma once
#include "art.hpp"
#include "char_anim.hpp"
namespace av {
void button(int x, int y, int w, int h, const std::string &s,
            bool active = true) {
  box(x, y, w, h, active ? 0xff34463a : 0xff243229, active ? GOLD : EDGE);
  rect(x + 2, y + 2, w - 4, 1, active ? 0xff617461 : EDGE);
  center(x + w / 2, y + (h - 7) / 2, s, active ? WHITE : DIM);
}
void panel(int x, int y, int w, int h) {
  shade(x, y, w, h, PANEL, 235);
  rect(x, y, w, 1, EDGE);
  rect(x, y + h - 1, w, 1, EDGE);
  rect(x, y, 1, h, EDGE);
  rect(x + w - 1, y, 1, h, EDGE);
}
void bar(int x, int y, int w, int h, float p, C c) {
  box(x, y, w, h, INK, EDGE);
  rect(x + 2, y + 2, int((w - 4) * clamp(p, 0, 1)), h - 4, c);
  rect(x + 2, y + 2, int((w - 4) * clamp(p, 0, 1)), 1, mix(c, WHITE, 65));
}
} // namespace av
#include "living_visuals.hpp"
#include "look.hpp"
#include "journey_visuals.hpp"
#include "vista.hpp"
#include "graphics_runtime.hpp"
#include "medium_visuals.hpp"
namespace av {
void tiles() {
  if (g.openWorld) {
    frontierTerrain();
    return;
  }
  bool desert = g.scene != HUB && g.world;
  int ix = std::max(0, int(g.camx / T) - 1),
      iy = std::max(0, int(g.camy / T) - 1), ex = std::min(MW, ix + W / T + 4),
      ey = std::min(MH, iy + H / T + 4);
  for (int y = iy; y < ey; y++)
    for (int x = ix; x < ex; x++) {
      int a = y * MW + x, xx = sx(x * T), yy = sy(y * T);
      uint32_t h = hash(x + y * MW + g.world * 599);
      int m = g.map[a], ground = g.ground[a];
      if (m == 4) {
        C water = desert ? 0xff579f9b : 0xff4faaa7;
        rect(xx, yy, T, T, water);
        rect(xx, yy + 8, T, 6, desert ? 0xff4a9696 : 0xff459a9d);
        int shift = int(g.time * 9 + y * 3) % 18;
        line(xx + shift - 3, yy + 4, xx + shift + 5, yy + 4, 0xffa3d3bc);
        line(xx + 2, yy + 19, xx + 11, yy + 19, 0xff83c5b6);
        if (x > 0 && g.map[a - 1] != 4)
          rect(xx, yy, 2, T, 0xffc1d5a4);
        if (x < MW - 1 && g.map[a + 1] != 4)
          rect(xx + T - 2, yy, 2, T, 0xffd0d6a5);
        continue;
      }
      if (m == 0) {
        rect(xx, yy, T, T, desert ? 0xff79684d : 0xff376b52);
        for (int j = 0; j < 3; j++)
          rect(xx + int(h >> (j * 5)) % 18, yy + j * 8, 7, 4,
               desert ? 0xffa1825c : 0xff4e8056);
        continue;
      }
      C base = desert ? (h % 3 ? 0xffd4b27a : 0xffcfac73)
                      : (h % 3 ? 0xff8bb06b : 0xff86aa65);
      if (ground == 2)
        base = desert ? 0xffc79e6a : 0xffc8bd86;
      if (ground == 4)
        base = desert ? 0xffc5a075 : 0xffaeb582;
      rect(xx, yy, T, T, base);
      if (ground == 3) {
        rect(xx, yy, T, T, 0xff8b7553);
        for (int j = 0; j < T; j += 6) {
          rect(xx + 1, yy + j + 1, T - 2, 4, 0xffc3a472);
          rect(xx + 2, yy + j + 1, T - 4, 1, 0xffddc38c);
        }
        rect(xx, yy, 2, T, 0xff665b43);
        rect(xx + T - 2, yy, 2, T, 0xff665b43);
        continue;
      }
      for (int k = 0; k < 7; k++) {
        uint32_t z = hash(h + k * 317);
        int px = xx + z % 21, py = yy + (z >> 9) % 22;
        C col = desert ? (k % 2 ? 0xffe1c18b : 0xffb99767)
                       : (k % 2 ? 0xffaac47b : 0xff719956);
        if (ground == 2)
          col = k % 2 ? 0xffdec994 : 0xffb4a978;
        rect(px, py, k % 3 + 1, 1, col);
        if (!desert && !ground && k % 2 == 0)
          line(px, py, px + 1, py - 2, col);
      }
      if (!ground && h % 19 == 0) {
        int wx = xx + 8, wy = yy + 16, sway = int(std::sin(g.time * 2 + x) * 1);
        line(wx, wy, wx + sway, wy - 5, desert ? 0xff8e9250 : 0xff567e46);
        rect(wx + sway - 1, wy - 7, 3, 3, h % 2 ? 0xffe6d17c : 0xffeedec3);
      }
    }
}
void telegraphs() {
  for (auto &e : g.enemies)
    if (e.alive && e.wind > 0) {
      int x = sx(e.x), y = syAt(e.x, e.y);
      float progress = clamp(1 - e.wind / e.windMax, 0, 1);
      if (e.boss()) {
        if (e.pattern == 1) {
          float dx = e.tx - e.x, dy = e.ty - e.y,
                l = std::max(1.f, len(dx, dy));
          dx /= l;
          dy /= l;
          for (int j = 1; j < 5; j++)
            circle(x + int(dx * j * 52), y + int(dy * j * 52),
                   int(8 + progress * 7), j % 2 ? RED : GOLD);
        } else if (e.pattern == 0) {
          float a = std::atan2(e.ty - e.y, e.tx - e.x);
          arc(x, y, 112, a - 1.1f, a + 1.1f, RED, 2);
          arc(x, y, int(112 * progress), a - 1.1f, a + 1.1f, GOLD, 1);
        } else if (e.pattern == 4) {
          circle(sx(e.tx), syAt(e.tx, e.ty), 75, RED);
          circle(sx(e.tx), syAt(e.tx, e.ty), int(75 * progress), GOLD);
        } else if (e.pattern == 5) {
          float a = std::atan2(e.ty - e.y, e.tx - e.x);
          for (int j = -2; j <= 2; j++)
            circle(x + int(std::cos(a + j * .25f) * 130),
                   y + int(std::sin(a + j * .25f) * 130), 8, RED);
        }
      } else if (e.kind == 1) {
        circle(x, y, 15, RED);
        arc(x, y, 18, -PI / 2, -PI / 2 + progress * 2 * PI, GOLD, 2);
      } else if (e.kind != 4) {
        circle(x, y, e.kind == 3 ? 60 : 35, RED);
        arc(x, y, e.kind == 3 ? 60 : 35, -PI / 2, -PI / 2 + progress * 2 * PI,
            GOLD, 2);
      }
    }
  for (auto &h : g.hazards) {
    int x = sx(h.x), y = syAt(h.x, h.y);
    if (h.type == 1) {
      int r = h.wind > 0 ? int(h.r * .28f) : int(h.radius);
      arc(x, y, r, h.angle + .54f, h.angle + 2 * PI - .54f,
          h.wind > 0 ? RED : GOLD, 3);
      if (h.wind > 0) {
        arc(x, y, r, h.angle - .5f, h.angle + .5f, TEAL, 3);
      }
    } else if (h.wind > 0) {
      circle(x, y, int(h.r), RED);
      circle(x, y, int(h.r * clamp(1 - h.wind / h.maxWind, 0, 1)), GOLD);
      line(x - 5, y - 5, x + 5, y + 5, RED);
      line(x + 5, y - 5, x - 5, y + 5, RED);
    } else {
      circle(x, y, int(h.r), GOLD);
      circle(x, y, int(h.r * .75f), h.color);
      for (int j = 0; j < 8; j++) {
        float a = j * PI / 4;
        int xx = x + int(std::cos(a) * h.r * .6f),
            yy = y + int(std::sin(a) * h.r * .6f);
        rect(xx, yy - 8, 4, 12, h.color);
      }
    }
  }
}
void minimap(int x = 550, int y = 11, int scale = 1, bool full = false) {
  panel(x - 4, y - 4, MW * scale + 8, MH * scale + 8);
  for (int j = 0; j < MH; j++)
    for (int i = 0; i < MW; i++) {
      int a = j * MW + i;
      C c = g.map[a] == 4   ? 0xff519eaa
            : g.map[a] == 1 ? (g.ground[a] ? 0xffc2b780 : 0xff6b8a5a)
                            : 0xff334b3f;
      rect(x + i * scale, y + j * scale, scale, scale, c);
    }
  for (int r = 0; r < 3; r++) {
    int xx = x + int(g.roomX[r] / T) * scale,
        yy = y + int(g.roomY[r] / T) * scale;
    rect(xx - 2, yy - 2, 5, 5, g.rooms[r] ? TEAL : RED);
    if (full)
      text(xx + 5, yy - 2, num(r + 1), WHITE);
  }
  int bx = x + int(g.bossX / T) * scale, by = y + int(g.bossY / T) * scale;
  circle(bx, by, full ? 5 : 2, g.bossKilled ? DIM : GOLD, true);
  int px = x + int(g.px / T) * scale, py = y + int(g.py / T) * scale;
  circle(px, py, full ? 4 : 2, WHITE, true);
  if (full)
    circle(px, py, 7, INK);
}
void control(int x, int y, int r, const std::string &label, float cd,
             int icon) {
  circle(x, y, r, 0xff163a40, true);
  circle(x, y, r, cd > 0 ? 0xff4e7779 : GOLD);
  circle(x, y, r - 3, 0xff315d5c);
  C col = cd > 0 ? DIM : WHITE;
  if (icon == 0) {
    weaponArt(x - 4, y - 1, g.weapon, -.95f, 0, col);
  }
  if (icon == 1) {
    for (int k = 0; k < 3; k++)
      arc(x, y - 4, 8 + k * 3, -2.6f, -.4f, col, 1);
  }
  if (icon == 2) {
    line(x - 9, y - 11, x + 4, y - 5, col);
    line(x + 4, y - 5, x - 9, y + 1, col);
    line(x - 2, y - 11, x + 11, y - 5, col);
    line(x + 11, y - 5, x - 2, y + 1, col);
  }
  if (icon == 3) {
    circle(x, y - 5, 9, col);
    line(x - 5, y - 5, x + 5, y - 5, col);
    line(x, y - 10, x, y, col);
  }
  if (icon == 4) {
    rect(x - 6, y - 10, 12, 13, col);
    rect(x - 3, y - 15, 6, 5, GOLD);
    rect(x - 4, y - 6, 8, 7, 0xff85b88d);
  }
  center(x, y + r - 12, label, col);
  if (cd > 0) {
    box(x - 9, y - 10, 18, 12, PANEL, EDGE);
    center(x, y - 8, num(int(std::ceil(cd))), GOLD);
  }
}
void controls() {
  if (g.openWorld) {
    drawJourneyControls();
    return;
  }
  int jx = g.joystick >= 0 ? int(g.joyx) : 76,
      jy = g.joystick >= 0 ? int(g.joyy) : 282;
  circle(jx, jy, 43, 0xff5b877b);
  circle(jx, jy, 42, 0xff315854);
  circle(jx, jy, 28, 0xff6f9a84);
  circle(jx + int(g.mx * 24), jy + int(g.my * 24), 16, 0xff244d50, true);
  circle(jx + int(g.mx * 24), jy + int(g.my * 24), 16, 0xffb6d4ab);
  center(76, 337, "MOVE", WHITE);
  button(145, 317, 117, 28,
         std::string(g.weapon == 0   ? "BLADE"
                     : g.weapon == 1 ? "AXE"
                                     : "BOW") +
             " / CHANGE");
  if (g.scene != PLAY)
    return;
  control(587, 293, 34, "ATTACK", 0, 0);
  control(518, 263, 26,
          g.weapon == BOW   ? "VOLLEY"
          : g.weapon == AXE ? "QUAKE"
                            : "SWEEP",
          g.powerCd, 1);
  control(519, 325, 24, "DODGE", g.dodgeCharges ? 0 : 3.4f - g.dodgeRecharge,
          2);
  control(585, 218, 23, "WARD", g.wardCd, 3);
  control(462, 324, 22, num(g.flasks), g.healCd, 4);
  for (int k = 0; k < 2; k++)
    rect(511 + k * 11, 295, 7, 4, k < g.dodgeCharges ? TEAL : EDGE);
}
void objectivePointer() {
  if (g.bossKilled)
    return;
  float tx = g.bossX, ty = g.bossY;
  if (!g.gate) {
    float best = 1e8;
    for (int r = 0; r < 3; r++)
      if (!g.rooms[r]) {
        float d = len(g.roomX[r] - g.px, g.roomY[r] - g.py);
        if (d < best) {
          best = d;
          tx = g.roomX[r];
          ty = g.roomY[r];
        }
      }
  }
  float dx = tx - g.px, dy = ty - g.py, d = len(dx, dy);
  if (d < 150)
    return;
  int x = sx(g.px) + int(dx / d * 73), y = syAt(g.px, g.py) + int(dy / d * 73);
  poly({{x, y},
        {x - int(dx / d * 13 - dy / d * 6), y - int(dy / d * 13 + dx / d * 6)},
        {x - int(dx / d * 9), y - int(dy / d * 9)},
        {x - int(dx / d * 13 + dy / d * 6), y - int(dy / d * 13 - dx / d * 6)}},
       GOLD);
}
void hud() {
  if (g.openWorld) {
    frontierHud();
    return;
  }
  panel(9, 9, 184, 51);
  box(14, 14, 28, 29, 0xff32514a, GOLD);
  center(28, 20, num(g.level), GOLD, 2);
  text(49, 15, "DAYBREAK RANGER", GOLD);
  bar(49, 28, 134, 12, g.hp / maxhp(), 0xffb95952);
  text(49, 45, num(int(g.hp)) + " / " + num(maxhp()) + " HP", WHITE);
  bar(14, 55, 169, 4, g.xp / float(g.level * 60), TEAL);
  text(13, 66, "G " + num(g.gold) + "   ATK " + num(damage()), INK);
  shade(10, 63, 180, 15, PANEL, 185);
  text(15, 67, "G " + num(g.gold) + "   ATK " + num(damage()), WHITE);
  button(435, 10, 43, 27, "BAG");
  button(483, 10, 44, 27, "II");
  if (g.scene == HUB) {
    panel(204, 10, 223, 48);
    center(315, 18, "DAYBREAK CAMP", GOLD, 2);
    center(315, 43, "THE WORLD IS BEAUTIFUL. STAY ALIVE.", WHITE);
    button(281, 303, 142, 41, "CHOOSE A WORLD");
    button(444, 310, 182, 34,
           g.upgrade >= 50 ? "FORGE MASTERED"
                           : "FORGE +3 / " + num(20 + g.upgrade * 15) + "G",
           g.upgrade < 50 && g.gold >= 20 + g.upgrade * 15);
  } else {
    panel(200, 9, 228, 51);
    center(314, 15, worldName(), GOLD);
    int n = int(g.rooms[0]) + int(g.rooms[1]) + int(g.rooms[2]);
    center(314, 30,
           g.bossKilled ? "GUARDIAN DEFEATED"
           : g.gate     ? "DEFEAT THE AWAKENED GUARDIAN"
                        : "PURIFY SHRINES  " + num(n) + " / 3",
           WHITE);
    center(314, 46, g.difficulty ? "VETERAN EXPEDITION" : "DAYLIGHT EXPEDITION",
           DIM);
    minimap();
    text(555, 69, "TAP FOR MAP", INK);
    for (auto &e : g.enemies)
      if (e.boss() && e.alive && g.gate && len(e.x - g.px, e.y - g.py) < 460) {
        panel(205, 267, 276, 31);
        center(343, 271, bossName(), GOLD);
        bar(215, 282, 256, 7, e.hp / e.maxhp, RED);
        bar(215, 292, 256, 3, e.stagger / 100, TEAL);
        if (e.wind > 0) {
          panel(200, 245, 288, 17);
          center(344, 250, patternName(e.pattern), GOLD);
        } else if (e.state == 4) {
          center(344, 251, "STAGGERED - BONUS DAMAGE", 0xff165d63);
        }
        center(610, 85, "P" + num(e.phase), INK);
      }
    objectivePointer();
    if (g.killStreak >= 3 && g.streakTime > 0) {
      text(14, 88, num(g.killStreak) + " KILL STREAK", GOLD, 2);
    }
    if (g.bossKilled)
      button(278, 308, 146, 35, "CLAIM VICTORY");
  }
  controls();
  if (g.toastTime > 0) {
    int w = std::min(600, tw(g.toast) + 22);
    panel(320 - w / 2, 126, w, 20);
    center(320, 132, g.toast, GOLD);
  }
}
void worldScene() {
  mediumBeginWorld();
  float tx = clamp(g.px - W * .47f, 0, float(MW * T - W)),
        ty = clamp(g.py - H * .54f, 0, float(MH * T - H));
  // Calm point-of-interest framing lets players observe a camp before provoking
  // it.
  if (g.openWorld) {
    bool combat = false;
    for (auto &e : g.enemies)
      if (e.alive && e.alertTime > 0 && len(e.x - g.px, e.y - g.py) < 360)
        combat = true;
    if (!combat) {
      const Prop *camp = nullptr;
      float nearest = 340;
      for (auto &p : g.props)
        if (p.kind == 11 && !p.open) {
          float d = len(p.x - g.px, p.y - g.py);
          if (d > 150 && d < nearest) {
            nearest = d;
            camp = &p;
          }
        }
      if (camp) {
        tx = clamp(tx + clamp(camp->x - g.px, -180, 180) * .60f, 0,
                   float(MW * T - W));
        ty = clamp(ty + clamp(camp->y - g.py, -110, 110) * .35f, 0,
                   float(MH * T - H));
      }
    }
  }
  if (g.scene == PLAY && g.gate && !g.bossKilled) {
    for (const auto &e : g.enemies)
      if (e.boss() && e.alive && (!g.openWorld || e.alertTime > 0) &&
          len(e.x - g.px, e.y - g.py) < 380) {
        tx = clamp(g.px + clamp(e.x - g.px, -100.f, 100.f) * .3f - W * .5f, 0,
                   float(MW * T - W));
        ty = clamp(g.py + clamp(e.y - g.py, -90.f, 90.f) * .45f - H * .58f, 0,
                   float(MH * T - H));
      }
  }
  if (g.scene == HUB) {
    tx = clamp(tx, 0, 32);
    ty = clamp(ty, 0, 170);
  }
  float factor = g.camReady ? 1 - std::exp(-(mediumEnabled()?9.f:13.f) * renderDt) : 1.f;
  g.camx += (tx - g.camx) * factor;
  g.camy += (ty - g.camy) * factor;
  g.camReady = true;
  float baseX = g.camx, baseY = g.camy;
  if (g.shake && g.shakeTime > 0) {
    g.camx += std::sin(g.time * 139) * 3;
    g.camy += std::cos(g.time * 117) * 2;
  }
  rect(0, 0, W, H, g.world && g.scene != HUB ? 0xffbe9b6b : 0xff679559);
  tiles();
  if (g.openWorld) {
    vistaReflections();
    drawVistaShadows();
    vistaGroundFocus();
    mediumGroundDecor();
  }
  if (g.scene == PLAY || g.scene == DEAD || g.scene == WIN) {
    if (!g.openWorld) {
      ring(g.bossX, g.bossY, 155, g.world ? 0xff9b805b : 0xff758e65);
      ring(g.bossX, g.bossY, 147, g.world ? 0xffead29a : 0xffd1d89b);
      for (int k = 0; k < 8; k++) {
        float a = k * PI / 4;
        int x = sx(g.bossX) + int(std::cos(a) * 151),
            y = sy(g.bossY) + int(std::sin(a) * 151);
        poly({{x, y - 5}, {x + 4, y}, {x, y + 5}, {x - 4, y}}, GOLD);
      }
    } else
      frontierTerritories();
    telegraphs();
  }
  if (g.ward > 0) {
    ring(g.wardx, g.wardy, 70, 0xffe2f5b2);
    ring(g.wardx, g.wardy, 65, TEAL);
    for (int k = 0; k < 8; k++) {
      float a = k * PI / 4 + g.time;
      rect(sx(g.wardx) + int(std::cos(a) * 66),
           syAt(g.wardx, g.wardy) + int(std::sin(a) * 66), 3, 3, 0xffc8edb7);
    }
  }
  for (auto &d : g.drops) {
    int x = sx(d.x), y = syAt(d.x, d.y);
    groundShadow(x, y, 8, 3);
    if (d.gear) {
      int bob = int(std::sin(g.time * 4) * 2);
      line(x, y - 5, x, y - 37, rarityColor(d.item.rarity));
      poly({{x, y - 18 + bob},
            {x + 7, y - 9 + bob},
            {x, y + bob},
            {x - 7, y - 9 + bob}},
           rarityColor(d.item.rarity));
      rect(x - 1, y - 13 + bob, 3, 6, WHITE);
    } else {
      rect(x - 5, y - 5, 7, 4, 0xffbc963e);
      rect(x - 4, y - 6, 5, 3, 0xffffdf79);
      rect(x + 2, y - 9, 4, 5, GOLD);
    }
  }
  for (auto &e : g.echoes)
    if (e.life > .07f && !g.openWorld) {
      groundShadow(sx(e.x), syAt(e.x, e.y), 12, 4);
      poly({{sx(e.x) - 8, syAt(e.x, e.y) - 26},
            {sx(e.x) + 8, syAt(e.x, e.y) - 26},
            {sx(e.x) + 12, syAt(e.x, e.y) - 5},
            {sx(e.x) - 12, syAt(e.x, e.y) - 5}},
           0xff9dd2b8);
    }
  struct Draw {
    float y;
    int type, index;
  };
  std::vector<Draw> order;
  order.reserve(g.props.size() + g.enemies.size() + g.animals.size() + 1);
  order.push_back({g.py, 0, 0});
  for (int i = 0; i < int(g.props.size()); i++)
    order.push_back({g.props[i].y, 1, i});
  for (int i = 0; i < int(g.animals.size()); i++)
    order.push_back({g.animals[i].y, 2, i});
  for (int i = 0; i < int(g.enemies.size()); i++)
    if (g.enemies[i].alive || g.enemies[i].death > 0)
      order.push_back({g.enemies[i].y, 3, i});
  if (g.openWorld)
    for (int i = 0; i < int(w.falling.size()); i++)
      order.push_back({w.falling[i].y, 4, i});
  std::vector<std::pair<int64_t, int64_t>> structures;
  if (g.openWorld)
    for (auto &[key, b] : j.built) {
      float px = float(key.first - o.originX) * T + 12,
            py = float(key.second - o.originY) * T + 12;
      if (px > g.camx - 50 && px < g.camx + W + 50 && py > g.camy - 20 &&
          py < g.camy + H + 60) {
        structures.push_back(key);
        order.push_back({py, 5, int(structures.size() - 1)});
      }
    }
  if (g.openWorld)
    for (int i = 0; i < int(vistaDecor.size()); i++)
      order.push_back({vistaDecor[i].y, 6, i});
  std::stable_sort(order.begin(), order.end(),
                   [](auto &a, auto &b) { return a.y < b.y; });
  for (auto &d : order) {
    if (d.type == 0)
      heroArt(sx(g.px), syAt(g.px, g.py), g.walkPhase, len(g.vx, g.vy) > 6);
    else if (d.type == 6)
      vistaDecorArt(vistaDecor[d.index]);
    else if (d.type == 1)
      propArt(g.props[d.index]);
    else if (d.type == 2)
      animalArt(g.animals[d.index]);
    else if (d.type == 5)
      drawJourneyStructure(structures[d.index],
                           j.built.at(structures[d.index]));
    else if (d.type == 4)
      {if(mediumEnabled())drawMediumFallingTree(w.falling[d.index]);else drawFallingTree(w.falling[d.index]);}
    else {
      enemyArt(g.enemies[d.index]);
      if (g.openWorld)
        drawRoutine(g.enemies[d.index]);
    }
  }
  // Journey renderer emits trails from the actual active blade angle.
  if (g.slash > 0 && !g.openWorld) {
    float a = std::atan2(g.aimy, g.aimx);
    float p = 1 - g.slash / .2f, r = g.attackWeapon == AXE ? 72 : 57;
    arc(sx(g.px), syAt(g.px, g.py) - 10, int(r), a - 1.1f + p * .6f,
        a + 1.2f + p * .6f, g.attackWeapon == AXE ? GOLD : 0xfff5f5d0, 3);
    arc(sx(g.px), syAt(g.px, g.py) - 10, int(r - 5), a - .9f + p * .6f,
        a + 1.f + p * .6f, 0xffe8cf94, 1);
  }
  if (g.sweep > 0) {
    int r = int((1 - g.sweep / .5f) * 110);
    ring(g.px, g.py - 5, r, GOLD);
    ring(g.px, g.py - 5, std::max(0, r - 5), 0xfff8efd0);
  }
  for (auto &b : g.bolts) {
    int x = sx(b.x), y = syAt(b.x, b.y);
    if (b.friendly) {
      float l = std::max(1.f, len(b.vx, b.vy));
      line(x - int(b.vx / l * 15), y - int(b.vy / l * 15), x, y, GOLD);
      line(x, y, x - int(b.vx / l * 5 - b.vy / l * 3),
           y - int(b.vy / l * 5 + b.vx / l * 3), WHITE);
      line(x, y, x - int(b.vx / l * 5 + b.vy / l * 3),
           y - int(b.vy / l * 5 - b.vx / l * 3), WHITE);
    } else {
      circle(x, y, 5, 0xffc56a50, true);
      circle(x, y, 3, b.color, true);
      point(x - 1, y - 1, WHITE);
    }
  }
  for (auto &p : g.particles)
    rect(sx(p.x), syAt(p.x, p.y) - int(p.z), p.size, p.size, p.color);
  for (auto &f : g.floats)
    center(sx(f.x), syAt(f.x, f.y), f.value, f.color, f.scale);
  // Ambient birds and pollen lend life without hiding danger cues.
  for (int k = 0; k < 7; k++) {
    int x = int(hash(k * 317) % W + g.time * (8 + k)) % W,
        y = 30 + hash(k * 553) % (H - 70);
    if (k < 3) {
      int flap = int(std::sin(g.time * 6 + k) * 3);
      line(x - 5, y + flap, x, y, 0xff4d7668);
      line(x, y, x + 5, y + flap, 0xff4d7668);
    } else
      rect(x, y, 2, 1, 0xffefe3a0);
  }
  if (g.damageVignette > 0) {
    int a = int(g.damageVignette / .28f * 130);
    shade(0, 0, 8, H, RED, a);
    shade(W - 8, 0, 8, H, RED, a);
    shade(0, 0, W, 6, RED, a);
    shade(0, H - 6, W, 6, RED, a);
  }
  if (g.openWorld) {
    drawJourneyDrops();
    if (!mediumEnabled()) { drawVistaLight(); drawMotionBlur(); }
    else mediumAtmosphere();
  }
  g.camx = baseX;
  g.camy = baseY;
  mediumCaptureWorld();
  if (ui7Active())
    ui7Hud();
  else {
    hud();
    if (g.openWorld)
      drawJourneyHud();
  }
}
void title() {
  if (cover.size() == W * H)
    std::copy(cover.begin(), cover.end(), pix);
  else {
    rect(0, 0, W, H, 0xff87c8ce);
    poly({{0, 270}, {180, 105}, {400, 280}, {640, 150}, {640, 360}, {0, 360}},
         0xff5c977a);
  }
  for (int x = 0; x < 390; x++)
    shade(x, 0, 1, H, 0xff0e3035, int(190 * (1 - x / 420.f)));
  shade(0, 300, W, 60, 0xff153c38, 115);
  text(32, 31, "AN ORIGINAL PIXEL-ART ACTION RPG", 0xffe1eccb);
  text(31, 73, "DEATH", 0xff16333b, 6);
  text(29, 71, "DEATH", WHITE, 6);
  text(31, 123, "WORLD", 0xff16333b, 6);
  text(29, 121, "WORLD", GOLD, 6);
  rect(33, 178, 173, 2, GOLD);
  text(33, 192, "BEAUTIFUL DAYS. DEADLY WILDS.", WHITE);
  text(33, 213, "TWO WORLDS / THREE WEAPONS", 0xffd0e3c4);
  button(32, 252, 241, 39,
         g.loaded ? "CONTINUE YOUR JOURNEY" : "ENTER DEATH WORLD");
  button(32, 302, 113, 28, "HOW TO PLAY");
  button(156, 302, 117, 28, "ARMORY");
  text(454, 338, "DAYBREAK UPDATE / 0.2", WHITE);
}
void inventory() {
  dim();
  box(94, 24, 452, 312, PANEL, GOLD);
  center(320, 38, "RANGER'S SATCHEL", GOLD, 2);
  button(500, 33, 30, 24, "X");
  text(111, 66,
       "GOLD " + num(g.gold) + "   BASE ATK " + num(damage()) + "   HP " +
           num(maxhp()),
       TEAL);
  int start = g.page * 5;
  for (int row = 0; row < 5; row++) {
    int n = start + row, y = 85 + row * 32;
    if (n >= int(g.bag.size()))
      break;
    auto &i = g.bag[n];
    box(111, y, 418, 29, n == g.selected ? 0xff31534d : 0xff17323a,
        n == g.selected ? GOLD : EDGE);
    text(121, y + 6, itemName(i), rarityColor(i.rarity));
    text(313, y + 6, "+" + num(i.value), TEAL);
    text(401, y + 6, equipped(i) ? "EQUIPPED" : "IN BAG",
         equipped(i) ? GOLD : DIM);
    text(121, y + 17,
         i.slot == 0   ? "ALL WEAPONS"
         : i.slot == 1 ? "ARMOR"
                       : "CHARM",
         DIM);
  }
  if (g.selected >= int(g.bag.size()))
    g.selected = 0;
  auto &i = g.bag[g.selected];
  std::string detail =
      i.slot == 0 ? "+" + num(i.value * 3) + " ATTACK / CURRENT +" +
                        num(equippedValue(0) * 3)
      : i.slot == 1
          ? "+" + num(i.value * 5) + " HP / +" + num(i.value * 2) + " ARMOR"
      : i.value > 0 ? "HEAL 3 HP ON EVERY KILL"
                    : "NO SPECIAL EFFECT";
  center(320, 256, detail, TEAL);
  button(111, 278, 98, 29, "< PAGE");
  button(217, 278, 99, 29, equipped(i) ? "EQUIPPED" : "EQUIP", !equipped(i));
  button(324, 278, 99, 29, "SALVAGE", !equipped(i));
  button(431, 278, 98, 29, "PAGE >");
  center(320, 318,
         "PAGE " + num(g.page + 1) + " / " + num((int(g.bag.size()) + 4) / 5) +
             "    ITEMS " + num(g.bag.size()) + " / 18",
         DIM);
}
void armory() {
  dim();
  box(33, 25, 574, 310, PANEL, GOLD);
  center(320, 40, "CHOOSE YOUR WEAPON", GOLD, 2);
  center(320, 67, "YOUR EQUIPPED WEAPON CORE IMPROVES ALL THREE.", DIM);
  button(565, 35, 28, 25, "X");
  const char *lines[3][4] = {{"QUICK THREE-HIT COMBO", "BALANCED CLOSE COMBAT",
                              "POWER: WHIRLWIND", "FINISHER DEALS +45%"},
                             {"SLOW. HEAVY. BRUTAL.", "85% MORE BASE DAMAGE",
                              "POWER: EARTHQUAKE", "BREAKS GUARD / STAGGER"},
                             {"MOBILE RANGED COMBAT", "ARROWS CAN PIERCE",
                              "POWER: FIVE-SHOT FAN",
                              "AIM ASSIST / LONG RANGE"}};
  for (int w = 0; w < 3; w++) {
    int x = 48 + w * 184;
    box(x, 87, 176, 229, w == g.weapon ? 0xff2f524e : 0xff1b353e,
        w == g.weapon ? GOLD : EDGE);
    circle(x + 88, 139, 32, 0xff315352, true);
    weaponArt(x + 75, 147, w, -.7f);
    center(x + 88, 185, weaponName(w), GOLD);
    for (int j = 0; j < 4; j++)
      center(x + 88, 207 + j * 15, lines[w][j], j % 2 ? DIM : WHITE);
    button(x + 15, 282, 146, 25, w == g.weapon ? "EQUIPPED" : "SELECT");
  }
}
void regionCard(int x, int y, int region) {
  box(x, y, 254, 203, 0xff1f3c40, region == g.world ? GOLD : EDGE);
  rect(x + 2, y + 2, 250, 102, region ? 0xffe0cb93 : 0xff97d3cf);
  circle(x + 214, y + 27, 15, 0xfff4e3b0, true);
  poly({{x + 2, y + 91},
        {x + 77, y + 27},
        {x + 125, y + 82},
        {x + 186, y + 20},
        {x + 252, y + 76},
        {x + 252, y + 104},
        {x + 2, y + 104}},
       region ? 0xffb88f68 : 0xff638e79);
  poly({{x + 2, y + 98},
        {x + 104, y + 56},
        {x + 161, y + 91},
        {x + 252, y + 42},
        {x + 252, y + 104},
        {x + 2, y + 104}},
       region ? 0xffd3ab74 : 0xff78a46d);
  if (region) {
    rockArt(x + 59, y + 101, 2, true);
    rockArt(x + 203, y + 103, 1, true);
  } else {
    treeArt(x + 42, y + 100, 2, false, 0);
    treeArt(x + 206, y + 103, 0, false, 0);
  }
  text(x + 14, y + 119, region ? "EMBERFALL REACH" : "SUNVEIL WILDS", GOLD, 2);
  text(x + 14, y + 144,
       region ? "CANYON / STONE / SUNFIRE" : "FOREST / RIVER / WILDLIFE",
       WHITE);
  text(x + 14, y + 160,
       region ? "BOSS: SUNFORGED COLOSSUS" : "BOSS: CROWNHORN SOVEREIGN", DIM);
  button(x + 14, y + 177, 226, 21, "ENTER WORLD " + num(region + 1));
}
void worlds() {
  dim();
  box(39, 24, 562, 310, PANEL, GOLD);
  center(320, 40, "TWO WORLDS. NO SAFE PATH.", GOLD, 2);
  center(320, 66, "PURIFY THREE SHRINES TO AWAKEN EACH GUARDIAN.", DIM);
  button(564, 34, 25, 25, "X");
  regionCard(51, 87, 0);
  regionCard(335, 87, 1);
  button(205, 301, 230, 25,
         g.difficulty ? "DIFFICULTY: VETERAN" : "DIFFICULTY: ADVENTURER");
}
void guide() {
  if (g.openWorld || g.scene >= SPLASH) {
    frontierGuide();
    return;
  }
  dim();
  box(49, 23, 542, 315, PANEL, GOLD);
  center(320, 40, "SURVIVE THE BEAUTIFUL WILDS", GOLD, 2);
  const char *ls[] = {"MOVE: DRAG THE LEFT STICK. HOLD ATTACK TO FIGHT.",
                      "CHANGE: CHOOSE BLADE, AXE OR BOW AT ANY TIME.",
                      "POWER: SWEEP, EARTHQUAKE OR VOLLEY BY WEAPON.",
                      "DODGE: TWO CHARGES. RECHARGES ONE AT A TIME.",
                      "WARD: SLOWS FOES AND REDUCES DAMAGE INSIDE.",
                      "FLASK: HEALS 50% HP. THREE CHARGES PER RUN.",
                      "PURIFY THREE SHRINES. THEIR CHESTS REFILL A FLASK.",
                      "GUARDIANS HAVE THREE PHASES. READ RED WARNINGS.",
                      "SHOCKWAVES HAVE A GREEN GAP. DODGE OR FIND IT.",
                      "HEAVY HITS BUILD STAGGER. PUNISH THE OPENING.",
                      "TAP THE MINI MAP. BAG EQUIPS GEAR OR SALVAGES.",
                      "AUTO-SAVE KEEPS GEAR. REOPENING RETURNS TO CAMP."};
  for (int j = 0; j < 12; j++)
    text(69, 79 + j * 17, ls[j], j % 2 ? DIM : WHITE);
  button(220, 297, 200, 28, "READY TO EXPLORE");
}
void pauseScreen() {
  dim();
  box(182, 28, 276, 304, PANEL, GOLD);
  center(320, 43, "TAKE A BREATH", GOLD, 2);
  button(204, 79, 232, 31, "RESUME");
  button(204, 119, 112, 29, g.muted ? "SOUND OFF" : "SOUND ON");
  button(324, 119, 112, 29, g.shake ? "SHAKE ON" : "SHAKE OFF");
  button(204, 158, 232, 29,
         g.lowPower ? "BATTERY MODE: ON (30)" : "BATTERY MODE: OFF (60)");
  button(204, 196, 112, 29, "ARMORY");
  button(324, 196, 112, 29, "GUIDE");
  button(204, 235, 232, 31,
         g.openWorld ? "SAVE AND EXIT WORLD" : "SAVE AND RETURN HOME");
  center(320, 287,
         g.openWorld ? "POSITION AND GEAR ARE SAVED."
                     : "LEAVING ENDS THIS EXPEDITION.",
         DIM);
  center(320, 304,
         g.openWorld ? "THIS WORLD HAS ITS OWN PROGRESS."
                     : "COLLECTED LOOT IS KEPT.",
         DIM);
}
void mapScreen() {
  if (g.openWorld) {
    frontierMap();
    return;
  }
  dim();
  box(56, 17, 528, 329, PANEL, GOLD);
  center(320, 30, worldName(), GOLD, 2);
  button(541, 26, 29, 25, "X");
  minimap(79, 66, 5, true);
  text(465, 88, "LEGEND", GOLD);
  text(465, 113, "WHITE: YOU", WHITE);
  text(465, 134, "RED: SHRINE", RED);
  text(465, 155, "TEAL: CLEAR", TEAL);
  text(465, 176, "GOLD: BOSS", GOLD);
  text(465, 213, "USE BRIDGES", DIM);
  text(465, 233, "OVER WATER", DIM);
  text(465, 279, "FOLLOW THE", WHITE);
  text(465, 294, "GOLD ARROW", WHITE);
}
void result(bool win) {
  dim();
  box(135, 62, 370, 237, PANEL, GOLD);
  center(320, 83, win ? "A GUARDIAN HAS FALLEN" : "THE WILDS CLAIM YOU",
         win ? GOLD : RED, 2);
  center(320, 121,
         win ? "DAYLIGHT RETURNS. THE DANGER REMAINS."
             : "LEARN THE PATTERN. RETURN STRONGER.",
         WHITE);
  center(320, 148,
         "LEVEL " + num(g.level) + " / " + num(g.kills) + " KILLS / " +
             num(g.gold) + " GOLD",
         TEAL);
  center(320, 177,
         win ? "A HARDER EXPEDITION IS NOW AVAILABLE."
             : "YOUR COLLECTED GOLD AND GEAR ARE SAFE.",
         DIM);
  button(185, 217, 270, 37,
         g.openWorld
             ? (j.home ? "RESPAWN AT YOUR BED" : "RESPAWN AT ORIGIN CAMP")
             : "RETURN TO DAYBREAK CAMP");
}
void render() {
  if (ui7Active()) {
    ui7Render();
    return;
  }
  if (g.scene >= SPLASH)
    frontierMenu();
  else if (g.scene == TITLE)
    title();
  else {
    worldScene();
    if (g.scene == DEAD || g.scene == WIN)
      result(g.scene == WIN);
  }
  if (g.overlay == 1)
    pauseScreen();
  if (g.overlay == 2)
    inventory();
  if (g.overlay == 3)
    guide();
  if (g.overlay == 4)
    worlds();
  if (g.overlay == 5)
    armory();
  if (g.overlay == 6)
    mapScreen();
  if (g.openWorld && g.overlay == 7)
    survivalScreen();
  if (g.openWorld && g.overlay == 8)
    ecologyScreen();
  if (g.openWorld && g.overlay == 9)
    woodsScreen();
  if (g.openWorld && g.overlay == 10)
    drawJourneyPanel();
}
} // namespace av
