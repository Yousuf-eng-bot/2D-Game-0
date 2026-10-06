// Rendering invariants, not a phone usability/performance certification.
#include "../native/engine.cpp"
#include <iostream>
using namespace av;
int checks = 0;
#define CHECK(x)                                                               \
  do {                                                                         \
    checks++;                                                                  \
    if (!(x)) {                                                                \
      std::cerr << "FAIL line " << __LINE__ << ": " << #x << "\n";             \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main() {
  auto root = std::filesystem::temp_directory_path() /
              ("vista-test-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  boot(root.string());
  o.draftName = "VISTA TEST";
  o.draftSeed = 20261004;
  createWorld();
  g.toastTime = 0;
  u.tips = false;
  g.shake = false;
  motionBlur = false;
  std::vector<C> pixels(W * H);
  pix = pixels.data();
  frame(pixels.data(), 0);
  auto state = encodeFrontier();
  auto terrain = j.heights;
  auto map = g.map;
  auto randomSeed = av::rng;
  for (int q = 0; q < 3; q++) {
    vistaQuality = q;
    frame(pixels.data(), 0);
    CHECK(encodeFrontier() == state);
    CHECK(j.heights == terrain);
    CHECK(map == g.map);
    CHECK(av::rng == randomSeed);
  }
  CHECK(vistaDecor.size() > 0);
  CHECK(vistaDecor.size() < 1000);
  vistaQuality = 1;
  drawVistaTerrain();
  auto a = pixels;
  drawVistaTerrain();
  CHECK(a == pixels);
  CHECK(vistaElevation(g.px, g.py) == heightLocal(g.px, g.py) * 6);
  CHECK(syAt(g.px, g.py) == sy(g.py) - vistaElevation(g.px, g.py));
  // Different material palettes and species/variants genuinely render
  // differently.
  CHECK(vistaGround(0, 0, 84, 36, 8, 8) != vistaGround(1, 0, 84, 36, 8, 8));
  CHECK(treeSprite(0, 0, 0).pixels != treeSprite(0, 0, 1).pixels);
  CHECK(treeSprite(0, 0, 0).pixels != treeSprite(1, 0, 0).pixels);
  for (int b = 0; b < 5; b++)
    for (int s = 0; s < 9; s++)
      for (int v = 0; v < 8; v++) {
        auto &t = treeSprite(s, b, v);
        CHECK(t.pixels.size() == 128 * 144);
        CHECK(std::count_if(t.pixels.begin(), t.pixels.end(),
                            [](C c) { return c != 0; }) > 100);
      }
  // Dynamic direction and soft alpha ramp. No world geography edits.
  o.seconds = 60;
  w.clockOffset = 0;
  drawVistaTerrain();
  drawVistaShadows();
  auto morning = vistaShadow;
  o.seconds = 540;
  drawVistaTerrain();
  drawVistaShadows();
  CHECK(morning != vistaShadow);
  int feather = std::count_if(vistaShadow.begin(), vistaShadow.end(),
                              [](uint8_t a) { return a > 0 && a < 24; });
  CHECK(feather > 10);
  // Reflection writes only to the water stencil, never to solid terrain/UI.
  auto propsBefore = g.props;
  g.props.clear();
  Prop prop{};
  prop.kind = 0;
  prop.biome = 0;
  prop.x = g.px;
  prop.y = g.py;
  g.props.push_back(prop);
  std::fill(pixels.begin(), pixels.end(), 0xff245663);
  vistaWater.fill(1);
  auto reflectionBefore = pixels;
  vistaReflections();
  CHECK(pixels != reflectionBefore);
  vistaWater.fill(0);
  reflectionBefore = pixels;
  vistaReflections();
  CHECK(pixels == reflectionBefore);
  g.props = propsBefore;
  // Focus is restricted to scenery pass; safe centre stays byte-identical.
  drawVistaTerrain();
  a = pixels;
  vistaFocus = false;
  vistaGroundFocus();
  CHECK(a == pixels);
  vistaFocus = true;
  vistaGroundFocus();
  int hx = sx(g.px), hy = syAt(g.px, g.py);
  for (int y = std::max(0, hy - 70); y < std::min(H, hy + 70); y++)
    for (int x = std::max(0, hx - 70); x < std::min(W, hx + 70); x++)
      CHECK(pixels[y * W + x] == a[y * W + x]);
  vistaQuality = 0;
  drawVistaTerrain();
  a = pixels;
  vistaGroundFocus();
  CHECK(a == pixels);
  vistaQuality = 2;
  g.lowPower = true;
  CHECK(vistaTier() == 0);
  g.lowPower = false;
  CHECK(vistaTier() == 2);
  // Light occlusion uses the existing solid/height grid.
  auto savedMap = g.map[20 * MW + 20];
  auto savedH = j.heights[20 * MW + 20];
  g.map[20 * MW + 20] = 5;
  CHECK(vistaLightBlocked(19 * T + 12, 20 * T + 12, 21 * T + 12, 20 * T + 12));
  g.map[20 * MW + 20] = savedMap;
  j.heights[20 * MW + 20] = savedH;
  // Visual prefs never change world or UI layout preference formats.
  vistaQuality = 2;
  vistaFocus = false;
  state = encodeFrontier();
  CHECK(vistaSave());
  vistaQuality = 0;
  vistaFocus = true;
  vistaLoad();
  CHECK(vistaQuality == 2 && !vistaFocus);
  CHECK(encodeFrontier() == state);
  atomicWorld((root / "visual8.cfg").string(), "BAD 9 -1\n");
  atomicWorld((root / "visual8.cfg.bak").string(), "BAD 9 -1\n");
  vistaLoad();
  CHECK(vistaQuality == 1 && vistaFocus);
  // Appearance menu actions and persistence, using actual registry coordinates.
  uOpen(12);
  u.settingsTab = 1;
  u.page = 2;
  frame(pixels.data(), 0);
  auto click = [&](int arg) {
    for (auto h : u.hits)
      if (h.action == 402 && h.arg == arg) {
        touch(0, 5, h.x + h.w / 2, h.y + h.h / 2);
        touch(1, 5, h.x + h.w / 2, h.y + h.h / 2);
        return true;
      }
    return false;
  };
  CHECK(click(8));
  CHECK(u.settingsTab == 3 && vistaQuality == 1);
  u.settingsTab = 1;
  u.page = 3;
  frame(pixels.data(), 0);
  CHECK(click(9));
  CHECK(!vistaFocus);
  ui7Boot();
  CHECK(vistaQuality == 1 && !vistaFocus);
  // Extreme valid coordinates, five biomes, three quality levels, all day
  // phases.
  g.overlay = 0;
  for (int b = 0; b < 5; b++) {
    findBiome(b);
    placePlayer(o.waypointX, o.waypointY);
    o.waypoint = false;
    for (int q = 0; q < 3; q++) {
      vistaQuality = q;
      for (int hour : {0, 6, 12, 18}) {
        w.clockOffset = hour * 60;
        o.seconds = 0;
        frame(pixels.data(), 0);
        CHECK(pixels[W * H / 2] >> 24 == 255);
      }
    }
  }
  placePlayer(-999999900, 999999900);
  frame(pixels.data(), 0);
  CHECK(g.openWorld);
  std::cout << "PASS Vista: materials, 360 tree sprites, deterministic "
               "non-mutating rendering, elevation projection, soft dynamic "
               "shadows, scenery-only focus, light occlusion, quality/prefs "
               "and 60 biome-quality-clock combinations. "
            << checks << " assertions\n";
  std::filesystem::remove_all(root);
}
