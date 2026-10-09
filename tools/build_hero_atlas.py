#!/usr/bin/env python3
"""Bake the high-detail hero atlas (``android/assets/hero.dwa``).

The main character is the one actor on screen at all times, so he is baked
from his own larger rig (64x80 cell, ~58 px figure) instead of the shared
48x48 one. The container is the same DWCA v1 file the engine already loads,
and the cell size and anchor are read from the file, so nothing in the
runtime needed a special case.

Animation clips and poses are shared with the character atlas - the hero uses
the same 16 clips, so every existing state machine keeps working.

    python3 tools/build_hero_atlas.py [--preview] [--jobs N]
"""

from __future__ import annotations

import argparse
import os
import pathlib
import struct
import sys
import time

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import build_character_atlas as B  # noqa: E402
import charparts as CP  # noqa: E402
import heroparts as P  # noqa: E402
from charrig import Canvas  # noqa: E402
from herorig import ANCHOR_X, ANCHOR_Y, CELL_H, CELL_W, HeroSkeleton  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[1]

DIRECTIONS = B.DIRECTIONS
ANIMS = B.ANIMS
ANIM_ORDER = B.ANIM_ORDER

# Hero layers. Names deliberately mirror the small atlas so the runtime can
# resolve the same slots; the hero file simply supplies richer art.
VARIANTS = [
    ("body",        "base",      P.body),
    ("body_armed",  "base",      lambda c, sk: P.body(c, sk, bare_arms=False)),
    ("head",        "head",      P.head),
    ("hair",        "hair",      P.hair),
    ("hood",        "headgear",  P.helmet_hood),
    ("helm_steel",  "headgear",  P.helmet_crown),
    ("coat_ranger", "coat",      P.armor_ranger),
    ("coat_mail",   "coat",      P.armor_plate),
    ("coat_bandit", "coat",      P.armor_ranger),
    ("coat_elite",  "coat",      P.armor_plate),
    ("pauldrons",   "shoulders", P.pauldrons),
    ("cape",        "cape",      P.cape),
    ("pack",        "pack",      P.pack),
    ("belt",        "belt",      P.belt),
    ("w_dawnblade", "weapon",    P.weapon_greatsword),
    ("w_riftaxe",   "weapon",    lambda c, sk: P.weapon_axe(c, sk, False)),
    ("w_rustaxe",   "weapon",    lambda c, sk: P.weapon_axe(c, sk, True)),
    ("w_windbow",   "weapon",    P.weapon_bow),
    ("w_pick",      "weapon",    P.weapon_pick),
    ("w_sword",     "weapon",    P.weapon_sword),
    ("w_sunsteel",  "weapon",    lambda c, sk: P.weapon_greatsword(c, sk, True, True)),
    ("acc_talisman", "accessory", P.accessory_talisman),
    ("acc_cinder",  "accessory", P.accessory_cinder),
]


def bake_one(args):
    vi, ai, di, fi = args
    _, _, painter = VARIANTS[vi]
    name = ANIM_ORDER[ai]
    frames, _loop, _fps, posefn = ANIMS[name]
    _dname, angle = DIRECTIONS[di]
    sk = HeroSkeleton(posefn(fi, frames), angle)
    c = Canvas(CELL_W, CELL_H)
    painter(c, sk)
    c.despeckle()
    c.outline(1)
    x, y, w, h = c.bounds()
    if w == 0:
        return (vi, ai, di, fi, 0, 0, 0, 0, b"")
    rows = bytearray()
    for j in range(h):
        run_idx, run_len = -1, 0
        for i in range(w):
            v = c.get(x + i, y + j)
            if v == run_idx and run_len < 255:
                run_len += 1
            else:
                if run_len:
                    rows.append(run_len)
                    rows.append(run_idx)
                run_idx, run_len = v, 1
        if run_len:
            rows.append(run_len)
            rows.append(run_idx)
    return (vi, ai, di, fi, x, y, w, h, bytes(rows))


