#pragma once
#include "frontier.hpp"
namespace av {
constexpr double DAY_SECONDS = 1440.0;
float worldHour() {
  return float(
      std::fmod(8.0 + (o.seconds + w.clockOffset) * 24.0 / DAY_SECONDS, 24.0));
}
const char *partName(int i) {
  const char *n[] = {"HEAD",      "TORSO",    "LEFT ARM",
                     "RIGHT ARM", "LEFT LEG", "RIGHT LEG"};
  return n[std::clamp(i, 0, 5)];
}
const char *speciesName(int i) {
  const char *n[] = {"DEER",      "HARE",  "LIZARD",    "WOLF",
                     "BOAR",      "FOX",   "JACKAL",    "VULTURE",
                     "SNOW HARE", "IBEX",  "SNOW WOLF", "FROG",
                     "CROCODILE", "RAVEN", "GOAT",      "INSECT"};
  return n[std::clamp(i, 0, SPECIES_COUNT - 1)];
}
const char *behaviourName(int i) {
  const char *n[] = {"ROAMING", "FEEDING",    "DRINKING", "FLEEING",
                     "HUNTING", "SCAVENGING", "RESTING",  "DEFENDING"};
  return n[std::clamp(i, 0, 7)];
}
const char *routineName(int i) {
  const char *n[] = {"PATROL",    "COOKING",  "EATING",      "WORKING",
                     "GATHERING", "SLEEPING", "FETCH WATER", "ON WATCH"};
  return n[std::clamp(i, 0, 7)];
}
bool finiteIn(double x, double a, double b) {
  return std::isfinite(x) && x >= a && x <= b;
}
std::string encodeLife() {
  std::ostringstream s;
  s << std::setprecision(15) << "LIFE1 " << v.stamina << ' ' << v.food << ' '
    << v.water << ' ' << v.meals << ' ' << v.cleanWater << ' ' << v.dirtyWater
    << ' ' << v.rawMeat << ' ' << v.berries << ' ' << v.bandages << ' '
    << v.splints << ' ' << v.fiber << ' ' << v.hunting << ' ' << v.hunts << ' '
    << v.predatorKills << ' ' << v.grazingEvents << ' ' << v.scavenges << ' '
    << v.routinesDone << '\n';
  for (int i = 0; i < 6; i++)
    s << v.injury[i] << ' ' << v.bleeding[i] << ' ';
  s << '\n';
  auto writeMap = [&](auto &m) {
    size_t count = 0;
    for (auto &[id, t] : m)
      if (t > o.seconds)
        count++;
    s << count << '\n';
    for (auto &[id, t] : m)
      if (t > o.seconds)
        s << id << ' ' << t << '\n';
  };
  writeMap(v.wildlifeGone);
  writeMap(v.plantsUsed);
  s << v.camps.size() << '\n';
  for (auto &[k, c] : v.camps)
    s << k.first << ' ' << k.second << ' ' << c.food << ' ' << c.raw << ' '
      << c.wood << ' ' << c.water << ' ' << c.lastMeal << '\n';
  return s.str();
}
bool decodeLife(const std::string &raw, Survival &out) {
  if (raw.size() > 8000000)
    return false;
  Survival a;
  std::istringstream f(raw);
  std::string magic;
  int hunt;
  if (!(f >> magic >> a.stamina >> a.food >> a.water >> a.meals >>
        a.cleanWater >> a.dirtyWater >> a.rawMeat >> a.berries >> a.bandages >>
        a.splints >> a.fiber >> hunt >> a.hunts >> a.predatorKills >>
        a.grazingEvents >> a.scavenges >> a.routinesDone) ||
      magic != "LIFE1")
    return false;
  if (!finiteIn(a.stamina, 0, 100) || !finiteIn(a.food, 0, 100) ||
      !finiteIn(a.water, 0, 100) || hunt < 0 || hunt > 1)
    return false;
  for (int value : {a.meals, a.cleanWater, a.dirtyWater, a.rawMeat, a.berries,
                    a.bandages, a.splints, a.fiber})
    if (value < 0 || value > 10000)
      return false;
  for (int value :
       {a.hunts, a.predatorKills, a.grazingEvents, a.scavenges, a.routinesDone})
    if (value < 0)
      return false;
  a.hunting = hunt;
  for (int i = 0; i < 6; i++)
    if (!(f >> a.injury[i] >> a.bleeding[i]) ||
        !finiteIn(a.injury[i], 0, 100) || !finiteIn(a.bleeding[i], 0, 2))
      return false;
  auto readMap = [&](auto &m) {
    size_t count;
    if (!(f >> count) || count > 20000)
      return false;
    for (size_t i = 0; i < count; i++) {
      uint64_t id;
      double time;
      if (!(f >> id >> time) || !finiteIn(time, 0, 1e12))
        return false;
      m[id] = time;
    }
    return true;
  };
  if (!readMap(a.wildlifeGone) || !readMap(a.plantsUsed))
    return false;
  size_t n;
  if (!(f >> n) || n > 200000)
    return false;
  for (size_t i = 0; i < n; i++) {
    int64_t x, y;
    CampLife c;
    if (!(f >> x >> y >> c.food >> c.raw >> c.wood >> c.water >> c.lastMeal) ||
        x < -WORLD_LIMIT || x > WORLD_LIMIT || y < -WORLD_LIMIT ||
        y > WORLD_LIMIT || c.food < 0 || c.food > 99 || c.raw < 0 ||
        c.raw > 99 || c.wood < 0 || c.wood > 99 || c.water < 0 ||
        c.water > 99 || !finiteIn(c.lastMeal, 0, 1e12))
      return false;
    a.camps[{x, y}] = c;
  }
  out = std::move(a);
  return true;
}
float staminaLimit() {
  return std::max(40.f, 100 - v.injury[0] * .12f - v.injury[1] * .2f -
                            (v.water < 20 ? 15 : 0));
}
bool useStamina(float n) {
  if (!g.openWorld)
    return true;
  if (v.task || v.stamina < n) {
    if (v.actionNotice <= 0) {
      notify(v.task ? "PREPARING - MOVE TO CANCEL"
                    : "EXHAUSTED - RECOVER STAMINA");
      v.actionNotice = 1.8f;
    }
    return false;
  }
  v.stamina -= n;
  return true;
}
float weaponEffort() {
  return !g.openWorld ? 1.f
                      : 1.f + (v.injury[2] + v.injury[3]) * .003f +
                            (v.food < 15 ? .18f : 0);
}
float movementFactor() {
  if (!g.openWorld)
    return 1;
  float injured = 1 - std::min(.48f, (v.injury[4] + v.injury[5]) * .0028f);
  float dehydrated = v.water < 15 ? .82f : 1.f;
  return injured * dehydrated *
         (v.sprint && v.stamina > 3 && v.task == 0 ? 1.62f : 1.f);
}
void bodyHit(int amount, float fromX, float fromY) {
  if (!g.openWorld)
    return;
  v.task = 0;
  v.taskTime = 0;
  v.sprint = false;
  float dx = fromX - g.px, dy = fromY - g.py, side = dx * g.fy - dy * g.fx;
  int roll = rnd(100), part = roll < 14   ? 0
                              : roll < 48 ? 1
                              : roll < 74 ? (side < 0 ? 2 : 3)
                                          : (side < 0 ? 4 : 5);
  float severity = clamp(float(amount) / maxhp() * 100, 3, 34);
  v.injury[part] = std::min(100.f, v.injury[part] + severity);
  if (amount >= maxhp() * .10f)
    v.bleeding[part] = std::min(.75f, v.bleeding[part] + severity * .009f);
  v.lastPart = part;
  v.hitMark = 5;
  v.selectedPart = part;
  o.dirty = true;
  if (v.injury[part] >= 60)
    notify(std::string(partName(part)) + " SEVERELY INJURED - OPEN BODY");
}
uint64_t plantKey(const Prop &p) {
  return coordinateHash(o.originX + int64_t(std::floor(p.x / T)),
                        o.originY + int64_t(std::floor(p.y / T)), 41501);
}
bool plantReady(const Prop &p) {
  auto it = v.plantsUsed.find(plantKey(p));
  return it == v.plantsUsed.end() || it->second <= o.seconds;
}
bool nearFire() {
  for (auto &p : g.props)
    if ((p.kind == 12 || p.kind == 22) && len(p.x - g.px, p.y - g.py) < 85)
      return true;
  return false;
}
bool nearWell() {
  for (auto &p : g.props)
    if (p.kind == 21 && len(p.x - g.px, p.y - g.py) < 55)
      return true;
  return false;
}
bool nearWater() {
  int px = int(g.px / T), py = int(g.py / T);
  for (int y = std::max(0, py - 2); y < std::min(MH, py + 3); y++)
    for (int x = std::max(0, px - 2); x < std::min(MW, px + 3); x++)
      if (g.map[y * MW + x] == 4 && g.biomes[y * MW + x] != 4 &&
          len(x * T + 12 - g.px, y * T + 12 - g.py) < 58)
        return true;
  return false;
}
void eatMeal() {
  if (v.food > 94) {
    notify("NOT HUNGRY YET");
    return;
  }
  if (v.meals > 0) {
    v.meals--;
    v.food = std::min(100.f, v.food + 38);
    notify("ATE A COOKED MEAL");
  } else if (v.berries > 0) {
    v.berries--;
    v.food = std::min(100.f, v.food + 14);
    v.water = std::min(100.f, v.water + 4);
    notify("ATE FORAGED FRUIT");
  } else {
    notify("NO FOOD - FORAGE OR HUNT AND COOK");
    return;
  }
  sfx(6);
  o.dirty = true;
}
void drinkWater() {
  if (v.water > 94) {
    notify("NOT THIRSTY YET");
    return;
  }
  if (v.cleanWater <= 0) {
    notify("NO CLEAN WATER - REFILL OR BOIL RAW WATER");
    return;
  }
  v.cleanWater--;
  v.water = std::min(100.f, v.water + 40);
  sfx(6);
  o.dirty = true;
  notify("DRANK CLEAN WATER");
}
void fillWater() {
  if (nearWell()) {
    v.cleanWater = 8;
    notify("PROTECTED WELL - CLEAN WATER REFILLED");
    sfx(6);
    o.dirty = true;
  } else if (nearWater()) {
    v.dirtyWater = 8;
    notify("RAW WATER COLLECTED - BOIL AT A FIRE");
    sfx(6);
    o.dirty = true;
  } else
    notify("STAND BESIDE WATER OR A CAMP WELL");
}
void bandagePart() {
  int i = v.selectedPart;
  if (v.bandages == 0 && v.fiber >= 3) {
    v.fiber -= 3;
    v.bandages++;
    notify("CRAFTED A BANDAGE FROM THREE FIBER");
    o.dirty = true;
    return;
  }
  if (v.bandages == 0) {
    notify("NO BANDAGES - FORAGE FIBER OR CLEAR CAMPS");
    return;
  }
  if (v.injury[i] < 1 && v.bleeding[i] <= 0) {
    notify("THIS BODY PART IS HEALTHY");
    return;
  }
  v.bandages--;
  v.bleeding[i] = 0;
  v.injury[i] = std::max(0.f, v.injury[i] - 12);
  sfx(6);
  o.dirty = true;
  notify(std::string(partName(i)) + " BANDAGED - BLEEDING STOPPED");
}
void splintPart() {
  int i = v.selectedPart;
  if (i < 2) {
    notify("SPLINTS ARE FOR INJURED LIMBS");
    return;
  }
  if (v.splints <= 0 || v.injury[i] < 35) {
    notify(v.splints <= 0 ? "NO SPLINTS - LOOT A CAMP CHEST"
                          : "NO SEVERE LIMB INJURY");
    return;
  }
  v.splints--;
  v.injury[i] = std::max(0.f, v.injury[i] - 35);
  o.dirty = true;
  notify(std::string(partName(i)) + " SUPPORTED - REST TO RECOVER");
  sfx(6);
}
void forage() {
  Prop *best = nullptr;
  float d = 72;
  for (auto &p : g.props)
    if ((p.kind == 20 || p.kind == 25) && plantReady(p)) {
      float n = len(p.x - g.px, p.y - g.py);
      if (n < d) {
        d = n;
        best = &p;
      }
    }
  if (!best) {
    notify("FIND A FRUIT SHRUB OR FIELD - STAND CLOSER");
    return;
  }
  v.plantsUsed[plantKey(*best)] = o.seconds + 240;
  v.berries = std::min(200, v.berries + 2);
  v.fiber = std::min(200, v.fiber + 1);
  o.dirty = true;
  sfx(3);
  dropMaterial(g.px,g.py,SEED,1);
  notify("FORAGED FRUIT / FIBER / SEED DROP");
}
void beginTask(int task) {
  if (!nearFire()) {
    notify("STAND NEAR A CAMPFIRE");
    return;
  }
  if (task == 1 && v.rawMeat < 1) {
    notify("HUNT AND COLLECT RAW MEAT FIRST");
    return;
  }
  if (task == 2 && v.cleanWater >= 8) {
    notify("CLEAN WATER STOCK IS FULL");
    return;
  }
  if (task == 2 && v.dirtyWater < 1) {
    notify("FILL RAW WATER BESIDE A WATER SOURCE");
    return;
  }
  if (task == 3 && (v.meals < 1 || v.cleanWater < 1)) {
    notify("REST NEEDS ONE MEAL AND ONE CLEAN WATER");
    return;
  }
  v.task = task;
  v.taskTime = v.taskTotal = task == 3 ? 8 : task == 1 ? 4 : 3;
  g.overlay = 0;
  clearInput();
  v.sprint = false;
  notify(task == 1   ? "COOKING MEAT..."
         : task == 2 ? "BOILING WATER..."
                     : "RESTING - MOVEMENT OR DAMAGE CANCELS");
}
void survivalTick(float dt) {
  if (!g.openWorld || g.scene != PLAY)
    return;
  v.hitMark = std::max(0.f, v.hitMark - dt);
  v.actionNotice = std::max(0.f, v.actionNotice - dt);
  bool moving = len(g.mx, g.my) > .1f;
  if (v.task) {
    if (moving || !nearFire()) {
      v.task = 0;
      notify("PREPARATION CANCELLED");
    } else {
      g.attacking = false;
      g.vx = g.vy = 0;
      v.taskTime -= dt;
      if (v.taskTime <= 0) {
        if (v.task == 1 && v.rawMeat > 0) {
          v.rawMeat--;
          v.meals = std::min(200, v.meals + 1);
          notify("COOKED MEAL READY");
        }
        if (v.task == 2 && v.dirtyWater > 0) {
          int n = std::min({3, v.dirtyWater, 8 - v.cleanWater});
          v.dirtyWater -= n;
          v.cleanWater += n;
          notify("WATER BOILED AND BOTTLED");
        }
        if (v.task == 3 && v.meals > 0 && v.cleanWater > 0) {
          v.meals--;
          v.cleanWater--;
          v.food = std::min(100.f, v.food + 20);
          v.water = std::min(100.f, v.water + 20);
          g.hp = std::min(float(maxhp()), g.hp + maxhp() * .35f);
          g.flasks = 3;
          for (auto &i : v.injury)
            i = std::max(0.f, i - 18);
          v.stamina = staminaLimit();
          notify("RESTED - BANDAGE ANY REMAINING BLEEDING");
        }
        v.task = 0;
        o.dirty = true;
        sfx(6);
      }
    }
  }
  bool running = moving && v.sprint && v.stamina > 3 && v.task == 0;
  if (running)
    v.stamina = std::max(0.f, v.stamina - dt * 17);
  else
    v.stamina =
        std::min(staminaLimit(), v.stamina + dt * (v.water < 15 ? 3.f : 10.f) *
                                                 (v.food < 15 ? .55f : 1.f));
  if (v.stamina <= 3)
    v.sprint = false;
  v.food = std::max(0.f, v.food - dt * (running ? .21f : .105f));
  float thirst = g.world == 1   ? .33f
                 : g.world == 4 ? .37f
                 : g.world == 2 ? .15f
                                : .20f;
  v.water = std::max(0.f, v.water - dt * thirst * (running ? 1.6f : 1));
  float bleed = 0;
  for (float b : v.bleeding)
    bleed += b;
  float loss = bleed + (v.water <= 0 ? 1.25f : 0) + (v.food <= 0 ? .45f : 0);
  g.hp = std::max(0.f, g.hp - loss * dt);
  if (g.hp <= 0) {
    g.scene = DEAD;
    clearInput();
    v.task = 0;
    notify(bleed > 0 ? "YOU COLLAPSED FROM YOUR WOUNDS"
                     : "YOU COLLAPSED - FOOD AND WATER MATTER");
    save();
  }
}
// Ecological roles are deliberately simplified. Insects represent the small
// primary-consumer layer; carrion and timed plant regrowth close the local
// loop.
bool flying(int s) { return s == VULTURE || s == RAVEN || s == INSECT; }
bool herbivore(int s) {
  return s == DEER || s == HARE || s == SNOWHARE || s == IBEX || s == GOAT ||
         s == INSECT || s == BOAR;
}
bool scavenger(int s) { return s == VULTURE || s == RAVEN; }
bool eatsSpecies(int a, int b) {
  if (a == LIZARD || a == FROG)
    return b == INSECT;
  if (a == FOX || a == JACKAL)
    return b == HARE || b == SNOWHARE || b == LIZARD || b == FROG ||
           b == INSECT;
  if (a == WOLF || a == SNOWWOLF)
    return b == DEER || b == HARE || b == SNOWHARE || b == IBEX || b == GOAT ||
           b == BOAR;
  if (a == CROCODILE)
    return b == BOAR || b == DEER || b == FROG || b == LIZARD;
  return false;
}
float animalMaxHP(int s) {
  const float hp[] = {65, 23, 20,  85, 140, 42, 55, 28,
                      24, 75, 100, 16, 175, 22, 70, 3};
  return hp[s];
}
float animalSpeed(int s) {
  const float speed[] = {172, 190, 110, 158, 145, 182, 165, 172,
                         198, 170, 165, 115, 92,  190, 165, 75};
  return speed[s];
}
bool animalFits(const Animal &a, float x, float y) {
  if (x < 10 || y < 10 || x > MW * T - 10 || y > MH * T - 10)
    return false;
  if (flying(a.species))
    return true;
  if (a.species == CROCODILE) {
    int ix = int(x / T), iy = int(y / T);
    int m = g.map[iy * MW + ix];
    return fits(x, y, 5) || (m == 4 && g.biomes[iy * MW + ix] != 4);
  }
  int src=int(a.y/T)*MW+int(a.x/T),dst=int(y/T)*MW+int(x/T);
  return fits(x, y, a.species == INSECT ? 2 : 5) && src>=0 && src<MW*MH && dst>=0 && dst<MW*MH && terrainLink(src,dst);
}
void animalMove(Animal &a, float dx, float dy, float speed, float dt) {
  float d = len(dx, dy);
  if (d < .8f)
    return;
  dx /= d;
  dy /= d;
  // Steering scores alternatives instead of oscillating against an obstacle.
  float best = -1e8f, bx = 0, by = 0;
  for (int k : {0, 1, -1, 2, -2, 3, -3, 4}) {
    float angle = k * PI / 4;
    float xx = dx * std::cos(angle) - dy * std::sin(angle),
          yy = dx * std::sin(angle) + dy * std::cos(angle);
    float look = std::max(14.f, speed * dt + 7);
    if (!animalFits(a, a.x + xx * look, a.y + yy * look))
      continue;
    float score = xx * dx + yy * dy + .15f * (xx * a.dx + yy * a.dy);
    if (score > best) {
      best = score;
      bx = xx;
      by = yy;
    }
  }
  if (best < -100)
    return;
  a.dx = bx;
  a.dy = by;
  float ox = a.x, oy = a.y;
  int steps = std::max(1, int(speed * dt / 4) + 1);
  for (int k = 0; k < steps; k++) {
    float xx = a.x + bx * speed * dt / steps,
          yy = a.y + by * speed * dt / steps;
    if (animalFits(a, xx, yy)) {
      a.x = xx;
      a.y = yy;
    }
  }
  a.step += len(a.x - ox, a.y - oy) * .20f;
}
void killAnimal(Animal &a, int killer) {
  if (!a.alive)
    return;
  a.alive = false;
  a.hp = 0;
  a.deadTime = 0;
  a.fear = 0;
  a.behaviour = FEED;
  v.wildlifeGone[a.id] = o.seconds + DAY_SECONDS;
  if (killer == 0) {
    v.hunts++;
    floating(a.x, a.y - 25, "HUNTED", GOLD);
    sfx(2);
  } else
    v.predatorKills++;
  o.dirty = true;
}
void hitAnimal(Animal &a, float damage, int killer = 0) {
  if (!a.alive)
    return;
  a.hp -= damage;
  a.flash = .16f;
  a.fear = 10;
  if (killer == 0)
    a.anger = 10;
  if (killer == 0) {
    burst(a.x, a.y - 5, 0xffd2bc91, 6);
    g.hitstop = std::max(g.hitstop, .025f);
    sfx(2);
  }
  if (a.hp <= 0)
    killAnimal(a, killer);
}
bool aimWildlife(float range, Enemy *enemy) {
  if (!g.openWorld || !v.hunting)
    return false;
  float best = enemy ? len(enemy->x - g.px, enemy->y - g.py) : range;
  if (enemy && enemy->alertTime > 0 && best < 160)
    return false;
  Animal *target = nullptr;
  for (auto &a : g.animals)
    if (a.alive && a.species != INSECT) {
      float d = len(a.x - g.px, a.y - g.py);
      if (d < best && sight(g.px, g.py, a.x, a.y)) {
        target = &a;
        best = d;
      }
    }
  if (!target)
    return false;
  float d = std::max(1.f, best);
  g.aimx = (target->x - g.px) / d;
  g.aimy = (target->y - g.py) / d;
  return true;
}
void huntMelee(float radius, float amount, bool all) {
  if (!g.openWorld || !v.hunting)
    return;
  for (auto &a : g.animals)
    if (a.alive) {
      float dx = a.x - g.px, dy = a.y - g.py, d = len(dx, dy);
      if (d < radius &&
          (all || (dx * g.aimx + dy * g.aimy) / std::max(1.f, d) > -.15f ||
           d < 22) &&
          sight(g.px, g.py, a.x, a.y))
        hitAnimal(a, amount);
    }
}
void ecologyArrow(Bolt &b) {
  if (!g.openWorld || !v.hunting || b.life <= 0)
    return;
  for (auto &a : g.animals)
    if (a.alive &&
        std::find(b.hitTargets.begin(), b.hitTargets.end(), a.id) ==
            b.hitTargets.end() &&
        len(b.x - a.x, b.y - a.y) < (a.species == INSECT ? 5 : 14)) {
      if (b.hitCount >= 4) {
        b.life = 0;
        break;
      }
      b.hitTargets[b.hitCount++] = a.id;
      hitAnimal(a, float(b.damage));
      if (b.pierce-- <= 0 || b.hitCount >= 4) {
        b.life = 0;
        break;
      }
    }
}
void seedWildlife() {
  std::erase_if(v.wildlifeGone, [](auto &p) { return p.second <= o.seconds; });
  std::erase_if(v.plantsUsed, [](auto &p) { return p.second <= o.seconds; });
  static const int table[5][10] = {
      {DEER, HARE, HARE, BOAR, FOX, WOLF, RAVEN, DEER, INSECT, INSECT},
      {HARE, LIZARD, LIZARD, JACKAL, VULTURE, GOAT, INSECT, INSECT, HARE, IBEX},
      {SNOWHARE, IBEX, IBEX, SNOWWOLF, RAVEN, SNOWHARE, GOAT, INSECT, INSECT,
       FOX},
      {FROG, FROG, BOAR, DEER, CROCODILE, LIZARD, RAVEN, INSECT, INSECT,
       INSECT},
      {GOAT, LIZARD, GOAT, JACKAL, VULTURE, HARE, INSECT, INSECT, LIZARD,
       GOAT}};
  for (int y = 2; y < MH - 2; y++)
    for (int x = 2; x < MW - 2; x++) {
      int i = y * MW + x;
      if (g.map[i] != 1)
        continue;
      uint64_t h = coordinateHash(o.originX + x, o.originY + y, 6147);
      if (h % 131 != 0 || g.animals.size() >= 72)
        continue;
      uint64_t id = h | 0x8000000000000000ULL;
      if (v.wildlifeGone.count(id))
        continue;
      bool exists = false;
      for (auto &a : g.animals)
        if (a.id == id) {
          exists = true;
          break;
        }
      if (exists)
        continue;
      Animal a;
      a.id = id;
      a.species = table[g.biomes[i]][(h >> 12) % 10];
      a.biome = g.biomes[i];
      a.kind = a.species == DEER                            ? 0
               : a.species == HARE || a.species == SNOWHARE ? 1
                                                            : 2;
      a.x = a.homeX = a.goalX = x * T + 12;
      a.y = a.homeY = a.goalY = y * T + 12;
      a.hp = a.maxhp = animalMaxHP(a.species);
      a.hunger = 35 + (h >> 20) % 40;
      a.thirst = 45 + (h >> 27) % 40;
      a.decision = float((h >> 32) % 80) / 10;
      a.timer = 2;
      a.meat = a.species == INSECT                                           ? 1
               : a.species == DEER || a.species == GOAT || a.species == IBEX ? 3
               : a.species == BOAR || a.species == CROCODILE ? 4
                                                             : 1;
      g.animals.push_back(a);
    }
}
void lifeProps() {
  size_t old = g.props.size();
  for (size_t i = 0; i < old; i++) {
    auto p = g.props[i];
    if (p.kind == 11) {
      g.props.push_back(
          {p.x - 45, p.y + 12, 22, 0, false, p.biome, p.campX, p.campY});
      g.props.push_back(
          {p.x + 58, p.y - 35, 23, 0, false, p.biome, p.campX, p.campY});
      g.props.push_back(
          {p.x - 56, p.y - 52, 24, 0, false, p.biome, p.campX, p.campY});
      g.props.push_back(
          {p.x + 14, p.y - 75, 26, 0, false, p.biome, p.campX, p.campY});
      g.props.push_back(
          {p.x + 88, p.y + 38, 25, 0, false, p.biome, p.campX, p.campY});
      g.props.push_back(
          {p.x - 86, p.y + 52, 21, 0, false, p.biome, p.campX, p.campY});
    }
    if (p.kind == 12) {
      g.props.push_back({p.x + 43, p.y - 7, 21, 0, false, 0});
      g.props.push_back({p.x - 47, p.y + 5, 23, 0, false, 0});
      g.props.push_back({p.x - 32, p.y - 45, 24, 0, false, 0});
    }
  }
  for (int y = 2; y < MH - 2; y++)
    for (int x = 2; x < MW - 2; x++)
      if (g.map[y * MW + x] == 1 && g.ground[y * MW + x] == 0) {
        uint64_t h = coordinateHash(o.originX + x, o.originY + y, 553);
        if (h % 101 == 0)
          g.props.push_back({float(x * T + 12), float(y * T + 12), 20,
                             int(h % 3), false, int(g.biomes[y * MW + x])});
      }
  // Origin plants make the survival loop discoverable without free perpetual
  // meals.
  for (int j = 0; j < 3; j++) {
    float x = float(-o.originX) * T - 58 + j * 24,
          y = float(-o.originY) * T + 56;
    if (x > 24 && y > 24 && x < MW * T - 24 && y < MH * T - 24)
      g.props.push_back({x, y, 20, j, false, 0});
  }
}
void updateEcology(float dt) {
  float hour = worldHour();
  for (size_t i = 0; i < g.animals.size(); i++) {
    auto &a = g.animals[i];
    if (a.species < 0)
      continue;
    a.flash = std::max(0.f, a.flash - dt);
    a.anger = std::max(0.f, a.anger - dt);
    a.cooldown = std::max(0.f, a.cooldown - dt);
    a.decision -= dt;
    if (!a.alive) {
      a.deadTime += dt;
      if (a.species != INSECT && a.meat > 0 &&
          len(a.x - g.px, a.y - g.py) < 28) {
        v.rawMeat = std::min(200, v.rawMeat + a.meat);
        a.meat = 0;
        notify("RAW MEAT COLLECTED - COOK AT A FIRE");
        sfx(3);
        o.dirty = true;
      }
      continue;
    }
    a.hunger = std::max(0.f, a.hunger - .055f * dt);
    a.thirst = std::max(0.f, a.thirst - .065f * dt);
    float pd = len(a.x - g.px, a.y - g.py);
    bool bold = a.species == WOLF || a.species == SNOWWOLF ||
                a.species == BOAR || a.species == CROCODILE;
    bool hostile = bold && ((a.anger > 0 && pd < 220) ||
                            (a.species == CROCODILE && pd < 58) ||
                            (a.hunger < 18 && pd < 140));
    float threatX = g.px, threatY = g.py, threat = pd;
    bool danger =
        !bold && a.species != INSECT && pd < (flying(a.species) ? 130 : 190);
    for (size_t j = 0; j < g.animals.size(); j++)
      if (i != j && g.animals[j].alive &&
          eatsSpecies(g.animals[j].species, a.species)) {
        float d = len(a.x - g.animals[j].x, a.y - g.animals[j].y);
        if (d < 150 && d < threat) {
          danger = true;
          threat = d;
          threatX = g.animals[j].x;
          threatY = g.animals[j].y;
        }
      }
    for (auto &e : g.enemies)
      if (e.alive && e.kind == 2) {
        float d = len(a.x - e.x, a.y - e.y);
        if (!bold && d < 130 && d < threat) {
          danger = true;
          threat = d;
          threatX = e.x;
          threatY = e.y;
        }
      }
    if (danger) {
      a.fear = 8;
      a.goalX = threatX;
      a.goalY = threatY;
    } else if (pd > 280 || bold)
      a.fear = std::max(0.f, a.fear - dt);
    if (hostile) {
      a.behaviour = ATTACKING;
      animalMove(a, g.px - a.x, g.py - a.y, animalSpeed(a.species) * .85f, dt);
      if (pd < 31 && a.cooldown <= 0) {
        hurt(a.species == CROCODILE ? 22
             : a.species == BOAR    ? 15
                                    : 12,
             a.x, a.y);
        a.cooldown = 1.6f;
      }
      continue;
    }
    if (a.fear > 0 && (!bold || a.species == BOAR)) {
      a.behaviour = FLEE;
      animalMove(a, a.x - a.goalX, a.y - a.goalY, animalSpeed(a.species), dt);
      continue;
    }
    // Small carnivores hunt insects; larger predators select their actual prey
    // species.
    Animal *meal = nullptr;
    float foodDist = 420;
    if (a.hunger < 72) {
      for (size_t j = 0; j < g.animals.size(); j++)
        if (i != j) {
          auto &b = g.animals[j];
          bool edible =
              b.alive ? eatsSpecies(a.species, b.species)
                      : b.meat > 0 &&
                            (scavenger(a.species) ||
                             eatsSpecies(a.species, b.species) ||
                             a.species == BOAR || a.species == WOLF ||
                             a.species == JACKAL || a.species == SNOWWOLF ||
                             a.species == CROCODILE);
          if (edible) {
            float d = len(a.x - b.x, a.y - b.y);
            if (d < foodDist) {
              foodDist = d;
              meal = &b;
            }
          }
        }
    }
    if (meal) {
      a.behaviour = meal->alive ? STALK : FEED;
      animalMove(a, meal->x - a.x, meal->y - a.y,
                 animalSpeed(a.species) * (meal->alive ? .88f : .35f), dt);
      if (foodDist < 24) {
        if (meal->alive) {
          if (a.cooldown <= 0) {
            hitAnimal(*meal, a.species == FROG || a.species == LIZARD ? 5 : 22,
                      1);
            a.cooldown = 1.1f;
          }
        } else {
          a.eatTimer += dt;
          if (a.eatTimer > 3) {
            a.eatTimer = 0;
            meal->meat--;
            a.hunger = std::min(100.f, a.hunger + 35);
            v.scavenges++;
            o.dirty = true;
          }
        }
      }
      continue;
    }
    if (a.thirst < 50 && !flying(a.species)) {
      int ax = int(a.x / T), ay = int(a.y / T);
      float best = 350, wx = a.x, wy = a.y;
      bool found = false;
      for (int y = std::max(1, ay - 12); y < std::min(MH - 1, ay + 13); y += 2)
        for (int x = std::max(1, ax - 12); x < std::min(MW - 1, ax + 13);
             x += 2)
          if (g.map[y * MW + x] == 4 && g.biomes[y * MW + x] != 4) {
            float d = len(x * T + 12 - a.x, y * T + 12 - a.y);
            if (d < best) {
              best = d;
              wx = x * T + 12;
              wy = y * T + 12;
              found = true;
            }
          }
      if (found) {
        a.behaviour = DRINK;
        if (best < 43) {
          a.thirst = std::min(100.f, a.thirst + 12 * dt);
          a.step += dt;
        } else
          animalMove(a, wx - a.x, wy - a.y, animalSpeed(a.species) * .28f, dt);
        continue;
      }
    }
    if ((herbivore(a.species) || a.species == RAVEN) && a.hunger < 80) {
      Prop *plant = nullptr;
      float best = 380;
      for (auto &p : g.props)
        if ((p.kind == 20 || p.kind == 25) && plantReady(p)) {
          float d = len(a.x - p.x, a.y - p.y);
          if (d < best) {
            best = d;
            plant = &p;
          }
        }
      if (plant) {
        a.behaviour = GRAZE;
        if (best > 19)
          animalMove(a, plant->x - a.x, plant->y - a.y,
                     animalSpeed(a.species) * .25f, dt);
        else {
          a.eatTimer += dt;
          a.step += dt * .6f;
          if (a.eatTimer > 4) {
            a.eatTimer = 0;
            a.hunger = std::min(100.f, a.hunger + 32);
            v.plantsUsed[plantKey(*plant)] = o.seconds + 150;
            v.grazingEvents++;
            o.dirty = true;
          }
        }
        continue;
      }
    }
    bool sleep = (hour > 21 || hour < 5) && a.species != WOLF &&
                 a.species != SNOWWOLF && a.species != FOX &&
                 a.species != JACKAL;
    if (sleep || a.hunger > 82) {
      a.behaviour = SLEEP;
      continue;
    }
    a.behaviour = WANDER;
    if (a.decision <= 0) {
      uint64_t h = mix64(a.id + uint64_t(o.seconds / 7));
      float angle = float(h % 628) / 100;
      a.goalX = a.homeX + std::cos(angle) * float(35 + (h >> 12) % 100);
      a.goalY = a.homeY + std::sin(angle) * float(35 + (h >> 12) % 100);
      a.decision = 7 + float(h % 70) / 10;
    }
    animalMove(a, a.goalX - a.x, a.goalY - a.y, animalSpeed(a.species) * .18f,
               dt);
  }
  // Corpses decay; a nearby grazed plant becomes fertile again. No offscreen
  // global simulation.
  for (auto &a : g.animals)
    if (!a.alive && a.deadTime > 150)
      for (auto &p : g.props)
        if (p.kind == 20 && len(a.x - p.x, a.y - p.y) < 70)
          v.plantsUsed.erase(plantKey(p));
  std::erase_if(g.animals,
                [](auto &a) { return !a.alive && a.deadTime > 150; });
}
void routineTick(Enemy &e, float dt) {
  float hour = worldHour();
  e.routineAnim += dt;
  e.needFood = std::max(0.f, e.needFood - dt * .08f);
  auto &stock = v.camps[{e.campX, e.campY}];
  int role = e.slot % 6, routine = WORKING;
  if (e.boss())
    routine = WATCHING;
  else if (e.kind == 2)
    routine = hour > 21 || hour < 5 ? SLEEPING : GATHERING;
  else if (role == 0)
    routine = WATCHING;
  else if (hour > 21 || hour < 5.5f)
    routine = SLEEPING;
  else if (role == 1)
    routine = COOKING;
  else if ((hour > 7 && hour < 9) || (hour > 18 && hour < 20) ||
           e.needFood < 25)
    routine = EATING;
  else
    routine = role == 2   ? GATHERING
              : role == 3 ? WORKING
              : role == 4 ? FETCHING
                          : DUTY;
  if (e.routine != routine) {
    e.routine = routine;
    e.chore = 0;
  }
  float tx = e.homeX, ty = e.homeY;
  if (routine == COOKING) {
    tx -= 45;
    ty += 22;
  }
  if (routine == WORKING) {
    tx += 58;
    ty -= 23;
  }
  if (routine == EATING) {
    tx += 14 + (e.slot % 3 - 1) * 19;
    ty -= 57;
  }
  if (routine == SLEEPING) {
    tx -= 56 + (e.slot % 3) * 15;
    ty -= 44 - (e.slot % 2) * 22;
  }
  if (routine == FETCHING) {
    tx -= 86;
    ty += 58;
  }
  if (routine == GATHERING) {
    bool carry = (e.routineStep % 2) == 1;
    tx += carry ? 58 : 88;
    ty += carry ? -23 : 38;
  }
  if (routine == WATCHING || routine == DUTY) {
    int point = int((o.seconds / 18 + e.slot)) % 4;
    float range = e.boss() ? 25 : std::min(100.f, e.territory * .65f);
    tx += (point == 0 ? -range : point == 2 ? range : 0);
    ty += (point == 1 ? -range : point == 3 ? range : 0);
  }
  if (e.kind == 2 && routine == GATHERING) {
    Animal *prey = nullptr;
    float best = 180;
    for (auto &a : g.animals)
      if (a.alive && !flying(a.species) && a.species != CROCODILE) {
        float d = len(a.x - e.x, a.y - e.y);
        if (d < best && len(a.x - e.homeX, a.y - e.homeY) < e.territory) {
          best = d;
          prey = &a;
        }
      }
    if (prey) {
      tx = prey->x;
      ty = prey->y;
      if (best < 24) {
        e.chore += dt;
        if (e.chore > 1.5f) {
          e.chore = 0;
          hitAnimal(*prey, 18, 2);
          if (!prey->alive) {
            stock.raw = std::min(99, stock.raw + 2);
            prey->meat = 0;
          }
        }
      }
    }
  }
  if (routine != DUTY && routine != WATCHING && routine != SLEEPING) {
    tx += (e.slot / 6) * 13;
    ty += (e.slot / 6) * 8;
  }
  float dx = tx - e.x, dy = ty - e.y, d = len(dx, dy);
  if (d > 9) {
    float ox = e.x, oy = e.y;
    move(e.x, e.y, dx / d * 38 * dt, dy / d * 38 * dt, e.boss() ? 14 : 7);
    e.step += len(e.x - ox, e.y - oy) * .14f;
    return;
  }
  e.chore += dt;
  if (e.chore < 6)
    return;
  e.chore = 0;
  e.routineStep++;
  bool changed = false;
  if (routine == COOKING && stock.raw > 0 && stock.wood > 0 &&
      stock.water > 0 && stock.food < 18) {
    stock.raw--;
    stock.wood--;
    stock.water--;
    stock.food = std::min(99, stock.food + 2);
    changed = true;
  }
  if (routine == EATING && stock.food > 0 && e.needFood < 92) {
    stock.food--;
    e.needFood = 100;
    stock.lastMeal = o.seconds;
    changed = true;
  }
  if (routine == GATHERING && e.kind != 2 && (e.routineStep % 2) == 1) {
    for (auto &p : g.props)
      if (p.kind == 25 && p.campX == e.campX && p.campY == e.campY &&
          plantReady(p)) {
        stock.raw = std::min(99, stock.raw + 2);
        v.plantsUsed[plantKey(p)] = o.seconds + 60;
        changed = true;
        break;
      }
  }
  if (routine == WORKING) {
    stock.wood = std::min(99, stock.wood + 1);
    changed = true;
  }
  if (routine == FETCHING) {
    stock.water = std::min(99, stock.water + 2);
    changed = true;
  }
  if (changed) {
    v.routinesDone++;
    o.dirty = true;
  }
}
} // namespace av
