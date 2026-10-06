#!/usr/bin/env python3
"""Re-export SOURCE-MANIFEST.json for the current tree.

`tools/check_source.py` verifies a snapshot against this manifest, so after a
deliberate change the manifest has to be regenerated - otherwise every later
check reports the intended edits as corruption.

Scope matches the original 0.9.2 export: active source and the runtime assets
the game needs, and nothing else. Private keys, APKs, build output, caches and
historical ZIPs stay out.

    python3 tools/export_source_manifest.py [--version 0.9.3]
"""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[1]

SCOPE = ("Active source and required runtime assets; no private keys, user "
         "uploads, APKs, SDK/build caches, historical source ZIPs or old "
         "generated screenshot galleries.")

# Top-level files always included.
ROOT_FILES = [
    ".clang-format", ".gitattributes", ".gitignore", "AGENTS.md",
    "CHANGELOG.md", "CMakeLists.txt", "CONTRIBUTING.md", "HANDOFF_BN.md",
    "LICENSE", "README.md", "SIGNING.md", "THIRD-PARTY-NOTICES.md",
]

DIRS = ["native", "tests", "tools", "android", "assets", "docs", ".github"]

# Anything matching these is build output, a secret, or a historical archive.
SKIP_SUFFIX = (".apk", ".jks", ".keystore", ".zip", ".o", ".so", ".pyc")
SKIP_PARTS = {"build", "__pycache__", ".git", "node_modules"}


def included(p: pathlib.Path) -> bool:
    if not p.is_file() or p.is_symlink():
        return False
    if any(part in SKIP_PARTS for part in p.parts):
        return False
    return not p.name.endswith(SKIP_SUFFIX)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--version", default=None,
                    help="version string to record (default: keep current)")
    args = ap.parse_args()

    current = {}
    manifest_path = ROOT / "SOURCE-MANIFEST.json"
    if manifest_path.is_file():
        current = json.loads(manifest_path.read_text())

    paths = []
    for name in ROOT_FILES:
        p = ROOT / name
        if included(p):
            paths.append(p)
    for d in DIRS:
        for p in sorted((ROOT / d).rglob("*")):
            if included(p):
                paths.append(p)

    files = []
    wavs = 0
    for p in sorted(set(paths)):
        rel = p.relative_to(ROOT).as_posix()
        data = p.read_bytes()
        files.append({"path": rel, "bytes": len(data),
                      "sha256": hashlib.sha256(data).hexdigest()})
        if rel.startswith("android/assets/") and rel.endswith(".wav"):
            wavs += 1

    out = {
        "format": 1,
        "project": current.get("project", "Death World"),
        "version": args.version or current.get("version", "0.9.2"),
        "file_count": len(files),
        "runtime_wav_count": wavs,
        "scope": SCOPE,
        "files": files,
    }
    manifest_path.write_text(json.dumps(out, indent=2) + "\n")
    print(f"SOURCE-MANIFEST.json: {len(files)} files, {wavs} runtime WAVs, "
          f"version {out['version']}")


if __name__ == "__main__":
    main()
