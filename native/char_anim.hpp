#pragma once
// Character animation state machines and layered actor drawing.
//
// Everything here is presentation only. No function in this file may touch
// simulation, combat, RNG, inventory or progress - it reads game state and
// decides which baked frame to show (AGENTS.md invariant 2).
#include "sprites.hpp"

namespace av {

// ---------------------------------------------------------------------------
// Palettes. One baked actor plus a palette is a distinct-looking character,
// which is how enemy colour coding works without extra frames.
// ---------------------------------------------------------------------------
constexpr CharPalette PAL_PLAYER = {{
    0x00000000, 0xff1b1a1f,
    0xff96684a, 0xffc6a078, 0xffe8c89e,            // skin
    0xff3a4a3e, 0xff566c56, 0xff7e9474,            // cloth: ranger green
    0xff4a3624, 0xff705438, 0xff987850,            // leather
    0xff5c686e, 0xff92a0a4, 0xffccd8d8,            // steel
    0xff7a581e, 0xffba8e36, 0xffeecc78,            // gold trim
    0xff2c221e, 0xff4a382e, 0xff6e5642,            // hair
    0xff5a4634, 0xff8a6e52, 0xffb69c7a,            // fur
    0xffffffff,
}};

// Common bandit: cold grey, no trim. Reads as "not the player" at a glance.
constexpr CharPalette PAL_BANDIT = {{
    0x00000000, 0xff141418,
    0xff6e5a48, 0xff96806a, 0xffb8a088,
    0xff2a3038, 0xff424c58, 0xff5e6a78,            // cloth: cold grey-blue
    0xff2e2a28, 0xff4a443e, 0xff6a6058,
    0xff44484c, 0xff6e767c, 0xff9aa4aa,
    0xff3c3a34, 0xff5c5850, 0xff807a6e,            // dull trim
    0xff1e1c1a, 0xff34302c, 0xff504a42,
    0xff4a4038, 0xff6e6256, 0xff928476,
    0xffffffff,
}};

// Elite: darker body, bright gold edge so it separates from common bandits.
constexpr CharPalette PAL_ELITE = {{
    0x00000000, 0xff0c0c10,
    0xff6a5240, 0xff8e7660, 0xffb09880,
    0xff1a1e28, 0xff2a3040, 0xff3e465a,            // near-black blue
    0xff241f1c, 0xff3c342e, 0xff564c42,
    0xff3a4450, 0xff5e6c7c, 0xff8c9cae,
    0xff8a6214, 0xffd8a633, 0xfffbe08a,            // bright gold
    0xff161418, 0xff282428, 0xff403a3c,
    0xff3e3630, 0xff5e5248, 0xff807060,
    0xffffffff,
}};

// ---------------------------------------------------------------------------
// Animation state
// ---------------------------------------------------------------------------
struct CharAnimState {
  int anim = -1, frame = 0;
  float t = 0;          // seconds inside the current frame
  float idleTimer = 4;  // countdown to the next idle flourish
  int idleVariant = 0;
  bool finished = false;

