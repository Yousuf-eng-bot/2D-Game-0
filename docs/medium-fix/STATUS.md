# 0.9.1 Medium fallback fix — 2026-10-05

## Report and diagnosis

User: Galaxy F23, 6 GB, Android 14. Screenshot shows the Low actor/HUD despite a Medium request. User selected the **frame-drops / battery / heat fallback** group, but did not provide the exact individual reason text or a copied device report. Thus the precise trigger on that phone is not independently established.

Two overly aggressive 0.9 policy paths were reproduced with the original header (`quality-v090.hpp`):

1. A constant 30 FPS stream with requested Medium / 60 target becomes Low, even though 30 FPS is the intended acceptable lower target.
2. Either legacy in-game battery saver or system battery saver unconditionally replaces Medium with Low, even with optional auto fallback disabled.

`reproduction.txt` shows old/new behavior using identical input. This is policy reproduction, not replayed phone telemetry.

## Changes

- `native/graphics_quality.hpp`: 4-second activation warm-up. Three consecutive 2-second slow windows at 60 first retain Medium and lower the effective cap to 30. Only five further consecutive slow windows at 30 after warm-up can demote to Low. Sparse spikes and normal 30 FPS do not demote. Manual quality/cap/auto-policy changes reset performance latches. Thermal severe/critical, actual memory pressure and unavailable renderer safeguards remain mandatory.
- Battery saving caps Medium to 30 rather than disabling its art/lighting. Low remains Low when explicitly selected. Recommendation accounts for OS-reserved RAM on nominal 6 GB devices; RAM remains advice, not a hard quality gate.
- `native/gpu_medium.hpp`: actual normal lighting, height shadow samples, bloom and UI composition now execute into a fixed **640×360 RGBA8 framebuffer**. A second one-sample nearest presentation pass scales this result to the display. Previously expensive lighting executed at physical display resolution. This reduces shaded fragment count, not a measured device speedup claim. Allocation/draw/EGL swap errors are retained for feedback.
- `MainActivity.java`: failed/lost GPU backend retries on the existing owner thread at 5-second intervals while a valid surface and focused running view exist. Canvas fallback retained. No CLEAR blend operation erases the underlying sibling TextureView.
- `graphics_runtime.hpp`, `ui7.hpp`, `ui7_fonts.hpp`, `tools/ui_strings.json`: explicit active graphics/cap/reason in Graphics settings; requested-Medium gameplay badge showing actual mode, cap or fallback reason. Normal requested-Low screenshots remain unchanged.
- `engine.cpp`: copied feedback now includes requested/effective mode, reason, frame-interval estimate, device RAM, separate game/system saving flags, thermal/memory flags, GPU driver/readiness/error and logical lighting size.
- Version 0.9.1 / code 10; existing package and signing certificate. No new sound/world generation/gameplay changes. Save formats unchanged; old worlds work without recreation.

## Executed validation

- `ctest.txt`: **14/14 pass, 14.36 seconds**.
- `policy-validation.txt`: **12,375 policy/config assertions pass**. Revised expectations reflect saver now preserving Medium.
- `gpu-validation.txt`: **12,850 actual GLES assertions pass**, Mesa llvmpipe. Low exact pixels; normal-vector probe changes 42,534 pixels; height-occlusion probe changes 6,431 pixels; 24 switches and repeated context recreation; larger physical surface exactly replicates every logical pixel 2×; actual Medium shader renders with both battery-saving inputs; severe-heat fallback and recovery work.
- `low-regression.txt`: fresh compiled original 0.8 versus current source: **72 Low frames, 66,355,200 bytes exact** and **11,024 legacy geography records, 44,096 bytes exact**. Same fixture hashes as the earlier release.
- `apk-validation.txt`: same certificate, code 10/0.9.1, three ABIs, 18 JNI methods, 16 KB alignment, ZIP validation, 38 exact asset/license files including 32 WAVs, no permissions.
- Native GPU camp-day capture visually inspected: red detailed Medium actor, new world/time HUD and visible mode/cap badge.

## Device validation limitation

Attempted a headless Android 29 x86_64 AVD with official emulator 37.2.12 and 32.1.14 using software acceleration (no /dev/kvm). Startup was rejected with **Insufficient RAM free for launching emulator** in this ~2 GB sandbox. There was no successful emulator boot, APK installation or device gameplay measurement. Do not describe these attempts as Android runtime tests passed. Galaxy F23 installation and sustained FPS/heat behavior still need the user's check.

The new APK is a bug-fix build, not the promised later sound/world expansion. Previous art/pipeline scope limitations remain.

APK SHA256: `73d66cdf247f8ba09535d71cdcfdc059b36eaf3eb4ad290bfc338ca88f486fd8`.
