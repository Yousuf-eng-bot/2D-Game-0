#!/usr/bin/env python3
"""Bake the Death World character sprite atlas.

    python3 tools/build_character_atlas.py [--preview] [--jobs N]

Produces:
  android/assets/characters.dwa   runtime atlas (indexed + RLE, no PNG decoder
                                  needed in the engine)
  docs/character-art/*.png        human-readable sprite sheets for review
                                  (only with --preview, needs Pillow)

Design notes
------------
* Every layer is baked separately and composited at runtime, so changing a
  coat or a weapon changes the look on the very next frame.
* Pixels are palette *indices*, not colours. The runtime supplies a palette per
  actor, which is how one baked bandit becomes a grey common bandit or a
  gold-trimmed elite without extra frames.
* All frames share one 48x48 cell and one anchor (centre of the feet), so no
  animation can make the character hop between frames.
* Eight facing directions are stored as five baked directions plus a runtime
  horizontal mirror.
"""
from __future__ import annotations

import argparse
import math
import os
import pathlib
import struct
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import charparts as P  # noqa: E402
from charrig import ANCHOR_X, ANCHOR_Y, CELL_H, CELL_W, Canvas, Pose, Skeleton  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[1]
TAU = math.pi * 2

# Five baked directions; W / NW / SW are the runtime mirror of E / NE / SE.
DIRECTIONS = [
    ("S", math.pi / 2),
    ("SE", math.pi / 4),
    ("E", 0.0),
    ("NE", -math.pi / 4),
    ("N", -math.pi / 2),
]


# ---------------------------------------------------------------------------
# Animation definitions
# ---------------------------------------------------------------------------

def _ease(t: float) -> float:
    return t * t * (3 - 2 * t)


def pose_idle(i, n, variant=0):
    u = i / n
    br = math.sin(u * TAU)
    p = Pose(breathe=br * 0.9, bob=br * 0.35,
             arm_main=-0.75 + br * 0.05, arm_off=2.25 - br * 0.05,
             weapon_angle=-0.8 + br * 0.04)
    if variant == 1:      # glance left and right
        p.head_turn = math.sin(u * TAU) * 2.4
    elif variant == 2:    # settle the belt
        s = math.sin(u * TAU)
        p.arm_off = 2.25 - max(0.0, s) * 1.1
        p.arm_off_r = 8.0 - max(0.0, s) * 2.0
    elif variant == 3:    # line the sword up
        s = math.sin(u * TAU)
        p.arm_main = -0.75 - max(0.0, s) * 0.8
        p.weapon_angle = -0.8 - max(0.0, s) * 1.3
        p.head_nod = max(0.0, s) * 1.2
    return p


def pose_walk(i, n, run=False):
    u = i / n
    sw = math.sin(u * TAU)
    amp = 1.35 if run else 1.0
    p = Pose(leg=sw * amp,
             bob=abs(math.sin(u * TAU * 2)) * (1.9 if run else 1.3),
             lift=3.4 * amp,
             lean=(3.2 if run else 1.0),
             breathe=abs(sw) * 0.4,
             arm_main=-0.75 - sw * 0.45 * amp,
             arm_off=2.25 - sw * 0.5 * amp,
             weapon_angle=-0.8 - sw * 0.3)
    return p


def pose_attack(i, n):
    """Three acts: anticipation (2), strike (2), follow-through (3)."""
    if i < 2:                       # wind the blade back
        t = _ease((i + 1) / 2)
        return Pose(lean=-2.2 * t, twist=-1.4 * t, bob=-0.6 * t,
                    arm_main=-0.75 - 1.5 * t, arm_main_r=9.0,
                    weapon_angle=-0.9 - 1.7 * t, head_nod=-0.6 * t)
    if i < 4:                       # the strike itself
        t = (i - 2) / 1
        return Pose(lean=4.5 + 2.0 * t, twist=1.6 + 0.8 * t, bob=0.4,
                    arm_main=-2.25 + 3.3 * t, arm_main_r=11.5,
                    weapon_angle=-2.6 + 3.9 * t, weapon_extend=3.0 + 2.0 * t,
                    squash=-0.12, head_nod=0.8)
    t = _ease((i - 3) / (n - 4))    # recover
    return Pose(lean=6.5 * (1 - t), twist=2.4 * (1 - t),
                arm_main=1.05 - 1.8 * t, arm_main_r=11.5 - 1.5 * t,
                weapon_angle=1.3 - 2.1 * t, weapon_extend=5.0 * (1 - t),
                squash=-0.06 * (1 - t))


