#pragma once
#include "raster.hpp"
#include "ui7_api.hpp"
#include <bit>
#include <chrono>
#include <deque>
#include <iomanip>
#include <limits>
#include <map>
#include <queue>
#include <set>
namespace av {
enum Scene {
  TITLE,
  HUB,
  PLAY,
  DEAD,
  WIN,
  SPLASH,
  HOME,
  WORLDS,
  CREATE,
  DELETE_WORLD,
  LOADING
};
enum Weapon { SWORD, AXE, BOW };
struct Item {
  int id = 0, slot = 0, value = 0, rarity = 0;
};
struct Enemy {
  float previousX = 0, previousY = 0;
  float x = 0, y = 0, hp = 1, maxhp = 1, cd = 1, wind = 0, windMax = 1, tx = 0,
        ty = 0, flash = 0, slow = 0, stun = 0, step = 0, vx = 0, vy = 0;
  float stateTime = 0, death = 0, stagger = 0, resist = 0, anim = 0;
  int kind = 0, room = 0, state = 0, pattern = 0, phase = 1, sequence = 0,
      hitBy = 0;
  uint64_t entityId = 0;
  int64_t campX = 0, campY = 0;
  int slot = -1, biome = 0;
  float homeX = 0, homeY = 0, territory = 0, alertTime = 0;
  int routine = 0, routineStep = 0;
  float chore = 0, needFood = 65, routineAnim = 0;
  bool alive = true, elite = false, impact = false;
  bool boss() const { return kind >= 8; }
};
struct Bolt {
  float x, y, vx, vy, life;
  int damage;
  bool friendly = false;
  int pierce = 0, id = 0;
  float r = 5;
  C color = GOLD;
  // Stable entity IDs survive streaming compaction; an arrow hits each once.
  std::array<uint64_t, 4> hitTargets{{0, 0, 0, 0}};
  int hitCount = 0;
};
struct Particle {
  float x, y, z, vx, vy, vz, life, total;
  C color;
  int size = 2;
};
struct FloatText {
  float x, y, life;
  std::string value;
  C color;
  int scale = 1;
};
struct Drop {
  float x, y;
  int gold = 0;
  Item item;
  bool gear = false;
  float age = 0;
};
// Hazard types: 0 delayed blast, 1 traveling ring with an escape wedge, 2
// lingering pool.
struct Hazard {
  float x, y, r, wind, life, maxWind, angle = 0;
  int damage = 20, type = 0;
  float radius = 0;
  bool hit = false;
  C color = RED;
};
struct Prop {
  float x, y;
  int kind, variant;
  bool open = false;
  int biome = -1;
  int64_t campX = 0, campY = 0;
};
struct Animal {
  float x, y, dx = 0, dy = 0, timer = 0, step = 0;
  int kind = 0;
  uint64_t id = 0;
  float previousX = 0, previousY = 0;
  int species = -1, behaviour = 0, biome = 0;
  float hp = 30, maxhp = 30, hunger = 65, thirst = 75, fear = 0;
  float homeX = 0, homeY = 0, goalX = 0, goalY = 0, decision = 0, cooldown = 0;
  float flash = 0, deadTime = 0, eatTimer = 0, anger = 0;
  int meat = 1;
  bool alive = true;
};
struct Echo {
  float x, y, life;
  int facing;
};
struct State {
  float previousX = 0, previousY = 0;
  Scene scene = TITLE;
  bool openWorld = false;
  uint64_t entitySerial = 0;
  int overlay = 0;
  bool loaded = false, migrated = false, muted = false, shake = true,
       lowPower = false;
  std::string path, toast;
  float toastTime = 0, time = 0, acc = 0;
  int level = 1, xp = 0, gold = 0, upgrade = 0, wins = 0, run = 0, uid = 4,
      eq[3] = {1, 2, 3};
  int weapon = 0, world = 0, worldWins[5] = {0, 0, 0, 0, 0}, difficulty = 0;
  std::vector<Item> bag{{1, 0, 0, 0}, {2, 1, 0, 0}, {3, 2, 0, 0}};
  float px = 0, py = 0, vx = 0, vy = 0, mx = 0, my = 0, fx = 1, fy = 0,
        aimx = 1, aimy = 0, hp = 120;
  float camx = 0, camy = 0, walkPhase = 0, invul = 0, hurtTime = 0, hurtx = 0,
        hurty = 0, shakeTime = 0, hitstop = 0;
  float attackCd = 0, attackTime = 0, attackLength = 0, comboTime = 0,
        powerCd = 0, dodgeCd = 0, dodgeRecharge = 0, wardCd = 0, healCd = 0;
  float slash = 0, sweep = 0, dash = 0, ward = 0, wardx = 0, wardy = 0,
        navTimer = 0, echoTimer = 0, damageVignette = 0;
  int combo = 0, attackWeapon = 0, dodgeCharges = 2, flasks = 3, kills = 0,
      selected = 0, page = 0;
  int joystick = -1, attackFinger = -1, attackSerial = 0, killStreak = 0;
  float streakTime = 0;
  float joyx = 76, joyy = 282;
  bool attacking = false, attackHit = false, gate = false, bossKilled = false,
       camReady = false;
  bool rooms[3] = {false, false, false};
  float roomX[3] = {19 * T, 39 * T, 53 * T},
        roomY[3] = {31 * T, 16 * T, 34 * T};
  float bossX = 59 * T, bossY = 12 * T, runPower = 20;
  int sfxQueue[8] = {}, sfxCount = 0, haptic = 0;
  std::array<uint8_t, MW * MH> map{}, ground{}, biomes{};
  std::array<int, MW * MH> distance{};
  std::vector<Enemy> enemies;
  std::vector<Bolt> bolts;
  std::vector<Particle> particles;
  std::vector<FloatText> floats;
  std::vector<Drop> drops;
  std::vector<Hazard> hazards;
  std::vector<Prop> props;
  std::vector<Animal> animals;
  std::vector<Echo> echoes;
} g;
struct WorldRecord {
  std::string id, name;
  uint64_t seed = 0;
  int level = 1, generator = 1;
  int64_t modified = 0;
  double seconds = 0;
  bool recovered = false;
};
struct Frontier {
  int generator = 1;
  bool atlasLarge = true;
  uint64_t seed = 1;
  int64_t originX = -36, originY = -26;
  std::string id, name, draftName = "MY FIRST WORLD", settingsCache;
  uint64_t draftSeed = 1;
  std::map<std::pair<int64_t, int64_t>, uint32_t> slain;
  std::vector<WorldRecord> worlds;
  int listPage = 0, selected = -1, textRequest = 0;
  float screenTime = 0, transition = 0, saveTimer = 0, stepTimer = 0;
  Scene transitionTo = HOME;
  bool transitionChanged = false, pendingCreate = false, dirty = false;
  double seconds = 0;
  int totalKills = 0, campsCleared = 0, discovered = 0;
  bool waypoint = false;
  int64_t waypointX = 0, waypointY = 0;
  int lastBiome = -1;
} o;
struct FallingTree {
  float x, y, time = 0;
  int species, biome, variant, dir;
};
struct Wildlands {
  double clockOffset = 0;
  int wood = 0, felled = 0;
  std::map<std::pair<int64_t, int64_t>, int> trees;
  std::map<std::pair<int64_t, int64_t>, std::array<uint64_t, 16>> explored;
  int64_t revealX = INT64_MAX, revealY = INT64_MAX;
  std::set<std::pair<int64_t, int64_t>> fires;
  bool atlasDirty = true, mapLimit = false;
  int mapZoom = 2;
  std::vector<FallingTree> falling;
} w;
bool motionBlur = true;
float renderDt = 1.f / 60;
std::string encodeWildlands();
bool decodeWildlands(const std::string &, Wildlands &);
bool treeCut(int64_t, int64_t);
bool treeCut(const Prop &);
int treeSpecies(const Prop &);
const char *treeName(int);
void revealTerrain();
bool exploredAt(int64_t, int64_t);
void wildlandsTick(float);
void chopTrees(float);
bool aimTree();
void drawNewTree(const Prop &);
void drawFallingTree(const FallingTree &);
void drawNewRanger(int, int, float, bool);
void drawNewEnemy(const Enemy &);
void drawNewPlants(const Prop &);
void stationDetail(const Prop &);
void drawNewRock(const Prop &);
void drawUnderstory(const Prop &);
void drawNewTerrain();
void drawDayLight();
void drawMotionBlur();
void drawWeaponRibbon();
void drawExploreAtlas();
bool knownBiomeWaypoint(int);
void woodsScreen();
void addPlayerFires();
bool woodsTouch(float, float);
const char *dayPhase();
enum Species {
  DEER,
  HARE,
  LIZARD,
  WOLF,
  BOAR,
  FOX,
  JACKAL,
  VULTURE,
  SNOWHARE,
  IBEX,
  SNOWWOLF,
  FROG,
  CROCODILE,
  RAVEN,
  GOAT,
  INSECT,
  SPECIES_COUNT
};
enum Behaviour { WANDER, GRAZE, DRINK, FLEE, STALK, FEED, SLEEP, ATTACKING };
enum Routine {
  DUTY,
  COOKING,
  EATING,
  WORKING,
  GATHERING,
  SLEEPING,
  FETCHING,
  WATCHING
};
struct CampLife {
  int food = 12, raw = 8, wood = 8, water = 12;
  double lastMeal = 0;
};
struct Survival {
  float stamina = 100, food = 100, water = 100;
  std::array<float, 6> injury{}, bleeding{};
  int meals = 3, cleanWater = 3, dirtyWater = 0, rawMeat = 0, berries = 0,
      bandages = 4, splints = 2, fiber = 0;
  bool sprint = false, hunting = true;
  int selectedPart = 1, lastPart = -1, journalBiome = 0, task = 0;
  float taskTime = 0, taskTotal = 0, hitMark = 0, thirstDamage = 0,
        actionNotice = 0;
  int hunts = 0, predatorKills = 0, grazingEvents = 0, scavenges = 0,
      routinesDone = 0;
  std::map<uint64_t, double> wildlifeGone, plantsUsed;
  std::map<std::pair<int64_t, int64_t>, CampLife> camps;
} v;
std::string encodeLife();
bool decodeLife(const std::string &, Survival &);
void survivalTick(float dt);
float movementFactor();
float weaponEffort();
bool useStamina(float n);
void bodyHit(int damage, float x, float y);
void updateEcology(float dt);
void seedWildlife();
void routineTick(Enemy &, float);
void lifeProps();
void ecologyArrow(Bolt &);
void huntMelee(float radius, float damage, bool all = false);
bool aimWildlife(float range, Enemy *enemy);
void drawLifeAnimal(const Animal &);
void drawLifeProp(const Prop &);
void drawRoutine(const Enemy &);
bool drawRoutineActor(const Enemy &);
C biomeColor(int);
C biomeAccent(int);
void drawSunShadows();
void drawAtmosphere();
void drawBodyMarks(int, int);
void survivalHud();
void survivalScreen();
void ecologyScreen();
void regionAtlas();
bool survivalTouch(float, float);
const char *biomeName(int b) {
  static const char *names[] = {"SUNVEIL FOREST", "AMBER DUNES", "FROSTWIND",
                                "MISTMARSH", "CINDER WASTES"};
  return names[std::clamp(b, 0, 4)];
}
void saveOpenWorld();
void saveSettings();
void loadSettings();
void flushOpenWorld();
void frontierTick(float dt);
void frontierStep(float dt);
void frontierHud();
void frontierMenu();
void frontierTerrain();
void frontierTerritories();
void frontierMap();
void frontierGuide();
void leaveFrontier();
void respawnFrontier();
void frontierKilled(Enemy &e);
bool frontierIdle(Enemy &e, float dt);
bool frontierTouch(float x, float y);
std::vector<C> cover;
const char *worldName() {
  if (g.openWorld)
    return biomeName(g.world);
  return g.world == 0 ? "SUNVEIL WILDS" : "EMBERFALL REACH";
}
const char *weaponName(int w) {
  return w == 0 ? "DAWNBLADE" : w == 1 ? "RIFT AXE" : "WIND BOW";
}
const char *bossName() {
  return g.world == 0 ? "CROWNHORN - WILD SOVEREIGN"
                      : "SOLKAR - SUNFORGED COLOSSUS";
}
void notify(const std::string &s) {
  g.toast = s;
  g.toastTime = 3.2f;
}
void sfx(int id) {
  if (g.muted)
    return;
  for (int i = 0; i < g.sfxCount; i++)
    if (g.sfxQueue[i] == id)
      return;
  if (g.sfxCount < 8)
    g.sfxQueue[g.sfxCount++] = id;
}
int equippedValue(int slot) {
  for (auto &i : g.bag)
    if (i.id == g.eq[slot])
      return i.value;
  return 0;
}
int maxhp() { return 120 + (g.level - 1) * 12 + equippedValue(1) * 5; }
int damage() { return 18 + g.level * 2 + g.upgrade * 3 + equippedValue(0) * 3; }
int armor() { return equippedValue(1) * 2; }
bool equipped(const Item &i) { return g.eq[i.slot] == i.id; }
std::string itemName(const Item &i) {
  if (i.slot == 0)
    return i.rarity >= 2 ? "SUNSTEEL CORE" : "WEAPON CORE";
  if (i.slot == 1)
    return i.rarity >= 2 ? "GROVEKEEPER MAIL" : "RANGER COAT";
  return i.value > 0 ? "CINDER HEART" : "FADED TALISMAN";
}
C rarityColor(int r) {
  return r == 3 ? GOLD : r == 2 ? 0xffcca5f7 : r == 1 ? TEAL : DIM;
}
void clearJourneyInput();
void clearInput() {
  ui7InputClear();
  clearJourneyInput();
  g.joystick = g.attackFinger = -1;
  g.mx = g.my = 0;
  g.attacking = false;
}
void resetCombat() {
  clearInput();
  g.vx = g.vy = 0;
  g.attackCd = g.attackTime = g.powerCd = g.dodgeCd = g.wardCd = g.healCd =
      g.slash = g.sweep = g.dash = g.ward = g.invul = g.hitstop = g.hurtTime =
          0;
  g.dodgeCharges = 2;
  g.dodgeRecharge = 0;
  g.combo = g.kills = g.killStreak = 0;
  g.flasks = 3;
  g.hp = float(maxhp());
  g.camReady = false;
  g.camx = g.camy = 0;
  g.overlay = 0;
  g.acc = g.navTimer = 0;
  g.bolts.clear();
  g.particles.clear();
  g.floats.clear();
  g.drops.clear();
  g.hazards.clear();
  g.echoes.clear();
}
float approach(float current, float target, float delta) {
  if (current < target)
    return std::min(target, current + delta);
  return std::max(target, current - delta);
}
float angleDelta(float a, float b) {
  float d = a - b;
  while (d > PI)
    d -= 2 * PI;
  while (d < -PI)
    d += 2 * PI;
  return d;
}
void floating(float x, float y, const std::string &s, C c = WHITE,
              int scale = 1) {
  if (g.floats.size() < 64)
    g.floats.push_back({x, y, 1.f, s, c, scale});
}
void burst(float x, float y, C c, int n, float strength = 1) {
  for (int i = 0; i < n && g.particles.size() < 320; i++) {
    float a = rnd(628) / 100.f, v = (15 + rnd(65)) * strength,
          l = .3f + rnd(55) / 100.f;
    g.particles.push_back({x, y, 8, std::cos(a) * v, std::sin(a) * v,
                           float(15 + rnd(70)), l, l, c, 2 + rnd(2)});
  }
}
void save();
void hub();
void expedition();
void hurt(int damage, float fromx, float fromy);
void hitEnemy(Enemy &e, float damage, float kx, float ky, bool heavy = false);
void attack();
void skill(int id);
} // namespace av
#include "journey_state.hpp"
