#pragma once
#include "survival.hpp"
namespace av {
const char *treeName(int s) {
  const char *n[] = {"OAK",    "BIRCH", "PINE",     "WILLOW", "PALM",
                     "ACACIA", "CEDAR", "DEADWOOD", "CYPRESS"};
  return n[std::clamp(s, 0, 8)];
}
std::pair<int64_t, int64_t> treeKey(const Prop &p) {
  return {o.originX + int64_t(std::floor(p.x / T)),
          o.originY + int64_t(std::floor(p.y / T))};
}
bool treeCut(int64_t x, int64_t y) {
  auto it = w.trees.find({x, y});
  return it != w.trees.end() && it->second == 0;
}
bool treeCut(const Prop &p) {
  auto k = treeKey(p);
  return treeCut(k.first, k.second);
}
int treeSpecies(const Prop &p) {
  auto k = treeKey(p);
  uint64_t h = coordinateHash(k.first, k.second, 8301);
  const int t[5][8] = {{0, 0, 1, 2, 6, 3, 0, 1},
                       {4, 5, 5, 7, 4, 5, 7, 4},
                       {2, 6, 2, 1, 6, 2, 6, 1},
                       {3, 8, 3, 8, 1, 3, 8, 0},
                       {7, 7, 7, 6, 7, 2, 7, 5}};
  return t[std::clamp(p.biome, 0, 4)][h % 8];
}
std::string encodeWildlands() {
  std::ostringstream s;
  s << std::setprecision(15) << "EARTH1 " << w.clockOffset << ' ' << w.wood
    << ' ' << w.felled << ' ' << w.trees.size() << '\n';
  for (auto &[k, hp] : w.trees)
    s << k.first << ' ' << k.second << ' ' << hp << '\n';
  s << w.fires.size() << '\n';
  for (auto &k : w.fires)
    s << k.first << ' ' << k.second << '\n';
  s << w.explored.size() << '\n';
  for (auto &[k, bits] : w.explored) {
    s << k.first << ' ' << k.second;
    for (uint64_t b : bits)
      s << ' ' << b;
    s << '\n';
  }
  return s.str();
}
bool decodeWildlands(const std::string &raw, Wildlands &result) {
  if (raw.size() > 8 * 1024 * 1024)
    return false;
  Wildlands a;
  std::istringstream f(raw);
  std::string magic;
  size_t count;
  if (!(f >> magic >> a.clockOffset >> a.wood >> a.felled >> count) ||
      magic != "EARTH1" || !finiteIn(a.clockOffset, 0, 1e9) || a.wood < 0 ||
      a.wood > 1000000 || a.felled < 0 || a.felled > 20000 || count > 20000)
    return false;
  for (size_t i = 0; i < count; i++) {
    int64_t x, y;
    int hp;
    if (!(f >> x >> y >> hp) || x < -WORLD_LIMIT || x > WORLD_LIMIT ||
        y < -WORLD_LIMIT || y > WORLD_LIMIT || hp < 0 || hp > 15)
      return false;
    a.trees[{x, y}] = hp;
  }
  if (!(f >> count) || count > 5000)
    return false;
  for (size_t i = 0; i < count; i++) {
    int64_t x, y;
    if (!(f >> x >> y) || x < -WORLD_LIMIT || x > WORLD_LIMIT ||
        y < -WORLD_LIMIT || y > WORLD_LIMIT)
      return false;
    a.fires.insert({x, y});
  }
  if (!(f >> count) || count > 16000)
    return false;
  for (size_t i = 0; i < count; i++) {
    int64_t x, y;
    if (!(f >> x >> y) || x < -WORLD_LIMIT / 32 - 1 ||
        x > WORLD_LIMIT / 32 + 1 || y < -WORLD_LIMIT / 32 - 1 ||
        y > WORLD_LIMIT / 32 + 1)
      return false;
    std::array<uint64_t, 16> bits{};
    for (auto &b : bits)
      if (!(f >> b))
        return false;
    a.explored[{x, y}] = bits;
  }
  result = std::move(a);
  return true;
}
bool exploredAt(int64_t x, int64_t y) {
  auto it = w.explored.find({floorDiv(x, 32), floorDiv(y, 32)});
  if (it == w.explored.end())
    return false;
  int n = floorMod(y, 32) * 32 + floorMod(x, 32);
  return (it->second[n / 64] >> (n % 64)) & 1;
}
void revealTerrain() {
  if (!g.openWorld)
    return;
  int64_t px = int64_t(std::floor(globalX())),
          py = int64_t(std::floor(globalY()));
  if (px == w.revealX && py == w.revealY)
    return;
  w.revealX = px;
  w.revealY = py;
  for (int y = -12; y <= 12; y++)
    for (int x = -12; x <= 12; x++)
      if (x * x + y * y <= 144) {
        int64_t xx = px + x, yy = py + y;
        auto key = std::make_pair(floorDiv(xx, 32), floorDiv(yy, 32));
        if (w.explored.size() >= 16000 && !w.explored.count(key)) {
          if (!w.mapLimit) {
            w.mapLimit = true;
            notify("EXPLORATION RECORD LIMIT - SAVE YOUR WORLD");
          }
          continue;
        }
        int n = floorMod(yy, 32) * 32 + floorMod(xx, 32);
        auto &bits = w.explored[key];
        uint64_t bit = 1ULL << (n % 64);
        if (!(bits[n / 64] & bit)) {
          bits[n / 64] |= bit;
          o.dirty = true;
          w.atlasDirty = true;
        }
      }
}
bool knownBiomeWaypoint(int b) {
  double best = 1e40;
  int64_t bx = 0, by = 0;
  for (auto &[k, bits] : w.explored)
    for (int i = 0; i < 16; i++) {
      uint64_t v = bits[i];
      while (v) {
        int bit = std::countr_zero(v), n = i * 64 + bit;
        v &= v - 1;
        int64_t x = k.first * 32 + n % 32, y = k.second * 32 + n / 32;
        if (biomeAt(x, y) != b)
          continue;
        double d = std::hypot(double(x) - globalX(), double(y) - globalY());
        if (d < best) {
          best = d;
          bx = x;
          by = y;
        }
      }
    }
  if (best == 1e40) {
    notify("NO EXPLORED LOCATION FOR THIS BIOME YET");
    return false;
  }
  o.waypoint = true;
  o.waypointX = bx;
  o.waypointY = by;
  notify("COMPASS SET TO AN EXPLORED LOCATION");
  return true;
}
const char *dayPhase() {
  float h = worldHour();
  return h < 5    ? "NIGHT"
         : h < 7  ? "DAWN"
         : h < 11 ? "MORN"
         : h < 15 ? "NOON"
         : h < 18 ? "LATE"
         : h < 20 ? "DUSK"
                  : "NIGHT";
}
void wildlandsTick(float dt) {
  for (auto &f : w.falling)
    f.time += dt;
  std::erase_if(w.falling, [](auto &f) { return f.time > 1.35f; });
}
bool aimTree() {
  Prop *best = nullptr;
  float d = 96;
  for (auto &p : g.props)
    if (p.kind == 0 && !treeCut(p)) {
      float n = len(p.x - g.px, p.y - g.py);
      if (n < d) {
        best = &p;
        d = n;
      }
    }
  if (!best)
    return false;
  g.aimx = (best->x - g.px) / std::max(1.f, d);
  g.aimy = (best->y - g.py) / std::max(1.f, d);
  return true;
}
void chopTrees(float radius) {
  Prop *best = nullptr;
  float nearest = radius;
  for (auto &p : g.props)
    if (p.kind == 0 && !treeCut(p)) {
      float dx = p.x - g.px, dy = p.y - g.py, d = len(dx, dy);
      if (d < nearest &&
          (dx * g.aimx + dy * g.aimy) / std::max(1.f, d) > .05f &&
          sight(g.px, g.py, p.x - dx / std::max(1.f, d) * 19,
                p.y - dy / std::max(1.f, d) * 19)) {
        nearest = d;
        best = &p;
      }
    }
  if (!best)
    return;
  if (w.trees.size() >= 20000 && !w.trees.count(treeKey(*best))) {
    notify("TREE RECORD LIMIT REACHED");
    return;
  }
  auto key = treeKey(*best);
  if (key.first < -WORLD_LIMIT || key.first > WORLD_LIMIT ||
      key.second < -WORLD_LIMIT || key.second > WORLD_LIMIT) {
    notify("WORLD COORDINATE LIMIT");
    return;
  }
  int species = treeSpecies(*best);
  auto it = w.trees.find(key);
  int hp = it == w.trees.end() ? (species == 0 || species == 8 ? 6
                                  : species == 7               ? 3
                                                               : 4)
                               : it->second;
  int toolBonus = j.durability[1] > 0 ? j.toolTier[1] / 2 : 0;
  hp -= 1 + (g.upgrade >= 8 ? 1 : 0) + toolBonus;
  if (j.durability[1] > 0)
    j.durability[1]--;
  w.trees[key] = std::max(0, hp);
  o.dirty = true;
  g.hitstop = std::max(g.hitstop, .04f);
  g.shakeTime = .06f;
  g.haptic = 1;
  sfx(25);
  burst(best->x, best->y - 18, 0xffad8758, 14);
  if (hp > 0) {
    floating(best->x, best->y - 65,
             std::string(treeName(species)) + " " + num(hp), GOLD);
    return;
  }
  w.felled++;
  int wood = species == 7 ? 3 : species == 0 || species == 8 ? 7 : 5;
  dropMaterial(best->x, best->y, LOG, wood);
  w.falling.push_back({best->x, best->y, 0, species, best->biome, best->variant,
                       g.aimx < 0 ? -1 : 1});
  int x = int(best->x / T), y = int(best->y / T);
  if (x >= 0 && x < MW && y >= 0 && y < MH)
    g.map[y * MW + x] = 1;
  navigation();
  sfx(26);
  notify(std::string(treeName(species)) + " FELLED / +" + num(wood) + " WOOD");
  burst(best->x, best->y - 45, 0xff76835a, 22);
  w.atlasDirty = true;
}
void addPlayerFires() {
  for (auto &key : w.fires) {
    float x = float(key.first - o.originX) * T + 12,
          y = float(key.second - o.originY) * T + 12;
    if (x > 0 && y > 0 && x < MW * T && y < MH * T)
      g.props.push_back({x, y, 22, 1, false, g.world});
  }
}
void craftSplint() {
  if (v.splints >= 200) {
    notify("LIMB SUPPORT STOCK IS FULL");
    return;
  }
  if (w.wood < 4 || v.fiber < 1) {
    notify("SPLINT NEEDS FOUR WOOD AND ONE FIBER");
    return;
  }
  w.wood -= 4;
  v.fiber--;
  v.splints = std::min(200, v.splints + 1);
  o.dirty = true;
  sfx(12);
  notify("CRAFTED A LIMB SUPPORT");
}
void buildCampfire() {
  if (w.wood < 8) {
    notify("CAMPFIRE NEEDS EIGHT WOOD");
    return;
  }
  if (w.fires.size() >= 5000) {
    notify("CAMPFIRE RECORD LIMIT REACHED");
    return;
  }
  auto key = std::make_pair(int64_t(std::floor(globalX())),
                            int64_t(std::floor(globalY())));
  if (w.fires.count(key) || nearFire()) {
    notify("A FIRE IS ALREADY NEARBY");
    return;
  }
  w.fires.insert(key);
  w.wood -= 8;
  g.props.push_back({float(key.first - o.originX) * T + 12,
                     float(key.second - o.originY) * T + 12, 22, 1, false,
                     g.world});
  o.dirty = true;
  sfx(12);
  notify("CAMPFIRE BUILT - COOK, BOIL AND REST HERE");
}
} // namespace av
