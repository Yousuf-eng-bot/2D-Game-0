#!/usr/bin/env python3
"""Pixel-art character rig used to bake Death World sprite atlases.

Why a rig instead of hand-drawn frames: every frame of every layer has to sit
on exactly the same anchor (centre of the feet) in eight facing directions, for
several equipment combinations. A parametric rig guarantees that by
construction, which is what keeps the animation from jittering and keeps
equipment layers registered on top of each other.

The projection is deliberately identical to the engine's existing skeletal
renderer (`drawJourneyRanger`), so baked sprites keep the established
perspective of the game:

    rx, ry = -sin(a), cos(a)
    screen_x = rx * side + cos(a) * forward
    screen_y = ry * side * 0.55 + sin(a) * forward * 0.65 - height

Output is pixel-perfect: nearest sampling only, hard edges, no anti-aliasing,
and a fixed 4-step ramp per material (outline / shadow / base / highlight).
"""
from __future__ import annotations

import math
from dataclasses import dataclass, field

# Sprite cell geometry. Anchor is the centre of the feet.
CELL_W, CELL_H = 48, 48
ANCHOR_X, ANCHOR_Y = 24, 42

TRANSPARENT = 0


@dataclass
class Ramp:
    """A 4-step material ramp: outline, shadow, base, highlight."""

    outline: int
    shadow: int
    base: int
    highlight: int

    def step(self, light: float) -> int:
        if light < -0.25:
            return self.shadow
        if light > 0.45:
            return self.highlight
        return self.base


class Canvas:
    """Indexed pixel buffer. Index 0 is transparent; other indices are palette
    slots resolved later, which is what makes runtime recolouring possible."""

    def __init__(self, w: int = CELL_W, h: int = CELL_H):
        self.w, self.h = w, h
        self.px = bytearray(w * h)
        self.depth = [-9999.0] * (w * h)

    def put(self, x: int, y: int, idx: int, z: float = 0.0) -> None:
        x, y = int(x), int(y)
        if idx == TRANSPARENT or x < 0 or y < 0 or x >= self.w or y >= self.h:
            return
        o = y * self.w + x
        if z >= self.depth[o]:
            self.depth[o] = z
            self.px[o] = idx

    def get(self, x: int, y: int) -> int:
        if x < 0 or y < 0 or x >= self.w or y >= self.h:
            return TRANSPARENT
        return self.px[y * self.w + x]

    def is_empty(self) -> bool:
        return not any(self.px)

    # -- primitives ------------------------------------------------------
    def disc(self, cx: float, cy: float, rx: float, ry: float, idx: int,
             z: float = 0.0) -> None:
        if rx <= 0 or ry <= 0:
            return
        for y in range(int(cy - ry) - 1, int(cy + ry) + 2):
            for x in range(int(cx - rx) - 1, int(cx + rx) + 2):
                dx, dy = (x - cx) / rx, (y - cy) / ry
                if dx * dx + dy * dy <= 1.0:
                    self.put(x, y, idx, z)

    def quad(self, pts, idx: int, z: float = 0.0) -> None:
        ys = [p[1] for p in pts]
        for y in range(int(min(ys)), int(max(ys)) + 1):
            xs = []
            n = len(pts)
            for i in range(n):
                x1, y1 = pts[i]
                x2, y2 = pts[(i + 1) % n]
                if (y1 <= y < y2) or (y2 <= y < y1):
                    xs.append(x1 + (y - y1) * (x2 - x1) / (y2 - y1))
            xs.sort()
            for i in range(0, len(xs) - 1, 2):
                for x in range(int(math.floor(xs[i])), int(math.ceil(xs[i + 1])) + 1):
                    self.put(x, y, idx, z)

    def capsule(self, a, b, r0: float, r1: float, ramp: Ramp,
                z: float = 0.0, light=(-0.6, -0.8)) -> None:
        """A tapered limb with a lit side and a shadowed side."""
        ax, ay = a
        bx, by = b
        steps = max(2, int(math.hypot(bx - ax, by - ay) * 2) + 2)
        lx, ly = light
        for i in range(steps + 1):
            t = i / steps
            cx, cy = ax + (bx - ax) * t, ay + (by - ay) * t
            r = r0 + (r1 - r0) * t
            for yy in range(int(cy - r) - 1, int(cy + r) + 2):
                for xx in range(int(cx - r) - 1, int(cx + r) + 2):
                    dx, dy = xx - cx, yy - cy
                    d = math.hypot(dx, dy)
                    if d > r:
                        continue
                    n = (dx / r * lx + dy / r * ly) if r > 0 else 0.0
                    self.put(xx, yy, ramp.step(-n), z + (yy - cy) * 0.001)

    def shaded_box(self, x: float, y: float, w: float, h: float, ramp: Ramp,
                   z: float = 0.0) -> None:
        for j in range(int(h)):
            for i in range(int(w)):
                if w <= 2 or h <= 2:
                    idx = ramp.base
                elif i <= 0 or j <= 0:
                    idx = ramp.highlight
                elif i >= w - 2 or j >= h - 2:
                    idx = ramp.shadow
                else:
                    idx = ramp.base
                self.put(x + i, y + j, idx, z)

    # -- post passes -----------------------------------------------------
    def outline(self, idx: int) -> None:
        """One-pixel hard outline around the silhouette. Runs after all shapes
        so layers composited at runtime each read as a solid object."""
        src = bytes(self.px)
        for y in range(self.h):
            for x in range(self.w):
                if src[y * self.w + x]:
                    continue
                touching = False
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < self.w and 0 <= ny < self.h and src[ny * self.w + nx]:
                        touching = True
                        break
                if touching:
                    self.px[y * self.w + x] = idx

    def despeckle(self) -> None:
        """Remove isolated single pixels. These are the main source of the
        'boiling' shimmer when a rig is sampled frame by frame."""
        src = bytes(self.px)
        for y in range(self.h):
            for x in range(self.w):
                if not src[y * self.w + x]:
                    continue
                n = 0
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < self.w and 0 <= ny < self.h and src[ny * self.w + nx]:
                        n += 1
                if n == 0:
                    self.px[y * self.w + x] = TRANSPARENT

    def bounds(self):
        minx, miny, maxx, maxy = self.w, self.h, -1, -1
        for y in range(self.h):
            row = self.px[y * self.w:(y + 1) * self.w]
            if not any(row):
                continue
            miny = min(miny, y)
            maxy = max(maxy, y)
            for x in range(self.w):
                if row[x]:
                    minx = min(minx, x)
                    maxx = max(maxx, x)
        if maxx < 0:
            return (0, 0, 0, 0)
        return (minx, miny, maxx - minx + 1, maxy - miny + 1)


