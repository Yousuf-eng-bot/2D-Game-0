// Historical coordinate adapter; new UI is exercised separately in ui7_tests.
#define DW_LEGACY_TEST_UI
#include "../native/engine.cpp"
#include <iostream>
using namespace av;
// This agent only chooses ordinary movement, attacks, skills, and inventory
// actions. It has perfect state knowledge, but never teleports, grants health,
// or changes damage.
bool bodySight(float tx, float ty) {
  int steps = std::max(1, int(std::ceil(len(tx - g.px, ty - g.py) / 4)));
  for (int i = 1; i <= steps; i++)
    if (!fits(g.px + (tx - g.px) * i / steps, g.py + (ty - g.py) * i / steps))
      return false;
  return true;
}
void route(float tx, float ty, float &dx, float &dy) {
  if (bodySight(tx, ty)) {
    dx = tx - g.px;
    dy = ty - g.py;
    return;
  }
  std::array<int, MW * MH> dist;
  dist.fill(-1);
  std::array<int, MW * MH> q{};
  int head = 0, tail = 0, target = int(ty / T) * MW + int(tx / T);
  if (target < 0 || target >= MW * MH)
    return;
  dist[target] = 0;
  q[tail++] = target;
  while (head < tail) {
    int a = q[head++], x = a % MW, y = a / MW;
    for (auto [xx, yy] : std::array<std::pair<int, int>, 4>{
             {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}}) {
      int nx = x + xx, ny = y + yy;
      if (nx >= 0 && nx < MW && ny >= 0 && ny < MH) {
        int b = ny * MW + nx;
        if (g.map[b] == 1 && dist[b] < 0) {
          dist[b] = dist[a] + 1;
          q[tail++] = b;
        }
      }
    }
  }
  int x = int(g.px / T), y = int(g.py / T), at = y * MW + x, best = dist[at],
      next = at;
  for (auto [xx, yy] :
       std::array<std::pair<int, int>, 4>{{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}}) {
    int nx = x + xx, ny = y + yy;
    if (nx >= 0 && nx < MW && ny >= 0 && ny < MH) {
      int b = ny * MW + nx;
      if (dist[b] >= 0 && (best < 0 || dist[b] < best)) {
        next = b;
        best = dist[b];
      }
    }
  }
  dx = (next % MW + .5f) * T - g.px;
  dy = (next / MW + .5f) * T - g.py;
}
void equipment() {
  for (int i = 0; i < int(g.bag.size()); i++)
    if (g.bag[i].value > equippedValue(g.bag[i].slot)) {
      g.overlay = 2;
      g.selected = i;
      touch(0, 0, 265, 290);
      g.overlay = 0;
    }
}
void inputAgent(int frame) {
  Enemy *t = nullptr;
  float best = 1e9;
  for (auto &e : g.enemies)
    if (e.alive && (!e.boss() || g.gate)) {
      float d = len(e.x - g.px, e.y - g.py);
      if (d < best) {
        best = d;
        t = &e;
      }
    }
  if (!t)
    return;
  float dx = 0, dy = 0;
  bool los = sight(g.px, g.py, t->x, t->y);
  float desired = g.weapon == BOW   ? (t->boss() ? 165 : 135)
                  : g.weapon == AXE ? 54
                                    : 44;
  if (best > desired + 8 || !los)
    route(t->x, t->y, dx, dy);
  else if (best < desired - 12) {
    dx = g.px - t->x;
    dy = g.py - t->y;
  }
  bool danger = false;
  for (auto &h : g.hazards) {
    float d = len(g.px - h.x, g.py - h.y);
    if (h.type == 0 && h.wind > 0 && h.wind < .75f && d < h.r + 28) {
      dx = g.px - h.x;
      dy = g.py - h.y;
      if (len(dx, dy) < 4) {
        dx = 1;
        dy = 1;
      }
      danger = true;
    }
    if (h.type == 1 && h.wind <= 0 && std::abs(d - h.radius) < 38 &&
        std::abs(angleDelta(std::atan2(g.py - h.y, g.px - h.x), h.angle)) >
            .53f) {
      dx = g.px - h.x;
      dy = g.py - h.y;
      danger = true;
    }
  }
  for (auto &e : g.enemies)
    if (e.alive && e.wind > 0) {
      float d = len(g.px - e.x, g.py - e.y);
      if (e.boss()) {
        if (e.pattern == 1 && e.wind < .45f) {
          float a = std::atan2(e.ty - e.y, e.tx - e.x);
          float along = (g.px - e.x) * std::cos(a) + (g.py - e.y) * std::sin(a),
                cross =
                    -(g.px - e.x) * std::sin(a) + (g.py - e.y) * std::cos(a);
          if (along > 0 && along < 300 && std::abs(cross) < 50) {
            dx = -std::sin(a) * (cross < 0 ? -1 : 1);
            dy = std::cos(a) * (cross < 0 ? -1 : 1);
            danger = true;
          }
        }
        if (e.pattern == 0 && e.wind < .35f && d < 125) {
          dx = g.px - e.x;
          dy = g.py - e.y;
          danger = true;
        }
        if (e.pattern == 4 && e.wind < .65f &&
            len(g.px - e.tx, g.py - e.ty) < 100) {
          dx = g.px - e.tx;
          dy = g.py - e.ty;
          if (len(dx, dy) < 3) {
            dx = 1;
            dy = 1;
          }
          danger = true;
        }
      } else if (e.wind < .25f && e.kind != 1 && e.kind != 4 &&
                 d < (e.kind == 3 ? 70 : 43)) {
        dx = g.px - e.x;
        dy = g.py - e.y;
        danger = true;
      }
    }
  if (g.weapon == BOW && best < 230 && los && !danger) {
    float side = frame / 240 % 2 ? 1 : -1;
    dx += (g.py - t->y) * .12f * side;
    dy -= (g.px - t->x) * .12f * side;
  }
  float d = len(dx, dy);
  if (d > 1) {
    dx /= d;
    dy /= d;
  }
  if (!fits(g.px + dx * 20, g.py + dy * 20)) {
    float ax = -dy, ay = dx;
    if (fits(g.px + ax * 22, g.py + ay * 22)) {
      dx = ax;
      dy = ay;
    } else if (fits(g.px - ax * 22, g.py - ay * 22)) {
      dx = -ax;
      dy = -ay;
    }
  }
  g.mx = dx;
  g.my = dy;
  g.attacking = true;
  if (danger && g.dodgeCd <= 0 && g.dodgeCharges > 0)
    skill(1);
  if (best < (g.weapon == BOW ? 260 : 105))
    skill(0);
  if (best < 170 && g.wardCd <= 0)
    skill(2);
  if (g.hp < maxhp() * .48f)
    skill(3);
  // Pick up nearby rewards and open legitimately unlocked chests between
  // fights.
  if (!danger && best > 140) {
    float ld = 90;
    for (auto &drop : g.drops) {
      float dd = len(drop.x - g.px, drop.y - g.py);
      if (dd < ld && bodySight(drop.x, drop.y)) {
        ld = dd;
        g.mx = (drop.x - g.px) / std::max(1.f, dd);
        g.my = (drop.y - g.py) / std::max(1.f, dd);
      }
    }
    for (auto &p : g.props)
      if (p.kind == 6 && !p.open && g.rooms[p.variant]) {
        float dd = len(p.x - g.px, p.y - g.py);
        if (dd < 140 && bodySight(p.x, p.y)) {
          g.mx = (p.x - g.px) / std::max(1.f, dd);
          g.my = (p.y - g.py) / std::max(1.f, dd);
        }
      }
  }
  if (frame % 60 == 0)
    equipment();
}
int main() {
  int won = 0, total = 0;
  for (int world = 0; world < 2; world++)
    for (int seed = 0; seed < 3; seed++)
      for (int weapon = 0; weapon < 3; weapon++) {
        boot("");
        g.world = world;
        g.run = seed;
        switchWeapon(weapon);
        expedition();
        int n = 0;
        for (; n < 60 * 420 && g.scene == PLAY && !g.bossKilled; n++) {
          inputAgent(n);
          tick(1.f / 60);
        }
        std::cout << "World " << world + 1 << ", seed " << seed << ", "
                  << weaponName(weapon) << ": "
                  << (g.bossKilled      ? "VICTORY"
                      : g.scene == DEAD ? "DEFEAT"
                                        : "TIMEOUT")
                  << " / " << n / 60 << "s / kills " << g.kills << " / HP "
                  << int(g.hp) << " / level " << g.level << "\n"
                  << std::flush;
        if (g.bossKilled)
          won++;
        total++;
      }
  std::cout << "Input-only agent victories: " << won << "/" << total
            << " (not a human difficulty measurement).\n";
  // Isolated guardian fixtures compare a non-dodging stationary attacker
  // against each boss.
  for (int w = 0; w < 2; w++) {
    boot("");
    g.world = w;
    expedition();
    g.enemies.erase(g.enemies.begin(), g.enemies.end() - 1);
    g.px = g.enemies[0].x;
    g.py = g.enemies[0].y + 65;
    std::fill(g.rooms, g.rooms + 3, true);
    g.gate = true;
    g.attacking = true;
    int i = 0;
    for (; i < 60 * 90 && g.scene == PLAY && !g.bossKilled; i++)
      tick(1.f / 60);
    std::cout << "Stationary guardian fixture " << w + 1 << ": "
              << (g.scene == DEAD ? "DEFEAT" : "SURVIVED") << " after "
              << i / 60 << "s\n";
  }
  return won == total ? 0 : 1;
}
