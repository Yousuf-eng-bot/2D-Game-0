# Medium / 0.9 implementation status

Date: 2026-10-05. Active source remains in `death-world-0.8/` for path continuity, but manifest/CMake/build output version is now **0.9.0 / versionCode 9**. The original 0.8 source is frozen separately in `docs/history/v08-final/code-v08.zip`. The delivered 0.8 APK has not been modified.

## What was delivered

`deliverables/death-world-0.9/` contains the signed **Medium Preview APK**, an offline comparison gallery, a 45-second paired native-GPU preview and a Bengali installation/limitations guide. This is a substantial playable prototype implementation, not a claim that every item of the original studio-scale art/performance specification is finished.

## Eight milestone assessment

| Milestone | Implemented / verified | Remaining or qualification |
|---|---|---|
| 1 Audit | Source inventory, original-source checkpoint, baseline tests, file-level plan | Audit is historical; use this file for current state |
| 2 Quality/platform | Persistent central Low/Medium, requested/effective distinction, legacy appearance import, Graphics tab, 30/60 cap, adaptive policy, RAM/GPU recommendation, Android safety inputs, EGL host and Canvas fallback | Native/host tests pass; no Android device/emulator lifecycle/touch validation yet |
| 3 Player | 72×88 procedural frame canvas, 8 directions, 8-frame clips, red signature, lowered idle weapon, clothing/pack/gloves/boots/face, armour tier treatment, existing combat/tool timing, canopy reveal, dodge ghosts | Shared authored drawing programs, not individually hand-painted sheets; decorative backpack is not a new inventory slot; bespoke polish remains |
| 4 Enemies | Humanoid role palettes/equipment/silhouettes, elite/guardian indication, health bars, action/telegraph/hurt/death presentation; scout flanking and wounded-raider retreat shared across qualities | Wolf and many existing creature assets/routines retained; full bespoke creature overhaul not complete |
| 5 Lighting | Real GLES 3 normal-map shader, directional/local lighting, nearest-light height-field occlusion, directional/contact shadows, bloom, palette grading, vignette/grain/focus approximation, light shafts/dapple, separate alpha UI | Approximate 2D height-field shadows/focus, not physical 3D optics; GPU tests ran on host software GL, not a phone |
| 6 Environment/VFX | Generator 5 sparse jittered-anchor curved paths and forest patches; old generator branches frozen; tree material/normal/wind/fall/reveal treatment; grass/flowers/mushrooms/water normals/ambient effects | Many props, wildlife, terrain art, weather/reflections and impact systems reused; full new art for every biome/prop and multilayer parallax/fog not complete |
| 7 UI/camera/audio | Readable world/time panel, damage-lag health bar, pressed/cooldown controls, camera follow/combat zoom, reused animation-aligned audio events, adjusted footstep accumulation; BN/EN/four layouts preserved | Full inventory-icon overhaul not done; classic expedition keeps legacy actors; Medium uses trails/echoes rather than the old full-frame motion-blur implementation |
| 8 Validation/performance | Final CTest 14/14, sanitizer run, actual shader/context tests, exact Low/geography comparison, ordinary new-world traversal, signed three-ABI APK verification, paired captures/video | No sustained Android FPS, thermal/battery soak or vendor-driver measurements; no fully GPU-atlas-batched/compressed world pipeline |

## Architecture actually chosen

The audit's separate GL-thread/render-snapshot design was a proposal, not a requirement already implemented. To preserve existing game-state ownership and the CPU Low path, the shipped prototype instead keeps **one Android UI-thread owner** for native simulation, input, dialogs, lifecycle and EGL submission. `TextureView` supplies a native Surface; the existing `GameView` drives frames and remains a Bitmap/Canvas fallback. No second game-state writer or asynchronous simulation thread was introduced.

- Low uses its preserved CPU renderer; a successful GL host uploads/presents the finished frame with nearest sampling. Failed/unavailable GL falls back to Canvas.
- Medium CPU-composes albedo and tangent-space normals from bounded cached procedural sprites and existing world rendering into a 640×360 G-buffer.
- GLES performs directional and up-to-four local-light shading, one local light's bounded six-sample height-field occlusion, restrained post-processing, then premultiplied-alpha UI composition.
- UI primitives track a separate premultiplied layer. This fixes the incorrect bright daytime scenery that would otherwise remain under translucent night-time controls.
- Sprite normal alpha packs approximate height plus an emissive bit. Normal vectors rotate with rotated sprite samples. Inverse nearest sampling avoids holes in rotated/scaled sprites.
- New actor cache: maximum 192 entries. New tree cache: maximum 64 entries. Canopy fade map is bounded. These are bounded caches, not a claim of a fully GPU-batched texture atlas system.
- Android `ActivityManager`/`PowerManager` provide available RAM/low-memory/power-save/thermal signals. Timing is an **onDraw start-to-start pacing estimate**, not hardware GPU timestamps or Android FrameTimeline certification.
- Assets are original procedural drawing programs; no third-party game sprites were imported. Existing audio/menu art/fonts remain licensed/offline as before.

## Concrete changed files

New production files:
- `native/graphics_runtime.hpp`
- `native/medium_visuals.hpp`
- `native/gpu_medium.hpp`
- `native/generation5.hpp`