def pose_strong(i, n):
    """Bigger wind-up, a short forward lunge, a long recovery."""
    if i < 4:
        t = _ease((i + 1) / 4)
        return Pose(lean=-4.0 * t, twist=-2.4 * t, bob=-1.2 * t, crouch=0.25 * t,
                    arm_main=-0.75 - 2.3 * t, arm_main_r=9.0,
                    weapon_angle=-0.9 - 2.4 * t, head_nod=-1.2 * t)
    if i < 6:
        t = (i - 4) / 1
        return Pose(lean=7.0 + 3.0 * t, twist=2.2 + 1.0 * t, bob=0.6,
                    arm_main=-3.05 + 4.3 * t, arm_main_r=12.5,
                    weapon_angle=-3.3 + 5.0 * t, weapon_extend=5.0 + 3.5 * t,
                    squash=-0.2, head_nod=1.2)
    t = _ease((i - 5) / (n - 6))
    return Pose(lean=10.0 * (1 - t), twist=3.2 * (1 - t), crouch=0.2 * (1 - t),
                arm_main=1.25 - 2.0 * t, arm_main_r=12.5 - 2.5 * t,
                weapon_angle=1.7 - 2.5 * t, weapon_extend=8.5 * (1 - t),
                squash=-0.1 * (1 - t))


def pose_tool(i, n, overhead=True):
    """Chop (overhead, horizontal bite) and Mine (steeper, shorter)."""
    reach = 1.0 if overhead else 0.8
    if i < 3:
        t = _ease((i + 1) / 3)
        return Pose(lean=-3.0 * t, crouch=0.15 * t, bob=-0.8 * t,
                    arm_main=-0.75 - 2.0 * t * reach, arm_main_r=9.5,
                    arm_off=2.25 - 0.9 * t,
                    weapon_angle=-0.9 - 2.2 * t * reach, head_nod=-0.9 * t)
    if i < 5:
        t = (i - 3) / 1
        return Pose(lean=5.0 + 2.0 * t, crouch=0.3 + 0.15 * t, bob=0.5,
                    arm_main=-2.75 + 3.4 * t * reach, arm_main_r=11.0,
                    arm_off=1.35 + 0.5 * t,
                    weapon_angle=-3.1 + (4.0 if overhead else 3.2) * t,
                    weapon_extend=2.5 + 2.0 * t, squash=-0.14, head_nod=1.0)
    t = _ease((i - 4) / (n - 5))
    return Pose(lean=7.0 * (1 - t), crouch=0.45 * (1 - t),
                arm_main=0.65 - 1.4 * t, arm_main_r=11.0 - 1.5 * t,
                arm_off=1.85 + 0.4 * (1 - t),
                weapon_angle=0.9 - 1.7 * t, weapon_extend=4.5 * (1 - t))


def pose_guard(i, n):
    """Braced hold loop: body low, blade across the front."""
    u = i / n
    br = math.sin(u * TAU) * 0.5
    return Pose(crouch=0.42, lean=1.6, bob=br * 0.4, breathe=br * 0.5,
                arm_main=-1.55, arm_main_r=8.5, arm_off=-1.9, arm_off_r=7.5,
                weapon_angle=-1.6, weapon_extend=2.0, head_nod=-0.5)


def pose_guard_hit(i, n):
    """Two-frame recoil when a blow lands on the guard."""
    k = 1.0 if i == 0 else 0.45
    return Pose(crouch=0.5, lean=-3.0 * k, bob=-0.8 * k,
                arm_main=-1.55 - 0.5 * k, arm_main_r=7.5, arm_off=-1.9,
                arm_off_r=6.8, weapon_angle=-1.6 - 0.6 * k,
                weapon_extend=-1.5 * k, squash=0.12 * k)


def pose_dodge(i, n):
    """Roll: crouch, tuck, over, rise. Dust is drawn by the engine."""
    u = i / (n - 1)
    tuck = math.sin(u * math.pi)
    return Pose(crouch=0.35 + tuck * 0.6, z=tuck * 5.0,
                lean=4.0 + tuck * 7.0, bob=tuck * 1.5,
                arm_main=-0.75 + tuck * 2.4, arm_main_r=7.0,
                arm_off=2.25 - tuck * 2.4, arm_off_r=7.0,
                weapon_angle=-0.9 + tuck * 2.6,
                leg=math.sin(u * math.pi * 2) * 0.8, lift=2.0,
                squash=-tuck * 0.25, head_nod=tuck * 2.0)


