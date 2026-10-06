# Third-party / asset notices — Death World 0.5

Gameplay sprites, bitmap font patterns, code-drawn studio crystal, terrain/props, new wildlife/routine/station/anatomy artwork, projected shadows, procedural animations and game implementation are supplied as part of this prototype. Coordinate generation uses common deterministic hashing/noise techniques, including SplitMix64-style integer mixing; it is not Mojang's world generator.

The original title artwork, Frontier panorama and v0.5 woodland title panorama were generated for this project as AI-assisted assets. The Python asset tools synthesize all bundled music, nature beds, transitions, footfalls and character/guardian effects, including new chopping/felling sounds. No Rockstar/Blizzard/Mojang art, character models, logo, screenshot, recorded voice or soundtrack is embedded.

OBSIDIAN GAMES and OBSIDIAN SYNDICATE are user-supplied branding. The prototype does not assert affiliation with Obsidian Entertainment or claim trademark clearance. See docs/RESEARCH-AND-DESIGN.md before public/commercial branding use.

The APK statically links the official Android NDK C++ runtime and associated support/unwind code. Complete official r27c LLVM and NDK sysroot notices are retained in android/assets/licenses and bundled in the APK. LLVM components include Apache 2.0 with LLVM exceptions and component-specific notices; consult those complete texts.

Android SDK/NDK, JDK/D8, CMake, Python/Pillow/NumPy, clang-format and the optional preview-video encoder are build/test dependencies, not bundled runtime services. No third-party game engine, network SDK, analytics service, ad library, cloud AI, external font download or online music service is included.

## 0.7 interface fonts

Noto Sans and Noto Sans Bengali from the Google Fonts repository, licensed
under the SIL Open Font License 1.1. Original font files and complete licenses
are in `assets/fonts/`; licenses are also bundled in the APK assets.
Bengali strings are shaped offline with Pillow's RAQM/HarfBuzz support, then
stored as compressed coverage masks. Latin glyphs use the same Noto family.
`tools/build_ui_fonts.py` reproduces the generated `native/ui7_fonts.hpp`.
The engine links the platform zlib library to decompress the fixed-size atlas;
no network, font download or external font service is used at runtime.
