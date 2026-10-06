// Historical coordinate adapter; new UI is exercised separately in ui7_tests.
#define DW_LEGACY_TEST_UI
#include "../native/engine.cpp"
#include <iostream>
#include <set>
using namespace av;
int checks = 0;
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      std::cerr << "FAIL line " << __LINE__ << ": " << #x << "\n";             \
      return 1;                                                                \
    }                                                                          \
    checks++;                                                                  \
  } while (0)
std::string readAll(const std::string &p) {
  std::ifstream f(p);
  std::ostringstream s;
  s << f.rdbuf();
  return s.str();
}
int main() {
  std::string root = (std::filesystem::temp_directory_path() /
                      ("frontier-test-" + hexId(entropy())))
                         .string();
  std::filesystem::create_directories(root);
  const std::string legacy =
      "ASHEN1 4 25 275 3 2 9 4 0 1 1 2 3 3\n1 0 2 1\n2 1 3 2\n3 2 3 3\n";
  {
    std::ofstream f(root + "/progress.sav");
    f << checksum(legacy) << '\n' << legacy;
  }
  std::string original = readAll(root + "/progress.sav");
  boot(root);
  CHECK(g.scene == SPLASH);
  CHECK(g.level == 4);
  for (int i = 0; i < 230; i++)
    tick(1.f / 60);
  CHECK(g.scene == HOME);
  CHECK(o.transition <= 0);
  touch(0, 1, 320, 235);
  for (int i = 0; i < 32; i++)
    tick(1.f / 60);
  CHECK(g.scene == WORLDS);
  CHECK(o.worlds.empty());
  touch(0, 1, 320, 82);
  for (int i = 0; i < 32; i++)
    tick(1.f / 60);
  CHECK(g.scene == CREATE);
  touch(0, 1, 320, 101);
  CHECK(av_text_request() == 1);
  CHECK(av_text_request() == 0);
  av_submit_text(1, "ALPHA FOREST");
  CHECK(o.draftName == "ALPHA FOREST");
  av_submit_text(2, "18446744073709551615");
  CHECK(o.draftSeed == std::numeric_limits<uint64_t>::max());
  av_submit_text(2, "-1");
  CHECK(o.draftSeed == std::numeric_limits<uint64_t>::max());
  av_submit_text(2, "123456789");
  g.weapon = BOW;
  touch(0, 1, 330, 317);
  for (int i = 0; i < 75; i++)
    tick(1.f / 60);
  CHECK(g.scene == PLAY && g.openWorld);
  CHECK(g.level == 1 && g.gold == 0);
  CHECK(g.weapon == BOW);
  CHECK(o.name == "ALPHA FOREST" && o.seed == 123456789);
  CHECK(fits(g.px, g.py));
  CHECK(!o.id.empty());
  const auto alpha = o.id;
  CHECK(readAll(root + "/progress.sav") == original);
  std::cout << "PASS intro > home > library > create, text bridge, unsigned "
               "seed, independent character, legacy untouched\n";
  CHECK(floorDiv(-1, 32) == -1 && floorMod(-1, 32) == 31);
  CHECK(floorDiv(-32, 32) == -1 && floorMod(-32, 32) == 0);
  // Original v1 generator regression. v2 scale/coverage has dedicated living_tests.
  o.generator=1;
  std::set<int> biomes;
  for (int seed = 0; seed < 36; seed++) {
    o.seed = seed * 717 + 41;
    for (int y = -144; y <= 144; y += 12)
      for (int x = -144; x <= 144; x += 12) {
        int b = biomeAt(x, y);
        CHECK(b >= 0 && b < 5);
        biomes.insert(b);
        Tile a = tileAt(x, y), c = tileAt(x, y);
        CHECK(a.map == c.map && a.ground == c.ground && a.biome == c.biome);
      }
    // Every cell boundary is a guaranteed connected walkable trail/bridge.
    for (int v = -320; v <= 320; v++) {
      CHECK(tileAt(32, v).map == 1);
      CHECK(tileAt(v, -32).map == 1);
    }
    for (int sign : {-1, 1}) {
      int64_t q = sign * 999999000LL;
      Tile a = tileAt(q, q);
      CHECK(a.biome < 5);
      CHECK(a.map >= 1 && a.map <= 4);
    }
  }
  CHECK(biomes.size() == 5);
  o.seed = 123456789;
  uint32_t digest1 = 0, digest2 = 0;
  for (int y = -40; y < 40; y++)
    for (int x = -40; x < 40; x++) {
      auto t = tileAt(x, y);
      digest1 = hash(digest1 + t.map + t.biome * 9 + t.ground * 59);
    }
  o.seed = 987654321;
  for (int y = -40; y < 40; y++)
    for (int x = -40; x < 40; x++) {
      auto t = tileAt(x, y);
      digest2 = hash(digest2 + t.map + t.biome * 9 + t.ground * 59);
    }
  CHECK(digest1 != digest2);
  o.seed = 123456789;
  std::cout << "PASS deterministic negative/positive/far coordinates, distinct "
               "seeds, all five biomes and connected travel grid\n";
  placePlayer(0, 0);
  std::map<std::pair<int64_t, int64_t>, Tile> before;
  for (int y = 0; y < MH; y++)
    for (int x = 0; x < MW; x++)
      before[{o.originX + x, o.originY + y}] = {
          g.map[y * MW + x], g.ground[y * MW + x], g.biomes[y * MW + x]};
  float x = g.px, y = g.py;
  double gx = globalX(), gy = globalY();
  fireArrow(x, y, 0, 10, 1);
  float rel = g.bolts[0].x - g.px;
  rebase(16, 12);
  CHECK(std::abs(globalX() - gx) < .0001 && std::abs(globalY() - gy) < .0001);
  CHECK(std::abs(g.bolts[0].x - g.px - rel) < .001f);
  for (int j = 0; j < MH; j++)
    for (int i = 0; i < MW; i++) {
      auto it = before.find({o.originX + i, o.originY + j});
      if (it != before.end()) {
        int a = j * MW + i;
        CHECK(it->second.map == g.map[a]);
        CHECK(it->second.ground == g.ground[a]);
        CHECK(it->second.biome == g.biomes[a]);
      }
    }
  rebase(-16, -12);
  CHECK(std::abs(g.px - x) < .001 && std::abs(g.py - y) < .001);
  // Stable entity identities are independent of vector compaction.
  g.bolts.clear();
  addEnemy(g.px + 50, g.py, 0, 3);
  uint64_t entity = g.enemies.back().entityId;
  fireArrow(g.px + 50, g.py, 0, 10, 1);
  g.bolts.back().vx = 0;
  g.enemies.back().hp = 10000;
  updateBolts(.01f);
  float after = g.enemies.back().hp;
  g.enemies.erase(g.enemies.begin());
  updateBolts(.01f);
  auto found = std::find_if(g.enemies.begin(), g.enemies.end(),
                            [&](auto &e) { return e.entityId == entity; });
  CHECK(found != g.enemies.end() && found->hp == after);
  std::cout << "PASS seamless rebase overlap, unchanged world/player/arrow "
               "positions, stable projectile target identity\n";
  placePlayer(0, 0);
  g.bolts.clear();
  int index = -1;
  for (int i = 0; i < int(g.enemies.size()); i++)
    if (g.enemies[i].slot >= 0 && !g.enemies[i].boss()) {
      index = i;
      break;
    }
  CHECK(index >= 0);
  auto enemy = g.enemies[index];
  g.px = enemy.homeX + enemy.territory + 260;
  g.py = enemy.homeY;
  CHECK(frontierIdle(g.enemies[index], .016f));
  CHECK(g.enemies[index].wind == 0);
  g.px = enemy.homeX;
  g.py = enemy.homeY;
  CHECK(!frontierIdle(g.enemies[index], .016f));
  hitEnemy(g.enemies[index], 100000, 1, 0, true);
  CHECK(killedMask(enemy.campX, enemy.campY) & (1u << enemy.slot));
  CHECK(!g.bossKilled);
  uint32_t mask = killedMask(enemy.campX, enemy.campY);
  g.level = 4;
  g.gold = 321;
  g.weapon = AXE;
  g.flasks = 1;
  g.healCd = 2.5f;
  g.hp = 91;
  placePlayer(-64.5, 32.5);
  gx = globalX();
  gy = globalY();
  flushOpenWorld();
  CHECK(!o.dirty);
  auto saveA = readAll(worldFile(alpha));
  CHECK(saveA.size() > 100);
  o.draftName = "BETA DUNES";
  o.draftSeed = 445566;
  CHECK(createWorld());
  const auto beta = o.id;
  CHECK(beta != alpha);
  CHECK(g.gold == 0 && g.level == 1 && g.flasks == 3);
  CHECK(o.slain.empty());
  g.gold = 77;
  flushOpenWorld();
  CHECK(enterWorld(alpha));
  CHECK(g.gold == 321 && g.level == 4 && g.weapon == AXE && g.flasks == 1);
  CHECK(g.hp == 91 && g.healCd == 2.5f);
  CHECK(std::abs(globalX() - gx) < .001 && std::abs(globalY() - gy) < .001);
  CHECK(killedMask(enemy.campX, enemy.campY) == mask);
  CHECK(enterWorld(beta));
  CHECK(g.gold == 77 && g.level == 1 && o.seed == 445566);
  CHECK(readAll(root + "/progress.sav") == original);
  std::cout << "PASS territory entry/leash, permanent kill delta, per-world "
               "gold/level/weapon/HP/flasks/position isolation\n";
  CHECK(enterWorld(alpha));
  g.gold = 400;
  flushOpenWorld();
  g.gold = 500;
  flushOpenWorld();
  {
    std::ofstream f(worldFile(alpha));
    f << "broken";
  }
  CHECK(enterWorld(alpha));
  CHECK(g.gold == 400);
  CHECK(o.name == "ALPHA FOREST");
  scanWorlds();
  CHECK(o.worlds.size() == 2);
  CHECK(!validId("../../progress.sav"));
  CHECK(!enterWorld("../../progress.sav"));
  flushOpenWorld();
  boot(root);
  CHECK(g.level == 4 && g.gold == 275);
  CHECK(o.worlds.size() == 2);
  CHECK(g.scene == SPLASH);
  CHECK(enterWorld(alpha));
  CHECK(g.gold == 400);
  g.overlay = 0;
  g.mx = 1;
  g.my = 1;
  suspend();
  CHECK(g.overlay == 1 && g.mx == 0 && g.my == 0);
  CHECK(!o.dirty);
  CHECK(readAll(root + "/progress.sav") == original);
  std::cout << "PASS checksum backup recovery, reboot/resume, lifecycle flush "
               "and legacy preservation\n";
  g.overlay = 0;
  placePlayer(0, 0);
  g.bolts.clear();
  size_t maxEnemies = 0, maxProps = 0;
  // Streaming fixtures deliberately place the player; this is not a live
  // playthrough.
  for (int i = 0; i < 180; i++) {
    double xx = (i - 90) * 31.0, yy = (i % 7 - 3) * 32.0;
    placePlayer(xx, yy);
    CHECK(fits(g.px, g.py));
    CHECK(g.enemies.size() <= 220);
    CHECK(g.props.size() < 1500);
    CHECK(g.animals.size() <= 72);
    maxEnemies = std::max(maxEnemies, g.enemies.size());
    maxProps = std::max(maxProps, g.props.size());
    for (auto &e : g.enemies) {
      CHECK(std::isfinite(e.x) && std::isfinite(e.y));
      CHECK(e.biome >= 0 && e.biome < 5);
    }
    for (int k = 0; k < 3; k++)
      tick(1.f / 60);
  }
  std::cout << "PASS 180 streaming fixtures / max active enemies " << maxEnemies
            << " / props " << maxProps << "\n";
  // Actual uninterrupted input-only walking across multiple windows, no
  // teleport.
  respawnFrontier();
  g.overlay = 0;
  g.mx = 1;
  g.my = 0;
  g.attacking = false;
  double startX = globalX();
  for (int i = 0; i < 60 * 65 && g.scene == PLAY; i++) {
    g.mx = 1;
    g.my = 0;
    tick(1.f / 60);
  }
  CHECK(g.scene == PLAY);
  CHECK(globalX() - startX > 260);
  CHECK(std::abs(globalY()) < 2);
  CHECK(g.enemies.size() <= 220);
  std::cout << "PASS 65s continuous ordinary-input trail walk: "
            << int(globalX() - startX)
            << " tiles, alive, streamed without map edge\n";
  for (int b = 0; b < 5; b++) {
    findBiome(b);
    CHECK(o.waypoint);
    CHECK(biomeAt(o.waypointX, o.waypointY) == b);
  }
  CHECK(av_audio() == g.world + 1);
  CHECK(av_music() == g.world + 2);
  g.muted = true;
  CHECK(av_audio() == 0 && av_music() == 0);
  g.muted = false;
  std::vector<C> pixels(W * H);
  for (int i = 0; i < 900; i++) {
    if (g.scene == DEAD)
      respawnFrontier();
    if (i % 45 == 0) {
      g.mx = (rnd(201) - 100) / 100.f;
      g.my = (rnd(201) - 100) / 100.f;
      g.attacking = true;
      skill(rnd(4));
    }
    frame(pixels.data(), 1.f / 60);
    CHECK(std::isfinite(g.px) && std::isfinite(g.py));
    CHECK(g.bolts.size() <= 180);
    CHECK(g.particles.size() <= 320);
  }
  leaveFrontier();
  for (int i = 0; i < 32; i++)
    tick(1.f / 60);
  CHECK(g.scene == WORLDS && !g.openWorld);
  CHECK(readAll(root + "/progress.sav") == original);
  CHECK(o.worlds.size() == 2);
  o.selected = 0;
  std::string removed = o.worlds[0].id;
  g.scene = DELETE_WORLD;
  touch(0, 1, 415, 255);
  for (int i = 0; i < 32; i++)
    tick(1.f / 60);
  CHECK(o.worlds.size() == 1);
  CHECK(!std::filesystem::exists(worldFile(removed)));
  CHECK(readAll(root + "/progress.sav") == original);
  std::cout << "PASS five biome compass, music/mute routing, 900 "
               "simulation/render frames, safe exit and confirmed deletion\n";
  std::filesystem::remove_all(root);
  std::cout << "ALL FRONTIER CHECKS PASSED: " << checks << " assertions\n";
}
