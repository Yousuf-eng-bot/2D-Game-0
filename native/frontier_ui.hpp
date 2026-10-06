#pragma once
#include "presentation.hpp"
namespace av {
C biomeColor(int b) {
  const C c[] = {0xff87b273, 0xffd9b779, 0xffcbdde3, 0xff799e87, 0xff77656b};
  return c[std::clamp(b, 0, 4)];
}
C biomeAccent(int b) {
  const C c[] = {0xffc6e9a3, 0xffffd394, 0xfff0fbff, 0xffb5e0c9, 0xffffab75};
  return c[std::clamp(b, 0, 4)];
}
const char *guardianName(int b) {
  const char *n[] = {"CROWNHORN / FOREST SOVEREIGN", "DUNEBREAKER / SAND TITAN",
                     "RIMEKEEPER / FROST TITAN", "MIREHEART / BOG WARDEN",
                     "PYREKING / ASH COLOSSUS"};
  return n[std::clamp(b, 0, 4)];
}
void panorama(bool dark = false) {
  if (cover.size() == W * H) {
    int offsetX = int((1 + std::sin(g.time * .08f)) * 43),
        offsetY = 18 + int(std::sin(g.time * .11f) * 13);
    for (int y = 0; y < H; y++)
      for (int x = 0; x < W; x++)
        pix[y * W + x] =
            cover[std::clamp(int(y * .88f) + offsetY, 0, H - 1) * W +
                  std::clamp(int(x * .86f) + offsetX, 0, W - 1)];
  } else {
    rect(0, 0, W, H, 0xff7fbbbc);
    poly({{0, 290},
          {170, 70},
          {340, 245},
          {510, 95},
          {640, 210},
          {640, 360},
          {0, 360}},
         0xff3d7267);
  }
  for (int k = 0; k < 22; k++) {
    int x = int(hash(k + 77) % 640 + g.time * (4 + k % 7)) % 640,
        y = 30 + hash(k + 999) % 290;
    rect(x, y, 1 + (k % 3 == 0), 1, 0xffe9e9ac);
  }
  shade(0, 0, W, H, 0xff0a202b, dark ? 178 : 55);
  shade(0, 290, W, 70, INK, 130);
}
void crystal(int x, int y, float t, int size = 32) {
  int spread = int((1 - clamp(t, 0, 1)) * 90);
  C edge = 0xffb1f3e0;
  poly({{x, y - size * 2 - spread},
        {x - size, y - size / 2},
        {x, y + size},
        {x + size, y - size / 2}},
       0xff253f56);
  poly({{x, y - size * 2 - spread}, {x - size, y - size / 2}, {x, y + size}},
       0xff4e8391);
  poly({{x, y - size * 2 - spread}, {x, y + size}, {x + size, y - size / 2}},
       0xff142c46);
  line(x, y - size * 2 - spread, x - size, y - size / 2, edge);
  line(x - size, y - size / 2, x, y + size, edge);
  line(x, y - size * 2 - spread, x + size, y - size / 2, GOLD);
  line(x, y - size * 2 - spread, x, y + size, 0xff85b6b3);
  for (int k = 0; k < 6; k++) {
    float a = k * PI / 3 + t * .7f;
    int xx = x + int(std::cos(a) * (size + 20 + spread)),
        yy = y - 12 + int(std::sin(a) * (size + 20));
    rect(xx, yy, 2, 2, edge);
  }
}
void biomePreview(int x, int y, int w, int h, int b) {
  rect(x, y, w, h, biomeColor(b));
  circle(x + w - 18, y + 12, 6, biomeAccent(b), true);
  poly({{x, y + h},
        {x + w / 3, y + 8},
        {x + w * 2 / 3, y + h},
        {x + w, y + 18},
        {x + w, y + h}},
       mix(biomeColor(b), INK, 50));
  if (b == 2)
    poly({{x + w / 3, y + 8}, {x + w / 3 - 9, y + 22}, {x + w / 3 + 9, y + 22}},
         WHITE);
  if (b == 4) {
    line(x + w / 3, y + 8, x + w / 2, y + h, 0xffff9869);
    rect(x + w / 3 - 4, y + 7, 8, 3, GOLD);
  }
  if (b == 0 || b == 3) {
    rect(x + 12, y + h - 22, 3, 20, 0xff755b46);
    circle(x + 13, y + h - 23, 9, b == 0 ? 0xff37775a : 0xff395c58, true);
  }
  if (b == 1) {
    rect(x + 15, y + h - 24, 4, 23, 0xff678661);
    line(x + 8, y + h - 14, x + 18, y + h - 14, 0xff678661);
    line(x + 8, y + h - 22, x + 8, y + h - 14, 0xff678661);
  }
}
void frontierMenu() {
  if (g.scene == SPLASH) {
    rect(0, 0, W, H, 0xff091620);
    float t = clamp(o.screenTime / 1.2f, 0, 1);
    float ease = t * t * (3 - 2 * t);
    for (int k = 0; k < 7; k++)
      circle(320, 128, 48 + k * 7, mix(0xff112b38, 0xff0a1822, k * 30));
    crystal(320, 132, ease, 30);
    if (o.screenTime > .65f)
      center(320, 224, "OBSIDIAN GAMES", WHITE, 3);
    if (o.screenTime > 1.2f) {
      rect(220, 258, 200, 1, GOLD);
      center(320, 275, "A STUDIO OF OBSIDIAN SYNDICATE", DIM);
    }
    center(320, 337, "TAP TO CONTINUE", 0xff668a93);
    if (o.screenTime < .4f)
      shade(0, 0, W, H, INK, int(255 * (1 - o.screenTime / .4f)));
  } else {
    panorama(g.scene != HOME);
    if (g.scene == HOME) {
      shade(130, 27, 380, 142, INK, 140);
      center(320, 43, "OBSIDIAN GAMES PRESENTS", 0xffc0e7d8);
      center(322, 76, "DEATH WORLD", INK, 4);
      center(320, 74, "DEATH WORLD", WHITE, 4);
      rect(230, 120, 180, 2, GOLD);
      center(320, 138, "COMBAT / CRAFT / SURVIVE", GOLD);
      int pulse = int(std::sin(g.time * 2) * 12);
      box(145, 204, 350, 63, 0xff204e50, mix(GOLD, WHITE, 65 + pulse));
      rect(149, 208, 342, 2, 0xff63978a);
      center(320, 224, "SEE WORLDS", WHITE, 3);
      center(320, 283, "CREATE. EXPLORE. RETURN TO YOUR OWN WORLD.", WHITE);
      button(26, 317, 118, 27, "HOW TO PLAY");
      button(155, 317, 117, 27, "CLASSIC 0.2");
      button(283, 317, 144, 27, "FEEDBACK INFO");
      button(455, 317, 158, 27, g.muted ? "SOUND: OFF" : "SOUND: ON");
      text(12, 10, "FORGED FRONTIER 0.6", WHITE);
    }
    if (g.scene == WORLDS) {
      panel(25, 15, 590, 332);
      text(43, 29, "YOUR WORLDS", WHITE, 3);
      button(544, 27, 54, 27, "HOME");
      box(42, 66, 556, 38, 0xff355d53, GOLD);
      center(320, 77, "+ CREATE WORLD", WHITE, 2);
      if (o.worlds.empty()) {
        center(320, 178, "YOUR NEXT ADVENTURE STARTS HERE.", GOLD);
        center(320, 203, "CREATE A WORLD WITH ITS OWN SEED AND SAVE.", DIM);
      }
      for (int row = 0; row < 3; row++) {
        int i = o.listPage * 3 + row;
        if (i >= int(o.worlds.size()))
          break;
        auto &r = o.worlds[i];
        int y = 115 + row * 63;
        box(42, y, 556, 56, 0xff173840, EDGE);
        biomePreview(46, y + 4, 64, 48, int(r.seed % 5));
        text(124, y + 8, r.name, WHITE, 2);
        text(124, y + 29,
             "LV " + num(r.level) + " / " + num(int(r.seconds / 60)) +
                 " MIN / SEED " + std::to_string(r.seed),
             DIM);
        text(124, y + 42,
             r.recovered        ? "BACKUP AVAILABLE"
             : r.generator == 3 ? "VAST BIOMES / EXPLORATION FOG"
             : r.generator == 2 ? "GREAT BIOMES / HARD SURVIVAL"
                                : "ORIGINAL GEOGRAPHY / HARD SURVIVAL",
             r.recovered ? GOLD : TEAL);
        button(553, y + 16, 32, 26, "X");
      }
      button(43, 313, 85, 24, "< PREV");
      button(513, 313, 85, 24, "NEXT >");
      center(320, 321,
             num(o.worlds.size()) + " SAVES / PAGE " + num(o.listPage + 1),
             DIM);
    }
    if (g.scene == CREATE) {
      panel(45, 15, 550, 332);
      center(320, 29, "CREATE A WORLD", WHITE, 3);
      center(320, 59, "VAST 32768-TILE BIOMES / DISCOVER THE UNKNOWN", GOLD);
      text(72, 76, "WORLD NAME / TAP TO EDIT", DIM);
      button(72, 89, 496, 31, o.draftName);
      text(72, 132, "SEED / TAP TO ENTER A NUMBER", DIM);
      button(72, 145, 360, 28, std::to_string(o.draftSeed));
      button(442, 145, 126, 28, "NEW SEED");
      const char *n[] = {"FOREST", "DESERT", "SNOW", "SWAMP", "VOLCANIC"};
      for (int b = 0; b < 5; b++) {
        int x = 72 + b * 101;
        biomePreview(x, 187, 92, 39, b);
        center(x + 46, 234, n[b], biomeAccent(b));
      }
      button(72, 259, 235, 27,
             g.difficulty ? "VETERAN DIFFICULTY" : "ADVENTURER DIFFICULTY");
      button(324, 259, 244, 27, std::string("START: ") + weaponName(g.weapon));
      button(72, 305, 128, 28, "CANCEL");
      button(214, 301, 354, 36, "CREATE AND ENTER");
    }
    if (g.scene == DELETE_WORLD) {
      box(107, 85, 426, 209, PANEL, RED);
      center(320, 104, "DELETE THIS WORLD?", RED, 2);
      if (o.selected >= 0 && o.selected < int(o.worlds.size()))
        center(320, 140, o.worlds[o.selected].name, WHITE, 2);
      center(320, 174, "ITS CHARACTER AND PROGRESS WILL BE REMOVED.", DIM);
      center(320, 194, "THIS CANNOT BE UNDONE IN THE GAME.", DIM);
      button(129, 239, 178, 32, "KEEP WORLD");
      button(333, 239, 178, 32, "DELETE WORLD");
    }
    if (g.scene == LOADING) {
      crystal(320, 135, 1, 27);
      center(320, 215,
             o.pendingCreate ? "WEAVING YOUR WORLD" : "RETURNING TO YOUR WORLD",
             WHITE, 2);
      center(320, 246, "SEED-BASED TERRAIN / LOCAL CHUNK STREAMING", DIM);
      for (int k = 0; k < 5; k++)
        rect(283 + k * 16, 275, 9, 5,
             k <= int(o.screenTime * 15) % 5 ? GOLD : EDGE);
    }
  }
  if (g.toastTime > 0 && g.scene != SPLASH) {
    panel(55, 176, 530, 24);
    center(320, 184, g.toast, GOLD);
  }
  if (o.transition > 0) {
    float a = o.transition > .22f ? (.44f - o.transition) / .22f
                                  : o.transition / .22f;
    shade(0, 0, W, H, INK, int(clamp(a, 0, 1) * 255));
  }
}
void frontierTerrain() { drawNewTerrain(); }
void frontierTerritories() {
  for (auto &p : g.props)
    if (p.kind == 11) {
      Camp c = campAt(p.campX, p.campY);
      bool clear = cleared(c);
      if (len(g.px - p.x, g.py - p.y) < c.radius + 350) {
        C col = clear ? 0xff96c9a8 : c.boss ? 0xffc67c62 : 0xffb7aa73;
        for (int k = 0; k < 48; k++) {
          float a = k * 2 * PI / 48;
          int x = sx(p.x) + int(std::cos(a) * c.radius),
              y = syAt(p.x,p.y) + int(std::sin(a) * c.radius);
          rect(x, y, 3, 2, col);
        }
        if (len(g.px - p.x, g.py - p.y) < c.radius + 40)
          center(sx(p.x), syAt(p.x,p.y) - 30,
                 clear    ? "CLEARED"
                 : c.boss ? "GUARDIAN STRONGHOLD"
                          : "MONSTER TERRITORY",
                 clear ? TEAL : GOLD);
      }
    }
  // Local weather is decorative and never covers telegraph readability.
  if (g.world == 2 || g.world == 3 || g.world == 4)
    for (int k = 0; k < 22; k++) {
      int x = int(hash(k + 561) % 640 + g.time * (g.world == 3 ? 9 : 4)) % 640;
      int y = int(hash(k + 734) % 360 + g.time * (g.world == 3   ? 85
                                                  : g.world == 2 ? 15
                                                                 : 7)) %
              360;
      if (g.world == 2)
        rect(x, y, 2, 2, 0xfff4faf5);
      else if (g.world == 3)
        line(x, y, x - 2, y + 5, 0xffa9c9bc);
      else
        rect(x, H - y, 2, 1, 0xffffad70);
    }
}
void localMap(int x, int y, int scale) {
  panel(x - 4, y - 4, MW * scale + 8, MH * scale + 8);
  for (int j = 0; j < MH; j++)
    for (int i = 0; i < MW; i++) {
      int a = j * MW + i;
      C col = g.map[a] == 4   ? (g.biomes[a] == 4 ? 0xffcf7754 : 0xff4f939f)
              : g.map[a] == 1 ? biomeColor(g.biomes[a])
                              : mix(biomeColor(g.biomes[a]), INK, 125);
      if (!exploredAt(o.originX + i, o.originY + j))
        col = 0xff030708;
      rect(x + i * scale, y + j * scale, scale, scale, col);
    }
  for (auto &p : g.props)
    if (p.kind == 11 &&
        exploredAt(o.originX + int(p.x / T), o.originY + int(p.y / T))) {
      Camp c = campAt(p.campX, p.campY);
      rect(x + int(p.x / T) * scale - 2, y + int(p.y / T) * scale - 2, 4, 4,
           cleared(c) ? TEAL
           : c.boss   ? GOLD
                      : RED);
    }
  circle(x + int(g.px / T) * scale, y + int(g.py / T) * scale,
         scale > 1 ? 4 : 2, WHITE, true);
}
void frontierHud() {
  panel(9, 9, 184, 51);
  box(14, 14, 28, 29, 0xff32514a, GOLD);
  center(28, 20, num(g.level), GOLD, 2);
  text(49, 15, "FRONTIER RANGER", GOLD);
  bar(49, 28, 134, 12, g.hp / maxhp(), RED);
  text(49, 45, num(int(g.hp)) + " / " + num(maxhp()) + " HP", WHITE);
  bar(14, 55, 169, 4, g.xp / float(g.level * 60), TEAL);
  panel(10, 63, 180, 17);
  text(15, 68, "G " + num(g.gold) + "  ATK " + num(damage()), WHITE);
  panel(200, 9, 228, 51);
  center(314, 15, biomeName(g.world), biomeAccent(g.world));
  center(314, 30, o.name, WHITE);
  center(314, 46,
         "X " + std::to_string(int64_t(globalX())) + "  Y " +
             std::to_string(int64_t(globalY())),
         DIM);
  button(435, 10, 43, 27, "BAG");
  button(483, 10, 44, 27, "II");
  localMap(550, 11, 1);
  text(548, 70, "BIOME ATLAS", INK);
  Enemy *boss = nullptr;
  float best = 420;
  for (auto &e : g.enemies)
    if (e.alive && e.boss() && e.alertTime > 0) {
      float d = len(e.x - g.px, e.y - g.py);
      if (d < best) {
        best = d;
        boss = &e;
      }
    }
  if (boss) {
    auto &e = *boss;
    panel(201, 105, 283, 31);
    center(343, 109, guardianName(e.biome), GOLD);
    bar(211, 120, 263, 7, e.hp / e.maxhp, RED);
    bar(211, 130, 263, 3, e.stagger / 100, TEAL);
    if (e.wind > 0) {
      panel(200, 142, 288, 17);
      center(344, 147, patternName(e.pattern), GOLD);
    }
  }
  if (o.waypoint) {
    double dx = double(o.waypointX) - globalX(),
           dy = double(o.waypointY) - globalY(), d = std::hypot(dx, dy);
    if (d < 5) {
      o.waypoint = false;
      notify("WAYPOINT REACHED");
    } else {
      int x = sx(g.px) + int(dx / d * 70), y = syAt(g.px,g.py) + int(dy / d * 70);
      poly({{x, y},
            {x - int(dx / d * 12 - dy / d * 6),
             y - int(dy / d * 12 + dx / d * 6)},
            {x - int(dx / d * 12 + dy / d * 6),
             y - int(dy / d * 12 - dx / d * 6)}},
           GOLD);
      panel(213, 84, 200, 17);
      center(313, 89, "WAYPOINT " + num(int(d)) + " TILES", GOLD);
    }
  }
  button(278, 309, 146, 35, v.sprint ? "SPRINT: ON" : "SPRINT: OFF");
  survivalHud();
  controls();
  if (g.toastTime > 0) {
    int w = std::min(616, tw(g.toast) + 16);
    panel(320 - w / 2, 151, w, 22);
    center(320, 158, g.toast, GOLD);
  }
}
void frontierMap() {
  dim();
  box(48, 17, 544, 329, PANEL, GOLD);
  text(65, 31, o.atlasLarge ? "REGION ATLAS" : "LOCAL ATLAS", GOLD, 2);
  button(433, 26, 101, 25, o.atlasLarge ? "LOCAL VIEW" : "REGION VIEW");
  button(546, 26, 30, 25, "X");
  if (o.atlasLarge)
    drawExploreAtlas();
  else
    localMap(69, 65, 5);
  const char *n[] = {"FOREST", "DESERT", "SNOW", "SWAMP", "VOLCANIC"};
  text(448, 70, "SET COMPASS", WHITE);
  for (int b = 0; b < 5; b++)
    button(445, 89 + b * 34, 132, 27,
           (o.discovered & (1 << b)) ? std::string(n[b]) + " +" : "UNEXPLORED");
  button(445, 265, 132, 27, "ORIGIN CAMP");
  text(446, 308, o.atlasLarge ? "BLACK: UNKNOWN" : "RED: CAMP",
       o.atlasLarge ? DIM : RED);
  text(446, 323, o.atlasLarge ? "KNOWN LAND ONLY" : "GOLD: BOSS", GOLD);
}
void frontierGuide() {
  dim();
  box(41, 22, 558, 316, PANEL, GOLD);
  center(320, 39, "THE FRONTIER / FIELD GUIDE", GOLD, 2);
  const char *lines[] = {"LIGHT COMBOS / HEAVY WINDUP / GUARD: TIMED PARRY.",
                         "DODGE HAS CANCEL WINDOWS. KEEP SOME STAMINA.",
                         "JUMP SMALL GAPS. CROUCH CYCLES TO PRONE CRAWL.",
                         "STAND TO FIGHT. HUNT OFF HELPS TARGET TREES/FOES.",
                         "EQUIP AXE, CUT TREES, APPROACH FLOATING LOG DROPS.",
                         "CRAFT PLANKS/STICKS, THEN PLACE A WORKBENCH.",
                         "MINE/TOOL: ROCKS OR CLEAR SOIL. UPGRADE YOUR PICK.",
                         "CYCLE TYPE / PLACE: BUILD IN THE ADJACENT CELL.",
                         "FURNACE: QUEUE ORE + COAL; STAY NEAR TO SIMULATE.",
                         "INTERACT: DOOR, RIPE CROP OR SUPPLIED BED REST.",
                         "BODY: FOOD, WATER, WOUNDS. BLACK MAP: UNEXPLORED.",
                         "NEW WORLDS GET HEIGHT. PAUSE > SAVE AND EXIT."};
  for (int i = 0; i < 12; i++)
    text(59, 75 + i * 17, lines[i], i % 2 ? DIM : WHITE);
  button(120, 298, 195, 28, "READY TO EXPLORE");
  button(325, 298, 195, 28, "FOOD WEBS");
}
bool frontierTouch(float x, float y) {
  if (o.transition > 0)
    return true;
  if (woodsTouch(x, y))
    return true;
  if (survivalTouch(x, y))
    return true;
  auto in = [&](int a, int b, int w, int h) {
    return x >= a && x < a + w && y >= b && y < b + h;
  };
  if (g.openWorld && g.overlay == 6) {
    if (in(433, 26, 101, 25)) {
      o.atlasLarge = !o.atlasLarge;
      return true;
    }
    if (in(546, 26, 30, 25)) {
      g.overlay = 0;
      return true;
    }
    for (int b = 0; b < 5; b++)
      if (in(445, 89 + b * 34, 132, 27)) {
        knownBiomeWaypoint(b);
        g.overlay = 0;
        sfx(15);
        return true;
      }
    if (in(445, 265, 132, 27)) {
      o.waypoint = true;
      o.waypointX = o.waypointY = 0;
      g.overlay = 0;
    }
    return true;
  }
  if (g.openWorld && g.overlay == 1 && in(204, 235, 232, 31)) {
    leaveFrontier();
    return true;
  }
  if (g.openWorld && g.scene == DEAD && !g.overlay && in(185, 217, 270, 37)) {
    respawnFrontier();
    return true;
  }
  if (g.openWorld && g.scene == PLAY && !g.overlay && in(278, 309, 146, 35)) {
    v.sprint = !v.sprint;
    notify(v.sprint ? "SPRINTING USES STAMINA AND WATER"
                    : "WALKING - SAVING ENERGY");
    return true;
  }
  if (g.overlay)
    return false;
  if (g.scene == SPLASH) {
    startTransition(HOME);
    return true;
  }
  if (g.scene == HOME) {
    if (in(145, 204, 350, 63)) {
      scanWorlds();
      startTransition(WORLDS);
    }
    if (in(26, 317, 118, 27))
      g.overlay = 3;
    if (in(155, 317, 117, 27)) {
      std::string path = g.path;
      g = State{};
      g.path = path;
      if (!loadOne(path + "/progress.sav"))
        loadOne(path + "/progress.sav.bak");
      g.openWorld = false;
      loadSettings();
      hub();
    }
    if (in(283, 317, 144, 27))
      o.textRequest = 3;
    if (in(455, 317, 158, 27)) {
      g.muted = !g.muted;
      g.sfxCount = 0;
      saveSettings();
    }
    return true;
  }
  if (g.scene == WORLDS) {
    if (in(544, 27, 54, 27))
      startTransition(HOME);
    if (in(42, 66, 556, 38)) {
      o.draftName = "NEW HORIZON " + num(int(o.worlds.size() + 1));
      o.draftSeed = entropy();
      startTransition(CREATE);
    }
    if (in(43, 313, 85, 24))
      o.listPage = std::max(0, o.listPage - 1);
    if (in(513, 313, 85, 24))
      o.listPage =
          std::min(std::max(0, (int(o.worlds.size()) - 1) / 3), o.listPage + 1);
    for (int row = 0; row < 3; row++) {
      int i = o.listPage * 3 + row, y0 = 115 + row * 63;
      if (i >= int(o.worlds.size()))
        break;
      if (in(42, y0, 556, 56)) {
        o.selected = i;
        if (in(553, y0 + 16, 32, 26))
          startTransition(DELETE_WORLD);
        else {
          o.pendingCreate = false;
          startTransition(LOADING);
        }
        break;
      }
    }
    return true;
  }
  if (g.scene == CREATE) {
    if (in(72, 89, 496, 31))
      o.textRequest = 1;
    if (in(72, 145, 360, 28))
      o.textRequest = 2;
    if (in(442, 145, 126, 28)) {
      o.draftSeed = entropy();
      sfx(12);
    }
    if (in(72, 259, 235, 27))
      g.difficulty = 1 - g.difficulty;
    if (in(324, 259, 244, 27))
      g.weapon = (g.weapon + 1) % 3;
    if (in(72, 305, 128, 28))
      startTransition(WORLDS);
    if (in(214, 301, 354, 36)) {
      o.pendingCreate = true;
      startTransition(LOADING);
    }
    return true;
  }
  if (g.scene == DELETE_WORLD) {
    if (in(129, 239, 178, 32))
      startTransition(WORLDS);
    if (in(333, 239, 178, 32) && o.selected >= 0 &&
        o.selected < int(o.worlds.size())) {
      auto id = o.worlds[o.selected].id;
      if (validId(id)) {
        std::error_code ec;
        std::filesystem::remove_all(
            std::filesystem::path(worldFile(id)).parent_path(), ec);
        if (ec)
          notify("DELETE FAILED - SAVE KEPT");
      }
      scanWorlds();
      startTransition(WORLDS);
    }
    return true;
  }
  return g.scene == LOADING;
}
} // namespace av
