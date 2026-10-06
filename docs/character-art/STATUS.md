# Character art and animation upgrade — phase 1

Date: 2026-10-06. Source base: 0.9.2 / versionCode 11.

## The architecture finding that shaped this work

The project had **no character sprite assets at all**. Every actor was drawn
from scratch in C++ on every frame (`drawJourneyRanger`, `drawMediumPlayer`,
`enemyArt`, `animalArt`) using `rect` / `poly` / `thickLine`, with "animation"
being continuous `sin()` maths rather than frames. There was no sprite sheet,
no atlas, no frame table and no equipment-driven appearance: the renderers
never read `g.eq[]` at all.

The owner chose to build the real sprite-sheet pipeline. This document records
what exists now and, just as importantly, what does not.

## The pipeline

```
tools/charrig.py              pixel-art rig: canvas, primitives, skeleton
tools/charparts.py            one painter per equipment layer
tools/build_character_atlas.py  bakes everything into the runtime atlas
        |
        v
android/assets/characters.dwa  1.4 MB, 9810 sprites, indexed + row-RLE
docs/character-art/*.png       review sheets, one per layer
        |
        v
native/sprites.hpp            loader, 8-way facing, palette blitter
native/char_anim.hpp          palettes, animation state machine, layer order
```

Key decisions:

* **Indexed pixels, not colours.** The engine has no PNG decoder and must stay
  dependency free, so frames are palette *indices* with a trimmed row-RLE. A
  palette is supplied per actor at draw time, which is how one baked bandit
  becomes a grey common bandit or a gold-trimmed elite with no extra frames.
* **Layers composite at runtime** (`pack → body → coat → belt → head → hair →
  headgear → weapon`, with the backpack moving in front when the actor walks
  away). Changing equipment changes the look on the next frame.
* **One cell, one anchor.** Every frame of every layer is 48×48 anchored at
  (24, 42), the centre of the feet, so no frame can make the character hop.
* **Pixel perfect.** Nearest sampling only, no anti-aliasing, a fixed 4-step
  ramp per material (outline / shadow / base / highlight), plus a despeckle
  pass that removes isolated pixels — the usual cause of frame-to-frame
  "boiling".
* **Eight directions from five bakes.** S, SE, E, NE, N are baked; W, NW, SW
  are the runtime horizontal mirror.
* **Side views keep body depth.** A pure `-sin(a)` across-body axis collapses
  to paper in side view, so the axis is clamped to a minimum apparent depth.

## Animations baked

| clip | frames | loop | notes |
| --- | --- | --- | --- |
| idle | 8 | yes | breathing: chest and shoulders rise ~1 px |
| idle_look / idle_adjust / idle_sword | 8 each | no | flourishes fired every 6–10 s |
| walk | 8 | yes | limb swing + 1–2 px body bounce |
| run | 8 | yes | larger stride and lean |
| attack | 7 | no | 2 anticipation, 2 strike, 3 follow-through |
| strong | 10 | no | 4 wind-up, 2 strike with lunge, 4 recovery |
| chop | 8 | no | overhead tool swing |
| mine | 8 | no | steeper, shorter pick swing |
| guard | 6 | yes | braced low hold |
| guard_hit | 2 | no | block recoil |
| dodge | 6 | no | roll with tuck |
| jump | 5 | no | compression, 3 air, landing squash |
| hurt | 3 | no | recoil pose |
| die | 6 | no | collapse |

Walk and run are driven by **distance travelled** (`g.walkPhase`), so feet keep
pace with the ground instead of sliding, and sprinting speeds the cycle up.
Attack, strong, chop and mine are **time-warped onto the real action window**
(`j.windup / j.activeTime / j.recovery`), so the strike frame lands exactly
when the hit box opens rather than merely looking like it does.

## Equipment layers

| slot | variants |
| --- | --- |
| base | body, body_armed (sleeved) |
| head | head |
| hair | hair |
| headgear | hood, helm_steel |
| coat | coat_ranger, coat_mail, coat_bandit, coat_elite |
| pack | pack (with bedroll) |
| belt | belt with buckle and hanging scabbard |
| weapon | w_dawnblade, w_riftaxe, w_rustaxe, w_windbow, w_pick, w_sword |

`playerOutfit()` maps live inventory onto layers: the chest slot picks Ranger
Coat below rarity 2 and Grovekeeper Mail at rarity 2+ (which also adds sleeves
and a hood, and a steel helm at rarity 3); the weapon layer follows `g.weapon`
and swaps to an axe or pick while a chop or mine action is running.

## Enemy readability

Three palettes ship: `PAL_PLAYER` (ranger green, gold trim), `PAL_BANDIT`
(cold grey-blue, dull trim) and `PAL_ELITE` (near-black blue, bright gold).
Bandits also use a distinct hunched, shoulder-padded coat silhouette, so they
separate from the player by shape as well as colour. A test asserts the elite's
gold is measurably brighter than the bandit's trim.

