# Changelog

All notable changes to this project are documented here.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added
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
