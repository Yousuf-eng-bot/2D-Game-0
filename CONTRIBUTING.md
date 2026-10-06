# Contributing to Death World

Thanks for your interest. This repository holds the full C++20 source of the
**Death World** Android prototype (package `com.ashenveil.game`).

## Ground rules

- Read [`AGENTS.md`](AGENTS.md) before any non-trivial change — it documents the
  invariants (save-format migration, quality policy, thermal behaviour).
- Never commit signing keys, keystores, APKs, tokens or private uploads. The
  `.gitignore` blocks the common cases; double-check `git status` anyway.
- Do not run the asset generators in `tools/` casually: they can overwrite
  curated art/audio that is already committed.

## Development setup (Linux host)

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake zlib1g-dev \
  libegl1-mesa-dev libgles2-mesa-dev
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native -j"$(nproc)"
ctest --test-dir build/native --output-on-failure
```

A fully configured run registers **14** tests. If you only see 13, the EGL/GLES
development packages are missing and the real-shader test was skipped.

## Coding style

- C++20, no compiler extensions (`CMAKE_CXX_EXTENSIONS OFF`).
- Formatting follows [`.clang-format`](.clang-format):
  ```sh
  find native tests -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i
  ```
  Do not reformat the generated `native/ui7_fonts.hpp`.
- Headers in `native/` are header-only modules included by `native/engine.cpp`;
  keep them self-contained and free of platform `#ifdef` sprawl where possible.

## Pull requests

1. Branch from the default branch.
2. Keep the change focused; explain *why* in the PR description.
3. Make sure `ctest` passes locally and in CI.
4. If you change packaged source files, note that `SOURCE-MANIFEST.json`
   hashes will no longer match — re-export with
   `python3 tools/export_source.py` when cutting a release snapshot.
5. Any APK release must **increase `versionCode`** and reuse the original
   signing identity (see [`SIGNING.md`](SIGNING.md)).

## Reporting bugs

Use the issue templates. For device-specific problems include the phone model,
Android version, chosen quality level and whether thermal throttling was active.