def pose_jump(i, n):
    """Compression, two airborne frames, landing squash."""
    table = [
        Pose(crouch=0.55, bob=-1.0, squash=-0.22, arm_main=-0.3, arm_off=2.7,
             leg=0.0, weapon_angle=-0.5),
        Pose(z=7.0, crouch=0.1, squash=0.16, leg=0.7, lift=4.0,
             arm_main=-1.5, arm_off=3.0, weapon_angle=-1.6),
        Pose(z=10.0, crouch=0.0, squash=0.10, leg=0.2, lift=3.0,
             arm_main=-1.2, arm_off=2.8, weapon_angle=-1.3),
        Pose(z=3.0, crouch=0.2, squash=-0.05, leg=-0.5, lift=2.0,
             arm_main=-0.6, arm_off=2.4, weapon_angle=-0.7),
        Pose(crouch=0.62, bob=-0.6, squash=-0.28, leg=0.0,
             arm_main=-0.2, arm_off=2.8, weapon_angle=-0.4),
    ]
    return table[min(i, len(table) - 1)]


def pose_hurt(i, n):
    k = [1.0, 0.65, 0.3][min(i, 2)]
    return Pose(lean=-5.0 * k, twist=-1.5 * k, bob=-0.5 * k, crouch=0.3 * k,
                arm_main=-0.2 + 0.6 * k, arm_off=2.6 + 0.5 * k,
                weapon_angle=-0.4, head_nod=-2.0 * k, squash=0.14 * k)


def pose_die(i, n):
    t = _ease(i / (n - 1))
    return Pose(crouch=0.4 + 0.6 * t, lean=-2.0 + 10.0 * t, bob=-2.0 * t,
                arm_main=-0.2 + 1.6 * t, arm_main_r=9.0, arm_off=2.6 + 0.8 * t,
                weapon_angle=0.3 + 1.2 * t, head_nod=3.0 * t,
                leg=0.4 * t, squash=-0.3 * t)


# name -> (frames, loop, fps, pose function)
ANIMS = {
    "idle":        (8, 1, 8,  lambda i, n: pose_idle(i, n, 0)),
    "idle_look":   (8, 0, 9,  lambda i, n: pose_idle(i, n, 1)),
    "idle_adjust": (8, 0, 9,  lambda i, n: pose_idle(i, n, 2)),
    "idle_sword":  (8, 0, 9,  lambda i, n: pose_idle(i, n, 3)),
    "walk":        (8, 1, 11, lambda i, n: pose_walk(i, n, False)),
    "run":         (8, 1, 16, lambda i, n: pose_walk(i, n, True)),
    "attack":      (7, 0, 18, pose_attack),
    "strong":      (10, 0, 16, pose_strong),
    "chop":        (8, 0, 14, lambda i, n: pose_tool(i, n, True)),
    "mine":        (8, 0, 14, lambda i, n: pose_tool(i, n, False)),
    "guard":       (6, 1, 8,  pose_guard),
    "guard_hit":   (2, 0, 18, pose_guard_hit),
    "dodge":       (6, 0, 18, pose_dodge),
    "jump":        (5, 0, 12, pose_jump),
    "hurt":        (3, 0, 14, pose_hurt),
    "die":         (6, 0, 9,  pose_die),
}
ANIM_ORDER = list(ANIMS.keys())

# Layer variants. "slot" groups mutually exclusive options in the engine.
VARIANTS = [
    ("body",        "base",    P.body),
    ("body_armed",  "base",    lambda c, sk: P.body(c, sk, bare_arms=False)),
    ("head",        "head",    P.head),
    ("hair",        "hair",    P.hair),
    ("hood",        "headgear", P.helmet_hood),
    ("helm_steel",  "headgear", P.helmet_steel),
    ("coat_ranger", "coat",    P.coat_ranger),
    ("coat_mail",   "coat",    P.coat_mail),
    ("coat_bandit", "coat",    P.coat_bandit),
    ("coat_elite",  "coat",    P.coat_elite),
    ("pack",        "pack",    P.pack),
    ("belt",        "belt",    P.belt),
    ("w_dawnblade", "weapon",  P.weapon_dawnblade),
    ("w_riftaxe",   "weapon",  lambda c, sk: P.weapon_axe(c, sk, False)),
    ("w_rustaxe",   "weapon",  lambda c, sk: P.weapon_axe(c, sk, True)),
    ("w_windbow",   "weapon",  P.weapon_bow),
    ("w_pick",      "weapon",  P.weapon_pick),
    ("w_sword",     "weapon",  P.weapon_sword),
]


