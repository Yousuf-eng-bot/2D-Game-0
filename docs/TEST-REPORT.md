# Death World 0.8 — validation report

2026-10-05. Scope: Vista rendering update, retained UI/gameplay and release artifact. **No physical Android phone test was performed.**

## Final automated results

| Check | Result |
|---|---|
| Release CMake/CTest | **11/11 PASS**, 9.99 s |
| New Vista suite | **20,419 assertions PASS** |
| Current UI suite | **82,260 assertions PASS** |
| Current Bengali ordinary-input progression | PASS; alive **HP 120**, no HP/items/teleport grants |
| ASAN + UBSAN + leak detection: Vista | PASS |
| ASAN + UBSAN + leak detection: current UI | PASS |
| ASAN + UBSAN + leak detection: UI progression | PASS |
| Authentic 0.6 save fixture | **3,875 geography/height records unchanged**, inventory/tools/base/crops/checkpoint retained |

Evidence: `ctest-v08.txt`, `vista_tests-sanitized-v08.txt`, `ui7_tests-sanitized-v08.txt`, `ui7_walkthrough-sanitized-v08.txt`.

Eight historical CTests retain their legacy UI adapter and establish backend regression, not new-UI usability. The current-UI tests and Vista suite are separate. Assertion totals include repeated data/pixel comparisons; they are not counts of human gestures or human art evaluations.

### Vista-specific coverage

- Render-only quality switching does not change serialized world state, tile map, height array or gameplay RNG.
- Deterministic material/scatter generation, distinct biome materials and tree variants; **360 species/biome/variant sprite-cache combinations** have valid pixel data. This does not mean 360 hand-drawn species.
- Existing terrain height projects consistently; renderer smoke across five biomes × three qualities × four clock phases = **60 combinations** and extreme valid world coordinates.
- Sun direction produces different masks at different hours; soft mask contains intermediate alpha/feather pixels.
- Reflection changes pixels when the water stencil allows it and does not write when the stencil is empty.
- Focus toggle/Low quality bypass tested; 140×140 central region remains byte-identical in the scenery focus pass.
- Existing wall grid blocks local light rays; quality and focus preferences save/load independently, malformed configuration falls back safely.
- Actual settings hit targets change quality/focus and preserve them through UI boot.

### Retained gameplay and saves

The Bengali-input walkthrough moves normally, selects weapon/hunting, cuts trees, gathers logs, crafts planks/bench/pick, places the bench, holds mining, picks up stone, saves/exits/resumes. No grants or teleport shortcuts in that test.

World generation, collision/progression and save serializers were not rewritten. Authentic 0.6 fixture provenance remains `tests/fixtures/README-v06.md`; the current UI migration test reads it. The unmodified 0.7 code checkpoint is also retained in `docs/history/v07-final/code-v07.zip` and was compiled to produce baseline comparison images. No claim is made that the user's actual private phone saves were inspected.

## APK

- **16,612,586 bytes**, package `com.ashenveil.game`, code **8**, version **0.8.0**.
- SHA256: `6debaa149ed4816b7d7aac9778e8eef58559616663e720c541e768edcd422eaf`
- Previous signer SHA256: `6860d6fd6332594af5f30916fb67d9717088f63ebecb1c8d34476d27ae8f710b`; v1/v2/v3 verified.
- Minimum API23 / target API35; arm64-v8a, armeabi-v7a, x86_64; all 14 JNI entry points per ABI.
- 16 KB ELF LOAD alignment, ZIP alignment/integrity, no requested permissions, exact bytes of **38 bundled assets/licenses** verified.
- 26 short PCM effects/beds within SoundPool decoded-size bound; six stored 38.4-second PCM music loops. No Android audio-latency measurement.
- Evidence: `artifact-verification-v08.txt`, `apk-build-v08.txt`. Private signing key is not distributed.

## Host render timing

`tests/vista_benchmark.cpp`, C++ -O2, 640×360, dt=0, 12 warm-up + 120 measured frames/scene. Simulation/Android presentation/audio/thermal excluded.

| Scene | Low mean ms | Balanced mean ms | High mean ms |
|---|---:|---:|---:|
| Forest | 4.46 | 6.60 | 6.28 |
| Desert | 2.82 | 4.97 | 4.66 |
| Snow | 2.67 | 7.13 | 6.21 |
| Swamp | 2.07 | 5.36 | 5.89 |
| Volcanic | 1.95 | 5.03 | 4.91 |
| Forest night | 3.92 | 7.45 | 7.06 |

Full mean/median/p95: `host-render-cost-v08.txt`. Differences include host variance and scene-dependent emitter/reflection coverage. These are **not phone FPS estimates** and do not establish a guaranteed frame budget on a 6–8 GB phone.

Parallel compilation initially exhausted this approximately 2 GB workspace's available resources and stalled. The compiler jobs were stopped, then APK and host builds were completed **sequentially**, with `cmake --build ... -j1`. This was a build-environment issue, not evidence of an Android runtime crash.

## Visual evidence and limits

The comparison page uses matching seed 20261004, matching coordinates and clock values, rendered by the original 0.7 and current 0.8 engines. Native staged screenshots were inspected for terrain, foliage, snow, desert and night camp. The 60-second preview includes five biomes and a night camp, with demo teleport/clock/invulnerability setup. Its audio is mixed from bundled assets; it is **not a phone recording or an input-only playthrough**.

DOF here means an optional artistic scenery-focus approximation, not optical lens simulation. Shadows, terrain-blocked light and water reflections are 2D approximations, not ray tracing, full 3D shadow maps or physically based materials. New clutter is decorative, not new harvestable inventory content. Character/combat animation systems remain; this is primarily an environment/material/lighting overhaul.

Phone update/install, actual touch comfort, glyph readability on the target display, sustained frame pacing, battery/heat and audio latency still need user testing. Low/High and Focus controls provide fallback, not a performance guarantee. Legacy flat worlds keep their original geography; the update does not insert new mountains into old saves.

## Workspace preservation

0.5, 0.6 and 0.7 released artifacts are retained. To avoid exceeding the workspace snapshot cap, redundant old source-art previews were removed from the active tree (originals remain in prior source archives), and duplicate runtime WAVs are restored on demand from the companion APK via `tools/restore_audio.py`. All 32 restored WAVs were SHA256/size-checked and the full APK verifier was rerun successfully. Build, verification and preview scripts invoke restoration automatically. This is source-workspace deduplication only; the APK still bundles all audio offline.