## Validation actually executed

- **15/15 CTest PASS** (new `characters` test added). The EGL/GLES real-shader
  test is still not registerable in this sandbox and is covered by CI.
- **`character_tests`: 65,774 assertions PASS** — atlas header integrity, all
  required variants and clips present with the required frame counts, every
  body frame non-empty and inside its cell, coat layers registered within
  6 px of the body centre on every walk frame and direction, the eight-facing
  map, in-bounds drawing for every clip and frame, clipping at screen edges,
  palette separation, one-shot vs looping state machine behaviour, equipment
  swaps changing the outfit immediately, and graceful fallback when the atlas
  file is absent.
- Visual review of the baked sheets and of an in-engine render of all clips,
  three palettes and multiple facings.

## Not done — do not read this as complete

- **No device or emulator run.** No FPS, thermal or battery measurement was
  taken. The 60 FPS checklist item is **unverified**; the blitter is a simple
  indexed RLE span copy, but that is reasoning, not a measurement.
- **Enemies and animals still use the old procedural renderers.** The palettes,
  bandit/elite coats and the whole atlas are in place for them, but
  `enemyArt` / `animalArt` are not yet wired to `drawCharActor`. Points 1.2
  (assets exist, wiring does not) - **now closed by phase 2 below.**

## Phase 2 - what the second pass added

- **One character on both quality tiers.** `drawMediumPlayer` no longer draws
  its own procedural actor; it calls the same `drawCharActor` as the Low /
  open-world path. The reported "the character looks different on Medium" bug
  is this change. Medium keeps its own extras on top: dash echo trail, contact
  shadows, weapon strike arcs, counter ring and landing dust.
- **Normals for Medium.** `drawCharLayer` now optionally writes a surface
  normal per painted pixel (`0x00RRGGBB`, R = nx+128, G = ny+128, B = nz),
  derived from the silhouette bevel plus the material shade step, so GLES
  lighting still lights characters correctly.
- **Enemies (1.2).** `enemyOutfit` / `enemyPalette` plus a per-entity
  animation cache, wired into both `drawNewEnemy` (Low) and `drawMediumEnemy`
  (Medium). Common bandits use the cold grey-blue `PAL_BANDIT`, elites and
  bosses the near-black `PAL_ELITE` with bright gold trim, a steel helm and a
  shoulder-padded coat - a different silhouette, not just a recolour.
- **Animals (1.4, 2.6).** A new rig (`tools/animalrig.py`) and atlas
  (`tools/build_animal_atlas.py` -> `android/assets/animals.dwa`,
  2,220 sprites, 6 variants, 10 clips, 5 directions). Deer, rabbit and bird,
  two colour variants each. Clips: deer grazing loop with an occasional head
  lift every 6-10 s, rabbit hop with real squash-and-stretch, bird ground
  peck plus a take-off `flee`. Species without baked art keep their original
  procedural drawing untouched.
- **Hit reaction (2.4).** `charKnockback` gives a 4-6 px directional shove on
  the hurt frames, and `updateHitLunge` / `charLunge` give the attacker a
  short forward lunge when a hit actually lands (read from `j.struck`).
- **Dust (2.5).** `drawActorDust` draws stepped puffs for the landing ring and
  the roll trail, on both tiers.
- All of this is presentation only: no simulation, AI, RNG, seed or save data
  is read differently or written.

## Still open

- Art quality is a first pass. Proportions were corrected once after review,
  but the front-facing torso still reads flat and the sprites have not been
  hand-tuned pixel by pixel.
- `SOURCE-MANIFEST.json` is still the 0.9.2 snapshot and therefore reports the
  new files as changes. Re-export it at the next release-style commit.
- Phone-side FPS has not been measured; the 60 FPS claim in the checklist is
  unverified on hardware.

## Test APK

`.github/workflows/apk.yml` builds an installable APK on every push to the
working branch and uploads it as the run artifact **`death-world-test-apk`**.
First successful run: GitHub Actions run `37442450060`, job time 1m12s, three
ABIs, signed with a **disposable test key**.

To get it: repository → **Actions** → **Test APK** → newest run → *Artifacts* →
`death-world-test-apk` → unzip → `Death-World-Test.apk`.

> ⚠️ This APK is signed with a throwaway identity, not the original release
> key, so Android will refuse to install it over the existing Death World.
> Install it **alongside** on a test device or profile. Do **not** uninstall
> the existing game to make room - that deletes save progress, because the
> game disables ordinary Android backup. For a real update, build locally with
> `SIGNING_KEY=/private/path/ashen-prototype.jks bash tools/build_apk.sh` and
> raise `versionCode`.
