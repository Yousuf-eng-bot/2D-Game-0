#!/usr/bin/env python3
"""Bake the animal sprite atlas (``android/assets/animals.dwa``).

Same DWCA v1 container as the character atlas, so the engine loads both with
one code path. Animals get their own file because their clip list (graze,
hop, peck, flee) has nothing to do with the player's.

    python3 tools/build_animal_atlas.py [--preview] [--jobs N]
"""

from __future__ import annotations

import argparse
import math
import pathlib
import struct
import sys
import time

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import animalrig as A  # noqa: E402
import charparts as P  # noqa: E402
from charrig import ANCHOR_X, ANCHOR_Y, CELL_H, CELL_W, Canvas  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[1]
TAU = math.pi * 2

# Identical direction set to the character atlas, so the engine's existing
# facing map works unchanged.
DIRECTIONS = [
    ("s", TAU * 0.25),
    ("se", TAU * 0.125),
    ("e", 0.0),
    ("ne", -TAU * 0.125),
    ("n", -TAU * 0.25),
]


def _wave(i, n, k=1.0):
    return math.sin(i / n * TAU * k)


# ---------------------------------------------------------------------------
# Clips. Every clip is at least 6 frames, per the animation spec.
# ---------------------------------------------------------------------------

def pose_idle(i, n):
    p = A.AnimalPose()
    p.bob = _wave(i, n) * 0.6
    p.head_turn = _wave(i, n, 0.5) * 0.4
    p.tail = _wave(i, n, 2) * 0.5
    p.ear = max(0.0, _wave(i, n, 3)) * 0.4
    return p


def pose_graze(i, n):
    """Nose down, slow chewing bob - the default standing loop for deer."""
    p = A.AnimalPose()
    t = i / n
    p.head_down = 0.82 + math.sin(t * TAU) * 0.08
    p.bob = math.sin(t * TAU * 2) * 0.4
    p.neck = math.sin(t * TAU * 4) * 0.5     # chew
    p.tail = _wave(i, n, 2) * 0.6
    return p


def pose_head_lift(i, n):
    """The occasional alert lift out of the graze loop and back into it."""
    p = A.AnimalPose()
    t = i / (n - 1)
    rise = math.sin(min(1.0, t * 1.15) * math.pi)
    p.head_down = 0.82 * (1.0 - rise)
    p.ear = rise
    p.head_turn = math.sin(t * TAU) * 0.9 * rise
    p.neck = rise * 1.5
    p.tail = _wave(i, n, 2) * 0.4
    return p


def pose_walk(i, n, run=False):
    p = A.AnimalPose()
    ph = i / n * TAU
    amp = 1.0 if run else 0.62
    p.front = math.sin(ph) * amp
    p.rear = math.sin(ph + math.pi * 0.85) * amp
    p.lift = 1.6 if run else 0.9
    p.bob = abs(math.sin(ph * 2)) * (1.6 if run else 0.9) - 0.5
    p.stretch = math.sin(ph * 2) * (0.5 if run else 0.2)
    p.head_down = 0.12
    p.tail = math.sin(ph) * 0.8
    return p


def pose_hop(i, n):
    """Rabbit hop: crouch, launch with stretch, float, land with squash."""
    p = A.AnimalPose()
    t = i / (n - 1)
    if t < 0.2:                      # gather
        p.stretch = -0.9 * (t / 0.2)
        p.front = -0.5
        p.rear = -0.8
    elif t < 0.5:                    # launch
        u = (t - 0.2) / 0.3
        p.stretch = -0.9 + u * 2.0
        p.z = u * 6.0
        p.front = 0.9
        p.rear = -0.3 + u * 1.1
    elif t < 0.78:                   # float
        u = (t - 0.5) / 0.28
        p.stretch = 1.1 - u * 0.5
        p.z = 6.0 - u * 4.0
        p.front = 0.9 - u * 0.4
        p.rear = 0.8
    else:                            # land
        u = (t - 0.78) / 0.22
        p.stretch = 0.6 - u * 1.4
        p.z = 2.0 * (1.0 - u)
        p.front = -0.6
        p.rear = 0.2 - u * 0.8
    p.lift = 1.0
    p.ear = -p.stretch * 0.5
    return p


