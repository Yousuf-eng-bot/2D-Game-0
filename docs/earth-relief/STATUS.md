# Generator 6 — Earth-like relief (phase 1)

Date: 2026-10-06. Source base: 0.9.2 / Android versionCode 11.

This is the first phase of the owner's second priority: *"পৃথিবীর মতো আরও ভালো
world — শুরু থেকেই পাহাড়, ভূপ্রকৃতি ও আরও প্রাকৃতিক উপাদান"*, planned so that
existing saved geography is not broken.

## What was added

A new versioned generator, **generator 6**, in `native/generation6.hpp`. It is
selected only by worlds created from this build onward (`createWorld()` sets
`o.generator = 6`). Generators 1–5 are not touched by a single line of logic.

Relief model, all coordinate-pure and seeded from `o.seed`:

- **Continental elevation** — four octaves of value noise (scales 320 / 112 /
  40 / 14), centred on the octave mean so the band mix is stable from seed to
  seed instead of producing "all mountain" or "all flat" worlds.
- **Mountain chains** — ridged noise (`1 - |2n-1|`) at scales 224 / 80 / 26,
  multiplied by an **orogenic belt** mask so ranges form localised chains
  rather than covering the whole map.
- **Home valley** — elevation is softened within 260 tiles of the origin, so
  the opening camp, the boss camp and the first trails stay in habitable
  lowland.
- **Bands** — lowland / foothills / above-treeline / rock / peak, with named
  constants `EARTH_HILL`, `EARTH_HIGH`, `EARTH_ROCK`, `EARTH_PEAK`.
- **Treeline** — canopy density falls with altitude and stops above
  `EARTH_HIGH`; stony ground replaces grass.
- **Benches and passes** — inside the rock band, low-slope shelves selected by
  *smooth* noise stay walkable, so shelves are connected ledges, not
  salt-and-pepper single tiles.
- **Drainage** — ridged channel noise, gated to below the treeline and widening
  as the land drops, gives rivers that run into lake-like mouths. No river can
  exist above the treeline (asserted in tests).

### Measured band distribution

Averaged over 12 seeds, a 1400×1400 tile window:

| band | lowland | foothills | above treeline | rock | peak |
| ---- | ------- | --------- | -------------- | ---- | ---- |
| %    | 47.8    | 22.8      | 17.2           | 10.6 | 1.6  |

## How existing worlds are protected

1. `baseTileAt` dispatches `>= 6` to the new code and `== 5` to the untouched
   organic layer; everything below is byte-identical legacy code.
2. The save container format is **unchanged** (`DWFRONTIER 4`). The generator
   number has been stored since container version 2, so a generator 6 world
   needs no new field. Header validation widened from `1..5` to `1..6` only.
3. The existing golden-save migration test still reports **4734 exact terrain
   records, progress and generator pinning, untouched archival copy**.
4. No change to simulation, combat, AI, RNG, inventory, progress or the
   quality policy. Relief only decides which tile sits at a coordinate.

## Walkability contract

Generator 6 is layered **on top of** generator 5's organic layer, which already
guarantees the spawn pocket, camp clearings and the trail graph. Relief is not
allowed to overwrite those, so trails behave as mountain passes and a world can
never be sealed. This is asserted, not assumed:

- the 13×13 spawn pocket is open ground on every tested seed;
- both guaranteed camps have walkable clearings;
- a 4-way flood fill from spawn reaches the boss camp on every tested seed;
- more than ⅛ of a 256×256 window stays reachable from spawn.

## Validation actually executed

Host: Debian container, GCC 12.2, CMake 4.x, locally built zlib 1.3.1.

- **14/14 CTest PASS** (`earth_relief` added). EGL/GLES development libraries
  could not be installed in this environment without root, so the real-shader
  `medium_actual_gpu` test was **not registered here**; CI installs them and
  runs the full set.
- **`earth_tests`: 1087 assertions PASS** — legacy purity, generator 5
  equality with the organic layer, determinism, seed sensitivity, elevation
  bounds, quality independence, spawn/camp habitability, spawn→boss
  connectivity, chain continuity (≥70 % of rock samples have ≥3 rock
  neighbours), band proportions, treeline thinning, no water above the
  treeline, generator 6 header round-trip and rejection of generator 7.
- `docs/earth-relief/generator6-vs-5.png` is a **synthetic top-down tile dump
  from the host test harness**, drawn by `tests/../tmp` tooling for review. It
  is **not** a phone screenshot, not in-game art and not a performance
  measurement.

## Not done / not claimed

- **No physical Galaxy F23 or emulator run.** Nothing here validates device
  FPS, temperature or memory. The 0.9.2 thermal fix is still unconfirmed on the
  owner's phone.
- No dedicated mountain/cliff/snow **art**. Relief reuses the existing rock
  (`map 3`) and stony-ground tiles, so mountains currently read as rock fields
  with pale footing rather than bespoke cliff faces. Phase 2.
- No elevation-aware rendering: no cliff shadows, no parallax from height, no
  altitude effect on fog or lighting. Phase 2.
- No gameplay response to altitude (climb stamina, cold, altitude wildlife,
  mountain resources). Deliberately deferred — it would change simulation, and
  that needs the owner's approval first.
- The owner's first priority, improved **sound design**, is not started.
