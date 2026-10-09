"""Layer painters for the high-detail hero.

Each function paints one equipment layer into an indexed :class:`Canvas` using
:class:`herorig.HeroSkeleton`. Layers are baked separately and composited at
runtime, so changing a piece of gear changes exactly one layer.

Palette ramps are shared with the small rig (`charparts`), with one addition:
BLOOD, used for the weapon gore the owner asked for.
"""

from __future__ import annotations

import math

from charrig import Canvas
from charparts import (ACCENT, BLOOD, CLOTH, FUR, HAIR, LEATHER, METAL,
                       OUTLINE, SKIN)
from herorig import SCALE, HeroSkeleton

S = SCALE


def _order(sk: HeroSkeleton):
    """Far side first, so the near limb overlaps it."""
    return (1.0, -1.0) if sk.ry > 0 else (-1.0, 1.0)


def _leg_phase(sk: HeroSkeleton, side: float) -> float:
    return sk.pose.leg * side


def _r(v: float) -> float:
    return v * S


# ---------------------------------------------------------------------------
# Body: armoured legs, plated boots, bare-ish arms, gauntlets.
# ---------------------------------------------------------------------------

def body(c: Canvas, sk: HeroSkeleton, bare_arms: bool = True) -> None:
    p = sk.pose
    for side in _order(sk):
        z = sk.depth(side)
        ph = _leg_phase(sk, side)
        hip = sk.pt(side * 3.2, p.lean * 0.2, 14 - 5 * p.crouch)
        knee = sk.knee(side, ph)
        foot = sk.foot(side, ph)
        # Thigh: cloth over a plated shin.
        c.capsule(hip, knee, _r(2.4), _r(2.0), CLOTH, z)
        c.capsule(knee, foot, _r(2.1), _r(1.7), LEATHER, z)
        # Knee cop and shin plate - small metal reads as armour at this size.
        c.disc(knee[0], knee[1], _r(1.7), _r(1.5), METAL.base, z + 0.25)
        c.put(knee[0], knee[1] - 1, METAL.highlight, z + 0.3)
        shin = ((knee[0] + foot[0]) / 2, (knee[1] + foot[1]) / 2)
        c.capsule(shin, foot, _r(1.5), _r(1.2), METAL, z + 0.2)
        # Sabaton.
        c.disc(foot[0] + sk.fx * _r(1.2), foot[1], _r(2.8), _r(1.8),
               METAL.shadow, z + 0.3)
        c.put(foot[0] - 1, foot[1] - 1, METAL.highlight, z + 0.35)
        c.put(foot[0] + 1, foot[1], METAL.base, z + 0.35)

    # Torso underlayer. The armour layer covers most of it, but it has to
    # read on its own for the unarmoured look.
    sh_l, sh_r = sk.shoulder_side(-1), sk.shoulder_side(1)
    hip_l = sk.pt(-3.0, p.lean * 0.3, 15 - 5 * p.crouch)
    hip_r = sk.pt(3.0, p.lean * 0.3, 15 - 5 * p.crouch)
    c.quad([sh_l, sh_r, hip_r, hip_l], CLOTH.base, 0.4)
    lit, dim = (-1.0, 1.0) if sk.fy >= 0 else (1.0, -1.0)
    c.quad([sk.shoulder_side(lit), sk.chest,
            sk.pt(lit * 3.0, p.lean * 0.3, 15 - 5 * p.crouch)],
           CLOTH.highlight, 0.45)
    c.quad([sk.shoulder_side(dim), sk.chest,
            sk.pt(dim * 3.0, p.lean * 0.3, 15 - 5 * p.crouch)],
           CLOTH.shadow, 0.45)
    c.capsule(sk.chest, sk.neck, _r(2.0), _r(1.7), SKIN, 0.5)

    # Arms.
    ramp = SKIN if bare_arms else CLOTH
    for side in _order(sk):
        z = sk.depth(side) + 1.6
        hand = (sk.hand(1, p.arm_main, p.arm_main_r) if side > 0
                else sk.hand(-1, p.arm_off, p.arm_off_r))
        sh = sk.shoulder_side(side)
        elbow = ((sh[0] + hand[0]) / 2 + sk.rx * side * _r(1.5),
                 (sh[1] + hand[1]) / 2 + _r(1.0))
        c.capsule(sh, elbow, _r(2.1), _r(1.8), ramp, z)
        c.capsule(elbow, hand, _r(1.8), _r(1.5), ramp, z)
        # Vambrace and gauntlet.
        mid = ((elbow[0] + hand[0]) / 2, (elbow[1] + hand[1]) / 2)
        c.capsule(mid, hand, _r(1.7), _r(1.6), METAL, z + 0.1)
        c.disc(hand[0], hand[1], _r(1.8), _r(1.7), METAL.base, z + 0.2)
        c.put(hand[0], hand[1] - 1, METAL.highlight, z + 0.25)


