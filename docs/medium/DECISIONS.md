# Approved scope — 2026-10-05

User selected via clarification UI:
- `current_look`: preserve existing appearance/preferences as new Low, not force the old 0.8 Low preset.
- `low_medium`: only Low + Medium for this update; no High tier.
- `common_gameplay`: approved AI/combat improvements are common to both tiers; graphics quality must not determine gameplay/difficulty.

Standing requirements: generator changes only for new worlds; old world geography/progress preserved; package/signing identity retained; 0.5 backup protected.

## Current development status — 0.9

The foundation is now integrated. A signed 0.9 Medium Preview APK, offline paired gallery, 45-second native GLES comparison and Bengali guide have been produced in `deliverables/death-world-0.9/`.

Implemented: runtime/persisted Low+Medium, Android same-owner EGL/TextureView+Canvas fallback, real normal-map lighting and local height-field shadows, premultiplied UI, new procedural player/humanoid frames, shared scout/retreat behavior, generator 5 natural paths/forest patches for new worlds, gated HUD/camera changes and adaptive/safety policy.

Final results: CTest 14/14, Medium sanitizer 23,194 assertions, real host GL 12,852 assertions; 72 old-source Low frames and 11,024 old-generation records exactly preserved; new-generation input route 95 tiles at HP120 without grants. APK signature/3 ABIs/18 JNI/38 assets/alignment checked.

Not all original scope is finished: fully GPU-atlas-batched/compressed world pipeline, full bespoke wildlife/props/inventory-icon art, advanced parallax/fog/slow-motion pass and physical-device FPS/thermal/lifecycle validation remain. No Android device/emulator test occurred. Explicitly label the release Medium Preview rather than claiming everything is complete.

Read `docs/medium/STATUS.md` for architecture, exact file changes, evidence and remaining work. Active source directory name remains death-world-0.8 but version/build scripts are 0.9.0/code9. Original 0.8 source is independently frozen and original APK untouched.

## Additional explicit authorization
User chose `remove_old_large_files`: removed only the five listed 0.6/0.7 large artifacts, freeing 54.44 MiB. Complete 0.5/0.8 releases, uploads and signing key verified unchanged. See cleanup-20261005.json. Original 0.8 source is also frozen in docs/history/v08-final/code-v08.zip, all 84 inventoried file hashes verified.
