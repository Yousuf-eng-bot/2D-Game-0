// Historical coordinate adapter; new UI is exercised separately in ui7_tests.
#define DW_LEGACY_TEST_UI
#include "../native/engine.cpp"
#include <iostream>
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
void flat() {
  g.enemies.clear();
  g.animals.clear();
  g.props.clear();
  g.bolts.clear();
  g.hazards.clear();
  g.drops.clear();
  g.map.fill(1);
  g.ground.fill(0);
  g.biomes.fill(0);
  g.scene = PLAY;
  g.openWorld = true;
  g.overlay = 0;
  g.px = 800;
  g.py = 650;
  g.fx = g.aimx = 1;
  g.fy = g.aimy = 0;
  g.mx = g.my = g.vx = g.vy = 0;
  g.dodgeCd = 0;
  g.dodgeCharges = 2;
  g.invul = g.attackCd = g.attackTime = g.hitstop = g.comboTime = g.dash = 0;
  g.hp = maxhp();
  g.bossKilled = false;
  g.gate = true;
  g.weapon = SWORD;
  g.attacking = false;
  v = Survival{};
  j = Journey{};
  w.wood = 0;
}
void actionFrames(int n) {
  for (int i = 0; i < n; i++) {
    journeyPreStep(1.f / 60);
    journeyStrikeStep(1.f / 60);
  }
}
int main() {
  auto root = std::filesystem::temp_directory_path() /
              ("journey-test-" + hexId(entropy()));
  boot(root.string());
  std::vector<C> pixels(W * H);
  pix = pixels.data();
  auto fixtures = std::filesystem::path(__FILE__).parent_path() / "fixtures";
  std::string old;
  CHECK(readChecked((fixtures / "legacy-world-v05.sav").string(), old));
  CHECK(decodeFrontier(old));
  CHECK(o.generator == 3 && g.level == 3 && g.gold == 456 && g.hp == 89 &&
        v.food == 71 && v.water == 66 && w.wood == 19 && w.felled == 1 &&
        w.trees.size() == 2 && w.fires.size() == 1);
  CHECK(o.seconds == 1234.5 && w.clockOffset == 0);
  int64_t x, y;
  int m, ground, biome, rows = 0;
  std::ifstream f(fixtures / "legacy-tiles-v05.txt");
  while (f >> x >> y >> m >> ground >> biome) {
    auto t = tileAt(x, y);
    CHECK(t.map == m && t.ground == ground && t.biome == biome);
    rows++;
  }
  CHECK(rows == 11731);
  o.id = "0000000000000005";
  CHECK(atomicWorld(worldFile(o.id), old));
  flushOpenWorld();
  std::string archived;
  CHECK(readChecked(worldFile(o.id) + ".v05", archived) && archived == old);
  CHECK(decodeFrontier(encodeFrontier()));
  CHECK(o.generator == 3 && w.wood == 19 && w.explored.size() > 0);
  std::cout << "PASS authentic 0.5 migration: " << rows
            << " terrain samples, progress, tree/fire/fog records, 24-minute "
               "phase, .v05 archive\n";
  o.draftName = "JOURNEY TEST";
  o.draftSeed = 20261004;
  CHECK(createWorld());
  CHECK(o.generator == 6 && j.stock[SEED] == 3);
  int high = 0, water = 0, bridges = 0;
  for (int yy = -140; yy <= 140; yy++)
    for (int xx = -140; xx <= 140; xx++) {
      int h = landHeight(xx, yy);
      CHECK(h >= 0 && h <= 5);
      CHECK(std::abs(h - landHeight(xx + 1, yy)) <= 1);
      if (std::abs(h - landHeight(xx, yy + 1)) > 1)
        std::cerr << "height discontinuity " << xx << "," << yy << ": " << h
                  << " -> " << landHeight(xx, yy + 1)
                  << " river=" << riverDistance(xx, yy) << ","
                  << riverDistance(xx, yy + 1) << "\n";
      CHECK(std::abs(h - landHeight(xx, yy + 1)) <= 1);
      high += h >= 3;
      auto t = tileAt(xx, yy);
      water += t.map == 4;
      bridges += t.ground == 3;
    }
  CHECK(high > 100 && water > 100 && bridges > 10);
  CHECK(landHeight(0, 0) == 0);
  flat();
  for (int weapon = 0; weapon < 3; weapon++)
    for (int heavy = 0; heavy < 2; heavy++)
      for (int combo = 0; combo < 3; combo++) {
        g.weapon = weapon;
        v.stamina = 100;
        j.active = false;
        g.comboTime = 1;
        j.combo = (combo + 2) % 3;
        beginStrike(heavy);
        CHECK(j.active);
        float prev = strikeAngle(0);
        for (float t = .001; t <= g.attackLength; t += .001) {
          float angle = strikeAngle(t);
          CHECK(std::isfinite(angle) && std::abs(angle - prev) < .12f);
          prev = angle;
        }
      }
  flat();
  addEnemy(g.px + 44, g.py, 0, 0, false);
  auto &e = g.enemies.back();
  e.hp = e.maxhp = 1000;
  e.alertTime = 8;
  beginStrike(false);
  actionFrames(12);
  CHECK(e.hp < 1000 && e.hp >= 970);
  float hitHP = e.hp;
  actionFrames(30);
  CHECK(e.hp == hitHP && !j.active);
  CHECK(j.struck.size() == 1);
  flat();
  addEnemy(g.px + 90, g.py, 0, 0, false);
  g.enemies.back().hp = 1000;
  beginStrike(false);
  actionFrames(40);
  CHECK(g.enemies.back().hp == 1000);
  flat();
  beginStrike(false);
  j.elapsed = g.attackLength - .08f;
  beginStrike(true);
  CHECK(j.queue > 0 && j.queuedHeavy);
  beginStrike(false);
  CHECK(j.queuedHeavy);
  actionFrames(7);
  CHECK(j.active && j.heavy);
  clearInput();
  CHECK(j.queue == 0 && j.guardFinger == -1);
  std::cout
      << "PASS continuous action angles for 18 moves, active-window swept "
         "hits, once-per-target damage, reach and input buffering\n";
  flat();
  addEnemy(g.px + 30, g.py, 0, 0, false);
  float hp = g.hp;
  beginGuard();
  CHECK(j.parry > 0);
  hurt(20, g.px + 30, g.py);
  CHECK(g.hp == hp && j.counter > 0 && g.enemies[0].stun > .5f);
  hurt(20, g.px - 30, g.py);
  CHECK(g.hp < hp);
  flat();
  j.guard = .5f;
  j.parry = 0;
  hp = g.hp;
  hurt(20, g.px + 30, g.py);
  CHECK(g.hp < hp && g.hp > hp - 15);
  flat();
  j.guard = .5f;
  v.stamina = 1;
  hurt(20, g.px + 30, g.py);
  CHECK(j.guard == 0 && g.hp < maxhp());
  flat();
  jumpPlayer();
  CHECK(j.z > 0);
  float maximum = 0;
  for (int i = 0; i < 50; i++) {
    journeyPreStep(1.f / 60);
    maximum = std::max(maximum, j.z);
  }
  CHECK(maximum > 30 && maximum < 36 && j.z == 0 && g.hp == maxhp());
  cycleStance();
  CHECK(stealthFactor() == 1);
  actionFrames(20);
  CHECK(j.stance == 1 && stanceSpeed() < .6f && stealthFactor() < 1);
  j.stanceCd = 0;
  cycleStance();
  CHECK(j.stance == 2 && stanceSpeed() < .3f);
  beginStrike(false);
  CHECK(!j.active);
  j.stanceCd = 0;
  cycleStance();
  CHECK(j.stance == 0);
  flat();
  g.px = 30 * T + 12;
  g.py = 30 * T + 12;
  int src = 30 * MW + 30, dst = src + 1;
  j.heights[dst] = 2;
  CHECK(!journeyMoveAllowed(g.px + T, g.py, g.px, g.py, 7, true));
  j.heights[dst] = 1;
  j.z = 25;
  CHECK(journeyMoveAllowed(g.px + T, g.py, g.px, g.py, 7, true));
  g.map[dst] = 5;
  CHECK(!journeyMoveAllowed(g.px + T, g.py, g.px, g.py, 7, true));
  std::cout
      << "PASS directional parry/counter, chip/block/guard break, "
         "jump/landing, crouch/prone restrictions and height/solid collision\n";
  flat();
  dropMaterial(g.px, g.py, LOG, 7);
  CHECK(w.wood == 0 && j.drops.size() == 1);
  journeyPreStep(.5f);
  CHECK(w.wood == 7 && j.drops.empty());
  j.stock[PLANK] = 64 * 23;
  dropMaterial(g.px, g.py, STONE, 150);
  journeyPreStep(.5f);
  CHECK(j.stock[STONE] == 0 && j.drops.size() == 1);
  w.wood = 0;
  journeyPreStep(.1f);
  CHECK(j.stock[STONE] == 64 && j.drops[0].count == 86);
  j.stock[PLANK] = 64 * 22;
  journeyPreStep(.1f);
  CHECK(j.stock[STONE] == 128 && j.drops[0].count == 22);
  Journey parsed;
  auto raw = encodeJourney();
  CHECK(decodeJourney(raw, parsed));
  CHECK(parsed.stock == j.stock && parsed.drops.size() == j.drops.size());
  CHECK(!decodeJourney("JOURNEY1 2 0 0", parsed));
  CHECK(!decodeJourney(raw + " EXTRA", parsed));
  CHECK(!decodeJourney("JOURNEY1 0 -9223372036854775808 0", parsed));
  flat();
  w.wood = 5;
  craftRecipe(0);
  CHECK(w.wood == 4 && j.stock[PLANK] == 4);
  craftRecipe(2);
  CHECK(j.stock[BENCH] == 1 && j.stock[PLANK] == 0);
  craftRecipe(3);
  CHECK(j.toolTier[0] == 0);
  auto key = std::make_pair(int64_t(std::floor(globalX())) + 1,
                            int64_t(std::floor(globalY())));
  Structure bench;
  bench.kind = BENCH;
  j.built[key] = bench;
  craftRecipe(0);
  craftRecipe(0);
  craftRecipe(1);
  craftRecipe(3);
  CHECK(j.toolTier[0] == 1 && j.durability[0] == 120);
  int wood = w.wood;
  j.stock[PLANK] = 64 * 24;
  craftRecipe(0);
  CHECK(w.wood == wood);
  j.stock.fill(0);
  Structure furnace;
  furnace.kind = FURNACE;
  auto furnaceKey = key;
  furnaceKey.second++;
  j.built[furnaceKey] = furnace;
  j.stock[ORE] = 2;
  j.stock[COAL] = 2;
  smeltOre();
  CHECK(j.stock[ORE] == 1 && j.stock[COAL] == 1 &&
        j.built[furnaceKey].storage[ORE] == 1);
  for (int a = 0; a < 490; a++)
    journeyPreStep(1.f / 60);
  CHECK(j.built[furnaceKey].storage[ORE] == 0);
  int iron = j.stock[IRON];
  for (auto &d : j.drops)
    if (d.kind == IRON)
      iron += d.count;
  CHECK(iron == 1);
  Structure chest;
  chest.kind = CHEST;
  auto chestKey = key;
  chestKey.first--;
  j.built[chestKey] = chest;
  int ore = j.stock[ORE];
  storeMaterials(false);
  CHECK(j.stock[ORE] == 0 && j.built[chestKey].storage[ORE] == ore);
  storeMaterials(true);
  CHECK(j.stock[ORE] == ore && j.built[chestKey].storage[ORE] == 0);
  raw = encodeJourney();
  CHECK(decodeJourney(raw, parsed));
  CHECK(parsed.built.size() == j.built.size());
  auto full = encodeFrontier();
  CHECK(decodeFrontier(full));
  CHECK(j.built.size() == 3 && j.toolTier[0] == 1);
  std::cout << "PASS bobbing material drops, partial pickup/capacity, strict "
               "state parsing, recipes/stations, fueled smelting, chest "
               "transactions and save/load\n";
  // Actual generated rock, resources granted only as isolated unit-fixture
  // setup.
  o.draftName = "MINING FIXTURE";
  o.draftSeed = 20261004;
  CHECK(createWorld());
  g.enemies.clear();
  g.animals.clear();
  Prop rock{};
  bool found = false;
  for (auto &p : g.props)
    if (p.kind == 1 &&
        coordinateHash(treeKey(p).first, treeKey(p).second, 7301) % 7 > 2 &&
        fits(p.x - 43, p.y, 7) && sight(p.x - 43, p.y, p.x - 19, p.y)) {
      rock = p;
      found = true;
      break;
    }
  CHECK(found);
  g.px = rock.x - 43;
  g.py = rock.y;
  g.fx = 1;
  g.fy = 0;
  auto rk = treeKey(rock);
  for (int a = 0; a < 4; a++) {
    v.stamina = 100;
    harvestPayload();
  }
  CHECK(j.mined.count(rk) && tileAt(rk.first, rk.second).map == 1 &&
        !j.drops.empty());
  full = encodeFrontier();
  CHECK(decodeFrontier(full) && j.mined.count(rk));
  // Placement, door interaction and checkpoint persistence at a real clear
  // site.
  placePlayer(20.5, 0.5);
  g.fx = 1;
  g.fy = 0;
  auto cell = frontCell();
  CHECK(tileAt(cell.first, cell.second).map == 1);
  j.stock[DOOR] = 1;
  j.selectedBuild = DOOR;
  placeStructure();
  CHECK(j.built.count(cell) && j.stock[DOOR] == 0 &&
        tileAt(cell.first, cell.second).map == 5);
  journeyInteract();
  CHECK(j.built[cell].open && tileAt(cell.first, cell.second).map == 1);
  reclaimStructure();
  CHECK(!j.built.count(cell) && j.stock[DOOR] == 1);
  j.stock[BED] = 1;
  j.selectedBuild = BED;
  placeStructure();
  CHECK(j.built.count(cell));
  journeyInteract();
  CHECK(j.home && j.homeX == cell.first && j.restTime > 0);
  double before = o.seconds;
  for (int a = 0; a < 310; a++)
    journeyPreStep(1.f / 60);
  CHECK(j.restTime <= 0 && o.seconds == before);
  full = encodeFrontier();
  CHECK(decodeFrontier(full) && j.home);
  respawnFrontier();
  CHECK(std::abs(globalX() - (cell.first + .5)) < 2);
  // Ordinary surface excavation is single-depth; dirt blocks restore one level.
  placePlayer(24.5, .5);
  g.fx = 1;
  g.fy = 0;
  cell = frontCell();
  int originalHeight = landHeight(cell.first, cell.second);
  digGround();
  CHECK(j.mined[cell] == 2);
  CHECK(landHeight(cell.first, cell.second) == std::max(0, originalHeight - 1));
  for (int n = 0; n < 60; n++)
    journeyPreStep(1.f / 60);
  CHECK(j.stock[DIRT] == 2);
  j.selectedBuild = DIRT;
  placeStructure();
  CHECK(j.built[cell].kind == DIRT && j.stock[DIRT] == 1);
  CHECK(landHeight(cell.first, cell.second) ==
        std::max(0, originalHeight - 1) + 1);
  reclaimStructure();
  CHECK(j.stock[DIRT] == 2 && !j.built.count(cell));
  auto count = j.drops.size();
  digGround();
  CHECK(j.drops.size() == count);
  j.stock[SEED] = 2;
  plantSeed();
  CHECK(j.built[cell].kind == 18 && j.stock[SEED] == 1);
  j.built[cell].progress = 179.9f;
  journeyPreStep(.2f);
  journeyInteract();
  for (int n = 0; n < 60; n++)
    journeyPreStep(1.f / 60);
  CHECK(!j.built.count(cell) && j.stock[GRAIN] == 3 && j.stock[SEED] == 3);
  flat();
  beginStrike(true);
  v.stamina = 0;
  skill(1);
  CHECK(j.active);
  v.stamina = 100;
  skill(1);
  CHECK(!j.active && g.dash > 0);
  flat();
  beginStrike(true);
  j.elapsed = j.windup * .8f;
  skill(1);
  CHECK(j.active && j.dodgeQueue > 0);
  j.elapsed = j.windup + j.activeTime;
  journeyPreStep(.016f);
  CHECK(!j.active && g.dash > 0);
  flat();
  w.wood = 999999;
  changeMaterial(LOG, 9);
  CHECK(w.wood == 1000000);
  changeMaterial(LOG, -1000001);
  CHECK(w.wood == 0);
  Structure testBed;
  testBed.kind = BED;
  auto bedKey = frontCell();
  j.built[bedKey] = testBed;
  v.meals = 2;
  v.cleanWater = 2;
  o.seconds = 900;
  w.clockOffset = 0;
  journeyInteract();
  CHECK(j.restTime > 0 && j.home);
  g.mx = 1;
  journeyPreStep(.016f);
  CHECK(j.restTime == 0 && v.meals == 2 && v.cleanWater == 2);
  g.mx = 0;
  journeyInteract();
  for (int n = 0; n < 310; n++)
    journeyPreStep(1.f / 60);
  CHECK(v.meals == 1 && v.cleanWater == 1 && std::abs(worldHour() - 8) < .01f);
  flat();
  Structure toolBench;
  toolBench.kind = BENCH;
  j.built[frontCell()] = toolBench;
  j.toolTier[0] = 3;
  j.durability[0] = 0;
  j.stock[STONE] = 3;
  j.stock[STICK] = 2;
  craftRecipe(4);
  CHECK(j.toolTier[0] == 2 && j.durability[0] == 240);
  j.toolTier[0] = 3;
  j.durability[0] = 1;
  j.stock[STONE] = 3;
  j.stock[STICK] = 2;
  craftRecipe(4);
  CHECK(j.toolTier[0] == 3 && j.durability[0] == 1 && j.stock[STONE] == 3);
  CHECK(journeyTouch(480, 83, 0) && g.overlay == 10);
  CHECK(journeyTouch(580, 37, 0) && g.overlay == 0);
  g.camReady = false;
  frame(pixels.data(), 0);
  CHECK(pixels[0] != 0);
  std::filesystem::remove_all(root);
  std::cout << "ALL JOURNEY CHECKS PASSED: " << checks << " assertions\n";
}
