#pragma once
#include "world.hpp"
#include <fcntl.h>
#include <unistd.h>
namespace av {
// Generator version 1. Never change its rules for existing saves without
// migration.
constexpr int CELL = 32;
constexpr int64_t WORLD_LIMIT =
    1000000000LL; // engineering guard, not a finite mission map
uint64_t mix64(uint64_t x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}
uint64_t coordinateHash(int64_t x, int64_t y, uint64_t salt = 0) {
  return mix64(uint64_t(x) ^ mix64(uint64_t(y) ^ o.seed) ^ salt);
}
int64_t floorDiv(int64_t a, int64_t b) {
  int64_t q = a / b, r = a % b;
  return q - (r < 0);
}
int floorMod(int64_t a, int n) { return int(a - floorDiv(a, n) * n); }
double globalX() { return double(o.originX) + g.px / double(T); }
double globalY() { return double(o.originY) + g.py / double(T); }
uint64_t entropy() {
  static uint64_t serial = 0;
  uint64_t x = uint64_t(
      std::chrono::high_resolution_clock::now().time_since_epoch().count());
  std::ifstream f("/dev/urandom", std::ios::binary);
  uint64_t r = 0;
  if (f)
    f.read(reinterpret_cast<char *>(&r), sizeof(r));
  return mix64(x ^ r ^ ++serial);
}
std::string hexId(uint64_t n) {
  std::ostringstream s;
  s << std::hex << std::setw(16) << std::setfill('0') << n;
  return s.str();
}
bool validId(const std::string &s) {
  return s.size() == 16 && std::all_of(s.begin(), s.end(), [](char c) {
           return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
         });
}
std::string cleanName(const std::string &s) {
  std::string a;
  for (unsigned char c : s)
    if (a.size() < 24 && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                          (c >= '0' && c <= '9') || c == ' ' || c == '-'))
      a += char(c);
  while (!a.empty() && a.back() == ' ')
    a.pop_back();
  return a.empty() ? "UNTITLED WORLD" : a;
}
std::string worldFile(const std::string &id) {
  return g.path + "/worlds/" + id + "/world.sav";
}
bool readChecked(const std::string &p, std::string &body) {
  std::error_code ec;
  auto size = std::filesystem::file_size(p, ec);
  if (ec || size > 25 * 1024 * 1024 || size < 4)
    return false;
  std::ifstream f(p);
  std::string sig;
  if (!std::getline(f, sig))
    return false;
  std::ostringstream s;
  s << f.rdbuf();
  body = s.str();
  try {
    size_t n = 0;
    auto h = std::stoull(sig, &n);
    return n == sig.size() && h == checksum(body);
  } catch (...) {
    return false;
  }
}
bool atomicWorld(const std::string &p, const std::string &body) {
  if (body.size() > 24 * 1024 * 1024)
    return false;
  std::error_code ec;
  std::filesystem::create_directories(std::filesystem::path(p).parent_path(),
                                      ec);
  if (ec)
    return false;
  {
    std::ofstream f(p + ".tmp", std::ios::trunc);
    f << checksum(body) << '\n' << body;
    f.flush();
    if (!f)
      return false;
  }
  int fd = ::open((p + ".tmp").c_str(), O_RDONLY);
  if (fd >= 0) {
    ::fsync(fd);
    ::close(fd);
  }
  std::string old;
  if (readChecked(p, old))
    std::filesystem::copy_file(
        p, p + ".bak", std::filesystem::copy_options::overwrite_existing, ec);
  if (std::rename((p + ".tmp").c_str(), p.c_str()) != 0)
    return false;
  fd = ::open(std::filesystem::path(p).parent_path().c_str(), O_RDONLY);
  if (fd >= 0) {
    ::fsync(fd);
    ::close(fd);
  }
  return true;
}
void saveSettings() {
  if (g.path.empty())
    return;
  std::string body = "DWSET2 " + num(g.muted) + " " + num(g.shake) + " " +
                     num(g.lowPower) + " " + num(motionBlur) + "\n";
  if (body == o.settingsCache)
    return;
  if (atomicWorld(g.path + "/settings.sav", body))
    o.settingsCache = body;
}
void loadSettings() {
  if (g.path.empty())
    return;
  std::string body;
  if (!readChecked(g.path + "/settings.sav", body) &&
      !readChecked(g.path + "/settings.sav.bak", body))
    return;
  std::istringstream f(body);
  std::string magic;
  int muted, shake, low;
  if (f >> magic >> muted >> shake >> low &&
      (magic == "DWSET1" || magic == "DWSET2") && muted >= 0 && muted <= 1 &&
      shake >= 0 && shake <= 1 && low >= 0 && low <= 1) {
    int blur = 1;
    if (magic == "DWSET2" && (!(f >> blur) || blur < 0 || blur > 1))
      return;
    motionBlur = blur;
    g.muted = muted;
    g.shake = shake;
    g.lowPower = low;
    o.settingsCache = body;
  }
}
int64_t timestamp() {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}
std::string encodeFrontier() {
  std::ostringstream s;
  s << std::setprecision(15);
  s << "DWFRONTIER 4 " << o.seed << ' ' << std::quoted(o.name) << ' '
    << globalX() << ' ' << globalY() << ' ' << g.hp << ' ' << g.flasks << ' '
    << g.healCd << ' ' << o.seconds << ' ' << o.totalKills << ' '
    << o.campsCleared << ' ' << o.discovered << ' ' << timestamp() << ' '
    << g.worldWins[2] << ' ' << g.worldWins[3] << ' ' << g.worldWins[4] << ' '
    << o.generator << '\n';
  s << std::quoted(encode()) << '\n' << o.slain.size() << '\n';
  for (auto &[key, mask] : o.slain)
    s << key.first << ' ' << key.second << ' ' << mask << '\n';
  s << std::quoted(encodeLife()) << '\n'
    << std::quoted(encodeWildlands()) << '\n'
    << std::quoted(encodeJourney()) << '\n';
  return s.str();
}
void saveOpenWorld() { o.dirty = true; }
void flushOpenWorld() {
  if (!g.openWorld || g.path.empty() || !validId(o.id))
    return;
  // Preserve each original legacy record once before upgrading its container.
  std::string previous;
  if (readChecked(worldFile(o.id), previous)) {
    std::string suffix = previous.rfind("DWFRONTIER 1 ", 0) == 0   ? ".v03"
                         : previous.rfind("DWFRONTIER 2 ", 0) == 0 ? ".v04"
                         : previous.rfind("DWFRONTIER 3 ", 0) == 0 ? ".v05"
                                                                   : "";
    if (!suffix.empty() && !std::filesystem::exists(worldFile(o.id) + suffix)) {
      std::error_code ec;
      std::filesystem::copy_file(worldFile(o.id), worldFile(o.id) + suffix, ec);
      if (ec) {
        notify("LEGACY BACKUP FAILED - ORIGINAL SAVE NOT OVERWRITTEN");
        return;
      }
    }
  }
  if (atomicWorld(worldFile(o.id), encodeFrontier())) {
    o.dirty = false;
    o.saveTimer = 0;
  } else {
    notify("SAVE FAILED - CHECK FREE STORAGE BEFORE EXITING");
    o.dirty = true;
  }
}
bool header(const std::string &body, WorldRecord &r) {
  std::istringstream f(body);
  std::string magic;
  int version;
  double x, y, hp, heal;
  int flasks, kills, camps, discovered, w2, w3, w4;
  if (!(f >> magic >> version >> r.seed >> std::quoted(r.name) >> x >> y >>
        hp >> flasks >> heal >> r.seconds >> kills >> camps >> discovered >>
        r.modified >> w2 >> w3 >> w4))
    return false;
  r.generator = 1;
  if (version >= 2 && !(f >> r.generator))
    return false;
  return magic == "DWFRONTIER" && (version >= 1 && version <= 4) &&
         r.generator >= 1 && r.generator <= 5 && r.name.size() <= 24 &&
         std::isfinite(x) && std::isfinite(y) && std::abs(x) <= WORLD_LIMIT &&
         std::abs(y) <= WORLD_LIMIT && std::isfinite(r.seconds) &&
         r.seconds >= 0 && r.seconds <= 1e9;
}
void scanWorlds() {
  o.worlds.clear();
  if (g.path.empty())
    return;
  std::error_code ec;
  auto dir = g.path + "/worlds";
  if (!std::filesystem::exists(dir, ec))
    return;
  for (auto it = std::filesystem::directory_iterator(dir, ec);
       !ec && it != std::filesystem::directory_iterator(); it.increment(ec)) {
    std::string id = it->path().filename().string();
    if (!validId(id))
      continue;
    std::string body;
    WorldRecord r;
    r.id = id;
    if (!readChecked(worldFile(id), body)) {
      if (!readChecked(worldFile(id) + ".bak", body))
        continue;
      r.recovered = true;
    }
    if (!header(body, r))
      continue;
    std::istringstream f(body);
    std::string line, profile, magic;
    std::getline(f, line);
    f >> std::quoted(profile);
    std::istringstream p(profile);
    p >> magic >> r.level;
    o.worlds.push_back(r);
  }
  std::sort(o.worlds.begin(), o.worlds.end(), [](auto &a, auto &b) {
    return a.modified != b.modified ? a.modified > b.modified : a.id < b.id;
  });
  o.listPage =
      std::clamp(o.listPage, 0, std::max(0, (int(o.worlds.size()) - 1) / 3));
}
int biomeAtLegacy(int64_t x, int64_t y) {
  if (std::abs(x) < 12 && std::abs(y) < 12)
    return 0;
  int64_t cx = floorDiv(x, 48), cy = floorDiv(y, 48);
  double best = 1e30;
  int biome = 0;
  for (int j = -1; j <= 1; j++)
    for (int i = -1; i <= 1; i++) {
      int64_t ax = cx + i, ay = cy + j;
      uint64_t h = coordinateHash(ax, ay, 517);
      double dx = double(ax * 48 + 12 + int(h % 25)) - x,
             dy = double(ay * 48 + 12 + int((h >> 12) % 25)) - y;
      double d = dx * dx + dy * dy;
      if (d < best) {
        best = d;
        biome = floorMod(ax + ay * 2 + int(o.seed % 5), 5);
      }
    }
  return biome;
}
int biomeAt(int64_t x, int64_t y) {
  if (o.generator == 1)
    return biomeAtLegacy(x, y);
  const int region = o.generator >= 3 ? 32768 : 768;
  const int jitter = o.generator >= 3 ? 5461 : 128;
  int64_t cx = floorDiv(x + region / 2, region),
          cy = floorDiv(y + region / 2, region);
  double best = 1e30;
  int b = 0;
  for (int j = -1; j <= 1; j++)
    for (int i = -1; i <= 1; i++) {
      int64_t ax = cx + i, ay = cy + j;
      uint64_t h = coordinateHash(ax, ay, 517);
      double dx = double(ax * region + int(h % (2 * jitter + 1)) - jitter) - x,
             dy = double(ay * region + int((h >> 12) % (2 * jitter + 1)) -
                         jitter) -
                  y;
      double d = dx * dx + dy * dy;
      if (d < best) {
        best = d;
        b = (ax == 0 && ay == 0) ? 0
                                 : floorMod(ax + ay * 2 + int(o.seed % 5), 5);
      }
    }
  return b;
}
struct Camp {
  int64_t cx, cy, x, y;
  uint64_t hash;
  bool exists, boss;
  int guards, biome;
  float radius;
};
Camp campAt(int64_t cx, int64_t cy) {
  uint64_t h = coordinateHash(cx, cy, 9317);
  Camp c{cx,
         cy,
         cx * CELL + 13 + int(h % 7),
         cy * CELL + 13 + int((h >> 9) % 7),
         h,
         h % 5 < 3,
         h % 7 == 0,
         0,
         0,
         0};
  if (cx == 0 && cy == 0) {
    c.exists = true;
    c.boss = false;
  }
  if (cx == 1 && cy == 0) {
    c.exists = true;
    c.boss = true;
  }
  if (o.generator >= 5 && !(cy == 0 && (cx == 0 || cx == 1)))
    c.exists = h % 11 < 2;
  c.guards = c.boss ? 12 + int((h >> 21) % 5) : 5 + int((h >> 21) % 4);
  c.radius = c.boss ? 215.f : 125.f;
  c.biome = biomeAt(c.x, c.y);
  return c;
}
uint32_t killedMask(int64_t cx, int64_t cy) {
  auto it = o.slain.find({cx, cy});
  return it == o.slain.end() ? 0 : it->second;
}
uint32_t enemyMask(const Camp &c) {
  return ((1u << c.guards) - 1u) | (c.boss ? (1u << 20) : 0);
}
bool cleared(const Camp &c) {
  return (killedMask(c.cx, c.cy) & enemyMask(c)) == enemyMask(c);
}
float noiseAt(int64_t x, int64_t y, int scale, uint64_t salt) {
  int64_t ax = floorDiv(x, scale), ay = floorDiv(y, scale);
  float tx = float(floorMod(x, scale)) / scale,
        ty = float(floorMod(y, scale)) / scale;
  tx = tx * tx * (3 - 2 * tx);
  ty = ty * ty * (3 - 2 * ty);
  auto v = [&](int i, int j) {
    return float(coordinateHash(ax + i, ay + j, salt) % 65536) / 65535;
  };
  return (v(0, 0) * (1 - tx) + v(1, 0) * tx) * (1 - ty) +
         (v(0, 1) * (1 - tx) + v(1, 1) * tx) * ty;
}
struct Tile {
  uint8_t map = 1, ground = 0, biome = 0;
};
} // namespace av
#include "generation5.hpp"
namespace av {
Tile baseTileAt(int64_t x, int64_t y) {
  if (o.generator >= 5) return naturalTileAt(x,y);
  Tile t;
  t.biome = uint8_t(biomeAt(x, y));
  Camp c = campAt(floorDiv(x, CELL), floorDiv(y, CELL));
  int lx = floorMod(x, CELL), ly = floorMod(y, CELL);
  bool road = lx <= 1 || ly <= 1 || lx == 31 || ly == 31;
  bool trail = std::abs(x - c.x) <= 1 || std::abs(y - c.y) <= 1;
  double dx = double(x - c.x), dy = double(y - c.y);
  bool clearing = c.exists && (dx * dx + dy * dy < (c.boss ? 100 : 42));
  bool spawn = std::abs(x) < 7 && std::abs(y) < 7;
  float n = noiseAt(x, y, 12, 2345) * .65f + noiseAt(x, y, 5, 441) * .35f;
  bool wet = n < (t.biome == 3 ? .37f : .24f);
  if (road || trail || clearing || spawn) {
    t.ground = clearing ? 4 : road || trail ? 2 : 0;
    if (wet && (road || trail) && !clearing && !spawn)
      t.ground = 3;
  } else if (wet)
    t.map = 4;
  else {
    uint64_t h = coordinateHash(x, y, 987);
    int density = t.biome == 0 ? 8 : t.biome == 3 ? 7 : t.biome == 1 ? 3 : 5;
    if (int(h % 100) < density)
      t.map = 2;
    else if (int(h % 100) < density + 3)
      t.map = 3;
  }
  if (t.map == 2 && treeCut(x, y))
    t.map = 1;
  return t;
}
Tile tileAt(int64_t x, int64_t y) {
  Tile t = baseTileAt(x, y);
  modifyJourneyTile(x, y, t.map, t.ground, t.biome);
  return t;
}
void generateWindow() {
  g.props.clear();
  uint32_t oldRng = rng;
  for (int y = 0; y < MH; y++)
    for (int x = 0; x < MW; x++) {
      int a = y * MW + x;
      int64_t wx = o.originX + x, wy = o.originY + y;
      Tile t = tileAt(wx, wy);
      g.map[a] = t.map;
      g.ground[a] = t.ground;
      g.biomes[a] = t.biome;
      uint64_t h = coordinateHash(wx, wy, 987);
      int kind = t.map == 2 ? 0 : t.map == 3 ? 1 : -1;
      if (t.map == 1 && t.ground == 0) {
        if (h % 19 == 0)
          kind = 3;
        else if (h % 107 == 0)
          kind = 4;
      }
      if (treeCut(wx, wy))
        kind = 0;
      if (kind >= 0)
        g.props.push_back({float(x * T + 12), float(y * T + 12), kind,
                           int(h % 3), false, int(t.biome)});
    }
  // Only nearby cells are materialized. Defeated identities are a sparse save
  // delta.
  for (int64_t cy = floorDiv(o.originY, CELL) - 1;
       cy <= floorDiv(o.originY + MH, CELL) + 1; cy++)
    for (int64_t cx = floorDiv(o.originX, CELL) - 1;
         cx <= floorDiv(o.originX + MW, CELL) + 1; cx++) {
      Camp c = campAt(cx, cy);
      if (!c.exists)
        continue;
      float hx = float(c.x - o.originX) * T + 12,
            hy = float(c.y - o.originY) * T + 12;
      if (hx < -260 || hy < -260 || hx > MW * T + 260 || hy > MH * T + 260)
        continue;
      uint32_t mask = killedMask(cx, cy);
      if (hx > 0 && hy > 0 && hx < MW * T && hy < MH * T) {
        g.props.push_back(
            {hx, hy, 11, c.boss ? 1 : 0, cleared(c), c.biome, cx, cy});
        g.props.push_back({hx + 46, hy + 42, 10, 0, (mask & (1u << 24)) != 0,
                           c.biome, cx, cy});
      }
      for (int slot = 0; slot < c.guards + (c.boss ? 1 : 0); slot++) {
        int id = slot == c.guards ? 20 : slot;
        if (mask & (1u << id))
          continue;
        float angle =
            float((c.hash >> 32) % 628) / 100 + slot * 2 * PI / c.guards;
        float radius = c.boss ? 108 + (slot % 3) * 23 : 52 + (slot % 3) * 15;
        float x = id == 20 ? hx : hx + std::cos(angle) * radius,
              y = id == 20 ? hy : hy + std::sin(angle) * radius;
        if (x < 18 || y < 18 || x > MW * T - 18 || y > MH * T - 18 ||
            !fits(x, y, id == 20 ? 14 : 7))
          continue;
        bool present = false;
        for (auto &e : g.enemies)
          if (e.slot == id && e.campX == cx && e.campY == cy) {
            present = true;
            break;
          }
        if (present || g.enemies.size() >= 220)
          continue;
        int oldWorld = g.world, oldWins = g.worldWins[c.biome];
        g.worldWins[c.biome] = 0;
        float oldPower = g.runPower;
        g.world = c.biome;
        g.runPower = 22;
        rng = uint32_t(coordinateHash(cx, cy, 991 + id));
        addEnemy(x, y, id == 20 ? (c.biome == 0 ? 8 : 9) : slot % 5, 3,
                 id != 20 && slot == c.guards - 1);
        auto &e = g.enemies.back();
        e.slot = id;
        e.campX = cx;
        e.campY = cy;
        e.biome = c.biome;
        e.homeX = hx;
        e.homeY = hy;
        e.territory = c.radius;
        // Distance, not the order of visiting cells or equipped gear, sets
        // regional strength.
        float scale =
            1 +
            (o.generator >= 3
                 ? std::min(12.f, float(std::log1p(
                                      double(std::abs(cx) + std::abs(cy)))) *
                                      .22f)
                 : std::min(20.f, float(std::abs(cx) + std::abs(cy)) * .05f));
        e.hp = e.maxhp = e.maxhp * scale;
        e.cd = 1;
        g.world = oldWorld;
        g.runPower = oldPower;
        g.worldWins[c.biome] = oldWins;
      }
    }
  float homeX = float(-o.originX) * T, homeY = float(-o.originY) * T;
  if (homeX > 0 && homeY > 0 && homeX < MW * T && homeY < MH * T)
    g.props.push_back({homeX, homeY, 12, 0, false, 0});
  rng = oldRng;
  lifeProps();
  addPlayerFires();
  refreshJourneyTerrain();
  seedWildlife();
  navigation();
}
void rebase(int dx, int dy) {
  if (!dx && !dy)
    return;
  o.originX += dx;
  o.originY += dy;
  float xx = float(dx * T), yy = float(dy * T);
  auto shift = [&](float &x, float &y) {
    x -= xx;
    y -= yy;
  };
  shift(g.px, g.py);
  shift(g.previousX, g.previousY);
  shift(g.camx, g.camy);
  shift(g.wardx, g.wardy);
  shift(g.bossX, g.bossY);
  for (auto &e : g.enemies) {
    shift(e.x, e.y);
    shift(e.previousX, e.previousY);
    shift(e.tx, e.ty);
    shift(e.homeX, e.homeY);
  }
  for (auto &a : g.animals) {
    shift(a.x, a.y);
    shift(a.previousX, a.previousY);
    shift(a.homeX, a.homeY);
    shift(a.goalX, a.goalY);
  }
  std::erase_if(g.animals, [](auto &a) {
    return a.x < -80 || a.y < -80 || a.x > MW * T + 80 || a.y > MH * T + 80;
  });
  for (auto &f : w.falling)
    shift(f.x, f.y);
  for (auto &b : g.bolts)
    shift(b.x, b.y);
  for (auto &p : g.particles)
    shift(p.x, p.y);
  for (auto &f : g.floats)
    shift(f.x, f.y);
  for (auto &d : g.drops)
    shift(d.x, d.y);
  for (auto &h : g.hazards)
    shift(h.x, h.y);
  for (auto &e : g.echoes)
    shift(e.x, e.y);
  std::erase_if(g.enemies, [](auto &e) {
    return e.x < -100 || e.y < -100 || e.x > MW * T + 100 || e.y > MH * T + 100;
  });
  std::erase_if(g.drops, [](auto &d) {
    return d.x < -100 || d.y < -100 || d.x > MW * T + 100 || d.y > MH * T + 100;
  });
  generateWindow();
}
void placePlayer(double x, double y) {
  o.originX = int64_t(std::floor(x)) - MW / 2;
  o.originY = int64_t(std::floor(y)) - MH / 2;
  g.px = float((x - o.originX) * T);
  g.py = float((y - o.originY) * T);
  g.enemies.clear();
  g.animals.clear();
  w.falling.clear();
  w.revealX = INT64_MAX;
  g.enemies.reserve(256);
  g.camReady = false;
  generateWindow();
  if (!fits(g.px, g.py)) {
    bool found = false;
    for (int r = 1; r < 12 && !found; r++)
      for (int j = -r; j <= r && !found; j++)
        for (int i = -r; i <= r && !found; i++) {
          float xx = (MW / 2 + i) * T + 12, yy = (MH / 2 + j) * T + 12;
          if (fits(xx, yy)) {
            g.px = xx;
            g.py = yy;
            found = true;
          }
        }
  }
  g.world =
      biomeAt(int64_t(std::floor(globalX())), int64_t(std::floor(globalY())));
}
bool decodeFrontier(const std::string &body) {
  WorldRecord rec;
  if (!header(body, rec))
    return false;
  std::istringstream f(body);
  std::string magic, name, profile;
  int ver, flasks, kills, camps, discovered, w[3];
  uint64_t seed;
  int64_t modified;
  double x, y, hp, heal, seconds;
  size_t count;
  f >> magic >> ver >> seed >> std::quoted(name) >> x >> y >> hp >> flasks >>
      heal >> seconds >> kills >> camps >> discovered >> modified >> w[0] >>
      w[1] >> w[2];
  int generator = 1;
  if (ver >= 2)
    f >> generator;
  f >> std::quoted(profile) >> count;
  if (!f || count > 200000 || profile.size() > 12000 || flasks < 0 ||
      flasks > 3 || !std::isfinite(hp) || hp < 0 || hp > 2000 ||
      !std::isfinite(heal) || heal < 0 || heal > 20 || kills < 0 || camps < 0 ||
      discovered < 0 || discovered > 31)
    return false;
  for (int v : w)
    if (v < 0 || v > 10000)
      return false;
  std::map<std::pair<int64_t, int64_t>, uint32_t> slain;
  for (size_t k = 0; k < count; k++) {
    int64_t cx, cy;
    uint32_t mask;
    if (!(f >> cx >> cy >> mask) || cx < -WORLD_LIMIT / CELL - 2 ||
        cx > WORLD_LIMIT / CELL + 2 || cy < -WORLD_LIMIT / CELL - 2 ||
        cy > WORLD_LIMIT / CELL + 2 || (mask & ~0x011fffffu))
      return false;
    slain[{cx, cy}] = mask;
  }
  Survival life;
  if (ver >= 2) {
    std::string raw;
    if (!(f >> std::quoted(raw)) || !decodeLife(raw, life))
      return false;
  }
  Wildlands forest;
  if (ver >= 3) {
    std::string raw;
    if (!(f >> std::quoted(raw)) || !decodeWildlands(raw, forest))
      return false;
  } else
    forest.clockOffset =
        seconds /
        3.0; // Retain old day/hour when changing an 18-minute day to 24.
  Journey journey;
  if (ver >= 4) {
    std::string raw;
    if (!(f >> std::quoted(raw)) || !decodeJourney(raw, journey))
      return false;
  }
  bool mute = g.muted, shake = g.shake, low = g.lowPower;
  if (!decode(profile))
    return false;
  g.muted = mute;
  g.shake = shake;
  g.lowPower = low;
  o.generator = generator;
  v = std::move(life);
  av::w = std::move(forest);
  av::j = std::move(journey);
  o.seed = seed;
  o.name = cleanName(name);
  o.seconds = seconds;
  o.totalKills = kills;
  o.campsCleared = camps;
  o.discovered = discovered;
  o.slain = std::move(slain);
  for (int i = 0; i < 3; i++)
    g.worldWins[i + 2] = w[i];
  g.openWorld = true;
  g.scene = PLAY;
  resetCombat();
  g.scene = PLAY;
  g.gate = true;
  g.bossKilled = false;
  std::fill(g.rooms, g.rooms + 3, true);
  g.flasks = flasks;
  g.healCd = float(heal);
  g.hp = float(std::min(hp, double(maxhp())));
  g.invul = 1.5f;
  if (g.hp <= 0) {
    v.injury.fill(0);
    v.bleeding.fill(0);
    v.food = std::max(55.f, v.food);
    v.water = std::max(55.f, v.water);
    v.stamina = 100;
    v.task = 0;
    v.sprint = false;
    g.hp = float(maxhp());
    g.flasks = 3;
    x = av::j.home ? double(av::j.homeX) + .5 : 0;
    y = av::j.home ? double(av::j.homeY) + .5 : 0;
  }
  g.runPower = damage();
  placePlayer(x, y);
  revealTerrain();
  o.lastBiome = g.world;
  o.saveTimer = 0;
  o.dirty = false;
  return true;
}
bool enterWorld(const std::string &id) {
  if (!validId(id))
    return false;
  std::string body;
  bool recovered = false;
  if (!readChecked(worldFile(id), body) || !decodeFrontier(body)) {
    if (!readChecked(worldFile(id) + ".bak", body) || !decodeFrontier(body)) {
      notify("WORLD COULD NOT BE READ - ORIGINAL FILES KEPT");
      return false;
    }
    recovered = true;
  }
  o.id = id;
  notify(recovered ? "BACKUP RECOVERED - YOUR WORLD IS SAFE"
                   : o.name + " - JOURNEY RESUMED");
  sfx(5);
  return true;
}
bool createWorld() {
  if (g.path.empty()) {
    notify("NO SAVE DIRECTORY AVAILABLE");
    return false;
  }
  std::string path = g.path;
  bool mute = g.muted, shake = g.shake, low = g.lowPower;
  int diff = g.difficulty, weapon = g.weapon;
  g = State{};
  g.path = path;
  g.muted = mute;
  g.shake = shake;
  g.lowPower = low;
  g.difficulty = diff;
  g.weapon = weapon;
  v = Survival{};
  av::w = Wildlands{};
  av::j = Journey{};
  av::j.stock[SEED] = 3;
  o.atlasLarge = false;
  o.generator = 5;
  o.seed = o.draftSeed;
  o.name = cleanName(o.draftName);
  o.slain.clear();
  o.seconds = 0;
  o.totalKills = o.campsCleared = o.discovered = 0;
  o.waypoint = false;
  do {
    o.id = hexId(entropy());
  } while (std::filesystem::exists(worldFile(o.id)));
  g.openWorld = true;
  g.scene = PLAY;
  resetCombat();
  g.scene = PLAY;
  g.gate = true;
  std::fill(g.rooms, g.rooms + 3, true);
  g.runPower = damage();
  placePlayer(0, 0);
  revealTerrain();
  o.lastBiome = g.world;
  o.dirty = true;
  flushOpenWorld();
  if (o.dirty) {
    g.openWorld = false;
    g.scene = WORLDS;
    return false;
  }
  notify("NEW WORLD - BODY OPENS FOOD, WATER AND INJURIES.");
  sfx(5);
  return true;
}
void startTransition(Scene next) {
  clearInput();
  o.transition = .44f;
  o.transitionChanged = false;
  o.transitionTo = next;
  sfx(15);
}
void leaveFrontier() {
  if (g.openWorld) {
    flushOpenWorld();
    if (o.dirty && !g.path.empty())
      return;
  }
  clearInput();
  g.openWorld = false;
  g.overlay = 0;
  scanWorlds();
  startTransition(WORLDS);
}
void respawnFrontier() {
  v.injury.fill(0);
  v.bleeding.fill(0);
  v.food = std::max(55.f, v.food);
  v.water = std::max(55.f, v.water);
  v.stamina = 100;
  v.task = 0;
  v.sprint = false;
  resetCombat();
  g.scene = PLAY;
  g.gate = true;
  g.bossKilled = false;
  j.active = false;
  j.z = 0;
  j.vz = 0;
  j.stance = 0;
  j.pose = 0;
  j.guard = 0;
  placePlayer(j.home ? j.homeX + .5 : 0, j.home ? j.homeY + .5 : 0);
  o.dirty = true;
  flushOpenWorld();
  notify("RESPAWNED AT ORIGIN - COLLECTED GEAR IS SAFE");
}
void frontierKilled(Enemy &e) {
  o.totalKills++;
  o.dirty = true;
  if (e.slot >= 0) {
    o.slain[{e.campX, e.campY}] |= 1u << e.slot;
    Camp c = campAt(e.campX, e.campY);
    if (cleared(c)) {
      o.campsCleared++;
      notify("TERRITORY CLEARED - OPEN ITS CHEST");
      sfx(5);
    }
  }
  if (e.boss()) {
    g.wins++;
    g.worldWins[e.biome]++;
    g.flasks = std::min(3, g.flasks + 1);
    g.hp = std::min(float(maxhp()), g.hp + maxhp() * .2f);
    notify("GUARDIAN DEFEATED - THE OPEN WORLD CONTINUES");
    sfx(5);
  }
}
bool frontierIdle(Enemy &e, float dt) {
  if (e.slot < 0)
    return false;
  float homeDistance = len(g.px - e.homeX, g.py - e.homeY),
        leash = len(e.x - e.homeX, e.y - e.homeY);
  if (homeDistance < e.territory * stealthFactor() || e.flash > 0)
    e.alertTime = 8;
  if (e.alertTime > 0 && homeDistance < e.territory + 220 &&
      leash < e.territory + 180) {
    e.alertTime = std::max(0.f, e.alertTime - dt);
    e.routine = -1;
    e.chore = 0;
    return false;
  }
  e.alertTime = 0;
  e.wind = 0;
  e.state = 0;
  e.vx = e.vy = 0;
  e.hp = std::min(e.maxhp, e.hp + e.maxhp * .08f * dt);
  routineTick(e, dt);
  return true;
}
void findBiome(int b) {
  int64_t px = int64_t(globalX()), py = int64_t(globalY()), bx = px, by = py;
  double best = 1e30;
  for (int y = -24; y <= 24; y++)
    for (int x = -24; x <= 24; x++) {
      int step = o.generator >= 3 ? 2048 : o.generator == 2 ? 64 : 8;
      int64_t tx = px + x * step, ty = py + y * step;
      if (biomeAt(tx, ty) != b || tileAt(tx, ty).map != 1)
        continue;
      double d = double(x) * x + double(y) * y;
      if (d < best) {
        best = d;
        bx = tx;
        by = ty;
      }
    }
  if (best < 1e29) {
    o.waypoint = true;
    o.waypointX = bx;
    o.waypointY = by;
    notify(std::string("COMPASS SET: ") + biomeName(b));
  } else
    notify("BIOME BEYOND LOCAL ATLAS - KEEP EXPLORING");
}
void frontierStep(float dt) {
  int dx = 0, dy = 0;
  if (g.px < 24 * T)
    dx = -16;
  else if (g.px > 48 * T)
    dx = 16;
  if (g.py < 18 * T)
    dy = -12;
  else if (g.py > 34 * T)
    dy = 12;
  if (std::abs(o.originX + dx) > WORLD_LIMIT - MW ||
      std::abs(o.originY + dy) > WORLD_LIMIT - MH) {
    notify("ENGINE COORDINATE LIMIT - TURN BACK");
    dx = dy = 0;
  }
  rebase(dx, dy);
  revealTerrain();
  wildlandsTick(dt);
  int b = g.biomes[std::clamp(int(g.py / T), 0, MH - 1) * MW +
                   std::clamp(int(g.px / T), 0, MW - 1)];
  g.world = b;
  o.discovered |= 1 << b;
  if (o.lastBiome != b) {
    o.lastBiome = b;
    notify(std::string("ENTERING ") + biomeName(b));
  }
  for (auto &p : g.props)
    if (p.kind == 10 && !p.open && len(g.px - p.x, g.py - p.y) < 40) {
      Camp c = campAt(p.campX, p.campY);
      if (cleared(c)) {
        p.open = true;
        o.slain[{c.cx, c.cy}] |= 1u << 24;
        g.gold += c.boss ? 120 : 35;
        v.meals = std::min(30, v.meals + 2);
        v.cleanWater = std::min(8, v.cleanWater + 1);
        v.bandages = std::min(20, v.bandages + 1);
        v.splints = std::min(10, v.splints + 1);
        g.flasks = std::min(3, g.flasks + 1);
        g.hp = std::min(float(maxhp()), g.hp + maxhp() * .15f);
        g.drops.push_back({p.x, p.y, 0,
                           Item{g.uid++, 0, 1 + g.level / 2 + c.biome, 2}, true,
                           0});
        o.dirty = true;
        notify("CAMP CHEST - GEAR, GOLD AND A FLASK");
        sfx(6);
      }
    }
  o.seconds += dt;
  o.saveTimer += dt;
  if (o.saveTimer >= 8) {
    o.dirty = true;
    flushOpenWorld();
  }
}
void frontierTick(float dt) {
  o.screenTime += dt;
  if (o.transition > 0) {
    o.transition -= dt;
    if (o.transition < .22f && !o.transitionChanged) {
      g.scene = o.transitionTo;
      g.overlay = 0;
      o.screenTime = 0;
      o.transitionChanged = true;
    }
  }
  if (g.scene == SPLASH && o.screenTime > 3.1f && o.transition <= 0)
    startTransition(HOME);
  if (g.scene == LOADING && o.screenTime > .35f && o.transition <= 0) {
    bool ok = o.pendingCreate
                  ? createWorld()
                  : (o.selected >= 0 && o.selected < int(o.worlds.size()) &&
                     enterWorld(o.worlds[o.selected].id));
    if (!ok)
      g.scene = WORLDS;
  }
}
} // namespace av
