#pragma once
#include "persistence.hpp"
namespace av {
bool walk(float x, float y) {
  int i = int(std::floor(x / T)), j = int(std::floor(y / T));
  return i >= 0 && i < MW && j >= 0 && j < MH && g.map[j * MW + i] == 1;
}
bool fits(float x, float y, float r = 7) {
  return walk(x - r, y - r) && walk(x + r, y - r) && walk(x - r, y + r) &&
         walk(x + r, y + r);
}
void move(float &x, float &y, float dx, float dy, float r = 7) {
  int steps =
      std::max(1, int(std::ceil(std::max(std::abs(dx), std::abs(dy)) / 4)));
  dx /= steps;
  dy /= steps;
  for (int i = 0; i < steps; i++) {
    if (journeyMoveAllowed(x + dx, y, x, y, r, &x == &g.px))
      x += dx;
    if (journeyMoveAllowed(x, y + dy, x, y, r, &x == &g.px))
      y += dy;
  }
}
bool sight(float x, float y, float xx, float yy) {
  float d = len(xx - x, yy - y);
  int n = std::max(1, int(d / 8));
  for (int i = 1; i <= n; i++)
    if (!walk(x + (xx - x) * i / n, y + (yy - y) * i / n))
      return false;
  return true;
}
void navigation() {
  g.distance.fill(-1);
  std::array<int, MW * MH> q{};
  int head = 0, tail = 0, start = int(g.py / T) * MW + int(g.px / T);
  if (start < 0 || start >= MW * MH || g.map[start] != 1)
    return;
  g.distance[start] = 0;
  q[tail++] = start;
  while (head < tail) {
    int a = q[head++], x = a % MW, y = a / MW;
    const int dx[4] = {1, -1, 0, 0}, dy[4] = {0, 0, 1, -1};
    for (int k = 0; k < 4; k++) {
      int xx = x + dx[k], yy = y + dy[k];
      if (xx >= 0 && xx < MW && yy >= 0 && yy < MH) {
        int b = yy * MW + xx;
        if (g.map[b] == 1 && g.distance[b] < 0 && terrainLink(a, b)) {
          g.distance[b] = g.distance[a] + 1;
          q[tail++] = b;
        }
      }
    }
  }
}
void steer(Enemy &e, float &dx, float &dy) {
  int x = int(e.x / T), y = int(e.y / T);
  if (x < 0 || x >= MW || y < 0 || y >= MH)
    return;
  int a = y * MW + x, best = g.distance[a], chosen = a;
  const int xx[4] = {1, -1, 0, 0}, yy[4] = {0, 0, 1, -1};
  for (int k = 0; k < 4; k++) {
    int nx = x + xx[k], ny = y + yy[k];
    if (nx >= 0 && nx < MW && ny >= 0 && ny < MH) {
      int b = ny * MW + nx;
      if (g.distance[b] >= 0 && (best < 0 || g.distance[b] < best)) {
        best = g.distance[b];
        chosen = b;
      }
    }
  }
  if (chosen != a) {
    dx = (chosen % MW + .5f) * T - e.x;
    dy = (chosen / MW + .5f) * T - e.y;
  } else
    dx = dy = 0;
}
void patch(int x, int y, int r, int ground = 0) {
  for (int j = y - r; j <= y + r; j++)
    for (int i = x - r; i <= x + r; i++)
      if (i > 1 && i < MW - 2 && j > 1 && j < MH - 2) {
        bool bridge = g.map[j * MW + i] == 4 || g.ground[j * MW + i] == 3;
        g.map[j * MW + i] = 1;
        if (ground)
          g.ground[j * MW + i] = ground == 2 && bridge ? 3 : ground;
      }
}
void road(int x, int y, int xx, int yy) {
  while (x != xx || y != yy) {
    patch(x, y, 1, 2);
    if (x != xx)
      x += (xx > x ? 1 : -1);
    else
      y += (yy > y ? 1 : -1);
  }
  patch(x, y, 1, 2);
}
void addEnemy(float x, float y, int kind, int room, bool elite = false) {
  Enemy e;
  e.entityId = ++g.entitySerial;
  e.x = x;
  e.y = y;
  e.kind = kind;
  e.room = room;
  e.elite = elite;
  e.cd = .4f + rnd(100) / 100.f;
  float power = std::pow(std::max(1.f, g.runPower / 22.f), .6f);
  float scale = power * (1 + .12f * std::min(g.worldWins[g.world], 12)) *
                (g.difficulty ? 1.25f : 1);
  float hp = kind >= 8   ? (g.world == 0 ? 1250.f : 1580.f)
             : kind == 0 ? 66.f
             : kind == 1 ? 48.f
             : kind == 2 ? 46.f
             : kind == 3 ? 140.f
                         : 78.f;
  e.hp = e.maxhp = hp * scale * (elite ? 1.65f : 1);
  g.enemies.push_back(e);
}
void addAnimal(float x, float y, int kind) {
  if (fits(x, y))
    g.animals.push_back({x, y, 0, 0, float(rnd(100)) / 50, 0, kind});
}
void hub() {
  g.scene = HUB;
  resetCombat();
  g.map.fill(0);
  g.ground.fill(0);
  g.enemies.clear();
  g.props.clear();
  g.animals.clear();
  for (int y = 2; y < 22; y++)
    for (int x = 2; x < 27; x++)
      g.map[y * MW + x] = 1;
  road(13, 6, 13, 20);
  road(5, 12, 23, 12);
  g.px = 13 * T;
  g.py = 15 * T;
  g.gate = false;
  g.bossKilled = false;
  for (int x = 2; x < 27; x += 3) {
    g.props.push_back({float(x * T + 12), float(4 * T), 0, x % 3});
    g.props.push_back({float(x * T + 12), float(21 * T), 0, (x + 1) % 3});
  }
  for (int j = 0; j < 18; j++)
    g.props.push_back(
        {float((4 + rnd(20)) * T), float((6 + rnd(12)) * T), 3, j % 3});
  g.props.push_back({6 * T, 11 * T, 8, 0});
  g.props.push_back({21 * T, 11 * T, 8, 1});
  g.props.push_back({21 * T, 16 * T, 9, 0});
  addAnimal(7 * T, 17 * T, 0);
  addAnimal(18 * T, 18 * T, 1);
  addAnimal(5 * T, 14 * T, 1);
  save();
  notify(g.migrated ? "WELCOME BACK. YOUR ORIGINAL PROGRESS IS SAFE."
                    : "DAYBREAK CAMP - CHOOSE A WORLD OR WEAPON");
}
void expedition() {
  g.scene = PLAY;
  resetCombat();
  g.map.fill(0);
  g.ground.fill(0);
  g.enemies.clear();
  g.enemies.reserve(96);
  g.props.clear();
  g.animals.clear();
  g.run++;
  rng = hash(8171 + g.run * 7 + g.world * 521);
  g.runPower = damage();
  g.gate = g.bossKilled = false;
  std::fill(g.rooms, g.rooms + 3, false);
  for (int y = 2; y < MH - 2; y++)
    for (int x = 2; x < MW - 2; x++)
      g.map[y * MW + x] = 1;
  // A river separates the wilderness. Deliberate roads create guaranteed
  // bridges.
  for (int y = 2; y < MH - 2; y++) {
    int rx = 32 + int(std::sin(y * .19f) * 2);
    for (int x = rx; x < rx + (g.world ? 3 : 5); x++)
      g.map[y * MW + x] = 4;
  }
  if (g.world) {
    g.roomX[0] = 18 * T;
    g.roomY[0] = 33 * T;
    g.roomX[1] = 41 * T;
    g.roomY[1] = 16 * T;
    g.roomX[2] = 55 * T;
    g.roomY[2] = 36 * T;
    g.bossX = 60 * T;
    g.bossY = 11 * T;
  } else {
    g.roomX[0] = 19 * T;
    g.roomY[0] = 31 * T;
    g.roomX[1] = 39 * T;
    g.roomY[1] = 16 * T;
    g.roomX[2] = 53 * T;
    g.roomY[2] = 34 * T;
    g.bossX = 59 * T;
    g.bossY = 12 * T;
  }
  road(8, 41, int(g.roomX[0] / T), 41);
  road(int(g.roomX[0] / T), 41, int(g.roomX[0] / T), int(g.roomY[0] / T));
  road(19, 31, 27, 31);
  road(18, 33, 27, 33);
  road(27, 33, 27, 17);
  road(27, 17, 60, 17);
  road(53, 17, 53, 36);
  road(53, 36, 60, 36);
  road(60, 36, 60, 11);
  patch(8, 41, 4);
  for (int i = 0; i < 3; i++)
    patch(int(g.roomX[i] / T), int(g.roomY[i] / T), 5);
  patch(int(g.bossX / T), int(g.bossY / T), 7, 4);
  for (int y = 3; y < MH - 3; y++)
    for (int x = 3; x < MW - 3; x++) {
      int a = y * MW + x;
      if (g.map[a] != 1 || g.ground[a])
        continue;
      bool safe = len(x * T - 8 * T, y * T - 41 * T) < 110 ||
                  len(x * T - g.bossX, y * T - g.bossY) < 195;
      for (int r = 0; r < 3; r++)
        safe |= len(x * T - g.roomX[r], y * T - g.roomY[r]) < 135;
      if (safe)
        continue;
      uint32_t h = hash(x + y * MW + g.world * 193);
      if (h % 100 < 4) {
        g.map[a] = 2;
        g.props.push_back(
            {float(x * T + 12), float(y * T + 12), 0, int(h % 3)});
      } else if (h % 100 < 6) {
        g.map[a] = 3;
        g.props.push_back(
            {float(x * T + 12), float(y * T + 12), 1, int(h % 3)});
      } else if (h % 13 == 0)
        g.props.push_back(
            {float(x * T + 12), float(y * T + 12), 3, int(h % 3)});
      else if (h % 37 == 0)
        g.props.push_back(
            {float(x * T + 12), float(y * T + 12), 4, int(h % 3)});
    }
  for (int r = 0; r < 3; r++) {
    g.props.push_back({g.roomX[r], g.roomY[r] - 90, 5, r});
    g.props.push_back({g.roomX[r] + 85, g.roomY[r] + 70, 6, r});
    int count = 8 + g.world * 2 + std::min(g.worldWins[g.world], 3);
    for (int j = 0; j < count; j++) {
      float a = j * 2 * PI / count;
      float d = 40 + rnd(60);
      float x = g.roomX[r] + std::cos(a) * d, y = g.roomY[r] + std::sin(a) * d;
      if (!fits(x, y))
        continue;
      addEnemy(x, y, j % 5, r, j == count - 1);
    }
  }
  for (int i = 0; i < 6; i++) {
    float x = (i < 3 ? 23 : 48) * T + (rnd(80) - 40), y = (22 + i * 2) * T;
    if (fits(x, y))
      addEnemy(x, y, i % 3, 3, false);
  }
  addEnemy(g.bossX, g.bossY, g.world ? 9 : 8, 4);
  g.props.push_back({g.bossX - 130, g.bossY - 120, 7, 0});
  g.props.push_back({g.bossX + 130, g.bossY - 120, 7, 1});
  for (int i = 0; i < 22; i++) {
    float x = (4 + rnd(MW - 8)) * T, y = (6 + rnd(MH - 12)) * T;
    addAnimal(x, y, g.world ? 2 : i % 2);
  }
  addAnimal(10 * T, 40 * T, g.world ? 2 : 0);
  addAnimal(6 * T, 39 * T, 1);
  addAnimal(12 * T, 43 * T, 1);
  g.px = 8 * T;
  g.py = 41 * T;
  g.fx = 1;
  g.fy = 0;
  save();
  notify(std::string(worldName()) + " - PURIFY ALL THREE WILD SHRINES");
}
void updateAnimals(float dt) {
  if (g.openWorld) {
    updateEcology(dt);
    return;
  }
  for (auto &a : g.animals) {
    a.timer -= dt;
    float d = len(a.x - g.px, a.y - g.py);
    if (d < 65) {
      a.dx = (a.x - g.px) / std::max(1.f, d);
      a.dy = (a.y - g.py) / std::max(1.f, d);
      a.timer = .5f;
    } else if (a.timer <= 0) {
      float an = rnd(628) / 100.f;
      a.dx = std::cos(an) * (rnd(3) ? 1 : 0);
      a.dy = std::sin(an) * (a.dx == 0 ? 0 : 1);
      a.timer = 1 + rnd(200) / 100.f;
    }
    float speed = d < 65 ? 90 : 14;
    float x = a.x, y = a.y;
    move(a.x, a.y, a.dx * dt * speed, a.dy * dt * speed, 4);
    a.step += len(a.x - x, a.y - y) * .15f;
  }
}
} // namespace av
