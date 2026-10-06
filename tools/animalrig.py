"""Animal rig for Death World.

A deliberately small sibling of ``charrig``/``charparts``: the humanoid rig is
biped and layer-based, animals are one solid piece, so they get their own
skeleton rather than being bent through the player rig.

Everything else is shared on purpose - the same indexed ``Canvas``, the same
projection maths and the same 48x48 cell with the anchor at the centre of the
feet - so an animal sprite lines up with a character sprite pixel for pixel.
"""

from __future__ import annotations

import math
from dataclasses import dataclass

from charrig import ANCHOR_X, ANCHOR_Y, Canvas, Ramp  # noqa: F401
from charparts import ACCENT, FUR, LEATHER, OUTLINE, SKIN

# Two colour variants per species are built from the same ramps; the second
# variant shifts the body into the leather ramp, which reads as a darker,
# browner coat without needing extra palette slots.
COAT_A = FUR
COAT_B = LEATHER


@dataclass
class AnimalPose:
    """One frame, in animal-local units: forward is along the facing."""

    bob: float = 0.0        # body bounce, pixels
    z: float = 0.0          # airborne height
    stretch: float = 0.0    # +stretch / -squash along the body
    front: float = 0.0      # front leg pair swing, -1..1
    rear: float = 0.0       # rear leg pair swing, -1..1
    lift: float = 0.0       # foot lift during the swing
    head_down: float = 0.0  # 0 alert, 1 nose to the ground
    head_turn: float = 0.0  # left/right head rotation
    neck: float = 0.0       # extra neck extension
    tail: float = 0.0       # tail flick
    wing: float = 0.0       # 0 folded, 1 fully raised
    ear: float = 0.0        # ear swivel
    roll: float = 0.0       # body tilt, used by the death pose


class AnimalSkeleton:
    """Projects an :class:`AnimalPose` for one facing angle."""

    def __init__(self, pose: AnimalPose, angle: float, scale: float = 1.0):
        self.pose = pose
        self.a = angle
        self.scale = scale
        self.fx, self.fy = math.cos(angle), math.sin(angle)
        # Same minimum apparent depth cheat as the humanoid rig, otherwise a
        # side-on animal collapses to a one pixel sliver.
        side_axis = -self.fy
        if abs(side_axis) < 0.55:
            side_axis = 0.55 if side_axis >= 0 else -0.55
        self.rx, self.ry = side_axis, self.fx
        self.root_y = ANCHOR_Y - pose.z - pose.bob

    def pt(self, side: float, forward: float, height: float):
        s = self.scale
        side, forward, height = side * s, forward * s, height * s
        x = ANCHOR_X + self.rx * side + self.fx * forward
        y = (self.root_y + self.ry * side * 0.55 + self.fy * forward * 0.65 -
             height + self.pose.roll * side * 0.2)
        return (x, y)

    def depth(self, side: float) -> float:
        return self.ry * side * 0.55

    def forward_depth(self, forward: float) -> float:
        return self.fy * forward * 0.65


def _leg(c: Canvas, sk: AnimalSkeleton, side: float, forward: float,
         top: float, phase: float, ramp: Ramp, thick: float = 1.6) -> None:
    """One leg: hip to hoof, swinging along the facing direction."""
    p = sk.pose
    swing = phase * 3.2
    lift = max(0.0, p.lift * math.sin(max(0.0, phase) * math.pi))
    hip = sk.pt(side, forward, top)
    knee = sk.pt(side, forward + swing * 0.45, top * 0.5 + lift * 0.4)
    hoof = sk.pt(side, forward + swing, 0.6 + lift)
    z = sk.depth(side)
    c.capsule(hip, knee, thick, thick * 0.8, ramp, z)
    c.capsule(knee, hoof, thick * 0.8, thick * 0.6, ramp, z)


# ---------------------------------------------------------------------------
# Species
# ---------------------------------------------------------------------------

def deer(c: Canvas, sk: AnimalSkeleton, ramp: Ramp, antlers: bool) -> None:
    p = sk.pose
    body_len = 7.5 + p.stretch * 1.5
    rear = sk.pt(0, -body_len, 12)
    shoulder = sk.pt(0, body_len * 0.85, 13)
    # Far legs first so the near pair overlaps them.
    for side in (1.0, -1.0):
        _leg(c, sk, side * 2.6, body_len * 0.7, 12, p.front * side, ramp)
        _leg(c, sk, side * 2.8, -body_len * 0.75, 11, p.rear * side, ramp)
    c.capsule(rear, shoulder, 5.0, 4.4, ramp, 0.5)
    # Tail.
    tail = sk.pt(p.tail * 2.0, -body_len - 2.0, 13.5)
    c.capsule(rear, tail, 1.8, 1.2, ACCENT, 0.4)
    # Neck and head drop towards the grass as head_down rises.
    nh = 20.0 - p.head_down * 11.0 + p.neck
    nf = body_len + 1.5 + p.head_down * 3.0
    neck = sk.pt(0, nf, nh)
    c.capsule(shoulder, neck, 3.0, 2.3, ramp, 0.9)
    head = sk.pt(p.head_turn * 1.4, nf + 2.6, nh + 1.4 - p.head_down * 1.2)
    c.capsule(neck, head, 2.6, 2.0, ramp, 1.0)
    c.disc(head[0], head[1], 2.6, 2.2, ramp.base, 1.1)
    c.put(head[0] + sk.fx * 2, head[1] + sk.fy * 1.3, ramp.shadow, 1.3)
    # Ears, and antlers only on the buck variant.
    for side in (1.0, -1.0):
        e = sk.pt(side * 2.2 + p.ear * side, nf + 1.4, nh + 3.2)
        c.capsule(head, e, 1.3, 0.8, ramp, 1.05)
        if antlers:
            base = sk.pt(side * 1.4, nf + 1.6, nh + 3.0)
            tip = sk.pt(side * 3.4, nf + 3.4, nh + 8.0)
            mid = sk.pt(side * 2.6, nf + 2.4, nh + 5.6)
            fork = sk.pt(side * 4.6, nf + 1.0, nh + 6.4)
            c.capsule(base, tip, 1.1, 0.7, ACCENT, 1.2)
            c.capsule(mid, fork, 0.9, 0.6, ACCENT, 1.2)


