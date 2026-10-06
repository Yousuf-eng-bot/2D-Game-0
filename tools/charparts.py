#!/usr/bin/env python3
"""Layer painters for the Death World character atlas.

Every painter draws ONE equipment layer for ONE frame into its own canvas.
Layers are composited at runtime, which is what makes equipment changes show
up immediately and keeps every combination registered on the same anchor.

Colours are written as *semantic palette indices*, never as RGB. The runtime
supplies the actual palette, so the same baked frames serve the player, a grey
bandit and a gold-trimmed elite - that is the enemy colour-coding feature.
"""
from __future__ import annotations

import math

from charrig import Canvas, Pose, Ramp, Skeleton

# --- palette slots ---------------------------------------------------------
OUTLINE = 1
SKIN = Ramp(OUTLINE, 2, 3, 4)
CLOTH = Ramp(OUTLINE, 5, 6, 7)
LEATHER = Ramp(OUTLINE, 8, 9, 10)
METAL = Ramp(OUTLINE, 11, 12, 13)
ACCENT = Ramp(OUTLINE, 14, 15, 16)
HAIR = Ramp(OUTLINE, 17, 18, 19)
FUR = Ramp(OUTLINE, 20, 21, 22)
PALETTE_SLOTS = 24


def _order(sk: Skeleton):
    """Return body sides ordered back-to-front for correct limb overlap."""
    return sorted((-1.0, 1.0), key=lambda s: sk.depth(s))


def _leg_phase(sk: Skeleton, side: float) -> float:
    return sk.pose.leg * (1.0 if side > 0 else -1.0)


# ---------------------------------------------------------------------------
# Base body: legs, torso, arms, hands. Always drawn.
# ---------------------------------------------------------------------------

def body(c: Canvas, sk: Skeleton, bare_arms: bool = True) -> None:
    p = sk.pose
    for side in _order(sk):
        z = sk.depth(side)
        ph = _leg_phase(sk, side)
        hip = sk.pt(side * 4, p.lean * 0.2, 14 - 5 * p.crouch)
        knee = sk.knee(side, ph)
        foot = sk.foot(side, ph)
        c.capsule(hip, knee, 2.1, 1.8, CLOTH, z)
        c.capsule(knee, foot, 1.8, 1.5, LEATHER, z)
        # boot
        c.disc(foot[0] + sk.fx * 1.0, foot[1], 2.4, 1.5, LEATHER.shadow, z + 0.2)
        c.put(foot[0] - 1, foot[1] - 1, LEATHER.highlight, z + 0.3)

    # torso
    sh_l, sh_r = sk.shoulder_side(-1), sk.shoulder_side(1)
    hip_l = sk.pt(-2.8, p.lean * 0.3, 15 - 5 * p.crouch)
    hip_r = sk.pt(2.8, p.lean * 0.3, 15 - 5 * p.crouch)
    c.quad([sh_l, sh_r, hip_r, hip_l], CLOTH.base, 0.4)
    # Light comes from the upper left, so the lit side follows the body axis
    # rather than the screen: on a front view that is the character's right,
    # on a back view it flips. Without this the torso reads as a flat card.
    lit, dim = (-1.0, 1.0) if sk.fy >= 0 else (1.0, -1.0)
    sh_lit = sk.shoulder_side(lit)
    sh_dim = sk.shoulder_side(dim)
    hip_lit = sk.pt(lit * 2.8, p.lean * 0.3, 15 - 5 * p.crouch)
    hip_dim = sk.pt(dim * 2.8, p.lean * 0.3, 15 - 5 * p.crouch)
    c.quad([sh_lit, sk.chest, hip_lit], CLOTH.highlight, 0.45)
    c.quad([sh_dim, sk.chest, hip_dim], CLOTH.shadow, 0.45)
    # Chest mass: a lit band across the pectorals and a shadow under the ribs,
    # which is what makes a front-facing torso look round at this size.
    if abs(sk.fy) > 0.2:
        chest_y = 27 - 8 * p.crouch + p.breathe
        band = sk.pt(0, p.lean * 0.7, chest_y)
        c.disc(band[0], band[1], 3.4, 1.6,
               CLOTH.highlight if sk.fy > 0 else CLOTH.shadow, 0.5)
        under = sk.pt(0, p.lean * 0.5, chest_y - 3.4)
        c.disc(under[0], under[1], 3.0, 1.2, CLOTH.shadow, 0.5)
    # Collar, and a waist shadow where the torso meets the hips.
    collar = sk.pt(0, p.lean, 32 - 10 * p.crouch + p.breathe)
    c.disc(collar[0], collar[1], 2.6, 1.1, CLOTH.shadow, 0.55)
    waist = sk.pt(0, p.lean * 0.3, 16 - 5 * p.crouch)
    c.disc(waist[0], waist[1], 3.0, 1.0, CLOTH.shadow, 0.5)

    # neck
    c.capsule(sk.chest, sk.neck, 1.8, 1.5, SKIN, 0.5)

    # arms
    ramp = SKIN if bare_arms else CLOTH
    for side in _order(sk):
        z = sk.depth(side) + 0.6
        if side > 0:
            hand = sk.hand(1, p.arm_main, p.arm_main_r)
        else:
            hand = sk.hand(-1, p.arm_off, p.arm_off_r)
        sh = sk.shoulder_side(side)
        elbow = ((sh[0] + hand[0]) / 2 + sk.rx * side * 1.5,
                 (sh[1] + hand[1]) / 2 + 1.0)
        c.capsule(sh, elbow, 1.8, 1.6, ramp, z)
        c.capsule(elbow, hand, 1.6, 1.3, ramp, z)
        c.disc(hand[0], hand[1], 1.5, 1.5, SKIN.base, z + 0.1)


