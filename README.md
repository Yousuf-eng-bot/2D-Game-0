# Death World

[![CI](https://github.com/Yousuf-eng-bot/2D-Game-0/actions/workflows/ci.yml/badge.svg)](https://github.com/Yousuf-eng-bot/2D-Game-0/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Language: C++20](https://img.shields.io/badge/C%2B%2B-20-00599C.svg)](CMakeLists.txt)
[![Platform: Android](https://img.shields.io/badge/platform-Android%2023%2B-3DDC84.svg)](android/AndroidManifest.xml)
[![Version](https://img.shields.io/badge/version-0.9.2%20(code%2011)-orange.svg)](CHANGELOG.md)

Mobile top-down pixel-art survival/action prototype. Mostly **C++20**, with an Android Java bridge for display/touch/audio/lifecycle/platform information. Android package: `com.ashenveil.game`; app name: **Death World**; version **0.9.2 / code 11**.

> **Status:** playable prototype. The 0.9.2 thermal fix has **not** yet been validated on physical hardware — see [Current functionality and limitations](#current-functionality-and-limitations).

## Quick start

```sh
git clone https://github.com/Yousuf-eng-bot/2D-Game-0.git
cd 2D-Game-0
sudo apt-get install -y build-essential cmake zlib1g-dev libegl1-mesa-dev libgles2-mesa-dev
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native -j"$(nproc)"
ctest --test-dir build/native --output-on-failure
```

For the Android APK see [Android APK build](#android-apk-build).
Contributions: [`CONTRIBUTING.md`](CONTRIBUTING.md) · Security: [`SECURITY.md`](SECURITY.md) · History: [`CHANGELOG.md`](CHANGELOG.md).

Start here: **[Bengali handoff / GitHub instructions](HANDOFF_BN.md)** · **[Agent instructions](AGENTS.md)** · **[Signing continuity](SIGNING.md)** · **[Current thermal-fix report](docs/medium-thermal-fix/STATUS.md)**.

## Repository contents

- `native/` — simulation, save migration, generation, art/UI, quality policy and real GLES lighting.
- `android/` — manifest, Java bridge, icon, cover, licenses and **all 32 runtime WAVs** in the source archive.
- `assets/` — editable title artwork, original Noto font files and licenses.
- `tests/` — unit/integration/walkthrough/real GLES tests and synthetic legacy-save fixtures.
- `tools/` — Android setup/build/verification, audio/asset generators, captures and source packaging.
- `docs/` — research, implementation status and historical test reports. The latest status is `docs/medium-thermal-fix/STATUS.md`; older reports are not current completion claims.
- `SOURCE-MANIFEST.json` — export-time hashes of every packaged file (except the manifest itself).

The portable ZIP contains the exact runtime WAVs even though the original workspace omits redundant WAV copies to save space. **Commit `android/assets/audio/` and `android/assets/music/` to GitHub.** With these files present, a fresh clone does not need an old APK to build. `restore_audio.py` remains a fallback for the original workspace only.

## First check after extraction/clone

```sh
python3 tools/check_source.py
```

This verifies the initial exported snapshot. After deliberate source edits, the old hashes will differ; re-export to create a new snapshot manifest. No GitHub login or network access is needed for this integrity check.

## Linux host tests

Ubuntu/Debian example:

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake zlib1g-dev libegl1-mesa-dev libgles2-mesa-dev
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native -j1
ctest --test-dir build/native --output-on-failure
```

Use a compiler with C++20 support (GCC 11+ or modern Clang). EGL/GLES development packages are needed for the real shader test; without them CMake may register only 13 tests. A complete configured run has **14**. The GLES test uses surfaceless software Mesa. That is correctness evidence, not phone FPS validation.

Pillow with RAQM/HarfBuzz, NumPy and FFmpeg are needed only for optional font regeneration/media tools; normal native builds use the committed generated font header. Do not run asset generators casually: they may overwrite curated/current asset files.

## Android APK build

Linux x86_64, JDK 17 (or compatible installed JDK), Python 3, `zip`, `unzip`. Review/accept applicable Android SDK/NDK licenses before downloading tools.

```sh
sudo apt-get install -y openjdk-17-jdk zip unzip python3
python3 tools/setup_android.py
# Restore the ORIGINAL signing key privately first; see SIGNING.md.
SIGNING_KEY=/private/path/ashen-prototype.jks bash tools/build_apk.sh
python3 tools/verify_apk.py Death-World-0.9.2-Thermal-Fix.apk
```

The helper downloads pinned official NDK r27c, Build Tools 34 and API35 files into `$HOME/.cache/android` (~0.8 GB download / several GB extracted). SDK tools and build outputs are not part of this repository. Native ABIs: arm64-v8a, armeabi-v7a and x86_64; minimum API23, target API35. The signing key is deliberately absent. The script fails early without it rather than silently breaking update compatibility.

Optional `APK_OUTPUT=/path/to/output.apk` selects another output path. Future APK releases must increase `versionCode` and preserve the original signing identity for updates. Source packaging changes made for this ZIP do not change the existing 0.9.2 APK.

## Current functionality and limitations

- Low preserves 0.8's appearance/preferences. Medium adds procedural detailed actors, a CPU G-buffer and real GLES normal lighting at 640×360 with nearest display scaling.
- New worlds use generator 5 organic paths/forest composition. Existing generators 1–4 and saved geography remain intact. Quality never selects world generation or combat difficulty.
- Most recent fix: thermal status/decision synchronization; Android thermal 2 keeps Medium at 30 cap, 3 keeps reduced-effects Medium at 20 cap, critical 4–6 uses mandatory Low safety. Caps are not measured FPS.
- **The owner has not yet confirmed the fix on the Galaxy F23.** No successful physical-phone or emulator runtime validation is claimed for 0.9.2.
- Full bespoke wildlife/props/inventory art, GPU-atlas batching/compression, advanced parallax/fog and physical-device performance validation remain incomplete.

## Packaging / safety

```sh
python3 tools/export_source.py --output ../Death-World-Source.zip
```

Packages active source, required assets and runtime audio; excludes keys, uploads, caches, APKs and historical source ZIPs. Build/test directories are generated locally. Old release backups and the private key remain in the original workspace and are **not** carried into a new chat by selecting this repository.

The existing MIT license is retained; third-party font/runtime notices are in `THIRD-PARTY-NOTICES.md` and bundled licenses. User-supplied studio branding does not imply affiliation/trademark clearance. Prefer a private repository until you have reviewed public release/branding/licensing choices.
