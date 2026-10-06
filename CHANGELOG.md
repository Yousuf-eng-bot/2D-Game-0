# Changelog

All notable changes to this project are documented here.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

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
