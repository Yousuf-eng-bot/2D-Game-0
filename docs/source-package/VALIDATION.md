# Portable source package validation — 2026-10-06

Current project version: 0.9.2 / code 11. No native game logic or Android Java source was changed for packaging. Added portable README/handoff/agent/signing instructions, Git hygiene files, source export/integrity tools and an early missing-key guard in the APK build script.

## Executed checks

- Source ZIP extracted into an isolated scratch directory, not built in the original source folder.
- Every exported file checked against its SHA256/size manifest; ZIP CRC passed.
- All 32 runtime WAVs included and original missing WAVs hash-verified against the retained release audio manifest.
- `restore_audio.py` executed with archive access forbidden: successful, proving the extracted source does not open an old APK to obtain audio.
- A temporary local Git repository staged every packaged file, including all WAVs and synthetic legacy-save fixtures. `.gitignore` correctly ignores signing keys, user uploads, build files, local environment and APKs. This was a local staging test, not a GitHub push.
- Archive paths checked; no private keys, tokens, uploaded screenshots, APKs, nested source ZIPs, SDK/NDK or build products included. Game code and Java/resources compared byte-for-byte with current source.
- `bash -n tools/build_apk.sh` passed. With an intentionally absent signing key, build exits 2 immediately and creates no new key. Original private key was not used or packaged during this test.
- Initial host test attempt found CMake/EGL development prerequisites absent. Installed the documented dependencies, then configured and compiled the extracted tree sequentially.
- **14/14 CTest PASS, 16.46 seconds**, including real host GLES test (`ctest-from-extracted-source.txt`).

After these tests, this validation document and its log were added to the final archive; the compiled code/assets are unchanged. Final manifest/CRC/Git tracking checks were repeated. The manifest itself is not self-hashed; all other packaged files are listed.

No Android SDK was downloaded and no new APK was built in this source-packaging step. Existing 0.9.2 APK validation is historical evidence in `docs/medium-thermal-fix/`; this package test is not a phone/runtime/FPS test. No repository was created on GitHub and nothing was pushed remotely.

## Intentional exclusions

- Private signing key, credentials and actual user uploads/saves.
- APK/release binaries, preview videos and old generated `docs/images/` galleries.
- `docs/history/` archived releases/source snapshots; mandatory backups remain in the original workspace.
- SDK/NDK, generated build products, dependency caches and Git metadata.

Active `native/`, Java, manifest/resources, source assets/fonts/licenses, tools and tests are complete. Current Medium/thermal implementation reports and host reference captures are included. Some historical docs mention excluded backup/release paths; those are archival references, not fresh-clone build requirements.