def rabbit(c: Canvas, sk: AnimalSkeleton, ramp: Ramp) -> None:
    p = sk.pose
    # Squash and stretch is the whole point of the hop, so the body length and
    # height are driven directly from the pose.
    body_len = 4.0 + p.stretch * 2.2
    body_h = 6.0 - p.stretch * 1.4
    rear = sk.pt(0, -body_len, body_h)
    front = sk.pt(0, body_len * 0.8, body_h + 0.6)
    for side in (1.0, -1.0):
        _leg(c, sk, side * 2.0, body_len * 0.6, body_h - 1.0,
             p.front * side, ramp, 1.3)
        _leg(c, sk, side * 2.2, -body_len * 0.7, body_h - 1.2,
             p.rear * side, ramp, 1.5)
    c.capsule(rear, front, 4.4 - p.stretch * 0.6, 3.6, ramp, 0.5)
    tail = sk.pt(0, -body_len - 1.6, body_h + 1.0)
    c.disc(tail[0], tail[1], 1.6, 1.5, SKIN.highlight, 0.45)
    nh = body_h + 4.0 - p.head_down * 4.0
    head = sk.pt(p.head_turn, body_len + 1.6, nh)
    c.capsule(front, head, 2.8, 2.4, ramp, 0.9)
    c.disc(head[0], head[1], 2.6, 2.3, ramp.base, 1.0)
    c.put(head[0] + sk.fx * 2, head[1] + sk.fy * 1.2, ramp.shadow, 1.2)
    for side in (1.0, -1.0):
        tipx = side * (1.6 + p.ear * 1.4)
        ear = sk.pt(tipx, body_len + 1.0 - p.head_down * 1.5,
                    nh + 5.4 - p.head_down * 2.0)
        c.capsule(head, ear, 1.3, 0.9, ramp, 1.05)


def bird(c: Canvas, sk: AnimalSkeleton, ramp: Ramp) -> None:
    p = sk.pose
    lift = p.z
    body_h = 5.0
    rear = sk.pt(0, -3.0, body_h)
    front = sk.pt(0, 2.6, body_h + 0.8)
    if lift < 1.0:
        for side in (1.0, -1.0):
            _leg(c, sk, side * 1.3, 0.3, body_h - 1.5, p.front * side,
                 ACCENT, 0.9)
    # Wings: folded along the back at wing=0, swept up at wing=1.
    for side in (1.0, -1.0):
        tip = sk.pt(side * (2.0 + p.wing * 4.5), -1.0 - p.wing * 1.0,
                    body_h + 1.0 + p.wing * 5.5)
        root = sk.pt(side * 1.6, 0.4, body_h + 1.6)
        c.capsule(root, tip, 2.2, 1.0, ramp, sk.depth(side) + 0.6)
    c.capsule(rear, front, 3.6, 3.0, ramp, 0.5)
    tail = sk.pt(0, -5.4, body_h + 1.2 + p.tail)
    c.capsule(rear, tail, 2.0, 1.2, ACCENT, 0.4)
    nh = body_h + 3.6 - p.head_down * 5.0
    head = sk.pt(p.head_turn, 3.2 + p.head_down * 1.6, nh)
    c.capsule(front, head, 2.0, 1.8, ramp, 0.9)
    c.disc(head[0], head[1], 2.0, 1.9, ramp.base, 1.0)
    beak = sk.pt(p.head_turn, 5.6 + p.head_down * 2.0, nh - p.head_down * 1.0)
    c.capsule(head, beak, 1.2, 0.6, ACCENT, 1.1)
    c.put(head[0] + sk.fx * 1.4, head[1] + sk.fy * 0.9, OUTLINE, 1.3)


# name -> (painter, scale)
SPECIES = {
    "deer_doe":   (lambda c, sk: deer(c, sk, COAT_A, False), 1.0),
    "deer_buck":  (lambda c, sk: deer(c, sk, COAT_B, True), 1.08),
    "rabbit_grey": (lambda c, sk: rabbit(c, sk, COAT_A), 1.0),
    "rabbit_brown": (lambda c, sk: rabbit(c, sk, COAT_B), 0.94),
    "bird_drab":  (lambda c, sk: bird(c, sk, COAT_A), 1.0),
    "bird_blue":  (lambda c, sk: bird(c, sk, COAT_B), 0.96),
}
