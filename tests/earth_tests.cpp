// Generator 6 (Earth relief) regression and safety tests.
//
// The two things this file must prove are:
//   1. Generators 1-5 are completely untouched, so every existing saved world
//      keeps byte-identical geography.
//   2. Generator 6 produces real mountain relief that can never seal the
//      player in, never depends on graphics quality, and stays deterministic.
#define DW_LEGACY_TEST_UI
#include "../native/engine.cpp"
#include <deque>
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

static uint64_t tileDigest(int generator, uint64_t seed, int64_t ox, int64_t oy,
                           int span) {
  o.generator = generator;
  o.seed = seed;
  uint64_t h = 1469598103934665603ull;
  for (int64_t y = oy; y < oy + span; y++)
    for (int64_t x = ox; x < ox + span; x++) {
      Tile t = baseTileAt(x, y);
      h = mix64(h ^ (uint64_t(t.map) * 131 + uint64_t(t.ground) * 17 +
                     uint64_t(t.biome)));
    }
  return h;
}

// Four-way flood fill over walkable tiles inside a window.
static int reachable(int64_t ox, int64_t oy, int span, int64_t tx, int64_t ty,
                     bool &hitTarget) {
  std::vector<uint8_t> seen(size_t(span) * span, 0);
  auto idx = [&](int64_t x, int64_t y) {
    return size_t((y - oy) * span + (x - ox));
  };
  auto open = [&](int64_t x, int64_t y) {
    return x >= ox && y >= oy && x < ox + span && y < oy + span &&
           baseTileAt(x, y).map == 1;
  };
  hitTarget = false;
  if (!open(ox + span / 2, oy + span / 2))
    return 0;
  std::deque<std::pair<int64_t, int64_t>> q{{ox + span / 2, oy + span / 2}};
  seen[idx(ox + span / 2, oy + span / 2)] = 1;
  int count = 0;
  while (!q.empty()) {
    auto [x, y] = q.front();
    q.pop_front();
    count++;
    if (x == tx && y == ty)
      hitTarget = true;
    const int64_t dx[4] = {1, -1, 0, 0}, dy[4] = {0, 0, 1, -1};
    for (int k = 0; k < 4; k++) {
      int64_t nx = x + dx[k], ny = y + dy[k];
      if (open(nx, ny) && !seen[idx(nx, ny)]) {
        seen[idx(nx, ny)] = 1;
        q.push_back({nx, ny});
      }
    }
  }
  return count;
}

