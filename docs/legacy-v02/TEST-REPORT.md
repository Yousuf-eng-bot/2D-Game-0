# Death World 0.2 verification — 2026-10-04

## Artifact and update identity

- Real compiled APK, not a project renamed to APK.
- Visible label: **Death World**.
- Package retained: `com.ashenveil.game`.
- Version: 0.2.0 / versionCode 2; old versionCode was 1.
- Min API 23, target API 35.
- ARM64, ARMv7, x86_64 native C++20 libraries.
- Same SHA-256 signing-certificate fingerprint as the original APK:
  `6860d6fd6332594af5f30916fb67d9717088f63ebecb1c8d34476d27ae8f710b`
- APK signature v1/v2/v3 and ZIP alignment verified.
- Native ELF load-segment alignment set to 0x4000 / 16 KB.
- No Android permissions requested, including no INTERNET permission.
- WAV assets are stored uncompressed so Android SoundPool's asset-file-descriptor API can load them.

## Executed native checks

The final gameplay source passed **29,422 assertions** in an AddressSanitizer + UndefinedBehaviorSanitizer host build, with no sanitizer findings in that run.

Covered:

1. Exact legacy `ASHEN1` field preservation and `DEATH2` round trip.
2. Legacy-save backup and corrupt-primary recovery.
3. Two regions × 100 encounter seeds: walkable player/enemy/boss spawns and navigable routes to all shrines/guardians.
4. Swept movement, wall blocking, analog acceleration, normalized diagonals and braking.
5. Independent movement/attack finger IDs, cancellation, native background pause and input release.
6. Sword/Axe startup timing, once-per-swing hit payloads, ranged arrows, once-per-target piercing volleys, and weapon-switch cooldown protection.
7. Two dodge charges and recharge, invulnerability, healing limitations, Ward mitigation and damage-feedback flags.
8. Guardian seal protection, three phases, multiple distinct patterns, bounded/deferred minion spawning.
9. Stagger resistance and a regression test for an arrow killing a boss without invalidating the projectile iterator.
10. One-time rewards, claim-victory flow, camp return, equipment stats, equipped-item salvage protection and loot constraints.
11. Distinct animated walking poses and 3,000 randomized simulation/render frames with bounded entities/effects and finite positions.

Actual output is in `core-test-results.txt`.

## Input-only playthrough checks

An automated agent completed **18/18 runs**: two regions × three weapons × three encounter seeds. It used normal movement, attacks, dodge/power/Ward/healing, and legitimately collected gear. It did not teleport, grant HP, modify damage or invent rewards. Its perfect state knowledge and instant reactions are advantages over a human player; these tests establish mechanical completion, not ideal human difficulty.

Simulated completion times ranged from 53 to 146 seconds. These are not expected human session lengths and not Android performance measurements.

Two additional isolated-guardian fixtures placed a default character in combat and held attack without movement, healing or defense. The stationary attacker was defeated after 11 seconds against CrownHorn and 14 seconds against Solkar. These are controlled difficulty checks, not full runs.

Actual output is in `playthrough-results.txt`.

## Visual inspection

The actual C++ renderer was used to capture title, camp, region selection, armory, forest/canyon landscapes, both boss telegraphs, inventory, guide, full map and walking poses. Visual fixtures reposition entities explicitly; they are not represented as live phone screenshots or uninterrupted gameplay recordings. They helped identify and fix boss UI occlusion by moving the guardian bars and adjusting combat camera framing.

The title illustration is an original generated asset; gameplay sprites/props are original C++ pixel drawings. Asset provenance and research are documented separately.

## Android emulator update test — limited

A software-only Android 9 / API 28 x86_64 emulator was booted without hardware acceleration. The pre-existing 0.1.0 app was present.

The first streamed-install attempt failed while the emulator's package/activity services were temporarily unavailable. Once the package service returned, a non-streaming `adb install --no-streaming -r` completed with **Success**. `dumpsys package` then reported versionCode 2 / versionName 0.2.0. No uninstall was performed, confirming that Android accepted the same-key in-place update.

The activity launched and the new Death World title artwork rendered. The emulator subsequently displayed its own **System UI isn't responding** dialog, which blocked reliable touch testing. The captured crash buffer was empty at inspection, but that does not prove crash-free gameplay. An attempt to stop the emulator's System UI did not make full input testing reliable; the emulator was stopped rather than claiming a successful full run.

`android-update-test.txt` records the successful update and version check. This validates installation/update and initial native rendering on that emulator, **not** a complete Android playthrough, physical-phone performance, or real multitouch/audio quality.

After this emulator check, a final native-only correction made overlapping piercing arrows remember their own targets, preventing repeated damage to the same enemy. Five additional regression assertions passed, and the full sanitizer suite was rerun. The test agent was also corrected to respect body-width clearance when seeking loot; it then completed all 18 runs with the final gameplay code. No gameplay difficulty was weakened to achieve these results.

The final APK retains the identical Java/Dex Android bridge, package identity and signer, but its native libraries include this last correction. **The final corrected native binary was not reinstalled in the emulator.** The final archive also includes updated license text. Signature, metadata, all three native ABI builds, alignment and packaged assets were checked again; see `final-artifact-verification.txt`.

## Not yet verified

- The new 0.2 build on the user's actual phone or any physical ARM device.
- Real-phone multi-touch feel, audio mix, haptics, thermal/battery behavior, long-session stability and sustained FPS.
- Modern Android cutout/system-bar behavior across vendors and actual 16 KB-page devices.
- Human boss difficulty, accessibility and control-layout comfort.
- Production signing, AAB/Play Store submission, multiplayer or iOS.

The user positively confirmed physical-phone functionality of version 0.1; that feedback is not treated as physical-phone verification of this new build.