def build(jobs: int):
    tasks = [(vi, ai, di, fi)
             for vi in range(len(VARIANTS))
             for ai, aname in enumerate(ANIM_ORDER)
             for di in range(len(DIRECTIONS))
             for fi in range(ANIMS[aname][0])]
    print(f"baking {len(tasks)} hero sprites on {jobs} workers", flush=True)
    if jobs > 1:
        import multiprocessing as mp
        with mp.Pool(jobs) as pool:
            return pool.map(bake_one, tasks, chunksize=32)
    return [bake_one(t) for t in tasks]


def write_atlas(results, path: pathlib.Path):
    blob = bytearray()
    cache = {}
    index = {}
    for vi, ai, di, fi, x, y, w, h, data in results:
        if data in cache:
            off, ln = cache[data]
        else:
            off, ln = len(blob), len(data)
            blob += data
            cache[data] = (off, ln)
        index[(vi, ai, di, fi)] = (x, y, w, h, off, ln)
    out = bytearray(b"DWCA")
    out += struct.pack("<HHHhhH", 1, CELL_W, CELL_H, ANCHOR_X, ANCHOR_Y,
                       CP.PALETTE_SLOTS)
    out += struct.pack("<HHH", len(VARIANTS), len(ANIM_ORDER),
                       len(DIRECTIONS))
    for name, slot, _ in VARIANTS:
        out += struct.pack("<B", len(name)) + name.encode()
        out += struct.pack("<B", len(slot)) + slot.encode()
    for name in ANIM_ORDER:
        frames, loop, fps, _ = ANIMS[name]
        out += struct.pack("<B", len(name)) + name.encode()
        out += struct.pack("<BBB", frames, loop, fps)
    for vi in range(len(VARIANTS)):
        for ai, aname in enumerate(ANIM_ORDER):
            for di in range(len(DIRECTIONS)):
                for fi in range(ANIMS[aname][0]):
                    out += struct.pack("<BBBBII", *index[(vi, ai, di, fi)])
    out += struct.pack("<I", len(blob))
    out += blob
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(bytes(out))
    return len(out), len(blob), len(cache)


def write_preview(results, outdir: pathlib.Path):
    from PIL import Image
    outdir.mkdir(parents=True, exist_ok=True)
    per = {}
    for r in results:
        per.setdefault(r[0], []).append(r)
    maxf = max(a[0] for a in ANIMS.values())
    for vi, rows in per.items():
        name = VARIANTS[vi][0]
        img = Image.new("RGBA",
                        (maxf * CELL_W,
                         len(ANIM_ORDER) * len(DIRECTIONS) * CELL_H),
                        (24, 26, 24, 255))
        px = img.load()
        for _vi, ai, di, fi, x, y, w, h, data in rows:
            ox, oy = fi * CELL_W, (ai * len(DIRECTIONS) + di) * CELL_H
            cx, cy, i = 0, 0, 0
            while i < len(data):
                run, idx = data[i], data[i + 1]
                i += 2
                for _ in range(run):
                    if idx:
                        px[ox + x + cx, oy + y + cy] = B.PREVIEW_PALETTE[idx]
                    cx += 1
                    if cx >= w:
                        cx, cy = 0, cy + 1
        img.save(outdir / f"{name}.png")
    print(f"preview sheets -> {outdir}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--jobs", type=int, default=0)
    ap.add_argument("--preview", action="store_true")
    ap.add_argument("--out", default="android/assets/hero.dwa")
    args = ap.parse_args()
    jobs = args.jobs or max(1, (os.cpu_count() or 2))
    t0 = time.time()
    results = build(jobs)
    size, blob, uniq = write_atlas(results, ROOT / args.out)
    print(f"{args.out}: {size} bytes ({blob} blob, {uniq} unique cells) "
          f"in {time.time() - t0:.1f}s")
    if args.preview:
        write_preview(results, ROOT / "docs" / "character-art" / "hero")


if __name__ == "__main__":
    main()
