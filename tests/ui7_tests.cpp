// Tests actual new UI hit regions and semantic action bindings. No legacy
// adapter.
#include "../native/engine.cpp"
#include <iostream>
#include <stdexcept>
using namespace av;
int checks = 0;
#define CHECK(c)                                                               \
  do {                                                                         \
    ++checks;                                                                  \
    if (!(c)) {                                                                \
      std::cerr << "FAIL " << __LINE__ << ": " << #c << " scene=" << g.scene   \
                << " overlay=" << g.overlay << "\n";                           \
      return 1;                                                                \
    }                                                                          \
  } while (0)
std::vector<C> screen(W *H);
void draw() { frame(screen.data(), 0); }
void advance(float seconds) {
  for (int i = 0; i < int(seconds * 60); i++)
    tick(1.f / 60);
  draw();
}
UIHit hit(int action, int arg = -999) {
  draw();
  for (auto h : u.hits)
    if (h.action == action && (arg == -999 || h.arg == arg))
      return h;
  throw std::runtime_error("missing UI action " + num(action) + " / " +
                           num(arg));
}
void press(UIHit b, int finger = 1) {
  touch(0, finger, b.x + b.w / 2, b.y + b.h / 2);
}
void click(int action, int arg = -999) {
  auto b = hit(action, arg);
  press(b);
  touch(1, 1, b.x + b.w / 2, b.y + b.h / 2);
  draw();
}
int main() {
  auto root = std::filesystem::temp_directory_path() /
              ("clear-ui-tests-" + hexId(entropy()));
  boot(root.string());
  CHECK(ui7Active() && u.language == 0 && u.layout == 0);
  CHECK(!uiFontBytes.empty());
  for (auto &e : uiFontEntries) {
    CHECK(e.w > 0 && e.h > 0 && e.offset >= 0 &&
          e.offset + e.length <= int(uiFontBytes.size()));
    int n = 0;
    for (int p = e.offset; p < e.offset + e.length; p += 2)
      n += uiFontBytes[p];
    CHECK(n == e.w * e.h);
  }
  for (auto &t : uiTranslations) {
    CHECK(uFont(t.second, 14) != nullptr);
    CHECK(uWidth(t.first, 14) > 0);
  }
  CHECK(uTranslate("Crafting") != "Crafting");
  advance(4);
  CHECK(g.scene == HOME);
  click(640);
  advance(.6);
  CHECK(g.scene == WORLDS);
  click(641);
  advance(.6);
  CHECK(g.scene == CREATE);
  click(646, 1);
  CHECK(av_text_request() == 1);
  submitText(1, "NEW UI INPUT TEST");
  submitText(2, "20261004");
  click(651);
  advance(1);
  CHECK(g.scene == PLAY && g.openWorld);
  auto world = o.id;
  auto seed = o.seed;
  u.tips = false;
  g.toastTime = 0;
  draw();
  CHECK(u.hits.size() == 8);
  int used = 0;
  for (int y = 0; y < H; y++)
    for (int x = 0; x < W; x++) {
      bool on = false;
      for (auto h : u.hits) {
        if (h.r > 0)
          on |= len(x - h.x - h.r, y - h.y - h.r) <= h.r;
        else
          on |= x >= h.x && x < h.x + h.w && y >= h.y && y < h.y + h.h;
      }
      used += on;
    }
  CHECK(used < W * H * .18f);
  std::cout << "PASS default Bangla, shaped fonts, UI home/create flow, simple "
               "HUD hit coverage="
            << 100.f * used / (W * H) << "%\n";
  click(106);
  int mapPixels = 0;
  for (int yy = 92; yy < 312; yy++)
    for (int xx = 40; xx < 355; xx++)
      mapPixels += screen[yy * W + xx] != 0xff060d12;
  CHECK(mapPixels > 800);
  CHECK(screen[100 * W + 40] == 0xff060d12);
  auto mapBefore = screen;
  draw();
  for (int yy = 92; yy < 312; yy++)
    for (int xx = 40; xx < 355; xx++)
      CHECK(screen[yy * W + xx] == mapBefore[yy * W + xx]);
  click(531, 1);
  CHECK(w.mapZoom == 1);
  click(531, -1);
  CHECK(w.mapZoom == 2);
  click(900);
  // All preset sizes fit and have distinct non-overlapping controls.
  for (int layout = 0; layout < 4; layout++)
    for (int size = 0; size < 3; size++) {
      u.layout = layout;
      u.size = size;
      u.offsets = {};
      u.custom = false;
      CHECK(uValidLayout());
      draw();
      for (auto h : u.hits) {
        CHECK(h.x >= 0 && h.y >= 0 && h.x + h.w <= W && h.y + h.h <= H);
      }
      auto a = uControls();
      CHECK(layout == 2 ? a[7].x > 320 : a[7].x < 320);
    }
  u.layout = 0;
  u.size = 1;
  draw();
  auto joy = hit(8);
  press(joy, 10);
  touch(2, 10, joy.x + joy.w / 2 + 30, joy.y + joy.h / 2);
  CHECK(g.mx > .8f);
  auto attack = hit(1);
  press(attack, 11);
  CHECK(g.attacking && g.joystick == 10);
  touch(1, 11, 0, 0);
  CHECK(!g.attacking && g.joystick == 10 && g.mx > .8f);
  touch(1, 10, 0, 0);
  CHECK(g.joystick == -1 && g.mx == 0);
  press(hit(8), 10);
  touch(2, 10, 100, 280);
  press(hit(1), 11);
  touch(3, 0, 0, 0);
  CHECK(!g.attacking && g.joystick == -1 && j.guardFinger == -1 &&
        u.mineFinger == -1);
  click(4);
  CHECK(g.overlay == 11 && !g.attacking);
  click(606);
  CHECK(g.overlay == 0);
  u.layout = 1;
  advance(.5);
  auto guard = hit(6);
  press(guard, 7);
  CHECK(j.guardFinger == 7);
  suspend();
  CHECK(j.guardFinger == -1 && g.overlay == 1 && !g.attacking);
  click(901);
  CHECK(g.overlay == 0);
  // Pause genuinely freezes simulation and stations while settings are open.
  click(101);
  double seconds = o.seconds;
  float food = v.food;
  advance(2);
  CHECK(o.seconds == seconds && v.food == food);
  click(112, 0);
  CHECK(g.overlay == 12 && u.settingsTab == 0);
  click(400, 2);
  CHECK(u.layout == 2);
  click(220, 2);
  click(403, 1);
  CHECK(u.language == 1 && std::string(av_text_value(100)) == "en");
  click(220, 1);
  click(402, 0);
  click(402, 1);
  click(402, 2);
  CHECK(u.textSize == 1);
  click(230, 1);
  bool labels = u.labels;
  click(402, 3);
  CHECK(u.labels != labels);
  click(230, 1);
  bool low = g.lowPower;
  click(402, 6);
  CHECK(g.lowPower != low);
  bool blur = motionBlur;
  click(402, 7);
  CHECK(motionBlur != blur);
  click(220, 0);
  click(400, 0);
  u.size = 1;
  u.labels = true;
  uSave();
  click(401);
  CHECK(g.overlay == 13);
  auto drag = hit(801);
  press(drag, 3);
  touch(2, 3, drag.x + drag.w / 2 - 12, drag.y + drag.h / 2);
  touch(1, 3, 0, 0);
  CHECK(u.draft[0].first == -12);
  click(406);
  CHECK(g.overlay == 12 && u.custom && u.offsets[0].first == -12);
  std::string prefs;
  CHECK(readChecked((root / "ui7.cfg").string(), prefs));
  auto oldX = globalX(), oldY = globalY();
  ui7Boot();
  CHECK(u.language == 1 && u.custom && u.offsets[0].first == -12 &&
        u.textSize == 1);
  CHECK(o.id == world && o.seed == seed && globalX() == oldX &&
        globalY() == oldY);
  // Cancel leaves the saved coordinates unchanged; an overlapping draft cannot
  // commit.
  uOpen(12);
  u.settingsTab = 0;
  click(401);
  auto a = uControls(true);
  press(hit(801), 4);
  touch(2, 4, a[1].x, a[1].y);
  touch(1, 4, 0, 0);
  click(406);
  CHECK(g.overlay == 13 && u.offsets[0].first == -12);
  click(407);
  CHECK(g.overlay == 12 && u.offsets[0].first == -12);
  CHECK(readChecked((root / "ui7.cfg").string(), prefs));
  std::string bad = "DWUI1 0 99 9 9 0 1 1 0\n";
  for (int i = 0; i < 8; i++)
    bad += "0 0\n";
  CHECK(atomicWorld((root / "ui7.cfg").string(), bad));
  ui7Boot();
  CHECK(u.layout == 0 && u.size == 1 && !u.custom);
  CHECK(o.id == world);
  u.language = 0;
  std::cout << "PASS 12 safe preset/size combinations, multitouch "
               "ownership/cancel, menu pause, language/preferences persistence "
               "and drag/apply/cancel validation\n";
  // Menus use the same geometry for painting and hit testing.
  g.overlay = 0;
  u.history.clear();
  u.size = 1;
  u.layout = 0;
  u.custom = false;
  u.offsets = {};
  for (int lang = 0; lang < 2; lang++)
    for (int large = 0; large < 2; large++)
      for (int overlay : {1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12}) {
        u.language = lang;
        u.textSize = large;
        g.overlay = overlay;
        u.tab = u.page = u.settingsTab = 0;
        draw();
        for (auto h : u.hits) {
          CHECK(h.x >= 0 && h.y >= 0 && h.x + h.w <= W && h.y + h.h <= H);
          CHECK(h.w >= 44 && h.h >= 44);
        }
        for (size_t i = 0; i < u.hits.size(); i++)
          for (size_t k = 0; k < i; k++) {
            auto a = u.hits[i], b = u.hits[k];
            CHECK(!(a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h &&
                    a.y + a.h > b.y));
          }
      }
  // Resource grants below are isolated UI transaction fixtures, not the input
  // walkthrough.
  g.overlay = 0;
  u.history.clear();
  u.language = 0;
  u.textSize = 0;
  uOpen(10);
  w.wood = 5;
  int wood = w.wood;
  click(300, 0);
  CHECK(w.wood == wood - 1 && j.stock[PLANK] == 4);
  click(300, 2);
  CHECK(j.stock[BENCH] == 1);
  click(210, 1);
  click(301, BENCH);
  CHECK(j.selectedBuild == BENCH);
  click(210, 2);
  CHECK(u.tab == 2);
  click(230, 1);
  CHECK(u.page == 1);
  click(210, 3);
  CHECK(u.tab == 3);
  click(210, 4);
  CHECK(u.tab == 4);
  CHECK(hit(210, 4).w >= 44);
  click(210, 0);
  CHECK(u.tab == 0);
  click(900);
  CHECK(g.overlay == 0);
  click(107);
  CHECK(g.overlay == 7);
  v.food = 40;
  int meals = v.meals;
  click(500, 0);
  CHECK(v.meals == meals - 1 && v.food > 40);
  v.water = 40;
  int water = v.cleanWater;
  click(500, 1);
  CHECK(v.cleanWater == water - 1 && v.water > 40);
  click(210, 1);
  click(501, 4);
  CHECK(v.selectedPart == 4);
  click(900);
  click(4);
  click(105);
  click(520, AXE);
  CHECK(g.weapon == AXE && g.overlay == 0);
  click(4);
  click(610);
  CHECK(j.stance == 1 && g.overlay == 0);
  advance(.3);
  click(4);
  click(610);
  CHECK(j.stance == 2);
  advance(.3);
  CHECK(uActionLabel(1) == "Stand");
  click(1);
  CHECK(j.stance == 0 && !g.attacking);
  // A contextual tree press selects the axe and restores hunting even on
  // cancel.
  Prop tree{};
  bool found = false;
  for (auto &p : g.props)
    if (p.kind == 0 && !treeCut(p) && fits(p.x - 43, p.y, 7)) {
      g.px = p.x - 43;
      g.py = p.y;
      if (uContext() == 3) {
        tree = p;
        found = true;
        break;
      }
    }
  CHECK(found);
  g.enemies.clear();
  g.animals.clear();
  g.px = tree.x - 43;
  g.py = tree.y;
  g.weapon = SWORD;
  v.hunting = true;
  g.fx = 1;
  g.fy = 0;
  CHECK(uContext() == 3);
  press(hit(3), 22);
  CHECK(g.weapon == AXE && u.chopFinger == 22 && !v.hunting && g.attacking);
  touch(3, 0, 0, 0);
  CHECK(v.hunting && !g.attacking && u.chopFinger == -1);
  // Save/exit/resume through new menu; no change to the world format.
  g.overlay = 0;
  g.hp = maxhp();
  click(101);
  click(631);
  advance(.6);
  CHECK(g.scene == WORLDS && !g.openWorld);
  click(643, 0);
  advance(1);
  CHECK(g.openWorld && o.id == world && o.seed == seed && g.weapon == AXE);
  click(101);
  click(631);
  advance(.6);
  click(644, 0);
  advance(.6);
  CHECK(g.scene == DELETE_WORLD && o.worlds.size() == 1);
  click(650);
  advance(.6);
  CHECK(o.worlds.size() == 1);
  back();
  advance(.6);
  CHECK(g.scene == HOME);
  click(642);
  CHECK(g.scene == HUB && !g.openWorld);
  click(4);
  CHECK(g.overlay == 11);
  for (auto h : u.hits)
    CHECK(h.action != 610 && h.action != 613 && h.action != 614);
  click(900);
  click(700);
  click(701, 0);
  CHECK(g.scene == PLAY && !g.openWorld && g.overlay == 0);
  std::cout << "PASS bilingual menu bounds, craft/body/weapon/stance bindings, "
               "contextual axe/hunt restore, save/resume, deletion "
               "confirmation and Classic routes\n";
  auto fixtures = std::filesystem::path(__FILE__).parent_path() / "fixtures";
  std::string legacy;
  CHECK(readChecked((fixtures / "legacy-world-v06.sav").string(), legacy));
  u.layout = 2;
  u.language = 0;
  CHECK(decodeFrontier(legacy));
  CHECK(u.layout == 2 && u.language == 0 && o.generator == 4 && g.level == 4 &&
        g.hp == 125 && g.gold == 77 && v.food == 63 && v.water == 47 &&
        w.wood == 31 && o.seconds == 501 && w.clockOffset == 17);
  CHECK(j.home && j.homeX == 53 && j.homeY == -30 && j.built.size() == 4 &&
        j.stock[STONE] == 67 && j.stock[ORE] == 3 && j.stock[IRON] == 2 &&
        j.toolTier[0] == 2 && j.durability[0] == 113 && j.drops.size() == 1);
  int64_t xx, yy;
  int mm, gg, bb, hh, rows = 0;
  std::ifstream fixture(fixtures / "legacy-tiles-v06.txt");
  while (fixture >> xx >> yy >> mm >> gg >> bb >> hh) {
    auto t = tileAt(xx, yy);
    CHECK(t.map == mm && t.ground == gg && t.biome == bb &&
          landHeight(xx, yy) == hh);
    rows++;
  }
  CHECK(rows == 3875);
  auto saved = encodeFrontier();
  CHECK(decodeFrontier(saved) && j.built.size() == 4 && j.stock[STONE] == 67);
  std::cout << "PASS authentic 0.6 save + " << rows
            << " terrain/height records unchanged, "
               "inventory/tools/base/crops/checkpoint preserved, preferences "
               "independent\n";
  std::filesystem::remove_all(root);
  std::cout << "ALL CLEAR UI CHECKS PASSED: " << checks << " assertions\n";
}