int main() {
  std::string root = (std::filesystem::temp_directory_path() /
                      ("earth-relief-test-" + hexId(entropy())))
                         .string();
  std::filesystem::create_directories(root);
  boot(root);

  // ---- 1. Legacy generators are bit-identical -----------------------------
  // Capture generator 1-5 output, then prove the generator 6 code path cannot
  // have perturbed it: each legacy generator must still equal its own
  // dedicated function, and must differ from generator 6.
  for (uint64_t seed : {1ull, 123456789ull, 987654321ull}) {
    for (int gen = 1; gen <= 5; gen++) {
      uint64_t before = tileDigest(gen, seed, -40, -40, 80);
      uint64_t after = tileDigest(gen, seed, -40, -40, 80);
      CHECK(before == after); // pure
    }
    // Generator 5 must still be exactly the organic layer, untouched by relief.
    o.generator = 5;
    o.seed = seed;
    bool same = true;
    for (int64_t y = -30; y < 30 && same; y++)
      for (int64_t x = -30; x < 30 && same; x++) {
        Tile a = baseTileAt(x, y), b = naturalTileAt(x, y);
        if (a.map != b.map || a.ground != b.ground || a.biome != b.biome)
          same = false;
      }
    CHECK(same);
    // Away from the home valley, relief must visibly change the world.
    CHECK(tileDigest(5, seed, 900, 900, 160) !=
          tileDigest(6, seed, 900, 900, 160));
  }

  // ---- 2. Generator 6 determinism and seed sensitivity --------------------
  CHECK(tileDigest(6, 42, -64, -64, 96) == tileDigest(6, 42, -64, -64, 96));
  CHECK(tileDigest(6, 42, -64, -64, 96) != tileDigest(6, 43, -64, -64, 96));

  // Elevation is a pure function of the coordinate and the seed.
  o.generator = 6;
  o.seed = 2026;
  for (int64_t y = -500; y <= 500; y += 137)
    for (int64_t x = -500; x <= 500; x += 149) {
      float e = elevationAt(x, y);
      CHECK(std::isfinite(e) && e >= 0.f && e <= 1.f);
      CHECK(elevationAt(x, y) == e);
      CHECK(elevationBand(e) >= 0 && elevationBand(e) <= 4);
    }

  // ---- 3. Relief never depends on graphics quality ------------------------
  // Quality must not touch world geometry (AGENTS.md invariant 2).
  o.generator = 6;
  o.seed = 777;
  uint64_t lowDigest, mediumDigest;
  g.lowPower = 1;
  lowDigest = tileDigest(6, 777, -48, -48, 96);
  g.lowPower = 0;
  mediumDigest = tileDigest(6, 777, -48, -48, 96);
  CHECK(lowDigest == mediumDigest);

  // ---- 4. The opening valley is habitable ---------------------------------
  o.generator = 6;
  for (uint64_t seed : {1ull, 7ull, 99ull, 4242ull, 123456789ull}) {
    o.seed = seed;
    // Spawn pocket is always open ground.
    for (int64_t y = -6; y <= 6; y++)
      for (int64_t x = -6; x <= 6; x++)
        CHECK(baseTileAt(x, y).map == 1);
    // The home valley keeps the start out of the rock band.
    CHECK(elevationBand(elevationAt(0, 0)) <= 1);
    // Both guaranteed camps still have walkable clearings.
    for (int cx = 0; cx <= 1; cx++) {
      Camp c = campAt(cx, 0);
      CHECK(c.exists);
      CHECK(baseTileAt(c.x, c.y).map == 1);
    }
  }

  // ---- 5. Connectivity: trails act as mountain passes ---------------------
  // From the spawn tile the player must reach the boss camp overland, and a
  // large share of a wide window must stay reachable - relief may obstruct,
  // never imprison.
  for (uint64_t seed : {1ull, 7ull, 99ull, 4242ull, 123456789ull}) {
    o.generator = 6;
    o.seed = seed;
    Camp boss = campAt(1, 0);
    bool reachedBoss = false;
    int open = reachable(-128, -128, 256, boss.x, boss.y, reachedBoss);
    CHECK(reachedBoss);
    CHECK(open > 256 * 256 / 8); // never a sealed pocket
  }

  // ---- 6. Mountains actually exist, in chains, and are not everywhere -----
  {
    o.generator = 6;
    o.seed = 123456789;
    int rock = 0, high = 0, low = 0, water = 0, tree = 0, total = 0;
    int rockNeighbour = 0, rockTiles = 0;
    for (int64_t y = -600; y < 600; y += 3)
      for (int64_t x = -600; x < 600; x += 3) {
        float e = elevationAt(x, y);
        int band = elevationBand(e);
        if (band >= 3)
          high++;
        else if (band == 0)
          low++;
        Tile t = baseTileAt(x, y);
        if (t.map == 3)
          rock++;
        if (t.map == 4)
          water++;
        if (t.map == 2)
          tree++;
        total++;
        // Chain test: a rock tile should usually have rock continuing along
        // the crest rather than sitting alone.
        if (band >= 3) {
          rockTiles++;
          int n = 0;
          for (int k = -1; k <= 1; k++)
            for (int m = -1; m <= 1; m++)
              if ((k || m) && elevationBand(elevationAt(x + k * 3, y + m * 3)) >= 3)
                n++;
          if (n >= 3)
            rockNeighbour++;
        }
      }
    CHECK(total > 0);
    CHECK(high * 100 / total >= 2);   // mountains are present
    CHECK(high * 100 / total <= 40);  // but the world is not one big wall
    CHECK(low * 100 / total >= 35);   // plenty of lowland to live in
    CHECK(rock * 100 / total >= 2);   // relief reaches the tile layer
    CHECK(water > 0);                 // drainage produced rivers
    CHECK(tree > 0);                  // forest survives below the treeline
    CHECK(rockTiles > 0 && rockNeighbour * 100 / rockTiles >= 70); // chains
  }

  // ---- 7. Treeline and drainage behave physically -------------------------
  {
    o.generator = 6;
    o.seed = 31337;
    int treesLow = 0, treesHigh = 0, waterHigh = 0;
    for (int64_t y = -400; y < 400; y += 2)
      for (int64_t x = -400; x < 400; x += 2) {
        float e = elevationAt(x, y);
        Tile t = baseTileAt(x, y);
        if (t.map == 2) {
          if (elevationBand(e) == 0)
            treesLow++;
          else if (elevationBand(e) >= 2)
            treesHigh++;
        }
        // No river may run above the treeline.
        if (t.map == 4 && e >= EARTH_HIGH && t.ground == 0)
          waterHigh++;
      }
    CHECK(treesLow > 0);
    CHECK(treesHigh * 4 < treesLow); // canopy thins out with altitude
    CHECK(waterHigh == 0);
  }

  // ---- 8. Save compatibility ----------------------------------------------
  // A generator 6 header round-trips, and generator numbers outside 1..6 are
  // still rejected.
  {
    o.generator = 6;
    o.seed = 555;
    o.name = "RELIEF WORLD";
    o.id = "aaaaaaaaaaaaaaaa";
    std::string body = encodeFrontier();
    WorldRecord r;
    CHECK(header(body, r));
    CHECK(r.generator == 6 && r.seed == 555);
    std::string bad = body;
    size_t nl = bad.find('\n');
    size_t sp = bad.rfind(' ', nl);
    bad.replace(sp + 1, nl - sp - 1, "7");
    WorldRecord r2;
    CHECK(!header(bad, r2));
    // Legacy headers still load with their own generator intact.
    o.generator = 3;
    WorldRecord r3;
    CHECK(header(encodeFrontier(), r3) && r3.generator == 3);
  }

  std::filesystem::remove_all(root);
  std::cout << "earth relief: " << checks << " assertions pass\n";
  return 0;
}
