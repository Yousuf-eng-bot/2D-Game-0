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


// Animal palettes. The baked animal art paints into the fur, leather and
// accent ramps, so one bake plus a palette gives each species its colour and
// each species two distinct coat variants.
constexpr CharPalette PAL_ANIMAL_DEER = {{
    0x00000000, 0xff1c1710,
    0xff6a4a30, 0xff8e6a46, 0xffb08a60,
    0xff44352a, 0xff5e4a38, 0xff80664c,
    0xff4a3826, 0xff6c5238, 0xff927050,            // darker coat variant
    0xff5c5248, 0xff86796c, 0xffb0a294,
    0xff6e5a2a, 0xffa88c46, 0xffe0c478,            // antlers / hooves
    0xff2a2018, 0xff423226, 0xff5e4836,
    0xff6b4f33, 0xff9a7450, 0xffc8a074,            // fur: warm deer brown
    0xffffffff,
}};

constexpr CharPalette PAL_ANIMAL_HARE = {{
    0x00000000, 0xff1b1a18,
    0xff8a7a66, 0xffb2a08a, 0xffe2d6c2,
    0xff4a4740, 0xff6a665c, 0xff8e8a7e,
    0xff5a4a3a, 0xff7e6a52, 0xffa48c6e,            // brown variant
    0xff5e6060, 0xff8a8c8a, 0xffb6b8b4,
    0xff8a7a52, 0xffb8a474, 0xffe4d2a2,
    0xff2a2824, 0xff423e38, 0xff5c564e,
    0xff6e6a60, 0xff9c968a, 0xffcac4b6,            // fur: soft grey
    0xffffffff,
}};

