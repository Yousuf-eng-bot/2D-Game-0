// New controls/menus, ordinary movement and touches; no resource/HP/position
// grants.
#include "../native/engine.cpp"
#include <iostream>
#include <stdexcept>
using namespace av;
std::vector<C> pixels(W *H);
void draw() { frame(pixels.data(), 0); }
void advance(float t) {
  for (int i = 0; i < int(t * 60); i++)
    tick(1.f / 60);
  draw();
}
UIHit find(int action, int arg = -999) {
  draw();
  for (auto b : u.hits)
    if (b.action == action && (arg == -999 || arg == b.arg))
      return b;
  throw std::runtime_error("missing control " + num(action));
}
void press(int action, int arg = -999, int id = 1) {
  auto b = find(action, arg);
  touch(0, id, b.x + b.w / 2, b.y + b.h / 2);
}
void click(int action, int arg = -999) {
  press(action, arg);
  touch(1, 1, 0, 0);
  draw();
}
#define REQUIRE(c)                                                             \
  do {                                                                         \
    if (!(c)) {                                                                \
      std::cerr << "FAIL " << __LINE__ << ": " << #c << " x=" << globalX()     \
                << " y=" << globalY() << " toast=" << g.toast << "\n";         \
      return 1;                                                                \
    }                                                                          \
  } while (0)
bool walk(int cell) {
  navigation();
  if (cell < 0 || g.distance[cell] < 0)
    return false;
  std::vector<int> path{cell};
  while (g.distance[cell] > 0) {
    int next = -1;
    for (int n : {cell - 1, cell + 1, cell - MW, cell + MW})
      if (n >= 0 && n < MW * MH && g.distance[n] == g.distance[cell] - 1 &&
          terrainLink(cell, n)) {
        next = n;
        break;
      }
    if (next < 0)
      return false;
    cell = next;
    path.push_back(cell);
  }
  std::reverse(path.begin(), path.end());
  auto ox = o.originX, oy = o.originY;
  for (int p : path) {
    double gx = ox + p % MW + .5, gy = oy + p / MW + .5;
    int k = 0;
    for (; k < 300; k++) {
      float dx = float(gx - globalX()) * T, dy = float(gy - globalY()) * T,
            d = len(dx, dy);
      if (d < 2)
        break;
      g.mx = dx / std::max(10.f, d);
      g.my = dy / std::max(10.f, d);
      tick(1.f / 60);
      if (g.scene != PLAY)
        return false;
    }
    if (k == 300)
      return false;
  }
  g.mx = g.my = 0;
  advance(.15);
  return true;
}
int approach(bool tree) {
  navigation();
  int best = -1, cost = 99999;
  for (auto &p : g.props)
    if (p.kind == (tree ? 0 : 1)) {
      auto key = treeKey(p);
      if (!tree && coordinateHash(key.first, key.second, 7301) % 7 <= 2)
        continue;
      int tx = int(p.x / T), ty = int(p.y / T);
      for (int dy = -2; dy <= 2; dy++)
        for (int dx = -2; dx <= 2; dx++) {
          int x = tx + dx, y = ty + dy;
          if (x < 1 || y < 1 || x >= MW - 1 || y >= MH - 1)
            continue;
          int c = y * MW + x;
          float px = (x + .5f) * T, py = (y + .5f) * T,
                d = len(px - p.x, py - p.y);
          if (d < 22 || d > 47 || g.distance[c] < 0 || g.distance[c] >= cost ||
              !sight(px, py, p.x + (px - p.x) * 19 / d,
                     p.y + (py - p.y) * 19 / d))
            continue;
          bool safe = true;
          for (auto &e : g.enemies)
            if (e.alive && len(e.x - px, e.y - py) < 180)
              safe = false;
          if (safe) {
            best = c;
            cost = g.distance[c];
          }
        }
    }
  return best;
}
int main() {
  auto root = std::filesystem::temp_directory_path() /
              ("new-ui-walk-" + hexId(entropy()));
  boot(root.string());
  advance(4);
  click(640);
  advance(.6);
  click(641);
  advance(.6);
  submitText(1, "CLEAR PLAY WALK");
  submitText(2, "20261004");
  click(651);
  advance(1);
  REQUIRE(g.scene == PLAY && g.openWorld && w.wood == 0);
  auto id = o.id;
  click(4);
  click(105);
  if (v.hunting)
    click(521);
  click(520, AXE);
  REQUIRE(g.weapon == AXE && !v.hunting && g.overlay == 0);
  int c = approach(true);
  REQUIRE(c >= 0 && walk(c));
  press(1, -999, 2);
  for (int i = 0; i < 900 && w.wood < 4 && g.scene == PLAY; i++)
    tick(1.f / 60);
  touch(1, 2, 0, 0);
  advance(.6);
  REQUIRE(w.felled >= 1 && w.wood >= 4 && g.scene == PLAY);
  int direction = -1;
  const int dx[] = {1, -1, 0, 0}, dy[] = {0, 0, 1, -1};
  for (int d = 0; d < 4; d++) {
    int64_t x = int64_t(std::floor(globalX())) + dx[d],
            y = int64_t(std::floor(globalY())) + dy[d];
    auto t = tileAt(x, y);
    if (t.map == 1 && t.ground != 4 && (std::abs(x) >= 7 || std::abs(y) >= 7)) {
      direction = d;
      break;
    }
  }
  REQUIRE(direction >= 0);
  g.mx = dx[direction];
  g.my = dy[direction];
  advance(.05);
  g.mx = g.my = 0;
  advance(.15);
  click(4);
  click(110);
  click(300, 0);
  click(300, 2);
  REQUIRE(j.stock[BENCH] == 1);
  click(210, 1);
  click(301, BENCH);
  click(302);
  REQUIRE(j.built.size() == 1 && g.overlay == 0);
  click(4);
  click(110);
  click(300, 0);
  click(300, 0);
  click(300, 1);
  click(230, 1);
  click(300, 3);
  REQUIRE(j.toolTier[0] == 1 && j.durability[0] == 120);
  click(900);
  if (g.overlay)
    click(900);
  REQUIRE(g.overlay == 0);
  c = approach(false);
  REQUIRE(c >= 0 && walk(c));
  click(4);
  press(613, -999, 3);
  for (int k = 0; k < 900 && j.stock[STONE] < 4 && g.scene == PLAY; k++)
    tick(1.f / 60);
  touch(1, 3, 0, 0);
  advance(.1);
  REQUIRE(j.stock[STONE] >= 4 && j.durability[0] < 120 && g.scene == PLAY);
  int stone = j.stock[STONE], dur = j.durability[0];
  click(101);
  click(631);
  advance(.6);
  REQUIRE(g.scene == WORLDS);
  click(643, 0);
  advance(1);
  REQUIRE(g.scene == PLAY && o.id == id && j.stock[STONE] == stone &&
          j.durability[0] == dur && j.built.size() == 1);
  std::cout << "PASS new Bengali UI: home/create > weapon/hunt selection > "
               "tree/log pickup > recipes > place bench > wood pick > hold "
               "mine > stone pickup > save/exit/resume. Alive HP="
            << g.hp << ", no HP/items/teleport grants.\n";
  std::filesystem::remove_all(root);
}