# ---------------------------------------------------------------------------
# Head and hair.
# ---------------------------------------------------------------------------

def head(c: Canvas, sk: Skeleton) -> None:
    hx, hy = sk.head
    c.disc(hx, hy, 3.5, 3.8, SKIN.base, 1.0)
    c.disc(hx - 0.8, hy - 0.8, 2.0, 2.0, SKIN.highlight, 1.05)
    c.disc(hx + 1.4, hy + 0.8, 1.6, 1.8, SKIN.shadow, 1.02)
    # Eyes only when the face is toward the camera.
    if sk.fy > 0.25:
        ex = 1.3 * (1 if sk.fx >= 0 else -1)
        c.put(hx - ex, hy, OUTLINE, 1.2)
        c.put(hx + ex, hy, OUTLINE, 1.2)


def hair(c: Canvas, sk: Skeleton) -> None:
    hx, hy = sk.head
    c.disc(hx, hy - 2.3, 3.4, 1.8, HAIR.base, 1.1)
    c.disc(hx - 0.8, hy - 2.8, 2.2, 1.0, HAIR.highlight, 1.15)
    if sk.fy < 0.1:  # back of the head
        c.disc(hx, hy - 0.6, 3.2, 2.8, HAIR.base, 1.12)
    else:
        c.disc(hx - 2.7, hy - 0.4, 1.0, 1.7, HAIR.shadow, 1.12)
        c.disc(hx + 2.7, hy - 0.4, 1.0, 1.7, HAIR.shadow, 1.12)


def helmet_hood(c: Canvas, sk: Skeleton) -> None:
    hx, hy = sk.head
    c.disc(hx, hy - 1.1, 4.0, 3.6, CLOTH.base, 1.3)
    c.disc(hx - 0.9, hy - 2.0, 2.4, 1.6, CLOTH.highlight, 1.35)
    if sk.fy > 0.25:
        c.disc(hx, hy + 0.9, 2.4, 1.8, SKIN.shadow, 1.4)


def helmet_steel(c: Canvas, sk: Skeleton) -> None:
    hx, hy = sk.head
    c.disc(hx, hy - 1.2, 3.8, 3.4, METAL.base, 1.3)
    c.disc(hx - 1.1, hy - 2.1, 2.1, 1.3, METAL.highlight, 1.35)
    c.quad([(hx - 3.8, hy - 0.8), (hx + 3.8, hy - 0.8), (hx + 3.8, hy + 0.3),
            (hx - 3.8, hy + 0.3)], METAL.shadow, 1.36)
    if sk.fy > 0.25:
        c.quad([(hx - 0.6, hy - 3), (hx + 0.6, hy - 3), (hx + 0.6, hy + 1.6),
                (hx - 0.6, hy + 1.6)], METAL.highlight, 1.45)


# ---------------------------------------------------------------------------
# Coats / armour.
# ---------------------------------------------------------------------------