constexpr CharPalette PAL_ANIMAL_BIRD = {{
    0x00000000, 0xff121216,
    0xff54504c, 0xff78726c, 0xff9c9690,
    0xff2a2c32, 0xff3e4249, 0xff585e68,
    0xff24344a, 0xff365068, 0xff4e7290,            // blue variant plumage
    0xff44484e, 0xff6a7076, 0xff949aa0,
    0xff7a5e1e, 0xffb88c34, 0xffe8c070,            // beak and legs
    0xff16161a, 0xff2a2a30, 0xff404048,
    0xff33332f, 0xff4e4e48, 0xff6e6e66,            // fur slot: drab plumage
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
      wSword = -1, wSunsteel = -1;
  int accTalisman = -1, accCinder = -1;
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
  c.wSunsteel = V("w_sunsteel");
  c.accTalisman = V("acc_talisman");
  c.accCinder = V("acc_cinder");
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
  int coat = -1, headgear = -1, weapon = -1, accessory = -1;
  bool pack = false, belt = true, hair = true, armedSleeves = false;
};

// Draw order matters: the backpack is behind the torso when the actor faces
// the camera and in front of it when walking away.
void drawCharActor(const CharOutfit &kit, const CharAnimState &st, float angle,
                   int px, int py, const CharPalette &pal, int flash = 0,
                   int alpha = 255, C *normals = nullptr) {
  if (!spriteCharactersReady() || st.anim < 0)
    return;
  int dir;
  bool mirror;
  charFacing(angle, dir, mirror);
  // The backpack only overdraws the torso on a genuine back view. On a side
  // view it sits behind the body in world space, so drawing it on top would
  // paste it across the character's belly.
  bool facingAway = std::sin(angle) < -0.25f;
  auto L = [&](int v) {
    if (v >= 0)
      drawCharLayer(v, st.anim, dir, st.frame, mirror, px, py, pal, flash,
                    alpha, normals);
  };
  if (kit.pack && !facingAway)
    L(charIds.pack);
  L(kit.armedSleeves ? charIds.bodyArmed : charIds.body);
  if (kit.coat >= 0)
    L(kit.coat);
  if (kit.belt)
    L(charIds.belt);
  if (kit.accessory >= 0)
    L(kit.accessory);
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
// Impact and footing effects
// ---------------------------------------------------------------------------

// Dust for the start and end of a roll and for a landing. Drawn as discrete
// stepped puffs rather than a smooth fade so it reads as pixel art.
void drawActorDust(int x, int y, float facing) {
  float fx = std::cos(facing), fy = std::sin(facing);
  // Landing: a flat ring that expands over the landing recovery.
  if (j.landing > 0) {
    float t = clamp(1.f - j.landing / .2f, 0.f, 1.f);
    int rx = 8 + int(t * 16), ry = 3 + int(t * 4);
    int a = int((1 - t) * 190);
    for (int k = 0; k < 2; k++)
      ellipse(x, y + 3 - k, rx - k * 3, ry - k, mix(0xff6f6a56, WHITE, a / 3));
  }
  // Roll: puffs trailing behind the direction of travel.
  if (j.dodgeQueue > 0 || g.dash > 0) {
    float t = g.dash > 0 ? clamp(1.f - g.dash / .22f, 0.f, 1.f) : 0.f;
    for (int k = 1; k <= 3; k++) {
      int px = x - int(fx * (6 + k * 7)), py = y + 2 - int(fy * (3 + k * 3));
      int r = 5 - k + int(t * 3);
      if (r > 0)
        ellipse(px, py, r, std::max(1, r / 2), 0xff7b7360);
    }
  }
}

// Knockback offset for a struck actor: a short directional shove that decays,
// on top of the engine's existing physical impulse. Presentation only.
inline void charKnockback(float hurtTime, float hx, float hy, int &x, int &y) {
  if (hurtTime <= 0)
    return;
  float k = clamp(hurtTime / .28f, 0.f, 1.f);
  float l = std::max(1.f, std::hypot(hx, hy));
  x += int(hx / l * k * 5.f);
  y += int(hy / l * k * 3.f);
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
  // Weapon slot: a rare core (Sunsteel) upgrades the blade in hand.
  int coreRarity = -1;
  for (auto &i : g.bag)
    if (i.slot == 0 && i.id == g.eq[0])
      coreRarity = i.rarity;
  // Trinket slot: Cinder Heart glows, Faded Talisman does not.
  for (auto &i : g.bag)
    if (i.slot == 2 && i.id == g.eq[2])
      k.accessory = i.value > 0 ? charIds.accCinder : charIds.accTalisman;
  // Hand weapon follows the current selection and the active tool action.
  if (j.toolAnim > 0)
    k.weapon = v.task == 2 ? charIds.wPick : charIds.wRustaxe;
  else if (g.weapon == 1)
    k.weapon = charIds.wRiftaxe;
  else if (g.weapon == 2)
    k.weapon = charIds.wWindbow;
  else
    k.weapon = coreRarity >= 2 ? charIds.wSunsteel : charIds.wDawnblade;
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

// Landing a hit gives a short forward lunge and squash. Detected from the
// growth of j.struck, which the combat code already fills in - no simulation
// state is touched.
float playerLunge = 0.f;
size_t playerStruckSeen = 0;

void updateHitLunge(float dt) {
  if (!j.active)
    playerStruckSeen = 0;
  if (j.struck.size() > playerStruckSeen) {
    playerStruckSeen = j.struck.size();
    playerLunge = j.heavy ? .18f : .12f;
  }
  playerLunge = std::max(0.f, playerLunge - dt);
}

// Apply the lunge offset to a screen position. Presentation only.
inline void charLunge(float angle, int &x, int &y) {
  if (playerLunge <= 0)
    return;
  float k = std::sin(clamp(playerLunge / .18f, 0.f, 1.f) * PI);
  x += int(std::cos(angle) * k * 4.f);
  y += int(std::sin(angle) * k * 2.5f);
}

void pruneEnemyAnimStates();
void pruneAnimalAnimStates();

void updatePlayerAnim(float dt) {
  // Actor animation caches are keyed by entity id; drop dead keys here so a
  // long session cannot grow them without bound.
  pruneEnemyAnimStates();
  pruneAnimalAnimStates();
  updateHitLunge(dt);
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


// ---------------------------------------------------------------------------
// Enemies
// ---------------------------------------------------------------------------

// Per-enemy animation state, keyed by the stable entity id so an actor keeps
// its phase across frames without storing anything in the simulation.
std::map<uint64_t, CharAnimState> enemyAnimStates;

const CharPalette &enemyPalette(const Enemy &e) {
  return (e.elite || e.boss()) ? PAL_ELITE : PAL_BANDIT;
}

CharOutfit enemyOutfit(const Enemy &e) {
  CharOutfit k;
  bool elite = e.elite || e.boss();
  // Hunched, shoulder-padded coats separate enemies from the player by
  // silhouette, before colour is even considered.
  k.coat = elite ? charIds.coatElite : charIds.coatBandit;
  k.headgear = elite ? charIds.helm : (e.entityId % 3 == 0 ? charIds.hood : -1);
  k.hair = k.headgear < 0;
  k.armedSleeves = elite;
  k.pack = false;
  k.belt = true;
  k.weapon = e.kind == 1   ? charIds.wWindbow
             : e.kind == 3 ? charIds.wRiftaxe
             : elite       ? charIds.wSword
                           : charIds.wRustaxe;
  return k;
}

// Choose the clip from enemy state. Mirrors the player's priority order.
int chooseEnemyAnim(const Enemy &e) {
  const CharIds &c = charIds;
  if (!e.alive)
    return c.aDie;
  if (e.stun > 0)
    return c.aHurt;
  if (e.stateTime > 0)
    return c.aAttack;
  if (e.wind > 0)
    return c.aAttack;
  if (e.alertTime <= 0 &&
      (e.routine == WORKING || e.routine == COOKING || e.routine == GATHERING))
    return c.aChop;
  float speed = std::hypot(e.x - e.previousX, e.y - e.previousY);
  if (speed > .55f)
    return c.aRun;
  if (speed > .03f)
    return c.aWalk;
  return c.aIdle;
}

// Advance and fetch the animation state for one enemy.
const CharAnimState &enemyAnimFor(const Enemy &e, float dt) {
  CharAnimState &st = enemyAnimStates[e.entityId];
  int want = chooseEnemyAnim(e);
  const CharIds &c = charIds;
  bool oneShot = want == c.aAttack || want == c.aHurt;
  st.play(want, oneShot && st.anim != want);
  int n = charAtlas.ready && want >= 0 ? charAtlas.animFrames[want] : 1;
  if (want == c.aAttack && (e.wind > 0 || e.stateTime > 0)) {
    // Map the real wind-up and strike window onto the clip so the hit lands
    // on the strike frame.
    float u = e.wind > 0
                  ? .55f * clamp(1 - e.wind / std::max(.01f, e.windMax), 0.f, 1.f)
                  : .55f + .45f * clamp((.18f - e.stateTime) / .18f, 0.f, 1.f);
    st.frame = std::min(n - 1, int(u * n));
  } else if (want == c.aWalk || want == c.aRun) {
    st.frame = (int(e.step * 1.7f) % n + n) % n;
  } else if (want == c.aDie) {
    st.frame = std::min(n - 1, int((1.5f - e.death) * n / 1.5f));
  } else {
    st.advance(dt);
  }
  return st;
}

// Drop states for enemies that no longer exist, so the map cannot grow without
// bound across a long session.
void pruneEnemyAnimStates() {
  if (enemyAnimStates.size() < 96)
    return;
  std::set<uint64_t> live;
  for (const auto &e : g.enemies)
    live.insert(e.entityId);
  for (auto it = enemyAnimStates.begin(); it != enemyAnimStates.end();)
    it = live.count(it->first) ? std::next(it) : enemyAnimStates.erase(it);
}

// Health bar and rank label, shared so both tiers label enemies identically.
void drawEnemyBanner(const Enemy &e, int x, int y) {
  if (!e.alive)
    return;
  box(x - 20, y - 79, 40, 5, INK, 0xff68716d);
  rect(x - 19, y - 78, int(38 * e.hp / e.maxhp), 3,
       (e.elite || e.boss()) ? GOLD : 0xffb66b60);
  if (e.elite || e.boss())
    center(x, y - 89, e.boss() ? "GUARDIAN" : "ELITE", GOLD);
  if (e.wind > 0)
    center(x, y - 100, "!", 0xffffd78e, 2);
}

// Shared enemy body draw used by both quality tiers.
void drawSpriteEnemy(const Enemy &e, int x, int y, C *normals) {
  const CharAnimState &st = enemyAnimFor(e, renderDt);
  float angle = e.alertTime > 0 ? std::atan2(g.py - e.y, g.px - e.x)
                                : std::atan2(e.y - e.previousY, e.x - e.previousX);
  if (std::hypot(e.x - e.previousX, e.y - e.previousY) < .001f && e.alertTime <= 0)
    angle = PI / 2;
  int alpha = e.alive ? 255 : int(clamp(e.death / .7f, 0.f, 1.f) * 255);
  int flash = e.flash > 0 ? 170 : 0;
  drawCharActor(enemyOutfit(e), st, angle, x, y, enemyPalette(e), flash, alpha,
                normals);
}


// ---------------------------------------------------------------------------
// Animals
// ---------------------------------------------------------------------------

struct AnimalIds {
  bool ok = false;
  int deerDoe = -1, deerBuck = -1, rabbitGrey = -1, rabbitBrown = -1;
  int birdDrab = -1, birdBlue = -1;
  int aIdle = -1, aGraze = -1, aHeadLift = -1, aWalk = -1, aRun = -1;
  int aHop = -1, aPeck = -1, aFlee = -1, aHurt = -1, aDie = -1;
} animalIds;

void resolveAnimalIds() {
  AnimalIds c;
  const CharAtlas &A = animalAtlas;
  auto V = [&](const char *n) { return A.variant(n); };
  auto N = [&](const char *n) { return A.anim(n); };
  c.deerDoe = V("deer_doe");
  c.deerBuck = V("deer_buck");
  c.rabbitGrey = V("rabbit_grey");
  c.rabbitBrown = V("rabbit_brown");
  c.birdDrab = V("bird_drab");
  c.birdBlue = V("bird_blue");
  c.aIdle = N("idle");
  c.aGraze = N("graze");
  c.aHeadLift = N("head_lift");
  c.aWalk = N("walk");
  c.aRun = N("run");
  c.aHop = N("hop");
  c.aPeck = N("peck");
  c.aFlee = N("flee");
  c.aHurt = N("hurt");
  c.aDie = N("die");
  c.ok = A.ready && c.deerDoe >= 0 && c.rabbitGrey >= 0 && c.birdDrab >= 0 &&
         c.aGraze >= 0 && c.aHop >= 0 && c.aPeck >= 0 && c.aFlee >= 0;
  animalIds = c;
}

bool spriteAnimalsReady() { return animalAtlas.ready && animalIds.ok; }

// Which baked animal, if any, covers this species. Everything not listed keeps
// its original procedural art, so nothing from the old game is lost.
int animalVariantFor(const Animal &a) {
  const AnimalIds &c = animalIds;
  bool alt = (a.id % 2) == 1;
  switch (a.species) {
  case DEER:
  case IBEX:
  case GOAT:
    return alt ? c.deerBuck : c.deerDoe;
  case HARE:
  case SNOWHARE:
    return alt ? c.rabbitBrown : c.rabbitGrey;
  case RAVEN:
  case VULTURE:
    return alt ? c.birdBlue : c.birdDrab;
  default:
    return -1;
  }
}

bool animalIsBird(const Animal &a) {
  return a.species == RAVEN || a.species == VULTURE;
}
bool animalIsRabbit(const Animal &a) {
  return a.species == HARE || a.species == SNOWHARE;
}

std::map<uint64_t, CharAnimState> animalAnimStates;

int chooseAnimalAnim(const Animal &a, float speed) {
  const AnimalIds &c = animalIds;
  if (!a.alive)
    return c.aDie;
  if (a.flash > 0)
    return c.aHurt;
  if (animalIsBird(a)) {
    if (a.behaviour == FLEE)
      return c.aFlee;
    if (speed > .04f)
      return c.aWalk;
    return c.aPeck;
  }
  if (animalIsRabbit(a))
    return speed > .02f ? c.aHop : c.aIdle;
  // Deer and company: grazing is the resting loop, with an occasional lift.
  if (speed > .55f)
    return c.aRun;
  if (speed > .03f)
    return c.aWalk;
  if (a.behaviour == GRAZE || a.behaviour == FEED || a.behaviour == DRINK) {
    // A head lift every 6-10 s, offset per animal so a herd is not in sync.
    float period = 6.f + float(a.id % 5);
    if (std::fmod(g.time + float(a.id % 97) * .11f, period) < 1.1f)
      return c.aHeadLift;
    return c.aGraze;
  }
  return c.aIdle;
}

// Draw one baked animal. Returns false when this species has no baked art and
// the caller should fall back to the original procedural drawing.
bool drawSpriteAnimal(const Animal &a, int x, int y, C *normals) {
  if (!spriteAnimalsReady())
    return false;
  int variant = animalVariantFor(a);
  if (variant < 0)
    return false;
  float speed = std::hypot(a.x - a.previousX, a.y - a.previousY);
  CharAnimState &st = animalAnimStates[a.id];
  int want = chooseAnimalAnim(a, speed);
  const AnimalIds &c = animalIds;
  bool oneShot = want == c.aFlee || want == c.aHurt || want == c.aHeadLift;
  st.play(want, oneShot && st.anim != want);
  CharSourceScope scope(animalAtlas);
  int n = want >= 0 ? animalAtlas.animFrames[want] : 1;
  if (want == c.aWalk || want == c.aRun || want == c.aHop)
    st.frame = (int(a.step * 1.6f) % n + n) % n; // footfalls follow movement
  else if (want == c.aDie)
    st.frame = std::min(n - 1, int(clamp(a.deadTime / .8f, 0.f, 1.f) * n));
  else
    st.advance(renderDt);
  float angle = speed > .002f ? std::atan2(a.y - a.previousY, a.x - a.previousX)
                              : (a.dx < 0 ? PI : 0.f);
  int dir;
  bool mirror;
  charFacing(angle, dir, mirror);
  int flash = a.flash > 0 ? 180 : 0;
  int alpha = a.alive ? 255 : int(clamp(1.f - a.deadTime / 1.2f, 0.f, 1.f) * 255);
  if (alpha <= 0)
    return true;
  for (int r = 0; r < 2; r++)
    ellipse(x, y + 1, 9 - r * 3, 3 - r, 0xff333c33);
  drawCharLayer(variant, st.anim, dir, st.frame, mirror, x, y,
                animalIsBird(a)     ? PAL_ANIMAL_BIRD
                : animalIsRabbit(a) ? PAL_ANIMAL_HARE
                                    : PAL_ANIMAL_DEER,
                flash, alpha, normals);
  return true;
}

void pruneAnimalAnimStates() {
  if (animalAnimStates.size() < 128)
    return;
  std::set<uint64_t> live;
  for (const auto &a : g.animals)
    live.insert(a.id);
  for (auto it = animalAnimStates.begin(); it != animalAnimStates.end();)
    it = live.count(it->first) ? std::next(it) : animalAnimStates.erase(it);
}

} // namespace av
