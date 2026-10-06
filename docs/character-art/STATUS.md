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
  (partially: assets exist, wiring does not), 1.4 and 2.6 are therefore
  **not delivered in the game yet**.
- The Medium renderer (`drawMediumPlayer`) still draws its own player. Only the
  Low / open-world ranger path uses the atlas so far.
- Hit-reaction knockback and the attacker's lunge/squash (2.4) are only
  partially present: the white/red flash is wired through the sprite blitter,
  but the existing knockback is the old combat-side impulse, not a new
  animation-driven one.
- Dust frames for dodge start/end and landing (2.5) are not drawn.
- Art quality is a first pass. Proportions were corrected once after review,
  but the front-facing torso still reads flat and the sprites have not been
  hand-tuned pixel by pixel.
