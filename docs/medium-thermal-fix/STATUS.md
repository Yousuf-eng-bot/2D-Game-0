# 0.9.2 — Thermal decision / Medium cooling fix

Date: 2026-10-05. User device: Galaxy F23, nominal 6 GB, Android 14.

## Evidence and limits of the diagnosis

The new screenshots explicitly show selected Medium, **Active graphics: Low**, **Device is hot**, **Auto fallback: Off**, and Suggested: Medium. The user subsequently reported the raw thermal status as **2**. We do NOT know that the screenshot and copied number were captured at the same instant.

The old 0.9.1 mapping grouped Android statuses 3, 4, 5 and 6 into the same Severe state and unconditionally chose Low. Therefore the screenshots establish the thermal fallback path, but not the raw status at screenshot time. A continuously sampled raw 2 should already have recovered on the next frame in 0.9.1; we do not claim otherwise.

An additional diagnostic inconsistency was reproduced: old JNI updated raw `platformThermal` without applying the policy until the next native frame. Between a 3→2 sample and that frame, feedback could say raw 2 but still report Low / hot. `reproduction.txt` and the source reproduction show that interval. This does not prove the interval alone explains the user's persistent symptoms.

## Implemented changes

1. Explicit Android 0–6 mapping. 0/1 normal/light; 2 moderate; 3 severe; 4/5/6 critical-or-higher. No enum-ordinal assumptions and no collapse of critical statuses into merely Severe.
2. Raw platform values and policy are committed together in `graphicsPlatformProfile`, the **same helper used by Android JNI and host tests**. Safety is also synchronized before feedback/rate reads. Java polls once a second and forces a fresh sample before feedback, on resume and on focus recovery.
3. Medium at moderate/2: normal Medium renderer, 30 cap. Medium at severe/3: **20 cap, one local light, no bloom/height-occlusion/dapple/shafts/focus/grain pass, reduced legacy scenery effects**. Detailed player/humanoid sprites, normal-map directional/local lighting, canopy handling and Medium HUD remain. This is reduced-work Medium, not Low with a renamed label.
4. Critical/4–6: Low with reduced effects and 20 cap, independently of optional auto fallback. GPU/memory safeguards remain. OS thermal protection is not disabled or overridden.
5. Settings show raw System thermal number and named level, active mode, actual cap and a thermal-protection explanation. The cap is a scheduling limit, not measured FPS. Feedback includes the applied enum and profile revision; no information is uploaded.
6. Package version 0.9.2 / code 11, same package/certificate/save formats. No sound/world-generation/gameplay content update.

Changed production: `graphics_quality.hpp`, `graphics_runtime.hpp`, `art.hpp` (forward declaration), `vista.hpp`, `gpu_medium.hpp`, `engine.cpp`, `ui7.hpp`, generated fonts/string dictionary, Java activity, manifest/version/build/verifier metadata.

## Validation actually executed

- **14/14 CTest PASS, 14.46 seconds** (`ctest.txt`).
- **12,452 policy/config assertions PASS**: raw 0–6 mapping, mandatory critical safety with auto off, immediate recovery from 3/4/5/6 to 2, consistent feedback/cap, battery flag cannot raise a stricter 20 cap, unchanged simulation boundary.
- **12,937 actual host-GLES assertions PASS** including capture checks: real Medium at raw 2/3, uniforms show heavy effects disabled and ≤1 local light at 3; flattening normals still changes the cooling render; raw 4–6 is genuine Low; immediate recovery; world/RNG/journey state unchanged. Normal probe 42,534 changed pixels; height probe 6,431. Exact 2× nearest upscale and repeated context/switch tests retained.
- Thermal-2 Settings and thermal-3 gameplay captures visually inspected and retained here. These are **staged host GLES captures, not Galaxy F23 screenshots**.
- 72 controlled Low frames and 11,024 legacy geography records match previously compiled frozen-0.8 reference hashes exactly (`low-regression.json`).
- APK: original certificate; code 11/0.9.2; 3 ABIs; all 18 JNI functions; 16KB ELF alignment; ZIP/38 asset-license files/32 WAVs verified; no permissions.

**No physical Galaxy F23 or successful emulator run was performed for this build.** Earlier emulator attempts failed due sandbox RAM and lack of KVM; they were not repeated. No device FPS/temperature/cooling-rate guarantee is claimed.

## Authorized cleanup

User explicitly requested deleting earlier uploaded files while retaining current attachments. Six old uploads (6,431,951 bytes) were deleted; both current screenshots retained unchanged. Details in `upload-cleanup.json`.

User separately approved deleting the obsolete 0.9.1 APK **after** new APK verification. Its guide/hash record remain as historical records. 0.5, 0.8, 0.9, current source/checkpoints, private signing key and the current two uploads are preserved. Temporary build/tools/audio duplicates are removed again after validation.

## Artifact

`deliverables/death-world-0.9.2/Death-World-0.9.2-Thermal-Fix.apk`

16,964,842 bytes; SHA256 `f21feccb7a27902212792b5adaac5c3db318b69763675517fd2c3b193d8a1f85`.