# ---------------------------------------------------------------------------
# Baking
# ---------------------------------------------------------------------------

def bake_one(args):
    vi, ai, di, fi = args
    _, _, painter = VARIANTS[vi]
    name = ANIM_ORDER[ai]
    frames, _loop, _fps, posefn = ANIMS[name]
    _dname, angle = DIRECTIONS[di]
    pose = posefn(fi, frames)
    sk = Skeleton(pose, angle)
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
    tasks = []
    for vi in range(len(VARIANTS)):
        for ai, aname in enumerate(ANIM_ORDER):
            frames = ANIMS[aname][0]
            for di in range(len(DIRECTIONS)):
                for fi in range(frames):
                    tasks.append((vi, ai, di, fi))
    print(f"baking {len(tasks)} sprites on {jobs} workers", flush=True)
    if jobs > 1:
        import multiprocessing as mp
        with mp.Pool(jobs) as pool:
            results = pool.map(bake_one, tasks, chunksize=64)
    else:
        results = [bake_one(t) for t in tasks]
    return results


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

    out = bytearray()
    out += b"DWCA"
    out += struct.pack("<HHHhhH", 1, CELL_W, CELL_H, ANCHOR_X, ANCHOR_Y,
                       P.PALETTE_SLOTS)
    out += struct.pack("<HHH", len(VARIANTS), len(ANIM_ORDER), len(DIRECTIONS))
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
                    x, y, w, h, off, ln = index[(vi, ai, di, fi)]
                    out += struct.pack("<BBBBII", x, y, w, h, off, ln)
    out += struct.pack("<I", len(blob))
    out += blob
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(bytes(out))
    return len(out), len(blob), len(cache)


# Preview palette, only used for the PNG review sheets. The engine ships its
# own palettes; these numbers are not the runtime source of truth.
PREVIEW_PALETTE = [
    (0, 0, 0, 0), (26, 22, 28, 255),
    (150, 104, 74, 255), (198, 160, 120, 255), (232, 200, 158, 255),
    (58, 74, 62, 255), (86, 108, 86, 255), (126, 148, 116, 255),
    (74, 54, 36, 255), (112, 84, 56, 255), (152, 120, 80, 255),
    (92, 104, 110, 255), (146, 160, 164, 255), (204, 216, 216, 255),
    (122, 88, 30, 255), (186, 142, 54, 255), (238, 204, 120, 255),
    (44, 34, 30, 255), (74, 56, 46, 255), (110, 86, 66, 255),
    (90, 70, 52, 255), (138, 110, 82, 255), (182, 156, 122, 255),
    (255, 255, 255, 255),
]


def write_preview(results, outdir: pathlib.Path):
    try:
        from PIL import Image
    except ImportError:
        print("Pillow not installed - skipping PNG preview sheets")
        return
    outdir.mkdir(parents=True, exist_ok=True)
    byvar = {}
    for vi, ai, di, fi, x, y, w, h, data in results:
        byvar.setdefault(vi, []).append((ai, di, fi, x, y, w, h, data))
    for vi, items in byvar.items():
        name = VARIANTS[vi][0]
        cols = max(ANIMS[a][0] for a in ANIM_ORDER)
        rows = len(ANIM_ORDER) * len(DIRECTIONS)
        img = Image.new("RGBA", (cols * CELL_W, rows * CELL_H), (0, 0, 0, 0))
        px = img.load()
        for ai, di, fi, x, y, w, h, data in items:
            row = ai * len(DIRECTIONS) + di
            bx, by = fi * CELL_W + x, row * CELL_H + y
            i = 0
            for j in range(h):
                col = 0
                while col < w and i + 1 < len(data) + 1:
                    if i + 1 >= len(data) + 1 or i >= len(data):
                        break
                    run, idx = data[i], data[i + 1]
                    i += 2
                    for k in range(run):
                        if idx:
                            px[bx + col + k, by + j] = PREVIEW_PALETTE[idx]
                    col += run
        img.save(outdir / f"{name}.png")
    print(f"preview sheets -> {outdir}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--preview", action="store_true")
    ap.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 2)))
    ap.add_argument("--out", default=str(ROOT / "android/assets/characters.dwa"))
    a = ap.parse_args()
    results = build(a.jobs)
    total, blob, uniq = write_atlas(results, pathlib.Path(a.out))
    print(f"atlas {a.out}: {total} bytes ({blob} pixel bytes, {uniq} unique)")
    if a.preview:
        write_preview(results, ROOT / "docs/character-art")


if __name__ == "__main__":
    main()