Updated production files:
- `native/graphics_quality.hpp`: policy now consumed by runtime, legacy appearance setter.
- `native/engine.cpp`: initialization/frame integration, version/quality feedback, rate cap, 4 new JNI methods.
- `native/raster.hpp`, `art.hpp`: optional normal/UI targets and Medium dispatch; inactive on Low.
- `native/presentation.hpp`, `vista.hpp`: Medium world composition/camera/lighting boundary and reused shadow budget.
- `native/frontier.hpp`: version 5 dispatch/creation/header acceptance; sparse new-version camps only.
- `native/combat.hpp`: common scout/retreat AI and footstep-distance accumulation.
- `native/ui7.hpp`, `ui7_fonts.hpp`, `tools/ui_strings.json`: Graphics menu and gated Medium HUD; fonts regenerated.
- `android/java/com/ashenveil/game/MainActivity.java`: same-owner TextureView/EGL integration and platform signals.
- Manifest, CMake, APK build/verifier scripts: optional GLES 3, version 9, EGL/GLES/android linkage, 18 JNI methods.

New validation/artifact helpers:
- `tests/medium_tests.cpp`, `medium_gpu_tests.cpp`, `low_regression_capture.cpp`, `medium_preview.cpp`.
- `tools/build_medium_gallery.py`, `make_medium_preview.py`.

The old straight-road survival walkthrough explicitly runs generator 4 after creation, because blindly walking east is no longer a valid generator-5 navigation test. A separate genuine input-followed generator-5 route now covers 95 tiles without health/item grants or invulnerability. Legacy fixture expectations remain pinned to their historical generator versions; only new-world default assertions changed to 5.

## Final executed evidence

- `ctest-v09.txt`: **14/14 PASS, 14.52 s**.
- `medium-sanitized.txt`: **23,194 assertions PASS**, ASAN + UBSAN + leak detection.
- `gpu-validation.txt`: **12,852 assertions PASS** on Mesa llvmpipe; 24 quality switches, 3 EGL destroy/recreate cycles. Flattening only normal RGB changed **42,658 pixels**; clearing only height occluders changed **7,463 pixels**. Opaque HUD pixels are exact, and the Low GPU presentation matches every logical CPU pixel.
- `low-regression.json`: **72 controlled Low gameplay frames, 66,355,200 bytes**, exactly equal to compiled original 0.8 source. Four legacy generators × camp/five-biome scenes × three preserved legacy appearance presets. Shake/motion blur disabled for this deterministic image fixture; the new Graphics menu, diagnostics and explicitly approved common AI changes are not claimed to be unchanged.
- Legacy tile/ground/biome/height sample: **44,096 bytes / 11,024 records**, generator versions 1–4 exactly equal to original source.
- `apk-validation.txt`: original certificate; version 9/0.9.0; min23/target35; 3 ABIs; 18 JNI; 16KB ELF LOAD alignment; ZIP alignment/integrity; 38 asset/license files; all 32 WAVs valid; no Android permissions requested.
- `capture-scenes.tsv`: exact staged scene coordinates/hour/seed for the paired stills.
- 45-second paired MP4: fully decoded successfully. It is staged native GLES footage, not Android recording. Preview-only invulnerability, stamina restoration and position/time staging are explicitly labelled in the guide.

APK SHA256: `4b4de4e284e86a7ed61725985ea9cca77c07d14f3ef5c5536d9aaa57be4170ff`.
Certificate SHA256: `6860d6fd6332594af5f30916fb67d9717088f63ebecb1c8d34476d27ae8f710b`.

## Do not overclaim

The device installation, Android lifecycle stress test and physical performance target have **not** been verified. No 60 FPS/minimum-30 promise is justified by host tests. Some requested art/atlas/parallax/inventory work remains. The release is explicitly called **Medium Preview** for those reasons.

Old-world road grids remain by design: preserving old geography and eliminating its roads are conflicting requirements. Only newly created generator-5 worlds get the natural layout; quality switching never selects a generator.

## Rebuild / reproduce

```sh
# Linux prerequisites; keep heavy builds sequential on the ~2 GB sandbox.
sudo apt-get install libegl1-mesa-dev libgles2-mesa-dev ffmpeg
python3 -m pip install cmake pillow numpy
python3 tools/setup_android.py
python3 tools/restore_audio.py  # uses the retained, hash-verified 0.8 APK
cmake -S . -B build/medium-native -DCMAKE_BUILD_TYPE=Release
cmake --build build/medium-native -j1
ctest --test-dir build/medium-native --output-on-failure
APK_OUTPUT=../deliverables/death-world-0.9/Death-World-0.9-Medium-Preview.apk bash tools/build_apk.sh
python3 tools/verify_apk.py ../deliverables/death-world-0.9/Death-World-0.9-Medium-Preview.apk
```

Fonts require Pillow built with RAQM/HarfBuzz if regenerating `ui7_fonts.hpp`. Normal builds use the committed generated header. Host GLES tests require the EGL/GLES development libraries; otherwise CMake cannot register the real-GL test. The GL test environment uses `EGL_PLATFORM=surfaceless;LIBGL_ALWAYS_SOFTWARE=1`.

Runtime WAV duplicates are removed again after packaging to stay within snapshot limits. `restore_audio.py` restores exact copies from the protected 0.8 APK before another APK build. Signing key remains private and is never part of a deliverable.
