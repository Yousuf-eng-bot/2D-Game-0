// Historical coordinate adapter; new UI is exercised separately in ui7_tests.
#define DW_LEGACY_TEST_UI
#include "../native/engine.cpp"
#include <iostream>
#include <set>
using namespace av;
int checks = 0;
#define CHECK(x)                                                               \
  do {                                                                         \
    checks++;                                                                  \
    if (!(x)) {                                                                \
      std::cerr << "FAIL " << __LINE__ << ": " << #x << '\n';                  \
      return 1;                                                                \
    }                                                                          \
  } while (0)
uint64_t digest(const std::vector<C> &p) {
  uint64_t h = 0;
  for (C c : p)
    h = mix64(h ^ c);
  return h;
}
int main() {
  std::vector<C> pixels(W * H);
  pix = pixels.data();
  auto root = std::filesystem::temp_directory_path() /
              ("earth-test-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  boot(root.string());
  auto fixtures = std::filesystem::path(__FILE__).parent_path() / "fixtures";
  std::string old;
  CHECK(readChecked((fixtures / "legacy-world-v04.sav").string(), old));
  CHECK(old.rfind("DWFRONTIER 2 ", 0) == 0);
  CHECK(decodeFrontier(old));
  CHECK(o.generator == 2 && g.gold == 777 && g.hp == 91 && g.level == 4);
  CHECK(v.water == 43 && v.food == 34 && v.injury[4] == 64 && v.rawMeat == 7);
  CHECK(std::abs(worldHour() - float(std::fmod(8 + 318.5 * 24 / 1080, 24))) <
        .0001f);
  std::ifstream f(fixtures / "legacy-tiles-v04.txt");
  int64_t xx, yy;
  int m, ground, b, n = 0;
  while (f >> xx >> yy >> m >> ground >> b) {
    auto t = tileAt(xx, yy);
    CHECK(t.map == m && t.ground == ground && t.biome == b);
    n++;
  }
  CHECK(n == 9048);
  auto encoded = encodeFrontier();
  CHECK(encoded.rfind("DWFRONTIER 4 ", 0) == 0);
  CHECK(decodeFrontier(encoded));
  CHECK(o.generator == 2 && v.water == 43 && w.clockOffset > 106 &&
        w.clockOffset < 107);
  std::cout << "PASS v0.4 golden save: " << n
            << " exact terrain records, life/progress, pinned generator and "
               "preserved clock phase\n";
  o.id = "0000000000000004";
  CHECK(atomicWorld(worldFile(o.id), old));
  flushOpenWorld();
  std::string archive;
  CHECK(readChecked(worldFile(o.id) + ".v04", archive));
  CHECK(archive == old);
  o.draftSeed = 20261004;
  o.draftName = "WILD EARTH TEST";
  CHECK(createWorld());
  CHECK(o.generator == 5 && w.wood == 0 && w.clockOffset == 0);
  CHECK(exploredAt(0, 0));
  CHECK(!exploredAt(13, 0));
  CHECK(!exploredAt(9, 9));
  CHECK(!exploredAt(20000, 20000));
  std::set<int> all;
  for (int seed = 0; seed < 12; seed++) {
    o.seed = uint64_t(seed * 8377 + 51);
    std::set<int> biomes;
    for (int y = -100000; y <= 100000; y += 4096)
      for (int x = -100000; x <= 100000; x += 4096) {
        int b = biomeAt(x, y);
        CHECK(b >= 0 && b < 5);
        CHECK(b == biomeAt(x, y));
        biomes.insert(b);
      }
    CHECK(biomes.size() == 5);
    for (int y = -6000; y <= 6000; y += 2000)
      for (int x = -6000; x <= 6000; x += 2000)
        CHECK(biomeAt(x, y) == 0);
  }
  o.seed = 20261004;
  int changes[2] = {};
  for (int gen = 2; gen <= 3; gen++) {
    o.generator = gen;
    int prev = biomeAt(-131072, 100);
    for (int x = -131008; x <= 131072; x += 64) {
      int b = biomeAt(x, 100);
      changes[gen - 2] += b != prev;
      prev = b;
    }
  }
  CHECK(changes[0] > changes[1] * 15 && changes[1] > 0);
  std::cout << "PASS 32768-tile v3 generator / 12-seed coverage / diagnostic "
               "transect "
            << changes[0] << " v2 vs " << changes[1] << " v3 boundaries\n";
  placePlayer(0, 0);
  revealTerrain();
  CHECK(knownBiomeWaypoint(0));
  CHECK(!knownBiomeWaypoint(4));
  CHECK(!exploredAt(500, 0));
  std::fill(pixels.begin(), pixels.end(), WHITE);
  localMap(10, 10, 1);
  CHECK(pixels[12 * W + 12] == 0xff030708);
  CHECK(pixels[(10 + int(g.py / T)) * W + 10 + int(g.px / T)] == WHITE);
  g.overlay = 6;
  o.atlasLarge = true;
  w.atlasDirty = true;
  drawExploreAtlas();
  CHECK(pixels[100 * W + 100] == 0xff030708);
  int zoom = w.mapZoom;
  woodsTouch(205, 300);
  CHECK(w.mapZoom == zoom + 1);
  woodsTouch(120, 300);
  CHECK(w.mapZoom == zoom);
  g.overlay = 0;
  placePlayer(-64, -64);
  revealTerrain();
  CHECK(exploredAt(int64_t(std::floor(globalX())),
                   int64_t(std::floor(globalY()))));
  CHECK(exploredAt(0, 0));
  CHECK(!exploredAt(3000, 3000));
  auto before = w.explored;
  encoded = encodeFrontier();
  CHECK(decodeFrontier(encoded));
  CHECK(w.explored == before);
  std::cout << "PASS exploration-only local/global map, unknown pixels black, "
               "no undiscovered biome compass, zoom, negative coordinates and "
               "saved visibility\n";
  // Isolated real generated tree fixture: no assertion here is an input-only
  // playthrough.
  placePlayer(0, 0);
  g.enemies.clear();
  g.animals.clear();
  Prop tree{};
  bool found = false;
  for (auto &p : g.props)
    if (p.kind == 0 && !treeCut(p) && fits(p.x - 44, p.y, 8) &&
        sight(p.x - 44, p.y, p.x - 19, p.y)) {
      tree = p;
      found = true;
      break;
    }
  CHECK(found);
  auto key = treeKey(tree);
  g.props = {tree};
  g.px = tree.x - 44;
  g.py = tree.y;
  g.aimx = 1;
  g.aimy = 0;
  g.weapon = AXE;
  int initial = w.wood;
  chopTrees(87);
  CHECK(w.trees.count(key) == 1 && !treeCut(tree));
  int remaining = w.trees[key];
  encoded = encodeFrontier();
  CHECK(decodeFrontier(encoded));
  CHECK(w.trees[key] == remaining && o.generator == 3);
  found = false;
  for (auto &p : g.props)
    if (p.kind == 0 && treeKey(p) == key) {
      tree = p;
      found = true;
      break;
    }
  CHECK(found);
  g.props = {tree};
  g.px = tree.x - 44;
  g.py = tree.y;
  g.aimx = 1;
  g.aimy = 0;
  for (int i = 0; i < remaining; i++)
    chopTrees(87);
  CHECK(treeCut(tree));
  CHECK(tileAt(key.first, key.second).map == 1);
  CHECK(g.map[int(tree.y / T) * MW + int(tree.x / T)] == 1);
  CHECK(!j.drops.empty() && w.felled == 1 && !w.falling.empty());
  for (auto &d : j.drops) {
    d.x = globalX();
    d.y = globalY();
  }
  journeyPreStep(.5f);
  CHECK(w.wood > initial);
  int wood = w.wood;
  chopTrees(87);
  CHECK(w.wood == wood && w.felled == 1);
  g.camx = tree.x - 320;
  g.camy = tree.y - 190;
  std::fill(pixels.begin(), pixels.end(), INK);
  drawFallingTree(w.falling[0]);
  auto upright = digest(pixels);
  w.falling[0].time = .7f;
  std::fill(pixels.begin(), pixels.end(), INK);
  drawFallingTree(w.falling[0]);
  CHECK(upright != digest(pixels));
  wildlandsTick(2);
  CHECK(w.falling.empty());
  encoded = encodeFrontier();
  CHECK(decodeFrontier(encoded));
  CHECK(w.wood == wood && treeCut(key.first, key.second));
  rebase(16, 12);
  rebase(-16, -12);
  CHECK(treeCut(key.first, key.second));
  bool stump = false;
  for (auto &p : g.props)
    if (p.kind == 0 && treeKey(p) == key)
      stump = true;
  CHECK(stump);
  w.wood = 20;
  v.fiber = 3;
  g.props.clear();
  int supports = v.splints;
  craftSplint();
  CHECK(v.splints == supports + 1 && w.wood == 16 && v.fiber == 2);
  buildCampfire();
  CHECK(w.wood == 8 && w.fires.size() == 1 && nearFire());
  buildCampfire();
  CHECK(w.wood == 8);
  encoded = encodeFrontier();
  CHECK(decodeFrontier(encoded));
  CHECK(w.fires.size() == 1 && nearFire());
  Wildlands decoded;
  CHECK(decodeWildlands(encodeWildlands(), decoded));
  CHECK(decoded.explored == w.explored && decoded.trees == w.trees &&
        decoded.fires == w.fires);
  CHECK(
      !decodeWildlands("EARTH1 0 0 0 1 -9223372036854775808 1 0 0 0", decoded));
  CHECK(!decodeWildlands("EARTH1 nan 0 0 0 0 0", decoded));
  CHECK(!decodeWildlands("EARTH1 0 0 0 20001", decoded));
  CHECK(!decodeWildlands("EARTH1 0 0 2147483647 0 0 0", decoded));
  auto savedState = w;
  w = Wildlands{};
  for (int i = 0; i < 20000; i++)
    w.trees[{-1000000000LL + i, -1000000000LL}] = 15;
  for (int i = 0; i < 5000; i++)
    w.fires.insert({-1000000000LL + i, -1000000000LL});
  for (int i = 0; i < 16000; i++)
    w.explored[{-31250000LL + i, -31250000LL}].fill(UINT64_MAX);
  auto maximumEarth = encodeWildlands();
  CHECK(maximumEarth.size() > 6 * 1024 * 1024);
  CHECK(decodeWildlands(maximumEarth, decoded));
  CHECK(decoded.trees.size() == 20000 && decoded.explored.size() == 16000 &&
        decoded.fires.size() == 5000);
  w = std::move(savedState);
  int savedSplints = v.splints, beforeWood = w.wood, beforeFiber = v.fiber;
  v.splints = 200;
  craftSplint();
  CHECK(w.wood == beforeWood && v.fiber == beforeFiber);
  v.splints = savedSplints;
  std::cout << "PASS damageable axe trees, felling animation, single wood "
               "reward, persistent walkable stumps, rebase, saved player fire, "
               "splint crafting and bounded parsing\n";
  w.clockOffset = 0;
  o.seconds = 0;
  CHECK(DAY_SECONDS == 1440);
  float hour = worldHour();
  o.seconds = 1440;
  CHECK(std::abs(worldHour() - hour) < .00001f);
  o.seconds += 60;
  CHECK(std::abs(worldHour() - (hour + 1)) < .00001f);
  double paused = o.seconds;
  g.overlay = 1;
  tick(.1f);
  CHECK(o.seconds == paused);
  g.overlay = 0;
  std::set<uint64_t> looks;
  g.props.clear();
  for (float hour : {0.f, 5.f, 6.5f, 9.f, 12.f, 17.f, 18.5f, 21.f}) {
    o.seconds = (hour < 8 ? hour + 16 : hour - 8) * 60;
    std::fill(pixels.begin(), pixels.end(), 0xff879679);
    drawDayLight();
    looks.insert(digest(pixels));
  }
  CHECK(looks.size() >= 7);
  std::set<uint64_t> trees;
  for (int type = 0; type < 9; type++) {
    auto &sprite = treeSprite(type, 0, 0);
    CHECK(sprite.pixels.size() == 128 * 144);
    int count = 0;
    for (C c : sprite.pixels)
      count += c != 0;
    CHECK(count > 100);
    trees.insert(digest(sprite.pixels));
  }
  CHECK(trees.size() == 9);
  std::set<uint64_t> poses;
  g.camx = g.px - 320;
  g.camy = g.py - 190;
  for (int weapon = 0; weapon < 3; weapon++)
    for (int pose = 0; pose < 5; pose++) {
      g.weapon = weapon;g.attackWeapon=weapon;j.active=true;j.windup=.2f;j.activeTime=.35f;j.recovery=.45f;j.elapsed=pose*.19f;j.aim=0;j.combo=pose%3;g.walkPhase=pose*2;
      g.attackLength = 1;
      g.attackTime = .95f - pose * .18f;
      g.aimx = 1;
      g.aimy = 0;
      std::fill(pixels.begin(), pixels.end(), INK);
      drawNewRanger(320, 190, pose * .5f, true);
      poses.insert(digest(pixels));
    }
  CHECK(poses.size() >= 12);
  std::cout << "PASS 24-minute active day, paused clock, distinct "
               "dawn/day/dusk/night grades, nine distinct tree sprites and "
               "weapon/walk poses\n";
  g.scene = PLAY;
  g.overlay = 0;
  g.lowPower = false;
  motionBlur = true;
  g.vx = 150;
  renderDt = 1.f / 60;
  std::fill(pixels.begin(), pixels.end(), INK);
  drawMotionBlur();
  std::fill(pixels.begin(), pixels.end(), WHITE);
  drawMotionBlur();
  CHECK(pixels[190 * W + 320] != WHITE);
  motionBlur = false;
  std::fill(pixels.begin(), pixels.end(), WHITE);
  drawMotionBlur();
  CHECK(pixels[190 * W + 320] == WHITE);
  motionBlur = true;
  g.lowPower = true;
  std::fill(pixels.begin(), pixels.end(), WHITE);
  drawMotionBlur();
  CHECK(pixels[190 * W + 320] == WHITE);
  g.lowPower = false;
  g.overlay = 9;
  woodsTouch(160, 210);
  CHECK(!motionBlur);
  bool stored = motionBlur;
  saveSettings();
  motionBlur = !stored;
  loadSettings();
  CHECK(motionBlur == stored);
  g.overlay = 0;
  g.attackTime = 0;
  g.mx = g.my = 0;
  g.acc = 0;
  g.previousX = g.px - 4;
  g.previousY = g.py;
  float px = g.px, py = g.py;
  frame(pixels.data(), 1.f / 120);
  CHECK(g.px == px && g.py == py);
  double x0 = globalX(), y0 = globalY();
  frame(pixels.data(), 0);
  CHECK(globalX() == x0 && globalY() == y0);
  std::cout << "PASS subtle blur on/off/battery bypass, persisted preference, "
               "rendering interpolation leaves physics coordinates intact\n";
  std::filesystem::remove_all(root);
  std::cout << "ALL WILD EARTH CHECKS PASSED: " << checks << " assertions\n";
}