# ---------------------------------------------------------------------------
# Head, long hair, headgear.
# ---------------------------------------------------------------------------

def head(c: Canvas, sk: HeroSkeleton) -> None:
    hx, hy = sk.head
    c.disc(hx, hy, _r(3.9), _r(4.2), SKIN.base, 1.0)
    c.disc(hx - _r(1.0), hy - _r(1.0), _r(2.3), _r(2.3), SKIN.highlight, 1.05)
    c.disc(hx + _r(1.5), hy + _r(0.9), _r(1.7), _r(1.9), SKIN.shadow, 1.02)
    # Jaw and cheekbone, which a 48 px head had no room for.
    jaw = sk.pt(sk.pose.head_turn * 1.2, sk.pose.lean + sk.pose.head_nod,
                38.6 - 12 * sk.pose.crouch)
    c.disc(jaw[0], jaw[1], _r(2.4), _r(1.6), SKIN.shadow, 0.99)
    if sk.fy > 0.25:
        ex = _r(1.7) * (1 if sk.fx >= 0 else -1)
        for dx in (-ex, ex):
            c.put(hx + dx, hy, OUTLINE, 1.2)
            c.put(hx + dx - 1, hy, OUTLINE, 1.2)
            c.put(hx + dx, hy - 1, SKIN.highlight, 1.21)
        # Nose and mouth line.
        c.put(hx, hy + 1, SKIN.shadow, 1.21)
        c.put(hx, hy + 2, SKIN.shadow, 1.21)
        c.put(hx - 1, hy + _r(2.2), SKIN.shadow, 1.21)
        c.put(hx, hy + _r(2.2), SKIN.shadow, 1.21)
        # Brow line gives the face a hard, grim read.
        for i in range(-2, 3):
            c.put(hx + i, hy - _r(1.7), SKIN.shadow, 1.22)


