// Historical coordinate adapter; new UI is exercised separately in ui7_tests.
#define DW_LEGACY_TEST_UI
// Deterministic perfect-map controller using player input only after UI
// creation. No teleports, inventory/HP/stamina grants, terrain edits or cleared
// actors.
#include "../native/engine.cpp"
#include <iostream>
using namespace av;
void advance(float seconds) {
  for (int k = 0; k < int(seconds * 60); k++)
    tick(1.f / 60);
}
void tap(int x, int y) {
  touch(0, 1, x, y);
  touch(1, 1, x, y);
}
#define REQUIRE(x)                                                             \
  do {                                                                         \
    if (!(x)) {                                                                \
      std::cerr << "FAIL " << __LINE__ << ": " << #x << " pos=" << globalX()   \
                << "," << globalY() << " hp=" << g.hp << " toast=" << g.toast  \
                << '\n';                                                       \
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
              ("journey-walk-" + hexId(entropy()));
  boot(root.string());
  advance(4);
  tap(320, 235);
  advance(.6);
  tap(320, 82);
  advance(.6);
  submitText(1, "FIRST DAY INPUT TEST");
  submitText(2, "20261004");
  tap(320, 318);
  advance(.8);
  REQUIRE(g.scene == PLAY && g.openWorld && w.wood == 0);
  auto id = o.id;
  tap(140, 133);
  REQUIRE(!v.hunting);
  tap(200, 328);
  tap(300, 130);
  REQUIRE(g.weapon == AXE);
  int c = approach(true);
  REQUIRE(c >= 0 && walk(c));
  touch(0, 2, 587, 293);
  for (int k = 0; k < 900 && w.wood < 4 && g.scene == PLAY; k++)
    tick(1.f / 60);
  touch(1, 2, 587, 293);
  advance(.6);
  REQUIRE(w.felled >= 1 && w.wood >= 4 && g.scene == PLAY);
  std::cout << "PASS normal-input tree chopping and ground pickup; logs="
            << w.wood << " HP=" << g.hp << '\n';
  // Face a free adjacent non-camp cell, using the movement input.
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
  tap(480, 83);
  REQUIRE(g.overlay == 10);
  tap(130, 177);
  tap(130, 235);
  REQUIRE(j.stock[BENCH] == 1);
  tap(240, 299);
  REQUIRE(j.built.size() == 1 && j.stock[BENCH] == 0);
  tap(480, 83);
  tap(130, 177);
  tap(130, 177);
  tap(130, 206);
  tap(130, 264);
  REQUIRE(j.toolTier[0] == 1 && j.durability[0] == 120);
  tap(580, 37);
  std::cout << "PASS gathered logs -> planks/sticks -> placed workbench -> "
               "wood pick, no grants\n";
  c = approach(false);
  REQUIRE(c >= 0 && walk(c));
  for (int k = 0; k < 15 && j.stock[STONE] < 4 && g.scene == PLAY; k++) {
    tap(167, 240);
    advance(.6);
  }
  REQUIRE(j.stock[STONE] >= 4 && j.mined.size() >= 1 && j.durability[0] < 120);
  std::cout << "PASS reachable stone mining -> floating stone pickup, tool "
               "wear; stone="
            << j.stock[STONE] << " HP=" << g.hp << '\n';
  int stone = j.stock[STONE], dur = j.durability[0];
  tap(504, 23);
  tap(320, 250);
  advance(.6);
  REQUIRE(g.scene == WORLDS);
  tap(270, 140);
  advance(.6);
  REQUIRE(g.scene == PLAY && o.id == id && j.stock[STONE] == stone &&
          j.durability[0] == dur && j.built.size() == 1 && j.mined.size() >= 1);
  std::cout << "PASS save/exit/resume: station, inventory, tool wear and mined "
               "terrain persisted; alive without grants\n";
  std::filesystem::remove_all(root);
}
