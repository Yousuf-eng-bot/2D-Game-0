# Death World — instructions for the next coding session

Read `HANDOFF_BN.md`, `README.md`, `SIGNING.md` and `docs/medium-thermal-fix/STATUS.md` before changing code. Communicate with the owner in Bengali. Do not infer that selecting a repository transfers previous chat history or private signing credentials.

## Current source

- Death World 0.9.2 / Android versionCode 11 / package `com.ashenveil.game`.
- In-game New Horizon Z naming does NOT authorize renaming the application/package.
- Mostly C++20; Android Java bridge owns simulation/input/EGL on a single UI thread. GLES 3 is optional, with Canvas fallback.
- Current Medium: CPU-composed G-buffer plus actual GLES normal lighting at 640×360, nearest upscale. Not a fully GPU-atlas-batched pipeline.
- The latest issue is Medium falling back to Low on the owner's Galaxy F23 / 6 GB / Android 14. The 0.9.2 thermal fix passed host tests, but the owner has NOT yet confirmed resolution on the phone. Do not say it is proven fixed there.
- Raw Android thermal 2: Medium / 30 cap; 3: reduced-effects Medium / 20 cooling cap; 4–6: mandatory Low / 20 cap. Critical protection is not defeated by optional Auto fallback Off. Raw status and decision are applied together by `graphicsPlatformProfile`.

## Invariants

1. Low preserves the original 0.8 appearance/preferences; do not force a different legacy preset.
2. Only Low and Medium; High is deferred. Graphics never changes simulation, enemy difficulty, world generator, inventory, progress, or RNG.
3. Existing world geography is immutable. Generator 5 applies only to newly created worlds. Keep legacy generators 1–4 and authentic save fixtures.
4. Keep controls readable, Bengali default plus English, four layouts and layout editing. Preserve movement, combat and weapon selection. Do not restore enemy-to-player targeting lines.
5. Never fabricate phone FPS, temperature, audio latency, emulator results or art completion. Staged native captures are not phone footage or ordinary survival playthroughs.
6. Do not commit private signing keys, tokens, actual player saves, uploads, SDK/NDK or build products. Synthetic `tests/fixtures/*.sav` MUST stay tracked.
7. Do not silently generate another signing key to bypass a missing one. That breaks updates of the owner's installed game. Read `SIGNING.md`. Never tell the owner to uninstall/clear data as a workaround.
8. Source ZIP includes 32 runtime WAVs. They must be committed in the repository. The original workspace can reconstruct omitted copies from its old APK, but a fresh clone should not require that external APK.
9. Old 0.5/0.8 backups and private signing key remain in the original workspace, not this repository package. Do not assume a new chat has them, or claim they were uploaded.

## Validation

Start with `python3 tools/check_source.py` on the untouched imported snapshot. The manifest is an export-time integrity record; source edits legitimately change hashes. Then configure/build/test with CMake, sequentially (`-j1`) in a ~2 GB environment. Install EGL/GLES headers/libs so the actual GPU test is registered; expect 14 tests, not silently 13. Run `ctest --test-dir build/native --output-on-failure`.

Host GPU tests use surfaceless Mesa/llvmpipe and are NOT phone performance benchmarks. The latest packaged APK uses the original certificate; its checksum is in the handoff and validation report. New APKs need increasing versionCode and the original private signing identity.

## Next priorities

Confirm the thermal fix with the owner first. After that: design improved sound, then expand Earth-like world composition including mountains from the beginning. Full bespoke wildlife/prop/inventory art, full GPU atlas batching/compression, advanced parallax/fog/cinematic work and physical-device performance verification remain incomplete. Plan/test in phases rather than describing prototypes as finished studio-quality work.