def hair(c: Canvas, sk: HeroSkeleton) -> None:
    """Long hair: a skull cap plus four trailing locks that move with the body.

    The owner asked specifically for long hair, so this is the layer that
    carries most of the hero's silhouette. Locks are drawn with an outline
    gap between them, otherwise they merge into one brown blob.
    """
    hx, hy = sk.head
    # Cap, sitting slightly back off the brow so the face stays readable.
    cap = sk.pt(0, sk.pose.lean - 0.7, 44.3 - 12 * sk.pose.crouch +
                sk.pose.breathe)
    c.disc(cap[0], cap[1], _r(4.0), _r(2.5), HAIR.base, 1.1)
    c.disc(cap[0] - _r(1.2), cap[1] - _r(0.8), _r(2.3), _r(1.3),
           HAIR.highlight, 1.15)
    # Side-burn wisps framing the face, kept clear of the eyes.
    for side in (-1.0, 1.0):
        a = sk.pt(side * 4.3, sk.pose.lean - 0.6,
                  43.6 - 12 * sk.pose.crouch)
        b = sk.pt(side * 4.8, sk.pose.lean - 1.2,
                  38.0 - 12 * sk.pose.crouch)
        c.capsule(a, b, _r(1.2), _r(0.9), HAIR, 0.6 if sk.fy > 0 else 1.2)
    if sk.fy < 0.3:
        c.disc(hx, hy, _r(4.0), _r(4.0), HAIR.base, 1.12)
    # Locks. Outer pair is long and wide, inner pair shorter, and each is
    # separated by a dark gap so the hair reads as strands, not a helmet.
    specs = ((-1.0, 2.6, 1.0), (1.0, 2.6, 1.0),
             (-0.5, 1.8, 0.78), (0.5, 1.8, 0.78))
    for side, width, length in specs:
        chain = sk.hair_chain(side)
        if length < 1.0:
            chain = chain[:4]
        # In front of the head when seen from behind, behind the
        # face when seen from the front - otherwise the hair hides the face.
        z = sk.depth(side) + (1.25 if sk.fy < 0 else 0.55)
        for i in range(len(chain) - 1):
            t = i / max(1, len(chain) - 2)
            w0 = _r(width * (1.0 - 0.42 * t))
            w1 = _r(width * (1.0 - 0.42 * min(1.0, t + 0.25)))
            c.capsule(chain[i], chain[i + 1], w0, w1, HAIR, z + i * 0.01)
        # Dark gap on the inboard edge separates this lock from the next.
        for i in range(len(chain)):
            c.put(chain[i][0] - side * _r(width * 0.7), chain[i][1],
                  OUTLINE, z + 0.25)
        # Lit strand catching the light down the outboard edge.
        for i in range(0, len(chain), 2):
            c.put(chain[i][0] + side * _r(width * 0.45), chain[i][1],
                  HAIR.highlight, z + 0.3)
    # Tail tips, so the ends of the hair are pointed rather than cut off.
    for side in (-1.0, 1.0):
        chain = sk.hair_chain(side)
        tip = chain[-1]
        c.put(tip[0], tip[1] + 1, HAIR.shadow, sk.depth(side) + 1.1)


def helmet_crown(c: Canvas, sk: HeroSkeleton) -> None:
    """Horned circlet, after the reference screenshots.

    Deliberately a circlet rather than a full helm: the owner asked for long
    hair to stay visible, and a closed helm would swallow both the hair and
    the face.
    """
    p = sk.pose
    band_l = sk.pt(-3.8, p.lean - 0.3, 44.2 - 12 * p.crouch)
    band_r = sk.pt(3.8, p.lean - 0.3, 44.2 - 12 * p.crouch)
    c.capsule(band_l, band_r, _r(1.1), _r(1.1), METAL, 1.3)
    # Brow jewel.
    brow = sk.pt(0, p.lean + 0.6, 44.0 - 12 * p.crouch)
    c.disc(brow[0], brow[1], _r(1.3), _r(1.1), ACCENT.base, 1.34)
    c.put(brow[0], brow[1] - 1, ACCENT.highlight, 1.36)
    # Crown spikes rising from the band.
    for k, side in enumerate((-2.6, -1.1, 0.4, 1.9)):
        base = sk.pt(side, p.lean - 0.3, 44.6 - 12 * p.crouch)
        tip = sk.pt(side * 1.2, p.lean - 0.8,
                    48.4 + (1.4 if k % 2 else 0.0) - 12 * p.crouch)
        c.capsule(base, tip, _r(0.8), _r(0.3), METAL, 1.32)
    # Curling horns sweeping back and out.
    for side in (-1.0, 1.0):
        a = sk.pt(side * 3.6, -0.6, 43.6 - 12 * p.crouch)
        b = sk.pt(side * 6.2, -2.8, 46.2 - 12 * p.crouch)
        d = sk.pt(side * 7.6, -5.8, 42.6 - 12 * p.crouch)
        c.capsule(a, b, _r(1.4), _r(1.0), ACCENT, 1.28)
        c.capsule(b, d, _r(1.0), _r(0.45), ACCENT, 1.27)


