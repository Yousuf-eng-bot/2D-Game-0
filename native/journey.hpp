#pragma once
namespace av {
void clearJourneyInput() {
  j.queue = 0;
  j.dodgeQueue = 0;
  j.guardFinger = -1;
  j.guard = 0;
}
const char *materialName(int k) {
  static const char *n[] = {
      "LOG",   "STICK", "STONE",  "COAL", "IRON ORE",  "IRON INGOT",
      "PLANK", "TORCH", "WALL",   "DOOR", "WORKBENCH", "FURNACE",
      "CHEST", "BED",   "BRIDGE", "SEED", "GRAIN",     "DIRT"};
  return k >= 0 && k < MATERIALS ? n[k] : "UNKNOWN";
}
int materialCount(int k) { return k == LOG ? w.wood : j.stock[k]; }
void changeMaterial(int k, int n) {
  if (k == LOG)
    w.wood = std::clamp(w.wood + n, 0, 1000000);
  else
    j.stock[k] = std::clamp(j.stock[k] + n, 0, 1000000);
  o.dirty = true;
}
int occupiedSlots() {
  int n = 0;
  for (int k = 0; k < MATERIALS; k++)
    n += (materialCount(k) + 63) / 64;
  return n;
}
bool roomFor(int k, int n) {
  int old = materialCount(k);
  return n >= 0 && old + n <= 1000000 &&
         occupiedSlots() - (old + 63) / 64 + (old + n + 63) / 64 <= 24;
}
bool validWorldKey(int64_t x, int64_t y) {
  return x >= -WORLD_LIMIT && x <= WORLD_LIMIT && y >= -WORLD_LIMIT &&
         y <= WORLD_LIMIT;
}
std::string encodeJourney() {
  std::ostringstream s;
  s << std::setprecision(15) << "JOURNEY1 " << j.home << ' ' << j.homeX << ' '
    << j.homeY << '\n';
  for (int v : j.stock)
    s << v << ' ';
  for (int v : j.toolTier)
    s << v << ' ';
  for (int v : j.durability)
    s << v << ' ';
  s << '\n';
  for (auto *m : {&j.mined, &j.rockHP}) {
    s << m->size() << '\n';
    for (auto &[k, v] : *m)
      s << k.first << ' ' << k.second << ' ' << v << '\n';
  }
  s << j.built.size() << '\n';
  for (auto &[k, b] : j.built) {
    s << k.first << ' ' << k.second << ' ' << b.kind << ' ' << b.open << ' '
      << b.progress;
    for (int v : b.storage)
      s << ' ' << v;
    s << '\n';
  }
  s << j.drops.size() << '\n';
  for (auto &d : j.drops)
    s << d.x << ' ' << d.y << ' ' << d.kind << ' ' << d.count << ' ' << d.age
      << '\n';
  return s.str();
}
bool decodeJourney(const std::string &raw, Journey &dest) {
  if (raw.size() > 4 * 1024 * 1024)
    return false;
  Journey a;
  std::istringstream f(raw);
  std::string magic;
  int home;
  size_t count;
  if (!(f >> magic >> home >> a.homeX >> a.homeY) || magic != "JOURNEY1" ||
      home < 0 || home > 1 || !validWorldKey(a.homeX, a.homeY))
    return false;
  a.home = home;
  for (int &v : a.stock)
    if (!(f >> v) || v < 0 || v > 1000000)
      return false;
  for (int &v : a.toolTier)
    if (!(f >> v) || v < 0 || v > 3)
      return false;
  for (int &v : a.durability)
    if (!(f >> v) || v < 0 || v > 600)
      return false;
  for (auto *m : {&a.mined, &a.rockHP}) {
    if (!(f >> count) || count > 20000)
      return false;
    for (size_t i = 0; i < count; i++) {
      int64_t x, y;
      int v;
      if (!(f >> x >> y >> v) || !validWorldKey(x, y) || v < 0 || v > 20)
        return false;
      (*m)[{x, y}] = v;
    }
  }
  if (!(f >> count) || count > 4096)
    return false;
  for (size_t i = 0; i < count; i++) {
    int64_t x, y;
    Structure b;
    int open;
    if (!(f >> x >> y >> b.kind >> open >> b.progress) ||
        !validWorldKey(x, y) || b.kind < TORCH || b.kind > 18 ||
        b.kind == SEED || b.kind == GRAIN || open < 0 || open > 1 ||
        !finiteIn(b.progress, 0, 100000))
      return false;
    b.open = open;
    for (int &v : b.storage)
      if (!(f >> v) || v < 0 || v > 1000000)
        return false;
    a.built[{x, y}] = b;
  }
  if (!(f >> count) || count > 512)
    return false;
  for (size_t i = 0; i < count; i++) {
    GroundStack d;
    if (!(f >> d.x >> d.y >> d.kind >> d.count >> d.age) ||
        !finiteIn(d.x, -WORLD_LIMIT, WORLD_LIMIT) ||
        !finiteIn(d.y, -WORLD_LIMIT, WORLD_LIMIT) || d.kind < 0 ||
        d.kind >= MATERIALS || d.count < 1 || d.count > 4096 ||
        !finiteIn(d.age, 0, 1201))
      return false;
    d.z = 0;
    d.vz = 0;
    a.drops.push_back(d);
  }
  f >> std::ws;
  if (!f.eof())
    return false;
  dest = std::move(a);
  return true;
}
float riverDistance(int64_t x, int64_t y) {
  double phase = (o.seed % 1000) * .01;
  double bend = std::sin(double(x) / 85 + phase) * 24 +
                std::sin(double(x) / 237 + phase) * 21;
  double band = std::round((double(y) - bend) / 220);
  double main = std::abs(double(y) - (band * 220 + bend));
  double creek = std::abs(
      double(x) -
      (std::round((double(x) - std::sin(double(y) / 51 + phase) * 13) / 310) *
           310 +
       std::sin(double(y) / 51 + phase) * 13));
  return float(std::min(main, creek + 1.8));
}
int landHeight(int64_t x, int64_t y) {
  if (o.generator < 4 || std::abs(x) < 12 && std::abs(y) < 12)
    return 0;
  float n = noiseAt(x, y, 32, 90131) * .62f + noiseAt(x, y, 96, 90877) * .38f;
  float height = clamp((n - .24f) * 9, 0, 5);
  float originFade =
      clamp(float(std::max(std::abs(x), std::abs(y)) - 11) / 18, 0, 1);
  int h = int(height * originFade);
  float river = riverDistance(x, y);
  h = std::min(h, std::max(0, int((river - 2.7f) / 2)));
  auto key = std::make_pair(x, y);
  auto dug = j.mined.find(key);
  if (dug != j.mined.end() && dug->second == 2)
    h = std::max(0, h - 1);
  auto raised = j.built.find(key);
  if (raised != j.built.end() && raised->second.kind == DIRT)
    h++;
  return h;
}
void modifyJourneyTile(int64_t x, int64_t y, uint8_t &map, uint8_t &ground,
                       uint8_t &biome) {
  auto key = std::make_pair(x, y);
  if (o.generator >= 4 && !(std::abs(x) < 12 && std::abs(y) < 12)) {
    float r = riverDistance(x, y);
    if (r < 2.7f) {
      if (ground == 2 || ground == 3 || ground == 4) {
        map = 1;
        ground = 3;
      } else {
        map = 4;
        ground = 0;
      }
    } else if (map == 4 && r > 6 && noiseAt(x, y, 20, 77813) > .3f) {
      map = 1;
      ground = 0;
    }
  }
  if (j.mined.count(key)) {
    map = 1;
    ground = 0;
  }
  auto it = j.built.find(key);
  if (it != j.built.end()) {
    int k = it->second.kind;
    if (k == WALL || k == DOOR && !it->second.open) {
      map = 5;
      ground = 0;
    } else {
      map = 1;
      if (k == BRIDGE)
        ground = 3;
    }
  }
}
void refreshJourneyTerrain() {
  for (int y = 0; y < MH; y++)
    for (int x = 0; x < MW; x++) {
      auto t = tileAt(o.originX + x, o.originY + y);
      g.map[y * MW + x] = t.map;
      g.ground[y * MW + x] = t.ground;
      j.heights[y * MW + x] = landHeight(o.originX + x, o.originY + y);
    }
  for (auto &[k, b] : j.built)
    if (b.kind == TORCH || b.kind == FURNACE) {
      float x = float(k.first - o.originX) * T + 12,
            y = float(k.second - o.originY) * T + 12;
      if (x > 0 && y > 0 && x < MW * T && y < MH * T)
        g.props.push_back({x, y, 22, 0, false, biomeAt(k.first, k.second)});
    }
}
bool terrainLink(int a, int b) {
  if (!g.openWorld || o.generator < 4)
    return true;
  int dh = std::abs(int(j.heights[a]) - int(j.heights[b]));
  return dh == 0 || dh == 1 && (g.ground[a] > 0 || g.ground[b] > 0 ||
                                ((a % MW + int(o.originX % 8)) % 8 == 0) ||
                                ((a / MW + int(o.originY % 8)) % 8 == 0));
}
bool journeyMoveAllowed(float x, float y, float oldx, float oldy, float r,
                        bool player) {
  if (!g.openWorld)
    return fits(x, y, r);
  int a = int(oldy / T) * MW + int(oldx / T), b = int(y / T) * MW + int(x / T);
  if (a < 0 || a >= MW * MH || b < 0 || b >= MW * MH)
    return false;
  bool pass = fits(x, y, r);
  if (!pass && player && j.z > 12) {
    pass = true;
    for (float dx : {-r, r})
      for (float dy : {-r, r}) {
        int xx = int((x + dx) / T), yy = int((y + dy) / T);
        if (xx < 0 || yy < 0 || xx >= MW || yy >= MH ||
            g.map[yy * MW + xx] != 1 && g.map[yy * MW + xx] != 4)
          pass = false;
      }
  }
  if (!pass)
    return false;
  if (terrainLink(a, b))
    return true;
  int dh = int(j.heights[b]) - int(j.heights[a]);
  return player && (j.z > 14 && dh <= 1 && dh >= -2);
}
void dropMaterial(float x, float y, int k, int n) {
  if (n <= 0 || k < 0 || k >= MATERIALS)
    return;
  double wx = o.originX + x / double(T), wy = o.originY + y / double(T);
  for (auto &d : j.drops)
    if (d.kind == k && std::hypot(d.x - wx, d.y - wy) < 1.5 &&
        d.count + n <= 4096) {
      d.count += n;
      d.z = 10;
      d.vz = 55;
      d.age = 0;
      o.dirty = true;
      return;
    }
  if (j.drops.size() >= 512) {
    changeMaterial(k, n);
    notify("DROP LIMIT: MATERIAL RECOVERED DIRECTLY");
    return;
  }
  j.drops.push_back({wx, wy, k, n, 10, 60, 0});
  o.dirty = true;
}
float stanceSpeed() {
  return j.guard > 0 ? .40f : j.stance == 1 ? .55f : j.stance == 2 ? .27f : 1.f;
}
float stealthFactor() {
  return j.active || j.noise > 0 ? 1.f
         : j.stance == 1         ? .60f
         : j.stance == 2         ? .35f
                                 : 1.f;
}
void jumpPlayer() {
  if (!g.openWorld || g.scene != PLAY || j.z > 0 || j.landing > 0 || j.active ||
      j.guard > 0)
    return;
  if (!useStamina(7))
    return;
  j.stance = 0;
  j.jumpStartHeight = landHeight(int64_t(std::floor(globalX())),
                                 int64_t(std::floor(globalY())));
  j.z = .01f;
  j.vz = 185;
  j.safeX = globalX();
  j.safeY = globalY();
  j.noise = 1;
  g.haptic = 1;
  sfx(17);
}
void cycleStance() {
  if (j.z > 0 || j.active || j.stanceCd > 0)
    return;
  j.stance = (j.stance + 1) % 3;
  v.sprint = false;
  j.stanceCd = .25f;
  g.attacking = false;
  j.guard = 0;
  notify(j.stance == 0   ? "STANDING"
         : j.stance == 1 ? "CROUCH / QUIETER MOVEMENT"
                         : "PRONE / CRAWL / STAND TO FIGHT");
}
void beginGuard() {
  if (j.guardCd > 0 || j.z > 0 || j.stance == 2)
    return;
  if (j.active && j.elapsed < j.windup + j.activeTime)
    return;
  if (!useStamina(3))
    return;
  j.active = false;
  g.attackTime = 0;
  g.attacking = false;
  j.guard = .75f;
  j.parry = .17f;
  j.guardCd = .9f;
  g.fx = g.aimx;
  g.fy = g.aimy;
  sfx(12);
}
bool defendHit(int &damage, float fromx, float fromy) {
  if (!g.openWorld)
    return false;
  if (j.z > 22 && std::hypot(fromx - g.px, fromy - g.py) < 100)
    return true;
  if (j.guard <= 0)
    return false;
  float dx = fromx - g.px, dy = fromy - g.py, d = std::max(1.f, len(dx, dy));
  if ((dx * g.fx + dy * g.fy) / d < .20f)
    return false;
  if (j.parry > 0 && useStamina(2)) {
    j.counter = 1.3f;
    j.parry = 0;
    g.hitstop = .045f;
    g.haptic = 1;
    burst(g.px + g.fx * 17, g.py + g.fy * 17, 0xffe5ddb2, 14);
    sfx(8);
    notify("PARRY / COUNTER READY");
    for (auto &e : g.enemies)
      if (e.alive && len(e.x - fromx, e.y - fromy) < 45) {
        e.stun = .8f;
        e.wind = 0;
        e.stateTime = .8f;
        if (e.boss()) {
          e.state = 3;
          e.stateTime = .7f;
        }
      }
    return true;
  }
  if (useStamina(std::max(4, damage / 2))) {
    damage = std::max(1, damage / 4);
    sfx(8);
  } else {
    j.guard = 0;
    g.hurtTime = .5f;
    notify("GUARD BROKEN");
  }
  return false;
}
bool enemyAttackSlot() {
  if (!g.openWorld)
    return true;
  int n = 0;
  for (auto &e : g.enemies)
    if (e.alive && (e.wind > 0 || e.boss() && e.state == 2) &&
        len(e.x - g.px, e.y - g.py) < 350)
      n++;
  return n < 2;
}
float easeJourney(float t) {
  t = clamp(t, 0, 1);
  return t * t * (3 - 2 * t);
}
float strikeAngle(float elapsed) {
  float side = j.combo == 1 ? -1.f : 1.f;
  if (g.attackWeapon == BOW)
    return j.aim;
  float idle = j.aim - side * .65f, back = j.aim - side * 1.7f,
        end = j.aim + side * 1.25f;
  if (elapsed < j.windup)
    return idle +
           (back - idle) * easeJourney(elapsed / std::max(.001f, j.windup));
  if (elapsed < j.windup + j.activeTime)
    return back +
           (end - back) * easeJourney((elapsed - j.windup) / j.activeTime);
  return end + (idle - end) * easeJourney((elapsed - j.windup - j.activeTime) /
                                          j.recovery);
}
void beginStrike(bool heavy) {
  if (g.scene != PLAY || g.dash > 0 || j.stance == 2 || j.guard > 0)
    return;
  if (j.active || g.attackCd > 0) {
    if (j.queue > 0 && j.queuedHeavy && !heavy)
      return;
    j.queue = .20f;
    j.queuedHeavy = heavy;
    return;
  }
  float cost = heavy ? (g.weapon == AXE ? 17 : 11) : (g.weapon == AXE ? 9 : 4);
  if (!useStamina(cost * weaponEffort()))
    return;
  j.heavy = heavy;
  j.active = true;
  j.emitted = false;
  j.elapsed = 0;
  j.struck.clear();
  j.trail.clear();
  j.noise = 2;
  j.combo = g.comboTime > 0 ? (j.combo + 1) % 3 : 0;
  g.combo = j.combo;
  g.comboTime = 1.0f;
  g.attackWeapon = g.weapon;
  if (g.weapon == SWORD) {
    j.windup = heavy ? .23f : .075f;
    j.activeTime = heavy ? .14f : .11f;
    j.recovery = heavy ? .23f : .13f;
  } else if (g.weapon == AXE) {
    j.windup = heavy ? .40f : .20f;
    j.activeTime = heavy ? .19f : .15f;
    j.recovery = heavy ? .32f : .25f;
  } else {
    j.windup = heavy ? .53f : .23f;
    j.activeTime = .03f;
    j.recovery = .17f;
  }
  float effort = weaponEffort();
  j.windup *= effort;
  j.activeTime *= effort;
  j.recovery *= effort;
  g.attackLength = j.windup + j.activeTime + j.recovery;
  g.attackTime = g.attackLength;
  g.attackCd = 0;
  g.attackHit = false;
  g.attackSerial++;
  g.aimx = g.fx;
  g.aimy = g.fy;
  Enemy *best = nullptr;
  float near = g.weapon == BOW ? 300 : 95;
  for (auto &e : g.enemies)
    if (e.alive) {
      float dx = e.x - g.px, dy = e.y - g.py, d = len(dx, dy);
      if (d < near && (dx * g.fx + dy * g.fy) / std::max(d, 1.f) > .15f &&
          sight(g.px, g.py, e.x, e.y)) {
        best = &e;
        near = d;
      }
    }
  if (best) {
    g.aimx = (best->x - g.px) / std::max(1.f, near);
    g.aimy = (best->y - g.py) / std::max(1.f, near);
  } else {
    bool animal = aimWildlife(g.weapon == BOW ? 300 : 85, nullptr);
    if (g.weapon == AXE && !animal)
      aimTree();
  }
  j.aim = std::atan2(g.aimy, g.aimx);
  j.previousAngle = strikeAngle(0);
}
void journeyStrikeStep(float dt) {
  if (!j.active) {
    if (j.queue > 0) {
      bool h = j.queuedHeavy;
      j.queue = 0;
      beginStrike(h);
    }
    return;
  }
  float previous = j.elapsed;
  j.elapsed += dt;
  g.attackTime = std::max(0.f, g.attackLength - j.elapsed);
  float end = j.windup + j.activeTime;
  if (j.elapsed >= j.windup && !j.emitted) {
    j.emitted = true;
    sfx(g.attackWeapon == BOW ? 7 : 1);
    g.slash = .12f;
    if (g.attackWeapon == BOW) {
      int d = int(damage() * (j.heavy ? 1.7f : 1));
      fireArrow(g.px + g.aimx * 16, g.py + g.aimy * 16, j.aim, d,
                j.heavy ? 2 : 0);
    } else {
      huntMelee(g.attackWeapon == AXE ? 70 : 58,
                damage() * (j.heavy ? 1.6f : 1));
      if (g.attackWeapon == AXE)
        chopTrees(87);
    }
  }
  if (g.attackWeapon != BOW && j.elapsed >= j.windup && previous <= end) {
    float from = strikeAngle(std::max(previous, j.windup)),
          to = strikeAngle(std::min(j.elapsed, end));
    float reach = g.attackWeapon == AXE ? 64 : 57;
    if (j.heavy)
      reach += 9;
    for (auto &e : g.enemies)
      if (e.alive && !j.struck.count(e.entityId) && e.hp > 0) {
        float dx = e.x - g.px, dy = e.y - g.py, d = len(dx, dy);
        if (d > reach + (e.boss() ? 20 : 9) || !sight(g.px, g.py, e.x, e.y))
          continue;
        int pi = std::clamp(int(g.py / T) * MW + int(g.px / T), 0, MW * MH - 1),
            ei = std::clamp(int(e.y / T) * MW + int(e.x / T), 0, MW * MH - 1);
        if (std::abs(float(j.heights[pi]) + j.z / 12 - float(j.heights[ei])) >
            1.5f)
          continue;
        float a = std::atan2(dy, dx);
        bool hit = false;
        int samples = std::max(1, int(std::abs(to - from) / .07f));
        for (int k = 0; k <= samples; k++) {
          float angle = from + (to - from) * k / samples;
          if (d < 20 ||
              std::abs(angleDelta(a, angle)) <
                  .20f + (e.boss() ? 18.f : 9.f) / std::max(d, 12.f)) {
            hit = true;
            break;
          }
        }
        if (hit) {
          j.struck.insert(e.entityId);
          float mul = (g.attackWeapon == AXE ? 1.4f : 1.f) *
                      (j.heavy        ? 1.8f
                       : j.combo == 2 ? 1.25f
                                      : 1.f) *
                      (j.counter > 0 ? 1.5f : 1.f);
          hitEnemy(e, damage() * mul, dx, dy, j.heavy || g.attackWeapon == AXE);
        }
      }
  }
  j.previousAngle = strikeAngle(j.elapsed);
  if (j.elapsed >= g.attackLength) {
    j.active = false;
    g.attackTime = 0;
    j.counter = 0;
    g.attackCd = 0;
  }
}
void harvestPayload() {
  if (j.active || j.z > 0 || j.stance == 2)
    return;
  j.noise = 2;
  Prop *best = nullptr;
  float d = 66;
  for (auto &p : g.props)
    if (p.kind == 1) {
      float n = len(p.x - g.px, p.y - g.py);
      if (n < d && ((p.x - g.px) * g.fx + (p.y - g.py) * g.fy) > 0 &&
          sight(g.px, g.py, p.x - (p.x - g.px) / std::max(n, 1.f) * 19,
                p.y - (p.y - g.py) / std::max(n, 1.f) * 19)) {
        best = &p;
        d = n;
      }
    }
  if (!best) {
    digGround();
    return;
  }
  auto key = treeKey(*best);
  if (j.mined.count(key))
    return;
  if (!validWorldKey(key.first, key.second) || j.mined.size() >= 20000 ||
      j.rockHP.size() >= 20000 && !j.rockHP.count(key)) {
    notify("MINING RECORD LIMIT");
    return;
  }
  int kind = int(coordinateHash(key.first, key.second, 7301) % 7),
      resource = kind == 0   ? ORE
                 : kind <= 2 ? COAL
                             : STONE;
  int tier = j.durability[0] > 0 ? j.toolTier[0] : 0;
  if (resource == ORE && tier < 2 || resource == COAL && tier < 1) {
    notify(resource == ORE ? "IRON NEEDS A STONE PICK"
                           : "COAL NEEDS A WOOD PICK");
    return;
  }
  if (!useStamina(5))
    return;
  int &hp = j.rockHP[key];
  if (!hp)
    hp = resource == ORE ? 7 : resource == COAL ? 5 : 4;
  hp -= 1 + tier;
  if (tier)
    j.durability[0]--;
  sfx(25);
  burst(best->x, best->y - 10, 0xffa4a08e, 10);
  if (hp <= 0) {
    j.mined[key] = 1;
    j.rockHP.erase(key);
    dropMaterial(best->x, best->y, resource, resource == STONE ? 4 : 2);
    int slot = int(best->y / T) * MW + int(best->x / T);
    g.map[slot] = 1;
    best->kind = 3;
    navigation();
  }
  o.dirty = true;
}
void harvest() {
  if (j.toolCd > 0 || j.active || j.z > 0 || j.stance == 2)
    return;
  j.toolCd = .45f;
  j.toolAnim = .4f;
  j.toolAim = std::atan2(g.fy, g.fx);
}
struct Recipe {
  const char *name;
  int output, quantity, station;
  std::array<int, 3> kind, amount;
};
const std::vector<Recipe> &recipes() {
  static const std::vector<Recipe> r = {
      {"PLANKS", PLANK, 4, 0, {LOG, 0, 0}, {1, 0, 0}},
      {"STICKS", STICK, 4, 0, {PLANK, 0, 0}, {2, 0, 0}},
      {"WORKBENCH", BENCH, 1, 0, {PLANK, 0, 0}, {4, 0, 0}},
      {"WOOD PICK", 100, 1, BENCH, {PLANK, STICK, 0}, {3, 2, 0}},
      {"STONE PICK", 101, 1, BENCH, {STONE, STICK, 0}, {3, 2, 0}},
      {"IRON PICK", 102, 1, BENCH, {IRON, STICK, 0}, {3, 2, 0}},
      {"STONE AXE", 103, 1, BENCH, {STONE, STICK, 0}, {3, 2, 0}},
      {"IRON AXE", 104, 1, BENCH, {IRON, STICK, 0}, {3, 2, 0}},
      {"TORCHES", TORCH, 4, 0, {COAL, STICK, 0}, {1, 1, 0}},
      {"WOOD WALLS", WALL, 4, BENCH, {PLANK, 0, 0}, {4, 0, 0}},
      {"DOOR", DOOR, 1, BENCH, {PLANK, 0, 0}, {6, 0, 0}},
      {"FURNACE", FURNACE, 1, BENCH, {STONE, 0, 0}, {8, 0, 0}},
      {"STORAGE CHEST", CHEST, 1, BENCH, {PLANK, 0, 0}, {8, 0, 0}},
      {"BED", BED, 1, BENCH, {PLANK, LOG, 0}, {6, 2, 0}},
      {"BRIDGES", BRIDGE, 3, BENCH, {PLANK, STICK, 0}, {6, 2, 0}},
      {"BREAD", 105, 1, FURNACE, {GRAIN, COAL, 0}, {3, 1, 0}},
      {"CHARCOAL", COAL, 2, FURNACE, {LOG, 0, 0}, {3, 0, 0}}};
  return r;
}
Structure *nearbyStructure(int kind, float radius = 80) {
  Structure *result = nullptr;
  float best = radius;
  for (auto &[k, b] : j.built)
    if (b.kind == kind) {
      float d = float(
          std::hypot(k.first + .5 - globalX(), k.second + .5 - globalY()) * T);
      if (d < best) {
        best = d;
        result = &b;
      }
    }
  return result;
}
void craftRecipe(int index) {
  if (index < 0 || index >= int(recipes().size()))
    return;
  auto &r = recipes()[index];
  if (r.station && !nearbyStructure(r.station)) {
    notify(std::string("NEED A NEARBY ") + materialName(r.station));
    return;
  }
  if ((r.output >= 100 && r.output <= 102 && j.durability[0] > 0 &&
       j.toolTier[0] > r.output - 99) ||
      (r.output >= 103 && r.output <= 104 && j.durability[1] > 0 &&
       j.toolTier[1] > r.output - 101)) {
    notify("BETTER TOOL ALREADY OWNED");
    return;
  }
  if (r.output == 105 && v.meals >= 200) {
    notify("FOOD STOCK FULL");
    return;
  }
  for (int a = 0; a < 3; a++)
    if (materialCount(r.kind[a]) < r.amount[a]) {
      notify("MISSING CRAFTING MATERIALS");
      return;
    }
  for (int a = 0; a < 3; a++)
    if (r.amount[a])
      changeMaterial(r.kind[a], -r.amount[a]);
  if (r.output < MATERIALS && !roomFor(r.output, r.quantity)) {
    for (int a = 0; a < 3; a++)
      changeMaterial(r.kind[a], r.amount[a]);
    notify("PACK FULL - STORE MATERIALS");
    return;
  }
  if (r.output < 100)
    changeMaterial(r.output, r.quantity);
  else if (r.output <= 102) {
    j.toolTier[0] = r.output - 99;
    j.durability[0] = 120 * j.toolTier[0];
  } else if (r.output <= 104) {
    j.toolTier[1] = r.output - 101;
    j.durability[1] = 120 * j.toolTier[1];
  } else
    v.meals = std::min(200, v.meals + 1);
  sfx(12);
  notify(std::string("CRAFTED ") + r.name);
  o.dirty = true;
}
std::pair<int64_t, int64_t> frontCell() {
  int dx = 0, dy = 0;
  if (std::abs(g.fx) >= std::abs(g.fy))
    dx = g.fx < 0 ? -1 : 1;
  else
    dy = g.fy < 0 ? -1 : 1;
  return {int64_t(std::floor(globalX())) + dx,
          int64_t(std::floor(globalY())) + dy};
}
void digGround() {
  auto k = frontCell();
  if (!validWorldKey(k.first, k.second) || j.mined.size() >= 20000) {
    notify("DIG RECORD LIMIT");
    return;
  }
  auto it = j.built.find(k);
  if (it != j.built.end()) {
    notify("RECLAIM THE BUILT OBJECT FIRST");
    return;
  }
  auto t = tileAt(k.first, k.second);
  if (t.map != 1 || t.ground == 4 ||
      (std::abs(k.first) < 7 && std::abs(k.second) < 7) ||
      (j.mined.count(k) && j.mined[k] == 2)) {
    notify("DIG: FACE CLEAR UNPROTECTED GROUND");
    return;
  }
  if (!useStamina(4))
    return;
  j.mined[k] = 2;
  dropMaterial(float(k.first - o.originX) * T + 12,
               float(k.second - o.originY) * T + 12, DIRT, 2);
  generateWindow();
  navigation();
  sfx(25);
  notify("DUG SURFACE / +2 DIRT DROPS");
  o.dirty = true;
}
void placeStructure() {
  int kind = j.selectedBuild;
  auto k = frontCell();
  if (!validWorldKey(k.first, k.second) || j.built.size() >= 4096) {
    notify("BUILD RECORD LIMIT");
    return;
  }
  if (materialCount(kind) < 1) {
    notify("CRAFT THIS OBJECT FIRST");
    return;
  }
  if (j.built.count(k) || std::abs(k.first) < 7 && std::abs(k.second) < 7) {
    notify("OCCUPIED / PROTECTED ORIGIN");
    return;
  }
  Tile t = tileAt(k.first, k.second);
  if ((t.map != 1 && !(kind == BRIDGE && t.map == 4)) || t.ground == 4) {
    notify("NEED CLEAR GROUND / BRIDGES CAN SPAN WATER");
    return;
  }
  if (kind == WALL || kind == DOOR) {
    float px = float(k.first - o.originX) * T + 12,
          py = float(k.second - o.originY) * T + 12;
    if (std::abs(px - g.px) < 19 && std::abs(py - g.py) < 19) {
      notify("STEP BACK TO PLACE A SOLID OBJECT");
      return;
    }
    for (auto &e : g.enemies)
      if (e.alive && std::abs(e.x - px) < 22 && std::abs(e.y - py) < 22) {
        notify("AN ACTOR IS USING THIS SPACE");
        return;
      }
  }
  Structure b;
  b.kind = kind;
  j.built[k] = b;
  changeMaterial(kind, -1);
  generateWindow();
  navigation();
  sfx(12);
  notify(std::string("PLACED ") + materialName(kind));
}
void storeMaterials(bool take) {
  auto *b = nearbyStructure(CHEST);
  if (!b) {
    notify("APPROACH YOUR CHEST");
    return;
  }
  for (int k = 0; k < MATERIALS; k++) {
    int n = take ? b->storage[k] : materialCount(k);
    if (take) {
      int moved = 0;
      while (moved < n && roomFor(k, 1)) {
        changeMaterial(k, 1);
        moved++;
      }
      b->storage[k] -= moved;
    } else {
      int moved = std::min(n, 1000000 - b->storage[k]);
      b->storage[k] += moved;
      changeMaterial(k, -moved);
    }
  }
  o.dirty = true;
  sfx(3);
  notify(take ? "CHEST: RECOVERED WHAT FITS" : "MATERIALS STORED - TOOLS KEPT");
}
void smeltOre() {
  auto *b = nearbyStructure(FURNACE);
  if (!b) {
    notify("APPROACH YOUR FURNACE");
    return;
  }
  if (j.stock[ORE] < 1 || j.stock[COAL] < 1) {
    notify("SMELT NEEDS ORE AND COAL");
    return;
  }
  if (b->storage[ORE] >= 100) {
    notify("FURNACE QUEUE FULL");
    return;
  }
  changeMaterial(ORE, -1);
  changeMaterial(COAL, -1);
  b->storage[ORE]++;
  b->storage[COAL]++;
  o.dirty = true;
  notify("FURNACE QUEUED / 8 SECONDS PER INGOT");
}
void journeyInteract() {
  auto k = frontCell();
  auto it = j.built.find(k);
  float nearest = 75;
  for (auto n = j.built.begin(); n != j.built.end(); ++n) {
    float dx = float(n->first.first + .5 - globalX()) * T,
          dy = float(n->first.second + .5 - globalY()) * T;
    float d = len(dx, dy);
    if (d < nearest && (dx * g.fx + dy * g.fy) > 0) {
      it = n;
      k = n->first;
      nearest = d;
    }
  }
  if (it != j.built.end()) {
    auto &b = it->second;
    if (b.kind == DOOR) {
      b.open = !b.open;
      refreshJourneyTerrain();
      navigation();
      sfx(12);
      o.dirty = true;
      return;
    }
    if (b.kind == 18) {
      if (b.progress < 180) {
        notify("CROP IS STILL GROWING");
        return;
      }
      dropMaterial(float(k.first - o.originX) * T + 12,
                   float(k.second - o.originY) * T + 12, GRAIN, 3);
      dropMaterial(float(k.first - o.originX) * T + 12,
                   float(k.second - o.originY) * T + 12, SEED, 2);
      j.built.erase(it);
      o.dirty = true;
      return;
    }
    if (b.kind == BED) {
      j.home = true;
      j.homeX = k.first;
      j.homeY = k.second;
      o.dirty = true;
      if (v.meals < 1 || v.cleanWater < 1) {
        notify("BED BOUND / REST NEEDS MEAL AND WATER");
        return;
      }
      j.restTime = 5;
      notify("RESTING 5 SECONDS / MOVEMENT CANCELS");
      return;
    }
  }
  for (auto &[key, b] : j.built)
    if (b.kind == BED &&
        std::hypot(double(key.first) + .5 - globalX(),
                   double(key.second) + .5 - globalY()) < 2.5) {
      j.home = true;
      j.homeX = key.first;
      j.homeY = key.second;
      o.dirty = true;
      notify("BED BOUND AS RESPAWN POINT");
      return;
    }
  g.overlay = 10;
  clearInput();
}
void plantSeed() {
  auto k = frontCell();
  auto t = tileAt(k.first, k.second);
  if (!validWorldKey(k.first, k.second) || t.map != 1 || j.built.count(k) ||
      j.built.size() >= 4096 || j.stock[SEED] <= 0) {
    notify("PLANT: NEED SEED AND CLEAR GROUND");
    return;
  }
  Structure b;
  b.kind = 18;
  for (int y = -2; y <= 2; y++)
    for (int x = -2; x <= 2; x++)
      if (tileAt(k.first + x, k.second + y).map == 4)
        b.open = true;
  j.built[k] = b;
  changeMaterial(SEED, -1);
  notify("CROP PLANTED / WATER NEARBY HELPS GROWTH");
}
void reclaimStructure() {
  auto k = frontCell();
  auto it = j.built.find(k);
  if (it == j.built.end()) {
    notify("FACE A BUILT OBJECT TO RECLAIM");
    return;
  }
  for (int n : it->second.storage)
    if (n) {
      notify("EMPTY THIS STATION BEFORE RECLAIMING");
      return;
    }
  int type = it->second.kind;
  if (type == 18)
    type = SEED;
  if (!roomFor(type, 1)) {
    notify("PACK FULL");
    return;
  }
  if (j.home && j.homeX == k.first && j.homeY == k.second)
    j.home = false;
  j.built.erase(it);
  changeMaterial(type, 1);
  generateWindow();
  navigation();
  sfx(12);
}
void journeyPreStep(float dt) {
  auto dec = [dt](float &f) { f = std::max(0.f, f - dt); };
  dec(j.guard);
  dec(j.guardCd);
  dec(j.parry);
  dec(j.counter);
  dec(j.queue);
  dec(j.toolCd);
  dec(j.landing);
  dec(j.stanceCd);
  dec(j.noise);
  dec(j.dodgeQueue);
  if (j.dodgeQueue > 0 && (!j.active || j.elapsed >= j.windup + j.activeTime)) {
    j.dodgeQueue = 0;
    skill(1);
  }
  float oldTool = j.toolAnim;
  dec(j.toolAnim);
  if (oldTool > .21f && j.toolAnim <= .21f)
    harvestPayload();
  j.pose = approach(j.pose, float(j.stance), dt * 6);
  if (j.stance > 0)
    v.sprint = false;
  if (j.guardFinger >= 0 && j.guard > 0 && v.stamina > 1) {
    j.guard = std::max(j.guard, .1f);
    v.stamina = std::max(0.f, v.stamina - dt * 5);
  }
  if (j.restTime > 0) {
    bool danger = false;
    for (auto &e : g.enemies)
      if (e.alive && e.alertTime > 0 && len(e.x - g.px, e.y - g.py) < 230)
        danger = true;
    if (len(g.mx, g.my) > .1f || j.active || j.z > 0 || danger) {
      j.restTime = 0;
      notify("REST INTERRUPTED");
    } else {
      j.restTime -= dt;
      if (j.restTime <= 0 && v.meals > 0 && v.cleanWater > 0) {
        v.meals--;
        v.cleanWater--;
        g.hp = std::min(float(maxhp()), g.hp + maxhp() * .4f);
        v.stamina = 100;
        v.food = std::min(100.f, v.food + 25);
        v.water = std::min(100.f, v.water + 30);
        float h = worldHour();
        if (h >= 18 || h < 7)
          o.seconds += (h < 8 ? 8 - h : 32 - h) * 60;
        o.dirty = true;
        notify("RESTED / BED CHECKPOINT SAVED");
      }
    }
  }
  if (j.z > 0) {
    j.vz -= 520 * dt;
    j.z += j.vz * dt;
    if (j.z <= 0) {
      j.z = 0;
      j.vz = 0;
      j.landing = .12f;
      sfx(18);
      if (!fits(g.px, g.py, 7)) {
        placePlayer(j.safeX, j.safeY);
        hurt(9, g.px, g.py);
      } else {
        int fall =
            j.jumpStartHeight - landHeight(int64_t(std::floor(globalX())),
                                           int64_t(std::floor(globalY())));
        if (fall > 1)
          hurt((fall - 1) * 8, g.px, g.py);
        burst(g.px, g.py, 0xffaaa68e, 7, .6f);
      }
    }
  } else if (fits(g.px, g.py, 7)) {
    j.safeX = globalX();
    j.safeY = globalY();
  }
  for (auto &d : j.drops) {
    d.age += dt;
    d.z = std::max(0.f, d.z + d.vz * dt);
    d.vz -= 230 * dt;
    if (d.z == 0) {
      d.vz = std::abs(d.vz) > 15 ? -d.vz * .25f : 0;
    }
    float x = float(d.x - o.originX) * T, y = float(d.y - o.originY) * T,
          dist = len(x - g.px, y - g.py);
    if (d.age > .4f && dist < 48 && j.z < 8 && roomFor(d.kind, 1) &&
        sight(g.px, g.py, x, y)) {
      if (dist < 18) {
        int existing = materialCount(d.kind);
        int space = std::max(0, (24 - occupiedSlots()) * 64 +
                                    (existing % 64 ? 64 - existing % 64 : 0));
        int taken = std::min(d.count, space);
        changeMaterial(d.kind, taken);
        sfx(3);
        floating(g.px, g.py - 47,
                 std::string("+") + num(taken) + " " + materialName(d.kind),
                 GOLD);
        d.count -= taken;
      } else {
        float pull = std::min(1.f, dt * 8);
        d.x += (globalX() - d.x) * pull;
        d.y += (globalY() - d.y) * pull;
      }
    }
  }
  std::erase_if(j.drops, [](auto &d) { return d.count <= 0 || d.age > 1200; });
  for (auto &[key, b] : j.built) {
    if (key.first < o.originX || key.second < o.originY ||
        key.first >= o.originX + MW || key.second >= o.originY + MH)
      continue;
    if (b.kind == FURNACE && b.storage[ORE] > 0 && b.storage[COAL] > 0) {
      b.progress += dt;
      if (b.progress >= 8) {
        b.progress -= 8;
        b.storage[ORE]--;
        b.storage[COAL]--;
        dropMaterial(float(key.first - o.originX) * T + 12,
                     float(key.second - o.originY) * T + 12, IRON, 1);
        o.dirty = true;
      }
    }
    if (b.kind == 18 && b.progress < 180)
      b.progress = std::min(180.f, b.progress + dt * (b.open ? 1.5f : 1.f));
  }
}
} // namespace av