  void play(int a, bool restart = false) {
    if (a < 0 || (a == anim && !restart))
      return;
    anim = a;
    frame = 0;
    t = 0;
    finished = false;
  }
  void advance(float dt) {
    if (anim < 0 || !charAtlas.ready)
      return;
    int n = charAtlas.animFrames[anim];
    float step = 1.f / float(charAtlas.animFps[anim]);
    t += dt;
    while (t >= step) {
      t -= step;
      if (frame + 1 < n)
        frame++;
      else if (charAtlas.animLoop[anim])
        frame = 0;
      else
        finished = true;
    }
  }
};

struct CharIds {
  bool ok = false;
  int body = -1, bodyArmed = -1, head = -1, hair = -1, hood = -1, helm = -1;
  int coatRanger = -1, coatMail = -1, coatBandit = -1, coatElite = -1;
  int pack = -1, belt = -1;
  int wDawnblade = -1, wRiftaxe = -1, wRustaxe = -1, wWindbow = -1, wPick = -1,
      wSword = -1;
  int aIdle = -1, aIdleLook = -1, aIdleAdjust = -1, aIdleSword = -1;
  int aWalk = -1, aRun = -1, aAttack = -1, aStrong = -1, aChop = -1, aMine = -1;
  int aGuard = -1, aGuardHit = -1, aDodge = -1, aJump = -1, aHurt = -1,
      aDie = -1;
} charIds;

void bindCharacterAtlas() {
  CharIds c;
  if (!charAtlas.ready)
    return;
  auto V = [&](const char *n) { return charAtlas.variant(n); };
  auto A = [&](const char *n) { return charAtlas.anim(n); };
  c.body = V("body");
  c.bodyArmed = V("body_armed");
  c.head = V("head");
  c.hair = V("hair");
  c.hood = V("hood");
  c.helm = V("helm_steel");
  c.coatRanger = V("coat_ranger");
  c.coatMail = V("coat_mail");
  c.coatBandit = V("coat_bandit");
  c.coatElite = V("coat_elite");
  c.pack = V("pack");
  c.belt = V("belt");
  c.wDawnblade = V("w_dawnblade");
  c.wRiftaxe = V("w_riftaxe");
  c.wRustaxe = V("w_rustaxe");
  c.wWindbow = V("w_windbow");
  c.wPick = V("w_pick");
  c.wSword = V("w_sword");
  c.aIdle = A("idle");
  c.aIdleLook = A("idle_look");
  c.aIdleAdjust = A("idle_adjust");
  c.aIdleSword = A("idle_sword");
  c.aWalk = A("walk");
  c.aRun = A("run");
  c.aAttack = A("attack");
  c.aStrong = A("strong");
  c.aChop = A("chop");
  c.aMine = A("mine");
  c.aGuard = A("guard");
  c.aGuardHit = A("guard_hit");
  c.aDodge = A("dodge");
  c.aJump = A("jump");
  c.aHurt = A("hurt");
  c.aDie = A("die");
  c.ok = c.body >= 0 && c.head >= 0 && c.aIdle >= 0 && c.aWalk >= 0 &&
         c.aAttack >= 0;
  charIds = c;
}

bool spriteCharactersReady() { return charAtlas.ready && charIds.ok; }

// ---------------------------------------------------------------------------
// Layered composition
// ---------------------------------------------------------------------------
struct CharOutfit {
  int coat = -1, headgear = -1, weapon = -1;
  bool pack = false, belt = true, hair = true, armedSleeves = false;
};

// Draw order matters: the backpack is behind the torso when the actor faces
// the camera and in front of it when walking away.
void drawCharActor(const CharOutfit &kit, const CharAnimState &st, float angle,
                   int px, int py, const CharPalette &pal, int flash = 0,
                   int alpha = 255) {
  if (!spriteCharactersReady() || st.anim < 0)
    return;
  int dir;
  bool mirror;
  charFacing(angle, dir, mirror);
  bool facingAway = std::sin(angle) < 0.1f;
  auto L = [&](int v) {
    if (v >= 0)
      drawCharLayer(v, st.anim, dir, st.frame, mirror, px, py, pal, flash,
                    alpha);
  };
  if (kit.pack && !facingAway)
    L(charIds.pack);
  L(kit.armedSleeves ? charIds.bodyArmed : charIds.body);
  if (kit.coat >= 0)
    L(kit.coat);
  if (kit.belt)
    L(charIds.belt);
  if (kit.pack && facingAway)
    L(charIds.pack);
  L(charIds.head);
  if (kit.hair && kit.headgear < 0)
    L(charIds.hair);
  if (kit.headgear >= 0)
    L(kit.headgear);
  if (kit.weapon >= 0)
    L(kit.weapon);
}

// ---------------------------------------------------------------------------
// Player
// ---------------------------------------------------------------------------
CharAnimState playerAnim;

CharOutfit playerOutfit() {
  CharOutfit k;
  // Chest slot drives the coat: Grovekeeper Mail at rarity 2+, else Ranger Coat.
  int rarity = 0;
  bool wearing = false;
  for (auto &i : g.bag)
    if (i.id == g.eq[1]) {
      rarity = i.rarity;
      wearing = true;
    }
  k.coat = !wearing ? -1
           : rarity >= 2 ? charIds.coatMail
                         : charIds.coatRanger;
  k.armedSleeves = wearing && rarity >= 2;
  k.headgear = rarity >= 3 ? charIds.helm : rarity >= 2 ? charIds.hood : -1;
  // Hand weapon follows the current selection and the active tool action.
  if (j.toolAnim > 0)
    k.weapon = v.task == 2 ? charIds.wPick : charIds.wRustaxe;
  else
    k.weapon = g.weapon == 1   ? charIds.wRiftaxe
               : g.weapon == 2 ? charIds.wWindbow
                               : charIds.wDawnblade;
  k.pack = true;
  return k;
}

// Pick the animation from live game state. Priority runs from the most
// interrupting action down to locomotion.
int choosePlayerAnim(bool &restart) {
  restart = false;
  const CharIds &c = charIds;
  if (g.scene == DEAD)
    return c.aDie;
  if (j.z > 0.5f || j.landing > 0)
    return c.aJump;
  if (j.dodgeQueue > 0 || g.dodgeCd > 0.42f)
    return c.aDodge;
  if (j.parry > 0)
    return c.aGuardHit;
  if (j.guard > 0)
    return c.aGuard;
  if (j.toolAnim > 0)
    return v.task == 2 ? c.aMine : c.aChop;
  if (j.active)
    return j.heavy ? c.aStrong : c.aAttack;
  if (g.hurtTime > 0.08f)
    return c.aHurt;
  float speed = len(g.vx, g.vy);
  if (speed > 118)
    return c.aRun;
  if (speed > 6)
    return c.aWalk;
  return -1; // caller resolves the idle family
}

void updatePlayerAnim(float dt) {
  if (!spriteCharactersReady())
    return;
  bool restart = false;
  int want = choosePlayerAnim(restart);
  if (want < 0) {
    // Idle family: the calm breathing loop, with a short flourish every
    // 6-10 seconds so a standing character never looks frozen.
    const CharIds &c = charIds;
    bool inFlourish = playerAnim.anim == c.aIdleLook ||
                      playerAnim.anim == c.aIdleAdjust ||
                      playerAnim.anim == c.aIdleSword;
    if (inFlourish && !playerAnim.finished) {
      want = playerAnim.anim;
    } else {
      playerAnim.idleTimer -= dt;
      if (playerAnim.idleTimer <= 0) {
        int pick = int(coordinateHash(int64_t(g.time * 1000), g.run, 7717) % 3);
        want = pick == 0   ? c.aIdleLook
               : pick == 1 ? c.aIdleAdjust
                           : c.aIdleSword;
        playerAnim.idleTimer =
            6.f + float(coordinateHash(int64_t(g.time * 997), 5, 311) % 400) / 100.f;
        restart = true;
      } else {
        want = c.aIdle;
      }
    }
  }
  // One-shot actions restart whenever the action itself restarts.
  const CharIds &c = charIds;
  bool oneShot = want == c.aAttack || want == c.aStrong || want == c.aChop ||
                 want == c.aMine || want == c.aDodge || want == c.aGuardHit ||
                 want == c.aHurt;
  if (oneShot && playerAnim.anim != want)
    restart = true;
  playerAnim.play(want, restart);
  // Action animations are time-warped onto the real action window so the
  // strike frame lands exactly when the hit box opens.
  if (j.active && (want == c.aAttack || want == c.aStrong)) {
    float total = std::max(.01f, j.windup + j.activeTime + j.recovery);
    float u = std::clamp(j.elapsed / total, 0.f, 1.f);
    int n = charAtlas.animFrames[want];
    playerAnim.frame = std::min(n - 1, int(u * n));
  } else if (j.toolAnim > 0 && (want == c.aChop || want == c.aMine)) {
    float u = std::clamp(1.f - j.toolAnim / .45f, 0.f, 1.f);
    int n = charAtlas.animFrames[want];
    playerAnim.frame = std::min(n - 1, int(u * n));
  } else {
    // Walk and run march to the real distance travelled, so the feet keep
    // pace with the ground instead of sliding.
    if (want == c.aWalk || want == c.aRun) {
      int n = charAtlas.animFrames[want];
      playerAnim.frame = int(g.walkPhase * .65f / (2 * PI) * n + n * 4) % n;
    } else {
      playerAnim.advance(dt);
    }
  }
}

} // namespace av