def helmet_hood(c: Canvas, sk: HeroSkeleton) -> None:
    hx, hy = sk.head
    c.disc(hx, hy - _r(0.6), _r(4.2), _r(4.0), CLOTH.base, 1.25)
    c.disc(hx - _r(1.2), hy - _r(1.6), _r(2.2), _r(1.8), CLOTH.highlight, 1.3)
    if sk.fy > 0.2:
        c.disc(hx, hy + _r(0.8), _r(2.6), _r(2.2), OUTLINE, 1.35)
        c.disc(hx, hy + _r(1.0), _r(2.0), _r(1.6), SKIN.shadow, 1.36)
    peak = sk.pt(0, -1.6, 45.5 - 12 * sk.pose.crouch)
    c.capsule(sk.head, peak, _r(2.4), _r(0.9), CLOTH, 1.2)


# ---------------------------------------------------------------------------
# Armour.
# ---------------------------------------------------------------------------

def _cuirass(c: Canvas, sk: HeroSkeleton, ramp, trim, length: float,
             filigree: bool) -> None:
    p = sk.pose
    sway = math.sin(p.leg * 1.4) * 1.0
    sh_l, sh_r = sk.shoulder_side(-1.12), sk.shoulder_side(1.12)
    hem_l = sk.pt(-3.1 + sway * 0.4, p.lean * 0.2 - sway,
                  15 - length - 5 * p.crouch)
    hem_r = sk.pt(3.1 + sway * 0.4, p.lean * 0.2 - sway,
                  15 - length - 5 * p.crouch)
    c.quad([sh_l, sh_r, hem_r, hem_l], ramp.base, 0.7)
    # Form: lit chest band, rib shadow, waist shadow.
    lit, dim = (-1.0, 1.0) if sk.fy >= 0 else (1.0, -1.0)
    c.quad([sk.shoulder_side(lit * 1.12), sk.chest,
            sk.pt(lit * 3.1, p.lean * 0.2, 15 - length - 5 * p.crouch)],
           ramp.highlight, 0.72)
    c.quad([sk.shoulder_side(dim * 1.12), sk.chest,
            sk.pt(dim * 3.1, p.lean * 0.2, 15 - length - 5 * p.crouch)],
           ramp.shadow, 0.72)
    if abs(sk.fy) > 0.2:
        band = sk.pt(0, p.lean * 0.7, 27 - 8 * p.crouch + p.breathe)
        c.disc(band[0], band[1], _r(4.0), _r(1.9),
               ramp.highlight if sk.fy > 0 else ramp.shadow, 0.75)
        ribs = sk.pt(0, p.lean * 0.5, 23 - 8 * p.crouch)
        c.disc(ribs[0], ribs[1], _r(3.6), _r(1.4), ramp.shadow, 0.75)
    collar = sk.pt(0, p.lean, 32.5 - 10 * p.crouch + p.breathe)
    c.disc(collar[0], collar[1], _r(3.0), _r(1.3), ramp.shadow, 0.78)
    if not filigree:
        return
    # Gold filigree: a cross over the chest and a trimmed hem, which is the
    # single most recognisable thing in the reference art.
    top = sk.pt(0, p.lean * 0.8, 30 - 9 * p.crouch + p.breathe)
    bot = sk.pt(0, p.lean * 0.3, 17 - 5 * p.crouch)
    c.capsule(top, bot, _r(0.9), _r(0.7), trim, 0.82)
    arm_l = sk.pt(-2.8, p.lean * 0.7, 25.5 - 8 * p.crouch)
    arm_r = sk.pt(2.8, p.lean * 0.7, 25.5 - 8 * p.crouch)
    c.capsule(arm_l, arm_r, _r(0.8), _r(0.8), trim, 0.82)
    c.disc(sk.chest[0], sk.chest[1] - _r(0.5), _r(1.5), _r(1.5),
           trim.highlight, 0.86)
    c.put(sk.chest[0], sk.chest[1] - _r(0.5), ramp.shadow, 0.88)
    c.capsule(hem_l, hem_r, _r(0.8), _r(0.8), trim, 0.84)
    c.capsule(sk.shoulder_side(-1.12), sk.shoulder_side(1.12),
              _r(0.7), _r(0.7), trim, 0.84)


