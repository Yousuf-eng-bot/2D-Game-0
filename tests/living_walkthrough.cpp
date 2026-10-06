// Historical coordinate adapter; new UI is exercised separately in ui7_tests.
#define DW_LEGACY_TEST_UI
// Ordinary-input survival walkthrough. No teleport, HP, item or damage grants.
#include "../native/engine.cpp"
#include <iostream>
using namespace av;
void advance(float t) {
  for (int i = 0; i < int(t * 60); i++)
    tick(1.f / 60);
}
void tap(int x, int y) {
  touch(0, 1, x, y);
  touch(1, 1, x, y);
}
#define REQUIRE(x)                                                             \
  do {                                                                         \
    if (!(x)) {                                                                \
      std::cerr << "FAIL " << __LINE__ << ": " << #x << '\n';                  \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main() {
  auto root = std::filesystem::temp_directory_path() /
              ("dw4-walkthrough-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  boot(root.string());
  advance(4);
  tap(320, 235);
  advance(.6);
  tap(320, 82);
  advance(.6);
  submitText(1, "SURVIVAL WALKTHROUGH");
  submitText(2, "20261004");
  tap(320, 318);
  advance(.8);
  REQUIRE(g.openWorld && g.scene == PLAY && o.generator == 5);
  // Legacy straight-road input regression: freeze its original geography.
  // V5 route-following is independently tested in medium_tests.cpp.
  o.generator=4; placePlayer(0,0); flushOpenWorld();
  auto id = o.id;
  tap(40, 133);
  REQUIRE(g.overlay == 7);
  tap(375, 267);
  REQUIRE(v.berries == 2 && v.fiber == 1);
  tap(580, 39);
  double start = globalX();
  g.mx = 1;
  g.my = 0;
  advance(65);
  g.mx = 0;
  advance(.2);
  double outward = globalX() - start;
  std::cerr<<"Outward x="<<globalX()<<" y="<<globalY()<<" hp="<<g.hp<<" scene="<<g.scene<<" map="<<int(g.map[int(g.py/T)*MW+int(g.px/T)])<<" height="<<int(j.heights[int(g.py/T)*MW+int(g.px/T)])<<" next="<<int(j.heights[int(g.py/T)*MW+int(g.px/T)+1])<<"\n";
  REQUIRE(outward > 350 && g.scene == PLAY);
  g.mx = -1;
  advance(65);
  g.mx = 0;
  advance(.2);
  std::cerr << "Return diagnostics: x=" << globalX() << " y=" << globalY()
            << " hp=" << g.hp << " scene=" << g.scene << " food=" << v.food
            << " water=" << v.water << "\n";
  REQUIRE(std::abs(globalX() - start) < 2 && g.scene == PLAY);
  int meals = v.meals, water = v.cleanWater;
  tap(40, 133);
  tap(375, 152);
  REQUIRE(v.meals == meals - 1 && v.food > 94);
  tap(515, 152);
  REQUIRE(v.cleanWater == water - 1 && v.water > 94);
  tap(515, 228);
  REQUIRE(v.cleanWater == 8);
  tap(515, 267);
  REQUIRE(v.task == 3 && g.overlay == 0);
  advance(8.3);
  REQUIRE(v.task == 0 && v.meals == meals - 2 && v.cleanWater == 7 &&
          g.scene == PLAY);
  tap(504, 23);
  REQUIRE(g.overlay == 1);
  tap(320, 250);
  advance(.6);
  REQUIRE(g.scene == WORLDS && !g.openWorld);
  tap(270, 140);
  advance(.6);
  REQUIRE(g.scene == PLAY && g.openWorld && o.id == id && o.generator == 4 &&
          v.berries == 2 && v.cleanWater == 7);
  std::cout << "PASS ordinary-input home/create > forage > " << int(outward)
            << " tiles outward + return > eat/drink > well refill > supplied "
               "camp rest > save/exit/resume. Alive, supplies persisted, no "
               "teleport or health/item grants.\n";
  std::filesystem::remove_all(root);
}
