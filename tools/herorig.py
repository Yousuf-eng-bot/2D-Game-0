"""High-detail hero rig for Death World's main character.

The ordinary character rig draws everyone at 48x48 with a ~34 px tall figure.
The hero is the one actor the player stares at all the time, so he gets his
own, much larger rig: a 80x96 cell with a ~62 px tall figure, which is close
to three times the pixel area to spend on detail.

Design brief from the owner, referencing Diablo Immortal: long flowing hair,
heavy ornate armour with gold filigree, spiked pauldrons, a cape, and a
dangerous blood-stained weapon. Everything is still indexed-palette pixel art
with a hard one-pixel outline - no anti-aliasing anywhere.

The projection is identical to the small rig, so the hero stands on the same
ground plane and uses the same five baked directions.
"""

from __future__ import annotations

import math
from dataclasses import dataclass

from charrig import Canvas, Pose, Ramp  # noqa: F401

# Hero cell. Anchor is the centre of the feet, like every other actor.
CELL_W, CELL_H = 80, 96
ANCHOR_X, ANCHOR_Y = 40, 86

# Everything below is written in the same character-local units as the small
# rig, then multiplied by SCALE. That keeps the two rigs' poses interchangeable.
SCALE = 1.62


class HeroSkeleton:
    """Projects a :class:`charrig.Pose` into hero-scale screen joints.

    Adds joints the small rig has no room for: a multi-strand hair chain, a
    cape hem, pauldron caps and a proper weapon hand frame.
    """

    def __init__(self, pose: Pose, angle: float):
        self.pose = pose
        self.a = angle
        self.fx, self.fy = math.cos(angle), math.sin(angle)
        side_axis = -self.fy
        if abs(side_axis) < 0.55:
            side_axis = 0.55 if side_axis >= 0 else -0.55
        self.rx, self.ry = side_axis, self.fx
        p = pose
        self.root_y = ANCHOR_Y - (p.z + p.bob) * SCALE
        self.sq = 1.0 + p.squash * 0.18

        def pt(side, forward, height):
            x = ANCHOR_X + (self.rx * side + self.fx * forward) * SCALE
            y = (self.root_y +
                 (self.ry * side * 0.55 + self.fy * forward * 0.65) * SCALE -
                 height * self.sq * SCALE)
            return (x, y)

        self.pt = pt
        c = p.crouch
        self.hip = pt(0, p.lean * 0.3, 15 - 5 * c)
        self.chest = pt(0, p.lean * 0.7, 25 - 8 * c + p.breathe)
        self.shoulder = pt(0, p.lean, 31 - 10 * c + p.breathe)
        self.neck = pt(0, p.lean, 34 - 11 * c + p.breathe)
        self.head = pt(p.head_turn * 1.2, p.lean + p.head_nod,
                       41.5 - 12 * c + p.breathe)

    # -- joints ----------------------------------------------------------
    def shoulder_side(self, side: float):
        p = self.pose
        return self.pt(side * (5.4 + p.twist * side * 1.5), p.lean,
                       30 - 10 * p.crouch + p.breathe)

    def hand(self, side: float, angle: float, reach: float):
        p = self.pose
        sh = self.shoulder_side(side)
        hx = sh[0] + math.cos(angle) * reach * SCALE
        hy = sh[1] + math.sin(angle) * reach * 0.8 * SCALE
        hx += self.fx * p.weapon_extend * SCALE
        hy += self.fy * p.weapon_extend * 0.65 * SCALE
        return (hx, hy)

    def foot(self, side: float, phase: float):
        p = self.pose
        lift = max(0.0, p.lift * math.sin(max(0.0, phase) * math.pi))
        return self.pt(side * 3.6, phase * 7.0 + p.lean * 0.2, 2 + lift)

    def knee(self, side: float, phase: float):
        p = self.pose
        return self.pt(side * 3.5, phase * 3.5 + p.lean * 0.2,
                       9.5 - 3 * p.crouch)

    def depth(self, side: float) -> float:
        return self.ry * side * 0.55

    def hair_chain(self, side: float, n: int = 5):
        """A chain of points down one lock of hair.

        The lock trails behind the head, swings with the body and lags one
        step behind the walk cycle, which is what sells long hair in motion.
        """
        p = self.pose
        swing = math.sin(p.leg * 1.0) * 1.6 + p.twist * 0.6
        out = []
        for i in range(n):
            t = i / (n - 1)
            drop = 10.5 * t
            back = -1.2 - 3.6 * t * t          # trails behind the shoulders
            # Locks must never cross the face, so each one keeps a minimum
            # distance from the centre line and only widens from there.
            spread = max(5.2, abs(side) * (5.4 + 2.2 * t))
            flare = math.copysign(spread, side) + swing * t * 1.6
            out.append(self.pt(flare + p.head_turn * (1 - t),
                               back + p.lean * 0.4 - p.head_nod * 0.4,
                               39.5 - drop - 11 * p.crouch + p.breathe * 0.4))
        return out

    def cape_chain(self, side: float, n: int = 5):
        """Cape hem, anchored at the pauldrons and billowing out behind."""
        p = self.pose
        billow = 1.0 + abs(math.sin(p.leg * 1.0)) * 1.4 + p.z * 0.25
        out = []
        for i in range(n):
            t = i / (n - 1)
            out.append(self.pt(side * (2.8 + 4.4 * t * t + 1.4 * t),
                               -2.0 - billow * (0.8 + 2.4 * t) + p.lean * 0.3,
                               29.5 - 24.0 * t - 9 * p.crouch))
        return out


@dataclass
class Blood:
    """Where to put blood on a blade, as fractions along its length."""

    spots: tuple = (0.22, 0.38, 0.55, 0.72, 0.88)