def armor_ranger(c: Canvas, sk: HeroSkeleton) -> None:
    """Starting kit: a long leather coat, bronze trim, no plate."""
    _cuirass(c, sk, LEATHER, ACCENT, 6.0, False)
    # Storm collar and a few rivets.
    collar = sk.pt(0, sk.pose.lean, 33 - 10 * sk.pose.crouch)
    c.disc(collar[0], collar[1], _r(3.4), _r(1.6), LEATHER.highlight, 0.8)
    for side in (-2.0, 0.0, 2.0):
        r = sk.pt(side, sk.pose.lean * 0.6, 22 - 7 * sk.pose.crouch)
        c.put(r[0], r[1], ACCENT.base, 0.85)


def armor_plate(c: Canvas, sk: HeroSkeleton) -> None:
    """Endgame kit: dark plate with gold filigree and a fauld of tassets."""
    _cuirass(c, sk, METAL, ACCENT, 6.5, True)
    p = sk.pose
    sway = math.sin(p.leg * 1.4) * 1.0
    for side in (-2.6, 0.0, 2.6):
        a = sk.pt(side, p.lean * 0.2, 14 - 5 * p.crouch)
        b = sk.pt(side * 1.15, p.lean * 0.2 - sway, 9.0 - 4 * p.crouch)
        c.capsule(a, b, _r(1.9), _r(1.6), METAL, 0.6)
        c.put(b[0], b[1], ACCENT.base, 0.65)


def pauldrons(c: Canvas, sk: HeroSkeleton) -> None:
    """Spiked shoulder plates. These dominate the silhouette, as asked."""
    p = sk.pose
    for side in _order(sk):
        z = sk.depth(side) + 0.9
        sh = sk.shoulder_side(side * 1.45)
        c.disc(sh[0], sh[1] - _r(0.4), _r(3.5), _r(2.8), METAL.base, z)
        c.disc(sh[0] - _r(0.8), sh[1] - _r(1.4), _r(2.0), _r(1.3),
               METAL.highlight, z + 0.05)
        c.disc(sh[0] + _r(1.0), sh[1] + _r(0.9), _r(1.8), _r(1.2),
               METAL.shadow, z + 0.05)
        # Gold rim.
        rim_a = sk.pt(side * 2.0, p.lean, 28 - 10 * p.crouch)
        rim_b = sk.pt(side * 7.4, p.lean, 29.5 - 10 * p.crouch)
        c.capsule(rim_a, rim_b, _r(0.8), _r(0.8), ACCENT, z + 0.1)
        # Two spikes per shoulder, swept up and back.
        for k, (out, up) in enumerate(((5.0, 34.5), (7.0, 32.0))):
            base = sk.pt(side * out, p.lean - 0.4, up - 10 * p.crouch)
            tip = sk.pt(side * (out + 1.8), p.lean - 2.6 - k,
                        up + 5.5 - k * 1.2 - 10 * p.crouch)
            c.capsule(base, tip, _r(1.2), _r(0.35), METAL, z + 0.15)
            c.put(tip[0], tip[1], METAL.highlight, z + 0.2)


