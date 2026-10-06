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
      std::cerr << "FAIL " << __LINE__ << ": " << #x << "\n";                  \
      return 1;                                                                \
    }                                                                          \
  } while (0)
void flat() {
  g.enemies.clear();
  g.animals.clear();
  g.props.clear();
  g.bolts.clear();
  g.particles.clear();
  g.floats.clear();
  g.hazards.clear();
  g.map.fill(1);
  g.ground.fill(0);
  g.biomes.fill(0);
  g.openWorld = true;
  g.scene = PLAY;
  g.world = 0;
  g.overlay = 0;
  g.px = 800;
  g.py = 650;
  g.camx = 800 - W / 2;
  g.camy = 650 - H / 2;
  g.invul = 0;
  g.hp = float(maxhp());
  g.gate = true;
  g.bossKilled = false;
  g.mx = g.my = 0;
  g.attackCd = g.attackTime = 0;
  v = Survival{};
  o.seconds = 0;
}
Animal animal(int species, float x, float y, uint64_t id) {
  Animal a{};
  a.species = species;
  a.id = id | 0x8000000000000000ULL;
  a.x = a.homeX = a.goalX = x;
  a.y = a.homeY = a.goalY = y;
  a.hp = a.maxhp = animalMaxHP(species);
  a.meat = 2;
  a.hunger = 60;
  a.thirst = 90;
  return a;
}
uint64_t pixelsHash(const std::vector<C> &p) {
  uint64_t h = 0;
  for (C c : p)
    h = mix64(h ^ c);
  return h;
}
int main() {
  auto root = std::filesystem::temp_directory_path() /
              ("living-test-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  boot(root.string());
  std::vector<C> pixels(W * H);
  pix = pixels.data();
  auto fixtures = std::filesystem::path(__FILE__).parent_path() / "fixtures";
  std::string old;
  CHECK(readChecked((fixtures / "legacy-world-v03.sav").string(), old));
  CHECK(old.rfind("DWFRONTIER 1 ", 0) == 0);
  CHECK(decodeFrontier(old));
  CHECK(o.generator == 1);
  CHECK(g.gold == 777 && g.level == 4 && g.hp == 91 && g.flasks == 2);
  CHECK((o.slain[{0, 0}] == 1));
  CHECK(v.food == 100 && v.water == 100 && v.bandages == 4);
  std::ifstream f(fixtures / "legacy-tiles-v03.txt");
  int64_t x, y;
  int map, ground, biome;
  int golden = 0;
  while (f >> x >> y >> map >> ground >> biome) {
    auto t = tileAt(x, y);
    CHECK(t.map == map && t.ground == ground && t.biome == biome);
    golden++;
  }
  CHECK(golden == 4734);
  std::string migrated = encodeFrontier();
  CHECK(migrated.rfind("DWFRONTIER 4 ", 0) == 0);
  CHECK(decodeFrontier(migrated));
  CHECK(o.generator == 1);
  CHECK(g.gold == 777 && g.level == 4 && g.hp == 91);
  CHECK(o.seconds == 123.5);
  o.id = "0000000000000001";
  CHECK(atomicWorld(worldFile(o.id), old));
  flushOpenWorld();
  CHECK(std::filesystem::exists(worldFile(o.id) + ".v03"));
  std::string archive;
  CHECK(readChecked(worldFile(o.id) + ".v03", archive));
  CHECK(archive == old);
  std::cout << "PASS 0.3 golden save migration: " << golden
            << " exact terrain records, progress, generator pinning, untouched "
               "archival copy\n";
  // A correctly checksummed but adversarial coordinate must fail without signed
  // abs UB.
  std::string hostile = old;
  size_t p = hostile.rfind("0 0 1\n");
  CHECK(p != std::string::npos);
  hostile.replace(p, 6, "-9223372036854775808 0 1\n");
  CHECK(!decodeFrontier(hostile));
  o.draftSeed = 20261004;
  o.draftName = "LIVING TEST";
  CHECK(createWorld());
  CHECK(o.generator == 6);
  o.generator = 2; // v0.4 biome regression; v3 is covered by canopy_tests.
  CHECK(v.food == 100 && v.stamina == 100);
  int crossings1 = 0, crossings2 = 0;
  for (int gen = 1; gen <= 2; gen++) {
    o.generator = gen;
    int prev = biomeAt(-6144, 100);
    for (int xx = -6143; xx <= 6144; xx++) {
      int b = biomeAt(xx, 100);
      if (b != prev)
        (gen == 1 ? crossings1 : crossings2)++;
      prev = b;
    }
  }
  CHECK(crossings1 > crossings2 * 8);
  CHECK(crossings2 > 0);
  for (int seed = 0; seed < 12; seed++) {
    o.seed = seed * 8719 + 31;
    std::set<int> types;
    for (int yy = -3072; yy <= 3072; yy += 128)
      for (int xx = -3072; xx <= 3072; xx += 128) {
        int b = biomeAt(xx, yy);
        CHECK(b >= 0 && b < 5);
        CHECK(b == biomeAt(xx, yy));
        types.insert(b);
      }
    CHECK(types.size() == 5);
    for (int yy = -100; yy <= 100; yy += 20)
      for (int xx = -100; xx <= 100; xx += 20)
        CHECK(biomeAt(xx, yy) == 0);
  }
  o.seed = 20261004;
  placePlayer(0, 0);
  for (int b = 0; b < 5; b++) {
    findBiome(b);
    CHECK(o.waypoint);
    CHECK(biomeAt(o.waypointX, o.waypointY) == b);
    CHECK(tileAt(o.waypointX, o.waypointY).map == 1);
  }
  std::set<int> species;
  for (int seed = 0; seed < 8; seed++) {
    o.seed = 20261004 + seed;
    placePlayer(0, 0);
    for (int b = 0; b < 5; b++) {
      findBiome(b);
      placePlayer(o.waypointX, o.waypointY);
      for (auto &a : g.animals)
        species.insert(a.species);
      CHECK(g.animals.size() <= 72);
    }
  }
  CHECK(species.size() == SPECIES_COUNT);
  std::cout << "PASS great-biome generator: crossings " << crossings1
            << " legacy vs " << crossings2 << " new; 12-seed coverage; all "
            << species.size() << " wildlife species seeded\n";
  // New format round-trips needs, resources, all six injuries, clocks and
  // ecology deltas.
  flat();
  v.food = 28.5;
  v.water = 19.25;
  v.stamina = 32;
  v.rawMeat = 7;
  v.berries = 9;
  v.fiber = 3;
  v.cleanWater = 2;
  v.dirtyWater = 6;
  v.hunts = 4;
  v.injury = {12, 24, 36, 48, 60, 72};
  v.bleeding[4] = .5f;
  v.wildlifeGone[123] = 850;
  v.plantsUsed[456] = 760;
  v.camps[{1, -2}] = {11, 12, 13, 14, 80};
  o.seconds = 123;
  o.generator = 2;
  std::string life = encodeLife();
  Survival parsed;
  CHECK(decodeLife(life, parsed));
  CHECK(parsed.injury == v.injury && parsed.bleeding == v.bleeding);
  CHECK(parsed.food == 28.5 && parsed.water == 19.25 && parsed.rawMeat == 7 &&
        parsed.dirtyWater == 6);
  CHECK((parsed.camps[{1, -2}].water == 14));
  CHECK(parsed.wildlifeGone == v.wildlifeGone &&
        parsed.plantsUsed == v.plantsUsed);
  CHECK(!decodeLife("LIFE1 nan 1 1", parsed));
  CHECK(!decodeLife(life.substr(0, life.size() / 2), parsed));
  auto body = encodeFrontier();
  CHECK(decodeFrontier(body));
  CHECK(v.injury[5] == 72 && v.bleeding[4] == .5f && o.generator == 2 &&
        o.seconds == 123);
  std::cout << "PASS versioned needs/anatomy/resources/ecology persistence and "
               "malformed-state rejection\n";
  flat();
  g.weapon = SWORD;
  float original = v.stamina;
  attack();
  CHECK(v.stamina < original);
  CHECK(g.attackTime > 0);
  v.stamina = 0;
  g.attackTime = g.attackCd = 0;
  attack();
  CHECK(g.attackTime == 0);
  flat();
  v.injury[4] = 80;
  v.injury[5] = 60;
  CHECK(movementFactor() < .7f);
  v.injury[2] = 70;
  v.injury[3] = 50;
  CHECK(weaponEffort() > 1.3f);
  g.mx = 1;
  v.sprint = true;
  float before = v.stamina;
  survivalTick(1);
  CHECK(v.stamina < before);
  CHECK(v.water < 100 && v.food < 100);
  flat();
  g.invul = 0;
  hurt(30, g.px - 40, g.py);
  int wounded = 0;
  float bleeding = 0;
  for (int i = 0; i < 6; i++) {
    wounded += v.injury[i] > 0;
    bleeding += v.bleeding[i];
  }
  CHECK(wounded == 1 && bleeding > 0 && v.lastPart >= 0);
  float hp = g.hp;
  survivalTick(2);
  CHECK(g.hp < hp);
  v.selectedPart = v.lastPart;
  int band = v.bandages;
  bandagePart();
  CHECK(v.bandages == band - 1 && v.bleeding[v.selectedPart] == 0);
  v.selectedPart = 4;
  v.injury[4] = 80;
  int splints = v.splints;
  splintPart();
  CHECK(v.splints == splints - 1 && v.injury[4] == 45);
  v.bandages = 0;
  v.fiber = 3;
  bandagePart();
  CHECK(v.bandages == 1 && v.fiber == 0);
  v.food = 20;
  int meals = v.meals;
  eatMeal();
  CHECK(v.food > 20 && v.meals == meals - 1);
  v.water = 15;
  int water = v.cleanWater;
  drinkWater();
  CHECK(v.water > 15 && v.cleanWater == water - 1);
  g.props.push_back({g.px + 20, g.py, 20, 0, false, 0});
  int berries = v.berries;
  forage();
  CHECK(v.berries == berries + 2);
  forage();
  CHECK(v.berries == berries + 2);
  g.props.push_back({g.px + 35, g.py, 21, 0, false, 0});
  fillWater();
  CHECK(v.cleanWater == 8);
  g.props.push_back({g.px + 15, g.py, 22, 0, false, 0});
  v.rawMeat = 2;
  meals = v.meals;
  beginTask(1);
  CHECK(v.task == 1);
  for (int i = 0; i < 260; i++)
    survivalTick(1.f / 60);
  CHECK(v.task == 0 && v.rawMeat == 1 && v.meals == meals + 1);
  v.cleanWater = 2;
  v.dirtyWater = 5;
  beginTask(2);
  for (int i = 0; i < 190; i++)
    survivalTick(1.f / 60);
  CHECK(v.cleanWater == 5 && v.dirtyWater == 2);
  v.injury[4] = 60;
  v.meals = 3;
  v.cleanWater = 3;
  g.hp = 50;
  beginTask(3);
  for (int i = 0; i < 490; i++)
    survivalTick(1.f / 60);
  CHECK(v.meals == 2 && v.cleanWater == 2 && g.hp > 50 && v.injury[4] == 42);
  beginTask(3);
  g.mx = 1;
  survivalTick(.1f);
  CHECK(v.task == 0 && v.meals == 2);
  g.mx = 0;
  v.water = v.food = 0;
  g.hp = 1;
  survivalTick(1);
  CHECK(g.scene == DEAD);
  respawnFrontier();
  CHECK(g.scene == PLAY && v.water >= 55 && v.food >= 55);
  for (float b : v.bleeding)
    CHECK(b == 0);
  std::cout << "PASS stamina costs, sprint, starvation, hydration, local "
               "wounds, bleeding, treatment, foraging, cooking, boiling, paid "
               "rest and interrupted actions\n";
  flat();
  g.animals.push_back(animal(HARE, g.px + 25, g.py, 1));
  float start = g.animals[0].x;
  for (int i = 0; i < 180; i++)
    updateEcology(1.f / 60);
  CHECK(g.animals[0].behaviour == FLEE);
  CHECK(len(g.animals[0].x - g.px, g.animals[0].y - g.py) > 280);
  CHECK(g.animals[0].x > start);
  g.px = g.animals[0].x - 50;
  g.py = g.animals[0].y;
  updateEcology(.1f);
  CHECK(g.animals[0].fear > 7 && g.animals[0].behaviour == FLEE);
  flat();
  g.animals.push_back(animal(BOAR, g.px + 45, g.py, 99));
  hitAnimal(g.animals[0], 1, 1); // Predator damage is not player provocation.
  updateEcology(.01f);
  CHECK(g.animals[0].behaviour != ATTACKING);
  hitAnimal(g.animals[0], 1, 0);
  updateEcology(.01f);
  CHECK(g.animals[0].behaviour == ATTACKING);
  flat();
  g.animals.push_back(animal(DEER, g.px + 35, g.py, 2));
  g.aimx = 1;
  g.aimy = 0;
  huntMelee(61, 100);
  CHECK(!g.animals[0].alive && v.hunts == 1 &&
        v.wildlifeGone.count(g.animals[0].id));
  g.px += 30;
  updateEcology(.01f);
  CHECK(v.rawMeat == 2 && g.animals[0].meat == 0);
  flat();
  g.animals.push_back(animal(DEER, g.px + 140, g.py, 3));
  g.weapon = BOW;
  CHECK(aimWildlife(330, nullptr));
  fireArrow(g.px, g.py, 0, 100);
  for (int i = 0; i < 60; i++)
    updateBolts(1.f / 60);
  CHECK(!g.animals[0].alive);
  flat();
  g.animals.push_back(animal(BOAR, g.px + 50, g.py, 4));
  g.weapon = AXE;
  g.powerCd = 0;
  skill(0);
  CHECK(g.animals[0].hp < g.animals[0].maxhp);
  v.hunting = false;
  float unhurt = g.animals[0].hp;
  huntMelee(100, 500, true);
  CHECK(g.animals[0].hp == unhurt);
  flat();
  auto insect = animal(INSECT, 220, 220, 5);
  insect.hp = 3;
  insect.meat = 1;
  insect.hunger = 100;
  auto frog = animal(FROG, 220, 220, 6);
  frog.hunger = 5;
  g.animals = {insect, frog};
  for (int i = 0; i < 480; i++)
    updateEcology(1.f / 60);
  CHECK(!g.animals[0].alive);
  CHECK(v.predatorKills > 0 && v.scavenges > 0 && g.animals[1].hunger > 20);
  flat();
  auto deer = animal(DEER, 230, 230, 7);
  deer.hunger = 10;
  g.animals = {deer};
  g.props.push_back({230, 230, 20, 0, false, 0});
  for (int i = 0; i < 300; i++)
    updateEcology(1.f / 60);
  CHECK(v.grazingEvents > 0 && v.plantsUsed.size() == 1 &&
        g.animals[0].hunger > 25);
  flat();
  o.seed = 20261004;
  placePlayer(0, 0);
  CHECK(!g.animals.empty());
  uint64_t killed = g.animals.front().id;
  killAnimal(g.animals.front(), 0);
  placePlayer(0, 0);
  bool exists = false;
  for (auto &a : g.animals)
    exists |= a.id == killed;
  CHECK(!exists);
  body = encodeFrontier();
  CHECK(decodeFrontier(body));
  exists = false;
  for (auto &a : g.animals)
    exists |= a.id == killed;
  CHECK(!exists);
  o.seconds += DAY_SECONDS + 1;
  placePlayer(0, 0);
  exists = false;
  for (auto &a : g.animals)
    exists |= a.id == killed;
  CHECK(exists);
  std::cout << "PASS prey escape hysteresis, melee/bow/power hunting, loot, "
               "trophic predation/scavenging/grazing, cooldown persistence and "
               "timed repopulation\n";
  flat();
  Enemy worker;
  worker.alive = true;
  worker.slot = 1;
  worker.kind = 0;
  worker.homeX = 300;
  worker.homeY = 300;
  worker.x = 255;
  worker.y = 322;
  worker.campX = 5;
  worker.campY = 5;
  worker.routine = COOKING;
  auto &stock = v.camps[{5, 5}];
  int food = stock.food;
  routineTick(worker, 6.1f);
  CHECK(worker.routine == COOKING && stock.food == food + 2 && stock.raw == 7 &&
        stock.wood == 7 && stock.water == 11);
  o.seconds = (11 - 8) * DAY_SECONDS / 24;
  worker.slot = 3;
  worker.x = 358;
  worker.y = 277;
  worker.routine = WORKING;
  int wood = stock.wood;
  routineTick(worker, 6.1f);
  CHECK(stock.wood == wood + 1);
  worker.slot = 4;
  worker.x = 214;
  worker.y = 358;
  worker.routine = FETCHING;
  water = stock.water;
  routineTick(worker, 6.1f);
  CHECK(stock.water == water + 2);
  o.seconds = (23 - 8) * DAY_SECONDS / 24;
  routineTick(worker, .1f);
  CHECK(worker.routine == SLEEPING);
  worker.territory = 125;
  worker.hp = worker.maxhp = 100;
  g.px = worker.homeX;
  g.py = worker.homeY;
  CHECK(!frontierIdle(worker, .1f));
  CHECK(worker.routine == -1 && worker.alertTime > 0);
  std::cout << "PASS clock-driven work/sleep, ingredient-consuming cooking, "
               "water stock, and immediate combat interruption\n";
  flat();
  std::fill(pixels.begin(), pixels.end(), 0xff8eb878);
  o.seconds = 0;
  drawSunShadows();
  auto morning = pixelsHash(pixels);
  std::fill(pixels.begin(), pixels.end(), 0xff8eb878);
  o.seconds = (12 - 8) * DAY_SECONDS / 24;
  drawSunShadows();
  auto noon = pixelsHash(pixels);
  CHECK(morning != noon);
  std::fill(pixels.begin(), pixels.end(), 0xff8eb878);
  g.px += 35;
  drawSunShadows();
  CHECK(noon != pixelsHash(pixels));
  o.seconds = (23 - 8) * DAY_SECONDS / 24;
  CHECK(sunProjection().alpha < 50);
  flat();
  g.camx = 0;
  g.camy = 0;
  Enemy archer;
  archer.alive = true;
  archer.kind = 1;
  archer.x = 100;
  archer.y = 200;
  archer.tx = 540;
  archer.ty = 200;
  archer.wind = archer.windMax = 1;
  g.enemies = {archer};
  std::fill(pixels.begin(), pixels.end(), INK);
  telegraphs();
  CHECK(pixels[200 * W + 320] == INK);
  CHECK(pixels[200 * W + 115] != INK);
  g.enemies.clear();
  hazard(100, 200, 150, 1, 20, 1, 0);
  std::fill(pixels.begin(), pixels.end(), INK);
  telegraphs();
  CHECK(pixels[200 * W + 150] == INK); // Escape arc, not a boss-to-player ray.

  flat();
  touch(0, 1, 40, 133);
  CHECK(g.overlay == 7);
  touch(0, 1, 150, 220);
  CHECK(v.selectedPart == 4);
  touch(0, 1, 495, 39);
  CHECK(g.overlay == 8);
  touch(0, 1, 45 + 3 * 111 + 20, 78);
  CHECK(v.journalBiome == 3);
  touch(0, 1, 580, 39);
  CHECK(g.overlay == 0);
  touch(0, 1, 320, 325);
  CHECK(v.sprint);
  touch(0, 1, 140, 133);
  CHECK(!v.hunting);
  std::cout << "PASS reactive sun/moon shadows, no enemy-to-player aim line, "
               "and ordinary touch routes for body/journal/sprint/hunting\n";
  std::filesystem::remove_all(root);
  std::cout << "ALL LIVING CHECKS PASSED: " << checks << " assertions\n";
}
