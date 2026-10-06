#pragma once
// Generator 6 — Earth-like relief: continental elevation, ridged mountain
// chains, foothills, a treeline and downhill drainage.
//
// INVARIANTS (see AGENTS.md):
//  * Used ONLY by worlds created as generator 6. Generators 1-5 never call any
//    function in this file, so existing saved geography is bit-identical.
//  * Coordinate-pure and deterministic: the result for (x,y) depends only on
//    the coordinate and `o.seed`. No frame state, no RNG draw, no cache that
//    could change between runs, and no dependency on graphics quality.
//  * Relief never changes simulation rules, enemy difficulty, loot or progress.
//    It only decides which tile sits at a coordinate.
//  * Walkability is guaranteed on top of the generator 5 organic layer: the
//    spawn pocket, camp clearings and the whole trail graph stay passable, so
//    trails act as mountain passes and the world can never be sealed off.
namespace av {

// Ridged noise: folds the [0,1] band around its centre so the maxima form
// continuous crests instead of round blobs. This is what turns scattered highs
// into mountain *chains*.
inline float ridgeNoise(int64_t x, int64_t y, int scale, uint64_t salt) {
  return 1.f - std::fabs(noiseAt(x, y, scale, salt) * 2.f - 1.f);
}

// Orogenic belt mask — where on the map mountain building is allowed at all.
// Without it the ridges would cover the whole world uniformly.
inline float orogenicBelt(int64_t x, int64_t y) {
  float belt = ridgeNoise(x, y, 416, 60221 + o.seed % 991) * .68f +
               ridgeNoise(x, y, 144, 62311 + o.seed % 991) * .32f;
  return std::clamp((belt - .52f) / .34f, 0.f, 1.f);
}

// Distance-based softening so the first valley the player ever sees is
// habitable: camp, boss camp and the opening trails are not walled in.
inline float homeValley(int64_t x, int64_t y) {
  double d = std::hypot(double(x), double(y));
  if (d >= 260.)
    return 0.f;
  float t = float(1. - d / 260.);
  return t * t;
}

// Continental elevation in [0,1]. Low ground is plains and wetland, the upper
// bands are foothills, highland, bare rock and finally peaks.
float elevationAt(int64_t x, int64_t y) {
  uint64_t s = o.seed % 7919;
  // The octave scales stay well inside normal travel distance. A very large
  // scale would be almost constant across a play session and would turn whole
  // seeds into "all mountain" or "all flat" worlds instead of giving every
  // world both plains and ranges.
  float continent = noiseAt(x, y, 320, 14011 + s) * .46f +
                    noiseAt(x, y, 112, 30211 + s) * .29f +
                    noiseAt(x, y, 40, 51217 + s) * .16f +
                    noiseAt(x, y, 14, 72313 + s) * .09f;
  float chain = ridgeNoise(x, y, 224, 41213 + s) * .58f +
                ridgeNoise(x, y, 80, 46619 + s) * .28f +
                ridgeNoise(x, y, 26, 49121 + s) * .14f;
  float belt = orogenicBelt(x, y);
  // Centred on the octave mean so the band mix is stable across seeds, then
  // mountains rise out of that base rather than replacing it: ranges sit on
  // raised ground and fall away into foothills.
  float e = .318f + (continent - .5f) * .60f + belt * chain * .52f;
  e -= homeValley(x, y) * .30f;
  return std::clamp(e, 0.f, 1.f);
}

// Band thresholds. Kept as named constants so tests and future art can share
// them instead of re-deriving magic numbers.
constexpr float EARTH_HILL = .455f;  // foothills / broken ground
constexpr float EARTH_HIGH = .560f;  // above the treeline
constexpr float EARTH_ROCK = .655f;  // impassable mountain mass
constexpr float EARTH_PEAK = .790f;  // bare peaks

inline int elevationBand(float e) {
  return e >= EARTH_PEAK   ? 4
         : e >= EARTH_ROCK ? 3
         : e >= EARTH_HIGH ? 2
         : e >= EARTH_HILL ? 1
                           : 0;
}

// Local steepness, used to keep cliffs rocky and benches walkable.
inline float elevationSlope(int64_t x, int64_t y) {
  float e = elevationAt(x, y);
  float dx = std::fabs(elevationAt(x + 3, y) - e);
  float dy = std::fabs(elevationAt(x, y + 3) - e);
  return dx + dy;
}

// Drainage channels: rivers follow narrow lines seeded independently of the
// mountains, then only survive where the land is actually low enough to carry
// water. Widening at low elevation reads as a river mouth or a lake margin.
bool drainageAt(int64_t x, int64_t y, float e) {
  if (e >= EARTH_HIGH)
    return false;
  float channel = ridgeNoise(x, y, 512, 88237 + o.seed % 613) * .74f +
                  ridgeNoise(x, y, 128, 90313 + o.seed % 613) * .26f;
  float width = .962f - std::max(0.f, (EARTH_HILL - e)) * .10f;
  return channel > width;
}

Tile earthTileAt(int64_t x, int64_t y) {
  // Start from the generator 5 organic layer. It already guarantees the spawn
  // pocket, camp clearings and the trail graph, which is exactly the
  // walkability contract relief must not break.
  Tile t = naturalTileAt(x, y);
  bool spawn = std::abs(x) < 7 && std::abs(y) < 7;
  bool carved = t.ground != 0 || spawn; // trail, mud, clearing or spawn
  float e = elevationAt(x, y);
  int band = elevationBand(e);
  if (carved) {
    // A pass stays open, but it looks stony once it climbs out of the forest.
    if (band >= 2 && t.ground == 3)
      t.ground = 2;
    if (band >= 2 && t.map == 2)
      t.map = 1;
    return t;
  }
  if (drainageAt(x, y, e)) {
    t.map = 4;
    t.ground = 0;
    return t;
  }
  uint64_t h = coordinateHash(x, y, 60617);
  switch (band) {
  case 0:
    break; // plains and forest exactly as the organic layer produced them
  case 1:
    // Foothills: thinner canopy, scattered boulders and drier ground.
    if (t.map == 2 && h % 100 < 26)
      t.map = 1;
    if (t.map == 1 && (h >> 11) % 100 < 7)
      t.map = 3;
    if (t.map == 4)
      t.map = 1;
    break;
  case 2:
    // Above the treeline: alpine meadow, stony footing, frequent outcrops.
    if (t.map == 2)
      t.map = (h % 100 < 18) ? 2 : 1;
    if (t.map == 1 && (h >> 11) % 100 < 19)
      t.map = 3;
    if (t.map == 4)
      t.map = 1;
    t.ground = 2;
    break;
  default: {
    // Mountain mass. Gentle benches and saddles inside a range stay walkable
    // so a route can exist even away from the trail graph; genuine cliffs and
    // peaks are solid. The bench field is smooth noise rather than a per-tile
    // hash, so shelves come out as connected ledges instead of scattered
    // single tiles.
    float bench = noiseAt(x, y, 18, 27361) * .6f + noiseAt(x, y, 7, 29383) * .4f;
    bool shelf = band == 3 && elevationSlope(x, y) < .013f && bench > .52f;
    t.map = shelf ? 1 : 3;
    t.ground = shelf ? 2 : 0;
    break;
  }
  }
  if (t.map == 2 && treeCut(x, y))
    t.map = 1;
  return t;
}
} // namespace av