def cape(c: Canvas, sk: HeroSkeleton) -> None:
    """A long cape that billows with the stride and with jump height.

    Drawn as vertical panels rather than one quad, so it has folds and does
    not read as a flat slab from behind.
    """
    z = -sk.fy * 3.4 - 0.6
    panels = (-1.0, -0.5, 0.0, 0.5, 1.0)
    chains = [sk.cape_chain(p) for p in panels]
    for k in range(len(panels) - 1):
        a, b = chains[k], chains[k + 1]
        # Alternate the panel shade to make folds.
        ramp = (CLOTH.base, CLOTH.shadow, CLOTH.base, CLOTH.highlight)[k]
        for i in range(len(a) - 1):
            c.quad([a[i], b[i], b[i + 1], a[i + 1]], ramp, z + i * 0.02)
    # Fold lines between the panels.
    for ch in chains[1:-1]:
        for i in range(len(ch) - 1):
            c.capsule(ch[i], ch[i + 1], _r(0.4), _r(0.4), CLOTH, z + 0.1)
    # Gold hem along the bottom edge and down both sides.
    hem = [ch[-1] for ch in chains]
    for i in range(len(hem) - 1):
        c.capsule(hem[i], hem[i + 1], _r(0.7), _r(0.7), ACCENT, z + 0.2)
    for ch in (chains[0], chains[-1]):
        for i in range(len(ch) - 1):
            c.capsule(ch[i], ch[i + 1], _r(0.5), _r(0.5), ACCENT, z + 0.18)


def belt(c: Canvas, sk: HeroSkeleton) -> None:
    hip = sk.hip
    c.quad([(hip[0] - _r(5.0), hip[1] - _r(1.1)),
            (hip[0] + _r(5.0), hip[1] - _r(1.1)),
            (hip[0] + _r(5.0), hip[1] + _r(1.4)),
            (hip[0] - _r(5.0), hip[1] + _r(1.4))], LEATHER.base, 1.0)
    # Buckle.
    c.disc(hip[0], hip[1], _r(1.6), _r(1.4), ACCENT.base, 1.1)
    c.put(hip[0], hip[1], ACCENT.highlight, 1.15)
    c.put(hip[0] + 1, hip[1] + 1, LEATHER.shadow, 1.15)
    # Scabbard on the off side.
    tip = sk.pt(-6.4, -5.5, 5 - 4 * sk.pose.crouch)
    top = sk.pt(-5.8, 1.0, 14 - 4 * sk.pose.crouch)
    z = sk.depth(-1) + 0.3
    c.capsule(top, tip, _r(1.5), _r(1.1), LEATHER, z)
    c.disc(top[0], top[1], _r(1.6), _r(1.6), ACCENT.base, z + 0.05)
    c.disc(tip[0], tip[1], _r(1.2), _r(1.2), METAL.base, z + 0.05)


def pack(c: Canvas, sk: HeroSkeleton) -> None:
    p = sk.pose
    back = sk.pt(0, -5.4, 24 - 8 * p.crouch + p.breathe)
    z = -sk.fy * 3.0 - 0.5
    c.disc(back[0], back[1], _r(3.6), _r(4.2), LEATHER.base, z)
    c.disc(back[0] - _r(0.9), back[1] - _r(1.3), _r(2.1), _r(2.3),
           LEATHER.highlight, z + 0.05)
    c.capsule((back[0] - _r(3.6), back[1] + _r(0.8)),
              (back[0] + _r(3.6), back[1] + _r(0.8)), _r(0.8), _r(0.8),
              ACCENT, z + 0.1)
    bedroll = sk.pt(0, -6.0, 29.5 - 9 * p.crouch)
    c.disc(bedroll[0], bedroll[1], _r(4.0), _r(1.4), FUR.base, z + 0.02)


# ---------------------------------------------------------------------------
# Weapons. Diablo-flavoured: heavy, asymmetric, and bloodied.
# ---------------------------------------------------------------------------

def _hand_frame(sk: HeroSkeleton):
    p = sk.pose
    hand = sk.hand(1, p.arm_main, p.arm_main_r)
    a = p.weapon_angle
    return hand, math.cos(a), math.sin(a) * 0.8