def _coat_shell(c: Canvas, sk: Skeleton, ramp: Ramp, length: float,
                sway: float) -> None:
    p = sk.pose
    sh_l, sh_r = sk.shoulder_side(-1.15), sk.shoulder_side(1.15)
    hem_l = sk.pt(-5.2 + sway, p.lean * 0.2, 15 - length - 5 * p.crouch)
    hem_r = sk.pt(5.2 + sway, p.lean * 0.2, 15 - length - 5 * p.crouch)
    c.quad([sh_l, sh_r, hem_r, hem_l], ramp.base, 0.7)
    mid_top = ((sh_l[0] + sh_r[0]) / 2, (sh_l[1] + sh_r[1]) / 2)
    mid_bot = ((hem_l[0] + hem_r[0]) / 2, (hem_l[1] + hem_r[1]) / 2)
    c.quad([sh_l, mid_top, mid_bot, hem_l], ramp.highlight, 0.72)
    c.quad([mid_top, sh_r, hem_r, mid_bot], ramp.shadow, 0.72)
    # shoulder caps give the silhouette its readable shape
    c.disc(sh_l[0], sh_l[1], 2.2, 1.8, ramp.highlight, 0.8)
    c.disc(sh_r[0], sh_r[1], 2.2, 1.8, ramp.shadow, 0.8)
    if sk.fy > 0.2:
        for t in range(0, 7):
            f = t / 6.0
            c.put(mid_top[0] + (mid_bot[0] - mid_top[0]) * f,
                  mid_top[1] + (mid_bot[1] - mid_top[1]) * f, ramp.shadow, 0.76)


def coat_ranger(c: Canvas, sk: Skeleton) -> None:
    sway = math.sin(sk.pose.leg * 1.4) * 1.6
    _coat_shell(c, sk, CLOTH, 9.0, sway)
    # collar
    n = sk.neck
    c.quad([(n[0] - 4, n[1] + 1), (n[0] + 4, n[1] + 1), (n[0] + 3, n[1] + 3),
            (n[0] - 3, n[1] + 3)], CLOTH.highlight, 0.9)


