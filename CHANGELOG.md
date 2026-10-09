# Changelog

All notable changes to this project are documented here.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Fixed
- **Test APK could fail to install** with "package appears to be invalid".
  `tools/build_apk.sh` now stores the native libraries uncompressed so
  `zipalign` can page-align them, and refuses to publish an APK unless the
  zip structure (`unzip -t`), the alignment (`zipalign -c`), the signature
  (`apksigner verify --min-sdk-version 23`) and every required entry
  (`classes.dex`, the manifest, `resources.arsc`, all three atlases and each
  requested ABI's `libashen.so`) are all present and valid.
- `versionCode` bumped to 12 (`0.9.3-test`) so a stale install can never
  conflict with the new test build.
- `tools/verify_apk.py` reads the expected version from the manifest instead
  of hard-coding it.

### Added
- The CI job also builds a **small arm64-only APK** next to the three-ABI
  one, and publishes `CHECKSUMS.txt` plus a run summary table of sizes and
  SHA-256 hashes, so a truncated download can be identified immediately.
- The home screen prints an **art pack status line** (`ART HERO CHARS
  ANIMALS`). A lower-case entry with a bang means that atlas failed to load
  and the game is drawing the old fallback art - which previously looked
  exactly like "the update changed nothing".

### Added
- **Redesigned main character, at roughly three times the detail.** New rig
  and layer set (`tools/herorig.py`, `tools/heroparts.py`,
  `tools/build_hero_atlas.py`) bake `android/assets/hero.dwa` on an 80x96
  cell with a ~62 px figure: long flowing hair, filigreed plate with a fauld,
  spiked pauldrons, a five-panel cape, a horned circlet, and a fullered
  greatsword with deterministic blood spatter and a drip. Loaded as a third
  atlas and selected with `drawCharActor(..., hero=true)`, so both quality
  tiers draw the same hero and every existing clip keeps working.
- Palette gained a blood ramp (slots 24-26); `CHAR_PALETTE_SLOTS` is now 27.

### Added
- **Equipment art completed.** New baked layers `w_sunsteel` (rare weapon
  core), `acc_talisman` and `acc_cinder` on a new `accessory` slot, wired to
  the trinket slot `g.eq[2]` and the weapon core rarity, so every equippable
  item in the game now changes the character's appearance.
- **Build identifier.** The home screen shows `BUILD <short sha>`
  (`DW_BUILD_ID`, compiled in by `tools/build_apk.sh`), and the CI artifact is
  named with the same sha, so a tester can confirm which build is installed.
- `tools/export_source_manifest.py` re-exports `SOURCE-MANIFEST.json`;
  `tools/check_source.py` passes again (252 files, 32 runtime WAVs).

### Fixed
- Character torsos read as flat cards: the lit side now follows the body axis
  rather than the screen, with a pectoral band, rib shadow, collar and waist
  shadow.
- `coat_mail` used a full checkerboard that turned to noise at 48 px; it now
  uses offset horizontal courses.
- The backpack was drawn over the character's belly on side views, because
  `facingAway` treated a pure side view as a back view.

### Added
- **Animal sprite atlas.** New rig and baker (`tools/animalrig.py`,
  `tools/build_animal_atlas.py`) produce `android/assets/animals.dwa`:
  2220 sprites, deer / rabbit / bird with two colour variants each, 10 clips
  (idle, graze, head_lift, walk, run, hop, peck, flee, hurt, die) across the
  same 5 baked directions, same 48x48 cell and same feet anchor as the
  character atlas. Wired into `drawLifeAnimal`; species without baked art keep
  their original procedural drawing.
- **Impact feedback.** `charKnockback` (4-6 px directional shove while hurt),
  `updateHitLunge`/`charLunge` (forward lunge when a hit lands) and
  `drawActorDust` (landing ring and roll dust), all presentation only.

### Changed
- **Medium now draws the same character as Low.** `drawMediumPlayer` and
  `drawMediumEnemy` render from the character atlas instead of the procedural
  CPU sprite cache, so the player and enemies look and animate identically on
  both quality tiers. Medium keeps its dash trail, strike arcs, counter ring
  and contact shadows.
- `drawCharLayer` can write per-pixel surface normals, so characters keep real
  normal lighting in the Medium GLES pass.
- Enemies use dedicated outfits and palettes (`PAL_BANDIT` cold grey-blue,
  `PAL_ELITE` near-black with gold trim, steel helm, shoulder-padded coat) in
  both renderers, making them readable against the player at a distance.

### Added
- **Layered character sprite atlas and animation system.** New bake pipeline
  (`tools/charrig.py`, `tools/charparts.py`, `tools/build_character_atlas.py`)
  produces `android/assets/characters.dwa`: 9810 sprites, 18 equipment layers,
  16 animation clips, 5 baked directions (8 at runtime via mirroring), indexed
  pixels with row-RLE so no image decoder is needed. Runtime loader and
  palette blitter in `native/sprites.hpp`; animation state machine, palettes
  and layer ordering in `native/char_anim.hpp`. Equipment changes are
  reflected immediately; enemy types are separated by palette and silhouette.
  The open-world player now renders from the atlas, falling back to the
  previous procedural ranger if the asset is absent. New `characters` test
  (65,774 assertions). See `docs/character-art/STATUS.md`.
- `.github/workflows/apk.yml` builds an installable **disposable-test-signed**
  APK artifact for on-device testing. It cannot update an installation signed
  with the original release key - see `SIGNING.md`.
- `dev-build.sh` host build helper for environments without root.
- **Generator 6 "Earth relief"** for newly created worlds (`native/generation6.hpp`):
  continental elevation, ridged mountain chains gated by an orogenic belt mask,
  foothills, a treeline, walkable rock benches and downhill river drainage, with
  the opening valley softened so the first camps stay habitable. Generators 1-5
  and all existing saved geography are untouched; the save container format is
  unchanged. New `earth_relief` test (1087 assertions) covers legacy purity,
  determinism, quality independence and spawn-to-boss connectivity.
  See `docs/earth-relief/STATUS.md`.
- Repository scaffolding for public development: GitHub Actions CI
  (native build + `ctest`, source-manifest check, clang-format), contribution
  guide, security policy, issue/PR templates, `.editorconfig` and this changelog.

### Changed
- The source archive was unpacked so the tree is a normal, buildable checkout
  instead of a committed ZIP.

## [0.9.2] - version code 11

### Fixed
- Thermal status/decision synchronization: Android thermal level 2 keeps Medium
  at a 30 FPS cap, level 3 keeps reduced-effects Medium at 20, and critical
  levels 4–6 force the Low safety path. Caps are targets, not measured FPS.

### Known limitations
- Not yet confirmed on physical hardware (Galaxy F23); no successful
  phone/emulator runtime validation is claimed for 0.9.2.
- Bespoke wildlife/props/inventory art, GPU-atlas batching and compression,
  advanced parallax/fog and device performance validation remain incomplete.

## [0.9.x and earlier]

See `docs/` for the historical research, implementation status and test reports.
Older reports describe the state at the time of writing and are not current
completion claims.