def _blood_on(c: Canvas, hand, dx, dy, start: float, end: float, z: float,
              width: float = 1.3) -> None:
    """Dried gore along a blade. Deterministic, so it cannot shimmer."""
    span = end - start
    for k, t in enumerate((0.12, 0.3, 0.46, 0.63, 0.81)):
        d = start + span * t
        px = hand[0] + dx * _r(d) + (k % 2) - 0.5
        py = hand[1] + dy * _r(d) + (k % 3) - 1.0
        ramp = BLOOD.base if k % 2 else BLOOD.shadow
        c.disc(px, py, _r(width * (0.5 + 0.12 * (k % 3))),
               _r(width * 0.55), ramp, z + 0.4)
    # A running drip near the tip.
    c.capsule((hand[0] + dx * _r(end * 0.86), hand[1] + dy * _r(end * 0.86)),
              (hand[0] + dx * _r(end * 0.86), hand[1] + dy * _r(end * 0.86) +
               _r(2.2)), _r(0.5), _r(0.3), BLOOD, z + 0.45)


def weapon_greatsword(c: Canvas, sk: HeroSkeleton, bloody: bool = True,
                      gold: bool = True) -> None:
    """The hero's signature blade: long, fullered, cruel quillons, blood."""
    hand, dx, dy = _hand_frame(sk)
    z = sk.depth(1) + 1.0
    grip_end = (hand[0] - dx * _r(4.2), hand[1] - dy * _r(4.2))
    c.capsule(grip_end, hand, _r(1.3), _r(1.1), LEATHER, z)
    c.disc(grip_end[0], grip_end[1], _r(1.8), _r(1.8),
           ACCENT.base if gold else METAL.base, z + 0.1)
    # Quillons swept toward the blade.
    for s in (1, -1):
        q = (hand[0] + dy * _r(4.2) * s + dx * _r(0.8),
             hand[1] - dx * _r(3.4) * s + dy * _r(0.8))
        c.capsule(hand, q, _r(1.2), _r(0.6),
                  ACCENT if gold else METAL, z + 0.15)
        c.put(q[0], q[1], METAL.highlight, z + 0.2)
    tip = (hand[0] + dx * _r(23.0), hand[1] + dy * _r(23.0))
    mid = (hand[0] + dx * _r(12.0), hand[1] + dy * _r(12.0))
    c.capsule(hand, mid, _r(2.6), _r(2.3), METAL, z + 0.2)
    c.capsule(mid, tip, _r(2.3), _r(0.6), METAL, z + 0.2)
    # Fuller: a dark groove with a lit edge beside it.
    c.capsule((hand[0] + dx * _r(3.5), hand[1] + dy * _r(3.5)),
              (hand[0] + dx * _r(19.0), hand[1] + dy * _r(19.0)),
              _r(0.7), _r(0.5), ACCENT if gold else METAL, z + 0.3)
    c.capsule((hand[0] + dx * _r(4.0) - dy, hand[1] + dy * _r(4.0) + dx),
              (hand[0] + dx * _r(20.0) - dy, hand[1] + dy * _r(20.0) + dx),
              _r(0.4), _r(0.3), METAL, z + 0.32)
    if bloody:
        _blood_on(c, hand, dx, dy, 5.0, 22.0, z, 1.5)


def weapon_axe(c: Canvas, sk: HeroSkeleton, rusty: bool = False) -> None:
    hand, dx, dy = _hand_frame(sk)
    z = sk.depth(1) + 1.0
    haft_end = (hand[0] - dx * _r(5.0), hand[1] - dy * _r(5.0))
    head_at = (hand[0] + dx * _r(15.0), hand[1] + dy * _r(15.0))
    c.capsule(haft_end, head_at, _r(1.3), _r(1.1), LEATHER, z)
    ramp = FUR if rusty else METAL
    # A big crescent bit plus a back spike.
    for s in (1, -1):
        blade = (head_at[0] + dy * _r(5.0) * s, head_at[1] - dx * _r(4.2) * s)
        mid = (head_at[0] + dy * _r(3.2) * s + dx * _r(2.6),
               head_at[1] - dx * _r(2.6) * s + dy * _r(2.6))
        c.quad([head_at, blade, mid], ramp.base, z + 0.2)
        c.capsule(head_at, blade, _r(2.2), _r(1.0), ramp, z + 0.2)
        if s > 0:
            c.capsule(blade, mid, _r(1.0), _r(0.6), ramp, z + 0.25)
    spike = (head_at[0] + dx * _r(5.0), head_at[1] + dy * _r(5.0))
    c.capsule(head_at, spike, _r(1.4), _r(0.4), ramp, z + 0.22)
    if not rusty:
        c.disc(head_at[0], head_at[1], _r(1.4), _r(1.4), ACCENT.base, z + 0.3)
        _blood_on(c, hand, dx, dy, 11.0, 18.0, z, 1.6)