# ---------------------------------------------------------------------------
# Skeleton
# ---------------------------------------------------------------------------

@dataclass
class Pose:
    """One frame of animation, in character-local units.

    `side` is left/right across the body, `forward` is along the facing
    direction and `height` is up from the ground, exactly like the engine rig.
    """

    bob: float = 0.0          # vertical body bounce, in pixels
    z: float = 0.0            # airborne height
    lean: float = 0.0         # forward lean of the upper body
    crouch: float = 0.0       # 0 standing, 1 crouched
    twist: float = 0.0        # shoulder rotation around the spine
    leg: float = 0.0          # leg swing phase, -1..1
    lift: float = 0.0         # trailing-foot lift
    arm_main: float = -0.9    # main-hand arm angle, radians
    arm_off: float = 2.2      # off-hand arm angle, radians
    arm_main_r: float = 10.0  # main-hand reach
    arm_off_r: float = 8.0
    weapon_angle: float = -0.9
    weapon_extend: float = 0.0
    head_turn: float = 0.0
    head_nod: float = 0.0
    squash: float = 0.0       # +stretch / -squash, for impacts and landings
    breathe: float = 0.0      # chest rise, in pixels


class Skeleton:
    """Projects a Pose into screen-space joints for one facing angle."""

    def __init__(self, pose: Pose, angle: float):
        self.pose = pose
        self.a = angle
        self.fx, self.fy = math.cos(angle), math.sin(angle)
        # The across-body axis must never collapse completely. With a pure
        # -sin(a) the side view would be one pixel wide paper, so the body
        # keeps a minimum apparent depth - the usual top-down sprite cheat.
        side_axis = -self.fy
        if abs(side_axis) < 0.55:
            side_axis = 0.55 if side_axis >= 0 else -0.55
        self.rx, self.ry = side_axis, self.fx
        p = pose
        self.root_y = ANCHOR_Y - p.z - p.bob
        sq = 1.0 + p.squash * 0.18
        self.sq = sq

        def pt(side, forward, height):
            x = ANCHOR_X + self.rx * side + self.fx * forward
            y = (self.root_y + self.ry * side * 0.55 +
                 self.fy * forward * 0.65 - height * sq)
            return (x, y)

        self.pt = pt
        c = p.crouch
        self.hip = pt(0, p.lean * 0.3, 15 - 5 * c)
        self.chest = pt(0, p.lean * 0.7, 25 - 8 * c + p.breathe)
        self.shoulder = pt(0, p.lean, 31 - 10 * c + p.breathe)
        self.neck = pt(0, p.lean, 34 - 11 * c + p.breathe)
        self.head = pt(p.head_turn * 1.2, p.lean + p.head_nod,
                       41 - 12 * c + p.breathe)

    def shoulder_side(self, side: float):
        p = self.pose
        return self.pt(side * (5 + p.twist * side * 1.5), p.lean,
                       30 - 10 * p.crouch + p.breathe)

    def hand(self, side: float, angle: float, reach: float):
        """Hand position: swing in the character's own vertical plane."""
        p = self.pose
        sh = self.shoulder_side(side)
        hx = sh[0] + math.cos(angle) * reach * (1 if side > 0 else 1)
        hy = sh[1] + math.sin(angle) * reach * 0.8
        # Push the hand along the facing direction so forward strikes read.
        hx += self.fx * p.weapon_extend
        hy += self.fy * p.weapon_extend * 0.65
        return (hx, hy)

    def foot(self, side: float, phase: float):
        """Foot placement. `phase` is -1..1 within the step cycle."""
        p = self.pose
        swing = phase * 7.0
        lift = max(0.0, p.lift * math.sin(max(0.0, phase) * math.pi))
        return self.pt(side * 5, swing + p.lean * 0.2, 2 + lift)

    def knee(self, side: float, phase: float):
        p = self.pose
        return self.pt(side * 5, phase * 3.5 + p.lean * 0.2, 9 - 3 * p.crouch)

    def depth(self, side: float) -> float:
        """Painter order for a limb on the given body side."""
        return self.ry * side * 0.55