def coat_mail(c: Canvas, sk: Skeleton) -> None:
    sway = math.sin(sk.pose.leg * 1.4) * 1.1
    _coat_shell(c, sk, METAL, 7.5, sway)
    # Mail texture. A full checkerboard reads as noise at 48 px, so the rings
    # are drawn as offset horizontal courses instead: every other row, every
    # other pixel, which still says "mail" but keeps the silhouette clean.
    ch = sk.chest
    for j in range(-6, 7, 2):
        for i in range(-5, 6, 2):
            c.put(ch[0] + i + (j // 2) % 2, ch[1] + j - 2, METAL.shadow, 0.78)
    # A lit course across the top of the chest so the plate still turns.
    for i in range(-4, 5, 2):
        c.put(ch[0] + i, ch[1] - 5, METAL.highlight, 0.79)
    sh_l, sh_r = sk.shoulder_side(-1.35), sk.shoulder_side(1.35)
    c.disc(sh_l[0], sh_l[1], 2.4, 1.9, ACCENT.base, 0.85)
    c.disc(sh_r[0], sh_r[1], 2.4, 1.9, ACCENT.shadow, 0.85)


def coat_bandit(c: Canvas, sk: Skeleton) -> None:
    """Hunched, padded-shoulder silhouette: reads as an enemy from a distance
    even before colour is considered."""
    sway = math.sin(sk.pose.leg * 1.4) * 1.3
    _coat_shell(c, sk, CLOTH, 6.5, sway)
    sh_l, sh_r = sk.shoulder_side(-1.6), sk.shoulder_side(1.6)
    c.disc(sh_l[0], sh_l[1] - 0.8, 3.0, 2.1, LEATHER.base, 0.86)
    c.disc(sh_r[0], sh_r[1] - 0.8, 3.0, 2.1, LEATHER.shadow, 0.86)
    c.disc(sh_l[0], sh_l[1] - 1.6, 1.6, 1.0, LEATHER.highlight, 0.88)


def coat_elite(c: Canvas, sk: Skeleton) -> None:
    coat_bandit(c, sk)
    ch = sk.chest
    c.quad([(ch[0] - 5, ch[1] - 4), (ch[0] + 5, ch[1] - 4),
            (ch[0] + 4, ch[1] - 2), (ch[0] - 4, ch[1] - 2)], ACCENT.highlight, 0.9)
    hip = sk.hip
    c.quad([(hip[0] - 6, hip[1] - 1), (hip[0] + 6, hip[1] - 1),
            (hip[0] + 6, hip[1] + 1), (hip[0] - 6, hip[1] + 1)], ACCENT.base, 0.9)


# ---------------------------------------------------------------------------
# Small kit: belt with buckle, scabbard, backpack.
# ---------------------------------------------------------------------------

def belt(c: Canvas, sk: Skeleton) -> None:
    hip = sk.hip
    c.quad([(hip[0] - 4.6, hip[1] - 1), (hip[0] + 4.6, hip[1] - 1),
            (hip[0] + 4.6, hip[1] + 1.2), (hip[0] - 4.6, hip[1] + 1.2)],
           LEATHER.base, 1.0)
    c.put(hip[0], hip[1], ACCENT.highlight, 1.1)
    c.put(hip[0] + 1, hip[1], ACCENT.base, 1.1)
    # scabbard hanging on the off side
    tip = sk.pt(-6, -5, 6 - 4 * sk.pose.crouch)
    top = sk.pt(-5.5, 1, 14 - 4 * sk.pose.crouch)
    c.capsule(top, tip, 1.3, 1.0, LEATHER, sk.depth(-1) + 0.3)
    c.disc(top[0], top[1], 1.4, 1.4, ACCENT.base, sk.depth(-1) + 0.35)


def pack(c: Canvas, sk: Skeleton) -> None:
    """Backpack sits behind the torso when facing the camera, in front of it
    when walking away - the depth value handles that automatically."""
    p = sk.pose
    back = sk.pt(0, -5.0, 24 - 8 * p.crouch + p.breathe)
    z = -sk.fy * 3.0 - 0.5
    c.disc(back[0], back[1], 3.4, 4.0, LEATHER.base, z)
    c.disc(back[0] - 0.8, back[1] - 1.2, 2.0, 2.2, LEATHER.highlight, z + 0.05)
    c.quad([(back[0] - 3.4, back[1] + 0.6), (back[0] + 3.4, back[1] + 0.6),
            (back[0] + 3.4, back[1] + 1.6), (back[0] - 3.4, back[1] + 1.6)],
           ACCENT.shadow, z + 0.1)
    bedroll = sk.pt(0, -5.6, 29 - 9 * p.crouch)
    c.disc(bedroll[0], bedroll[1], 3.8, 1.3, CLOTH.base, z + 0.02)


# ---------------------------------------------------------------------------
# Weapons, drawn in the main hand.
# ---------------------------------------------------------------------------

def _hand_frame(sk: Skeleton):
    p = sk.pose
    hand = sk.hand(1, p.arm_main, p.arm_main_r)
    a = p.weapon_angle
    return hand, math.cos(a), math.sin(a) * 0.8


def weapon_sword(c: Canvas, sk: Skeleton, blade: int = 13) -> None:
    (hx, hy), dx, dy = _hand_frame(sk)
    z = sk.depth(1) + 1.5
    tip = (hx + dx * blade, hy + dy * blade)
    guard_a = (hx - dy * 3.2, hy + dx * 3.2)
    guard_b = (hx + dy * 3.2, hy - dx * 3.2)
    c.capsule((hx - dx * 3, hy - dy * 3), (hx - dx * 0.5, hy - dy * 0.5),
              1.5, 1.5, LEATHER, z)
    c.capsule(guard_a, guard_b, 1.2, 1.2, ACCENT, z + 0.05)
    c.capsule((hx + dx * 1.5, hy + dy * 1.5), tip, 1.8, 0.8, METAL, z + 0.1)
    c.put(hx + dx * 5 - dy, hy + dy * 5 + dx, METAL.highlight, z + 0.2)
    c.put(hx + dx * 8 - dy, hy + dy * 8 + dx, METAL.highlight, z + 0.2)


def weapon_dawnblade(c: Canvas, sk: Skeleton) -> None:
    weapon_sword(c, sk, 15)
    (hx, hy), dx, dy = _hand_frame(sk)
    z = sk.depth(1) + 1.7
    for k in (6, 10, 14):
        c.put(hx + dx * k, hy + dy * k, ACCENT.highlight, z)


def weapon_axe(c: Canvas, sk: Skeleton, rusty: bool = False) -> None:
    (hx, hy), dx, dy = _hand_frame(sk)
    z = sk.depth(1) + 1.5
    head_pt = (hx + dx * 11, hy + dy * 11)
    c.capsule((hx - dx * 4, hy - dy * 4), head_pt, 1.6, 1.4, LEATHER, z)
    ramp = LEATHER if rusty else METAL
    c.quad([(head_pt[0] - dy * 5, head_pt[1] + dx * 5),
            (head_pt[0] + dx * 4 - dy * 4, head_pt[1] + dy * 4 + dx * 4),
            (head_pt[0] + dx * 4 + dy * 1, head_pt[1] + dy * 4 - dx * 1),
            (head_pt[0] + dy * 1, head_pt[1] - dx * 1)], ramp.base, z + 0.1)
    c.quad([(head_pt[0] - dy * 5, head_pt[1] + dx * 5),
            (head_pt[0] - dy * 4 + dx * 2, head_pt[1] + dx * 4 + dy * 2),
            (head_pt[0] - dy * 2, head_pt[1] + dx * 2)], ramp.highlight, z + 0.15)


def weapon_bow(c: Canvas, sk: Skeleton) -> None:
    (hx, hy), dx, dy = _hand_frame(sk)
    z = sk.depth(1) + 1.5
    for t in range(-9, 10):
        f = t / 9.0
        bend = (1 - f * f) * 3.0
        c.put(hx - dy * t + dx * bend, hy + dx * t + dy * bend, LEATHER.base, z)
    c.capsule((hx - dy * 9, hy + dx * 9), (hx + dy * 9, hy - dx * 9),
              0.6, 0.6, ACCENT, z + 0.05)


def weapon_pick(c: Canvas, sk: Skeleton) -> None:
    (hx, hy), dx, dy = _hand_frame(sk)
    z = sk.depth(1) + 1.5
    head_pt = (hx + dx * 10, hy + dy * 10)
    c.capsule((hx - dx * 4, hy - dy * 4), head_pt, 1.6, 1.4, LEATHER, z)
    c.capsule((head_pt[0] - dy * 6, head_pt[1] + dx * 6),
              (head_pt[0] + dy * 6, head_pt[1] - dx * 6), 1.3, 1.3, METAL, z + 0.1)


def weapon_none(c: Canvas, sk: Skeleton) -> None:
    return


# ---------------------------------------------------------------------------
# Accessories. These hang off the belt, so they ride the hip joint and are
# drawn after the belt but before the head.
# ---------------------------------------------------------------------------

def accessory_talisman(c: Canvas, sk: Skeleton) -> None:
    """Faded Talisman: a plain bone charm on a cord at the off hip."""
    p = sk.pose
    top = sk.pt(4.6, 0.6, 15 - 5 * p.crouch)
    low = sk.pt(5.2, 0.6, 11 - 4 * p.crouch)
    z = sk.depth(1) + 0.4
    c.capsule(top, low, 0.8, 0.7, LEATHER, z)
    c.disc(low[0], low[1], 1.6, 1.8, SKIN.highlight, z + 0.1)
    c.put(low[0], low[1], LEATHER.shadow, z + 0.2)


def accessory_cinder(c: Canvas, sk: Skeleton) -> None:
    """Cinder Heart: the same charm, but a glowing ember in a metal cage."""
    p = sk.pose
    top = sk.pt(4.6, 0.6, 15 - 5 * p.crouch)
    low = sk.pt(5.2, 0.6, 10.6 - 4 * p.crouch)
    z = sk.depth(1) + 0.4
    c.capsule(top, low, 0.9, 0.8, METAL, z)
    c.disc(low[0], low[1], 2.0, 2.2, METAL.shadow, z + 0.1)
    c.disc(low[0], low[1], 1.2, 1.4, ACCENT.highlight, z + 0.2)
    c.put(low[0], low[1] - 1, ACCENT.base, z + 0.3)


def weapon_sunsteel(c: Canvas, sk: Skeleton) -> None:
    """Sunsteel Core: a longer, gold-cored blade for the rare weapon slot."""
    hand, dx, dy = _hand_frame(sk)
    z = sk.depth(1) + 1.0
    grip_end = (hand[0] - dx * 3.4, hand[1] - dy * 3.4)
    c.capsule(grip_end, hand, 1.1, 1.0, LEATHER, z)
    guard_a = (hand[0] + dy * 3.6, hand[1] - dx * 3.0)
    guard_b = (hand[0] - dy * 3.6, hand[1] + dx * 3.0)
    c.capsule(guard_a, guard_b, 1.0, 1.0, ACCENT, z + 0.1)
    tip = (hand[0] + dx * 17.0, hand[1] + dy * 17.0)
    mid = (hand[0] + dx * 8.0, hand[1] + dy * 8.0)
    c.capsule(mid, tip, 1.9, 0.7, METAL, z + 0.2)
    c.capsule(hand, mid, 2.1, 1.9, METAL, z + 0.2)
    # Gold fuller running the length of the blade.
    c.capsule((hand[0] + dx * 3.0, hand[1] + dy * 3.0),
              (hand[0] + dx * 14.0, hand[1] + dy * 14.0),
              0.7, 0.5, ACCENT, z + 0.3)
    c.disc(grip_end[0], grip_end[1], 1.4, 1.4, ACCENT.base, z + 0.1)