def weapon_bow(c: Canvas, sk: HeroSkeleton) -> None:
    hand, dx, dy = _hand_frame(sk)
    z = sk.depth(1) + 1.0
    top = (hand[0] + dy * _r(10.5), hand[1] - dx * _r(8.5))
    bot = (hand[0] - dy * _r(10.5), hand[1] + dx * _r(8.5))
    belly = (hand[0] + dx * _r(3.0), hand[1] + dy * _r(3.0))
    c.capsule(top, belly, _r(1.1), _r(1.4), LEATHER, z)
    c.capsule(belly, bot, _r(1.4), _r(1.1), LEATHER, z)
    c.capsule(top, bot, _r(0.4), _r(0.4), METAL, z + 0.1)
    for e in (top, bot):
        c.disc(e[0], e[1], _r(1.1), _r(1.1), ACCENT.base, z + 0.15)


def weapon_pick(c: Canvas, sk: HeroSkeleton) -> None:
    hand, dx, dy = _hand_frame(sk)
    z = sk.depth(1) + 1.0
    haft_end = (hand[0] - dx * _r(4.5), hand[1] - dy * _r(4.5))
    head_at = (hand[0] + dx * _r(13.0), hand[1] + dy * _r(13.0))
    c.capsule(haft_end, head_at, _r(1.2), _r(1.0), LEATHER, z)
    a = (head_at[0] + dy * _r(6.0), head_at[1] - dx * _r(5.0))
    b = (head_at[0] - dy * _r(4.0), head_at[1] + dx * _r(3.4))
    c.capsule(head_at, a, _r(1.5), _r(0.4), METAL, z + 0.2)
    c.capsule(head_at, b, _r(1.4), _r(0.8), METAL, z + 0.2)


def weapon_sword(c: Canvas, sk: HeroSkeleton) -> None:
    weapon_greatsword(c, sk, bloody=False, gold=False)


def weapon_none(c: Canvas, sk: HeroSkeleton) -> None:
    return


# ---------------------------------------------------------------------------
# Accessories.
# ---------------------------------------------------------------------------

def accessory_talisman(c: Canvas, sk: HeroSkeleton) -> None:
    p = sk.pose
    top = sk.pt(5.0, 0.6, 15 - 5 * p.crouch)
    low = sk.pt(5.6, 0.6, 10.5 - 4 * p.crouch)
    z = sk.depth(1) + 0.4
    c.capsule(top, low, _r(0.8), _r(0.7), LEATHER, z)
    c.disc(low[0], low[1], _r(1.7), _r(1.9), SKIN.highlight, z + 0.1)
    c.put(low[0], low[1], LEATHER.shadow, z + 0.2)


def accessory_cinder(c: Canvas, sk: HeroSkeleton) -> None:
    p = sk.pose
    top = sk.pt(5.0, 0.6, 15 - 5 * p.crouch)
    low = sk.pt(5.6, 0.6, 10.0 - 4 * p.crouch)
    z = sk.depth(1) + 0.4
    c.capsule(top, low, _r(0.9), _r(0.8), METAL, z)
    c.disc(low[0], low[1], _r(2.1), _r(2.3), METAL.shadow, z + 0.1)
    c.disc(low[0], low[1], _r(1.3), _r(1.5), ACCENT.highlight, z + 0.2)
    c.put(low[0], low[1] - 1, BLOOD.highlight, z + 0.3)