def pose_peck(i, n):
    """Bird ground-pecking loop: two dips with a look-up between them."""
    p = A.AnimalPose()
    t = i / n
    dip = max(0.0, math.sin(t * TAU * 2))
    p.head_down = dip ** 0.6
    p.bob = dip * 0.8
    p.tail = (1.0 - dip) * 0.6
    p.front = math.sin(t * TAU) * 0.25
    p.head_turn = math.cos(t * TAU) * 0.5 * (1.0 - dip)
    return p


def pose_flee(i, n):
    """Four-beat take-off: crouch, downbeat, climb, cruise."""
    p = A.AnimalPose()
    t = i / (n - 1)
    p.z = (t ** 1.4) * 14.0
    p.wing = 0.5 - math.cos(t * TAU * 1.5) * 0.5
    p.stretch = 0.4 + t * 0.4
    p.tail = 0.6
    p.head_down = 0.0
    p.front = -0.8 if t > 0.25 else 0.4
    return p


def pose_hurt(i, n):
    p = A.AnimalPose()
    t = i / (n - 1)
    p.stretch = -0.8 * (1.0 - t)
    p.bob = -1.2 * (1.0 - t)
    p.head_turn = 0.9 * (1.0 - t)
    p.ear = -0.6
    return p


def pose_die(i, n):
    p = A.AnimalPose()
    t = i / (n - 1)
    p.roll = t * 4.0
    p.bob = -t * 7.0
    p.front = -0.9 * t
    p.rear = 0.9 * t
    p.head_down = t
    p.stretch = t * 0.8
    return p


ANIMS = {
    "idle":      (8, 1, 7,  pose_idle),
    "graze":     (8, 1, 6,  pose_graze),
    "head_lift": (8, 0, 8,  pose_head_lift),
    "walk":      (8, 1, 10, lambda i, n: pose_walk(i, n, False)),
    "run":       (8, 1, 16, lambda i, n: pose_walk(i, n, True)),
    "hop":       (8, 1, 13, pose_hop),
    "peck":      (8, 1, 9,  pose_peck),
    "flee":      (6, 0, 14, pose_flee),
    "hurt":      (6, 0, 14, pose_hurt),
    "die":       (6, 0, 9,  pose_die),
}
ANIM_ORDER = list(ANIMS.keys())

VARIANTS = [(name, name.split("_")[0], painter, scale)
            for name, (painter, scale) in A.SPECIES.items()]


def bake_one(args):
    vi, ai, di, fi = args
    _, _, painter, scale = VARIANTS[vi]
    name = ANIM_ORDER[ai]
    frames, _loop, _fps, posefn = ANIMS[name]
    _dname, angle = DIRECTIONS[di]
    sk = A.AnimalSkeleton(posefn(fi, frames), angle, scale)
    c = Canvas()
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
    print(f"baking {len(tasks)} animal sprites on {jobs} workers", flush=True)
    if jobs > 1:
        import multiprocessing as mp
        with mp.Pool(jobs) as pool:
            return pool.map(bake_one, tasks, chunksize=64)
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
                       P.PALETTE_SLOTS)
    out += struct.pack("<HHH", len(VARIANTS), len(ANIM_ORDER), len(DIRECTIONS))
    for name, slot, _p, _s in VARIANTS:
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
    import build_character_atlas as B
    outdir.mkdir(parents=True, exist_ok=True)
    per = {}
    for r in results:
        per.setdefault(r[0], []).append(r)
    maxf = max(a[0] for a in ANIMS.values())
    for vi, rows in per.items():
        name = VARIANTS[vi][0]
        img = Image.new("RGBA", (maxf * CELL_W,
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
    ap.add_argument("--out", default="android/assets/animals.dwa")
    args = ap.parse_args()
    jobs = args.jobs or max(1, (__import__("os").cpu_count() or 2))
    t0 = time.time()
    results = build(jobs)
    size, blob, uniq = write_atlas(results, ROOT / args.out)
    print(f"{args.out}: {size} bytes ({blob} blob, {uniq} unique cells) "
          f"in {time.time() - t0:.1f}s")
    if args.preview:
        write_preview(results, ROOT / "docs" / "character-art" / "animals")


if __name__ == "__main__":
    main()
