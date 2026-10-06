#pragma once
#include "canopy.hpp"
#include "journey.hpp"
namespace av {
void gainXP(int v) {
  g.xp += v;
  while (g.level < 10 && g.xp >= g.level * 60) {
    g.xp -= g.level * 60;
    g.level++;
    g.hp = std::min(float(maxhp()), g.hp + maxhp() * .3f);
    notify("LEVEL " + num(g.level) + " - YOUR STRENGTH GROWS");
    burst(g.px, g.py, GOLD, 28);
    sfx(5);
  }
  if (g.level == 10)
    g.xp = std::min(g.xp, 600);
}
Item rollItem(bool boss = false) {
  int slot = boss ? 2 : rnd(2), r = boss ? 3 : (rnd(100) < 30 ? 2 : 1);
  int v = boss ? 3
               : 1 + g.level / 2 + std::min(g.worldWins[g.world], 15) +
                     g.world + rnd(3);
  return {g.uid++, slot, v, r};
}
void hitEnemy(Enemy &e, float d, float kx, float ky, bool heavy) {
  if (!e.alive || (e.boss() && !g.gate) ||
      (e.boss() && e.pattern == 4 && e.state == 2))
    return;
  bool crit = rnd(100) < 10;
  if (crit)
    d *= 1.5f;
  if (e.kind == 3 && !heavy && e.stun <= 0) {
    float a = std::atan2(g.py - e.y, g.px - e.x),
          f = std::atan2(e.ty - e.y, e.tx - e.x);
    if (std::abs(angleDelta(a, f)) < .95f)
      d *= .55f;
  }
  if (e.state == 4)
    d *= 1.4f;
  e.hp -= d;
  e.flash = .11f;
  if (g.openWorld && e.slot >= 0)
    for (auto &other : g.enemies)
      if (other.alive && other.campX == e.campX && other.campY == e.campY)
        other.alertTime = 8;
  float l = std::max(1.f, len(kx, ky));
  if (!e.boss()) {
    e.vx = kx / l * (heavy ? 125 : 70);
    e.vy = ky / l * (heavy ? 125 : 70);
    e.stun = std::max(e.stun, heavy ? .3f : .10f);
  } else {
    e.stagger += ((heavy ? 8.f : 2.6f) / (e.resist > 0 ? 5 : 1));
    if (e.stagger >= 100 && e.state != 2) {
      e.stagger = 0;
      e.state = 4;
      e.stateTime = 2.2f;
      e.wind = 0;
      e.resist = 15;
      notify("GUARDIAN STAGGERED - STRIKE NOW!");
      sfx(8);
    }
  }
  floating(e.x, e.y - 38, (crit ? "CRIT " : "") + num(int(d)),
           crit ? GOLD : WHITE, crit ? 2 : 1);
  burst(e.x, e.y - 12, heavy ? GOLD : 0xffe6f6dd, heavy ? 10 : 6,
        heavy ? 1.4f : 1);
  g.hitstop = std::max(g.hitstop, heavy ? .055f : .025f);
  g.shakeTime = std::max(g.shakeTime, heavy ? .10f : .04f);
  sfx(heavy ? 8 : 2);
  if (e.hp <= 0) {
    e.hp = 0;
    e.alive = false;
    e.death = 1.5f;
    e.vx = kx / l * 95;
    e.vy = ky / l * 95;
    g.kills++;
    g.killStreak++;
    g.streakTime = 3.5f;
    gainXP(e.boss() ? 130 : e.elite ? 35 : 17);
    burst(e.x, e.y - 10, e.boss() ? GOLD : 0xff96c75e, e.boss() ? 55 : 15,
          1.3f);
    int coins =
        e.boss() ? 100 + g.world * 50 + g.worldWins[g.world] * 10 : 7 + rnd(9);
    g.drops.push_back({e.x, e.y, coins, {}, false, 0});
    if (e.boss() || e.elite || rnd(100) < 23)
      g.drops.push_back({e.x + 13, e.y + 7, 0, rollItem(e.boss()), true, 0});
    if (equippedValue(2) > 0)
      g.hp = std::min(float(maxhp()), g.hp + 3);
    if (g.openWorld) {
      frontierKilled(e);
    } else if (e.boss()) {
      g.bossKilled = true;
      g.wins++;
      g.worldWins[g.world]++;
      for (auto &shot : g.bolts)
        if (!shot.friendly)
          shot.life = 0;
      g.hazards.clear();
      sfx(5);
      notify("THE GUARDIAN FALLS. CLAIM YOUR VICTORY.");
      save();
    }
  }
}
void hurt(int d, float fromx, float fromy) {
  if (g.invul > 0 || g.scene != PLAY || g.bossKilled)
    return;
  if (defendHit(d, fromx, fromy))
    return;
  j.restTime = 0;
  int reduced = std::max(int(d * .45f), d - armor() / 2);
  if (g.ward > 0 && len(g.px - g.wardx, g.py - g.wardy) < 70)
    reduced = std::max(1, int(reduced * .65f));
  g.hp -= reduced;
  bodyHit(reduced, fromx, fromy);
  g.invul = .48f;
  g.hurtTime = .3f;
  g.damageVignette = .28f;
  g.shakeTime = .22f;
  g.haptic = 1;
  float dx = g.px - fromx, dy = g.py - fromy, l = std::max(1.f, len(dx, dy));
  g.hurtx = dx / l * 110;
  g.hurty = dy / l * 110;
  floating(g.px, g.py - 35, "-" + num(reduced), 0xffff8377, 2);
  burst(g.px, g.py - 14, RED, 12);
  sfx(g.openWorld ? 20 : 9);
  if (g.hp <= 0) {
    g.hp = 0;
    g.scene = DEAD;
    g.attackTime = 0;
    clearInput();
    save();
  }
}
Enemy *target(float range) {
  Enemy *best = nullptr;
  float nearest = range;
  for (auto &e : g.enemies)
    if (e.alive && (!e.boss() || g.gate)) {
      float d = len(e.x - g.px, e.y - g.py);
      if (d < nearest && sight(g.px, g.py, e.x, e.y)) {
        best = &e;
        nearest = d;
      }
    }
  return best;
}
void attack() {
  if (g.openWorld) {
    beginStrike(false);
    return;
  }
  if (g.attackCd > 0 || g.attackTime > 0 || g.dash > 0 || g.scene != PLAY)
    return;
  if (!useStamina((g.weapon == AXE   ? 13
                   : g.weapon == BOW ? 7
                                     : 6) *
                  weaponEffort()))
    return;
  Enemy *t = target(g.weapon == BOW ? 330 : 100);
  if (t) {
    float dx = t->x - g.px, dy = t->y - g.py, d = std::max(1.f, len(dx, dy));
    g.aimx = dx / d;
    g.aimy = dy / d;
  } else {
    g.aimx = g.fx;
    g.aimy = g.fy;
  }
  bool wildlife = aimWildlife(g.weapon == BOW ? 330 : 100, t);
  if (g.openWorld && g.weapon == AXE && !t && !wildlife)
    aimTree();
  if (g.comboTime <= 0)
    g.combo = 0;
  else
    g.combo = (g.combo + 1) % 3;
  g.comboTime = 1.3f;
  g.attackWeapon = g.weapon;
  g.attackLength = g.weapon == SWORD ? .43f : g.weapon == AXE ? .78f : .58f;
  g.attackLength *= weaponEffort();
  g.attackTime = g.attackLength;
  g.attackCd = g.attackLength;
  g.attackHit = false;
  g.attackSerial++;
  sfx(g.weapon == BOW ? 7 : 1);
}
void fireArrow(float x, float y, float a, int dmg, int pierce = 0) {
  if (g.bolts.size() < 180)
    g.bolts.push_back({x, y, std::cos(a) * 420, std::sin(a) * 420, .90f, dmg,
                       true, pierce, ++g.attackSerial, 4, 0xffffdf9c});
}
void attackPayload() {
  if (g.attackHit)
    return;
  g.attackHit = true;
  if (g.attackWeapon == BOW) {
    float a = std::atan2(g.aimy, g.aimx);
    fireArrow(g.px + g.aimx * 18, g.py + g.aimy * 18, a, int(damage() * .94f),
              g.combo == 2 ? 1 : 0);
    return;
  }
  float radius = g.attackWeapon == AXE ? 75 : 61,
        amount = damage() *
                 (g.attackWeapon == AXE ? 1.85f : (g.combo == 2 ? 1.45f : 1.f));
  g.slash = .2f;
  huntMelee(radius, amount);
  if (g.openWorld && g.attackWeapon == AXE)
    chopTrees(radius + 12);
  for (auto &e : g.enemies)
    if (e.alive) {
      float dx = e.x - g.px, dy = e.y - g.py, d = len(dx, dy);
      float facing = (dx * g.aimx + dy * g.aimy) / std::max(d, 1.f);
      if (d < radius && (facing > -.15f || d < 22) &&
          sight(g.px, g.py, e.x, e.y))
        hitEnemy(e, amount, dx, dy, g.attackWeapon == AXE);
    }
}
void switchWeapon(int w) {
  if (g.openWorld) {
    j.active = false;
    j.queue = 0;
    j.trail.clear();
  }
  if (w < 0 || w > 2 || w == g.weapon)
    return;
  g.weapon = w;
  g.attackTime = 0;
  g.attackHit = true;
  g.attackCd = std::max(g.attackCd, .25f);
  g.comboTime = 0;
  clearInput();
  sfx(12);
  save();
  notify(std::string(weaponName(w)) + " EQUIPPED");
}
void skill(int id) {
  if (g.scene != PLAY || g.overlay || g.hp <= 0)
    return;
  if (g.openWorld && id == 1) {
    float cost = 19 + (v.injury[4] + v.injury[5]) * .08f;
    if (g.dodgeCd > 0 || g.dodgeCharges <= 0 || v.stamina < cost)
      return;
    if (j.active) {
      if (j.elapsed >= j.windup * .6f && j.elapsed < j.windup + j.activeTime) {
        j.dodgeQueue = .18f;
        return;
      }
      j.active = false;
      j.queue = 0;
      g.attackTime = 0;
    }
  }
  if (id == 0 && g.powerCd <= 0) {
    if (!useStamina(24 * weaponEffort()))
      return;
    g.powerCd = g.weapon == AXE ? 7 : 6;
    g.sweep = .5f;
    sfx(4);
    if (g.weapon == BOW) {
      Enemy *t = target(340);
      float a =
          t ? std::atan2(t->y - g.py, t->x - g.px) : std::atan2(g.fy, g.fx);
      if (aimWildlife(340, t))
        a = std::atan2(g.aimy, g.aimx);
      for (int j = -2; j <= 2; j++)
        fireArrow(g.px, g.py, a + j * .14f, int(damage() * 1.15f), 1);
    } else {
      huntMelee(g.weapon == AXE ? 115 : 98,
                damage() * (g.weapon == AXE ? 2.7f : 1.9f), true);
      for (auto &e : g.enemies)
        if (e.alive &&
            len(e.x - g.px, e.y - g.py) < (g.weapon == AXE ? 115 : 98) &&
            sight(g.px, g.py, e.x, e.y)) {
          hitEnemy(e, damage() * (g.weapon == AXE ? 2.7f : 1.9f), e.x - g.px,
                   e.y - g.py, true);
          e.slow = 2;
        }
    }
  }
  if (id == 1 && g.dodgeCd <= 0 && g.dodgeCharges > 0) {
    if (!useStamina(19 +
                    (g.openWorld ? (v.injury[4] + v.injury[5]) * .08f : 0)))
      return;
    g.dodgeCharges--;
    g.dodgeCd = .34f;
    g.dash = .22f;
    g.invul = .28f;
    g.attackTime = 0;
    g.attackHit = true;
    g.hurtTime = 0;
    g.hurtx = g.hurty = 0;
    if (len(g.mx, g.my) > .1f) {
      float l = len(g.mx, g.my);
      g.fx = g.mx / l;
      g.fy = g.my / l;
    }
    sfx(11);
  }
  if (id == 2 && g.wardCd <= 0) {
    if (!useStamina(14))
      return;
    g.wardCd = 12;
    g.ward = 4.5f;
    g.wardx = g.px;
    g.wardy = g.py;
    burst(g.px, g.py, TEAL, 30);
    sfx(4);
  }
  if (id == 3 && g.healCd <= 0 && g.flasks > 0 && g.hp < maxhp()) {
    g.flasks--;
    g.healCd = 7;
    g.hp = std::min(float(maxhp()), g.hp + maxhp() * .50f);
    floating(g.px, g.py - 38, "RESTORED", TEAL);
    burst(g.px, g.py, TEAL, 22);
    sfx(6);
  }
}
int monsterDamage(int base) {
  return base + g.world * 3 + g.level / 2 + std::min(g.worldWins[g.world], 12) +
         (g.difficulty ? 5 : 0);
}
int bossDamage(float fraction) {
  return int(maxhp() * fraction) * (g.difficulty ? 1.15f : 1.f);
}
void hazard(float x, float y, float r, float warning, int damage, int type = 0,
            float angle = 0) {
  if (g.hazards.size() < 40)
    g.hazards.push_back({x, y, r, warning,
                         type == 1   ? 1.65f
                         : type == 2 ? 3.f
                                     : .35f,
                         warning, angle, damage, type, 0, false,
                         g.world ? 0xffea844e : 0xffcd7650});
}
void enemyBolt(float x, float y, float a, float speed, int dmg, C color = RED) {
  if (g.bolts.size() < 180)
    g.bolts.push_back({x, y, std::cos(a) * speed, std::sin(a) * speed, 3, dmg,
                       false, 0, ++g.attackSerial, 5, color});
}
const char *patternName(int p) {
  switch (p) {
  case 0:
    return "CLEAVING STRIKE";
  case 1:
    return "CHARGE - STEP ASIDE";
  case 2:
    return "ERUPTION - KEEP MOVING";
  case 3:
    return "SHOCKWAVE - FIND THE GAP";
  case 4:
    return "SKYFALL - LEAVE THE CIRCLE";
  default:
    return "VOLLEY - WATCH THE LANES";
  }
}
void beginBoss(Enemy &e) {
  int forest[6] = {0, 1, 2, 3, 0, 5}, canyon[6] = {5, 4, 2, 1, 3, 0};
  float d = len(g.px - e.x, g.py - e.y);
  e.pattern = (g.openWorld ? e.biome != 0 : g.world != 0)
                  ? canyon[e.sequence % 6]
                  : forest[e.sequence % 6];
  if (g.openWorld) {
    const int patterns[5][6] = {{0, 1, 2, 3, 0, 5},
                                {5, 4, 2, 1, 3, 0},
                                {3, 5, 1, 4, 3, 2},
                                {2, 0, 3, 2, 1, 5},
                                {4, 2, 5, 3, 4, 1}};
    e.pattern = patterns[e.biome][e.sequence % 6];
  }
  e.sequence++;
  if (d > 235)
    e.pattern = g.world ? 4 : 1;
  if (e.pattern == 0 && d > 110)
    e.pattern = 1;
  e.state = 1;
  e.impact = false;
  e.windMax = e.pattern == 1   ? .9f
              : e.pattern == 0 ? .72f
              : e.pattern == 5 ? .85f
                               : 1.1f;
  e.wind = e.windMax;
  e.tx = g.px + g.vx * .2f;
  e.ty = g.py + g.vy * .2f;
  if (e.pattern == 2) {
    hazard(g.px, g.py, 48, 1.15f, bossDamage(.24f));
    hazard(g.px + g.vx * .6f + 55, g.py + g.vy * .6f - 35, 42, 1.5f,
           bossDamage(.24f));
    hazard(g.px + g.vx * .9f - 50, g.py + g.vy * .9f + 40, 42, 1.85f,
           bossDamage(.24f));
  }
  if (e.pattern == 3)
    hazard(e.x, e.y, 260, 1.1f, bossDamage(.23f), 1,
           std::atan2(g.py - e.y, g.px - e.x) + .7f);
  sfx(g.openWorld ? 21 : 10);
}
void executeBoss(Enemy &e) {
  float dx = e.tx - e.x, dy = e.ty - e.y, d = std::max(1.f, len(dx, dy));
  e.state = 3;
  e.stateTime = e.phase == 3 ? .55f : .85f;
  if (e.pattern == 0) {
    float pd = len(g.px - e.x, g.py - e.y), a = std::atan2(dy, dx),
          pa = std::atan2(g.py - e.y, g.px - e.x);
    if (pd < 118 && std::abs(angleDelta(pa, a)) < 1.1f)
      hurt(bossDamage(.25f), e.x, e.y);
    burst(e.x + dx / d * 65, e.y + dy / d * 65, GOLD, 30);
  }
  if (e.pattern == 1) {
    e.state = 2;
    e.stateTime = .78f;
    e.vx = dx / d * (e.phase == 3 ? 390 : 335);
    e.vy = dy / d * (e.phase == 3 ? 390 : 335);
  }
  if (e.pattern == 4) {
    e.state = 2;
    e.stateTime = .52f;
    e.vx = dx / .52f;
    e.vy = dy / .52f;
  }
  if (e.pattern == 5) {
    float a = std::atan2(dy, dx);
    int count = e.phase == 1 ? 5 : 7;
    for (int i = 0; i < count; i++)
      enemyBolt(e.x, e.y, a + (i - (count - 1) * .5f) * .24f,
                e.phase == 3 ? 175 : 150, bossDamage(.16f),
                g.world ? 0xffffab55 : 0xffedda75);
  }
}
void updateEnemies(float dt) {
  std::vector<std::array<float, 3>> summons;
  int original = g.enemies.size();
  for (int index = 0; index < original; index++) {
    auto &e = g.enemies[index];
    e.flash = std::max(0.f, e.flash - dt);
    e.slow = std::max(0.f, e.slow - dt);
    e.resist = std::max(0.f, e.resist - dt);
    e.stun = std::max(0.f, e.stun - dt);
    if (!e.alive) {
      e.death = std::max(0.f, e.death - dt);
      move(e.x, e.y, e.vx * dt, e.vy * dt);
      e.vx *= std::exp(-7 * dt);
      e.vy *= std::exp(-7 * dt);
      continue;
    }
    float dx = g.px - e.x, dy = g.py - e.y, d = len(dx, dy);
    float oldx = e.x, oldy = e.y;
    if (g.openWorld && frontierIdle(e, dt))
      continue;
    if (g.openWorld && e.boss()) {
      g.bossX = e.homeX;
      g.bossY = e.homeY;
    }
    if (e.boss()) {
      if (!g.gate)
        continue;
      int phase = e.hp < e.maxhp * .30f ? 3 : e.hp < e.maxhp * .65f ? 2 : 1;
      if (phase > e.phase) {
        e.phase = phase;
        notify(phase == 3 ? "FINAL PHASE - THE GUARDIAN IS ENRAGED"
                          : "PHASE TWO - THE WILDS ANSWER");
        burst(e.x, e.y, GOLD, 50);
        for (int j = 0; j < 2; j++) {
          float x = e.x + (j ? 70 : -70), y = e.y + 50;
          if (fits(x, y))
            summons.push_back({x, y, float(g.world ? 1 : 2)});
        }
      }
      if (e.state == 4) {
        e.stateTime -= dt;
        if (e.stateTime <= 0) {
          e.state = 0;
          e.cd = .5f;
        }
        continue;
      }
      if (d > 560) {
        e.hp = std::min(e.maxhp, e.hp + e.maxhp * .035f * dt);
        float l = std::max(1.f, len(g.bossX - e.x, g.bossY - e.y));
        move(e.x, e.y, (g.bossX - e.x) / l * 70 * dt,
             (g.bossY - e.y) / l * 70 * dt, 14);
        continue;
      }
      if (e.state == 1) {
        e.wind -= dt;
        if (e.wind <= 0)
          executeBoss(e);
      } else if (e.state == 2) {
        e.stateTime -= dt;
        move(e.x, e.y, e.vx * dt, e.vy * dt, 14);
        if (e.pattern == 1) {
          if (len(e.x - g.px, e.y - g.py) < 34)
            hurt(bossDamage(.28f), e.x, e.y);
          if (rnd(3) == 0)
            burst(e.x, e.y, 0xffd7bb84, 1);
        }
        if (e.stateTime <= 0) {
          if (e.pattern == 4) {
            hazard(e.x, e.y, 75, 0, bossDamage(.30f));
            burst(e.x, e.y, GOLD, 35);
          }
          e.state = 3;
          e.stateTime = .85f;
          e.vx = e.vy = 0;
        }
      } else if (e.state == 3) {
        e.stateTime -= dt;
        if (e.stateTime <= 0) {
          e.state = 0;
          e.cd = e.phase == 3 ? .22f : .45f;
        }
      } else {
        e.cd -= dt;
        if (d > 100) {
          if (!sight(e.x, e.y, g.px, g.py))
            steer(e, dx, dy);
          float l = std::max(1.f, len(dx, dy));
          move(e.x, e.y, dx / l * 65 * dt, dy / l * 65 * dt, 14);
        }
        if (e.cd <= 0 && d < 310 && enemyAttackSlot())
          beginBoss(e);
      }
    } else {
      if (d > 450)
        continue;
      e.cd -= dt;
      if (e.stun > 0) {
        move(e.x, e.y, e.vx * dt, e.vy * dt);
        e.vx *= std::exp(-9 * dt);
        e.vy *= std::exp(-9 * dt);
        continue;
      }
      if (e.wind > 0) {
        e.wind -= dt;
        if (e.wind <= 0) {
          if (e.kind == 1) {
            float a = std::atan2(e.ty - e.y, e.tx - e.x);
            enemyBolt(e.x, e.y, a, 170, monsterDamage(14));
            if (e.elite) {
              enemyBolt(e.x, e.y, a - .2f, 155, monsterDamage(12));
              enemyBolt(e.x, e.y, a + .2f, 155, monsterDamage(12));
            }
          } else if (e.kind == 4) {
            hazard(e.tx, e.ty, 36, .65f, monsterDamage(17));
            for (auto &other : g.enemies)
              if (other.alive && !other.boss() &&
                  len(other.x - e.x, other.y - e.y) < 115)
                other.hp = std::min(other.maxhp, other.hp + 8);
            burst(e.x, e.y, TEAL, 14);
          } else if (len(g.px - e.x, g.py - e.y) < (e.kind == 3 ? 65 : 41) &&
                     sight(e.x, e.y, g.px, g.py))
            hurt(monsterDamage(e.kind == 3   ? 24
                               : e.kind == 2 ? 13
                                             : 17),
                 e.x, e.y);
          e.cd = e.kind == 4   ? 3.f
                 : e.kind == 1 ? 1.7f
                 : e.kind == 3 ? 1.3f
                               : 1.0f;
          e.stateTime = .18f;
        }
        continue;
      }
      if (e.stateTime > 0) {
        e.stateTime -= dt;
        continue;
      }
      bool los = sight(e.x, e.y, g.px, g.py);
      float reach = e.kind == 1   ? 220
                    : e.kind == 4 ? 190
                    : e.kind == 3 ? 51
                                  : 30;
      if (d < reach && los && e.cd <= 0 && enemyAttackSlot()) {
        e.windMax = e.kind == 3 ? .8f : e.kind == 2 ? .45f : .62f;
        e.wind = e.windMax;
        e.tx = g.px + g.vx * .12f;
        e.ty = g.py + g.vy * .12f;
      } else {
        float speed = e.kind == 2   ? 102
                      : e.kind == 3 ? 49
                      : e.kind == 1 ? 67
                                    : 70;
        if (e.slow > 0 ||
            (g.ward > 0 && len(e.x - g.wardx, e.y - g.wardy) < 70))
          speed *= .45f;
        if ((e.kind == 1 || e.kind == 4) && d < 120 && los) {
          dx = -dx;
          dy = -dy;
          speed = 48;
        } else if (d < reach * .85f && los) {
          dx = dy = 0;
        } else if (!los)
          steer(e, dx, dy);
        else if (e.kind == 0 && e.hp < e.maxhp*.22f && d < 110 && los) {
          // Wounded raiders retreat toward their group rather than suiciding.
          dx = -dx*.65f + (e.homeX-e.x)*.35f;
          dy = -dy*.65f + (e.homeY-e.y)*.35f;
          speed = 60;
        } else if (e.kind == 0 && e.entityId%3==1 && d>55 && d<180 && los) {
          // Stable scout role; shared by Low and Medium, no render RNG.
          float a=(e.entityId&8 ? 1.f:-1.f)*.82f;
          float ox=dx;dx=dx*std::cos(a)-dy*std::sin(a);dy=ox*std::sin(a)+dy*std::cos(a);
          speed=86;
        } else if (e.kind == 2 && d > 50 && d < 160) {
          float a = (index % 2 ? 1 : -1) * .65f;
          float ox = dx;
          dx = dx * std::cos(a) - dy * std::sin(a);
          dy = ox * std::sin(a) + dy * std::cos(a);
        }
        float l = std::max(1.f, len(dx, dy));
        move(e.x, e.y, dx / l * speed * dt, dy / l * speed * dt);
      }
    }
    e.step += len(e.x - oldx, e.y - oldy) * .14f;
  }
  // Deferred spawn keeps iteration references valid. Summons never block shrine
  // completion.
  for (auto &s : summons)
    if (g.enemies.size() < 80)
      addEnemy(s[0], s[1], int(s[2]), 4);
  // Gentle crowd separation: dangerous groups remain readable, not one
  // overlapping sprite.
  for (size_t i = 0; i < g.enemies.size(); i++)
    if (g.enemies[i].alive && !g.enemies[i].boss())
      for (size_t j = i + 1; j < g.enemies.size(); j++)
        if (g.enemies[j].alive && !g.enemies[j].boss()) {
          auto &a = g.enemies[i];
          auto &b = g.enemies[j];
          float dx = a.x - b.x, dy = a.y - b.y, d = len(dx, dy);
          if (d > 0 && d < 21) {
            float force = (21 - d) * dt * 3;
            move(a.x, a.y, dx / d * force, dy / d * force);
            move(b.x, b.y, -dx / d * force, -dy / d * force);
          }
        }
}
void updateHazards(float dt) {
  for (auto &h : g.hazards) {
    if (h.wind > 0) {
      h.wind -= dt;
      continue;
    }
    h.life -= dt;
    float d = len(g.px - h.x, g.py - h.y);
    if (h.type == 1) {
      h.radius += 180 * dt;
      float a = std::atan2(g.py - h.y, g.px - h.x);
      if (!h.hit && std::abs(d - h.radius) < 12 &&
          std::abs(angleDelta(a, h.angle)) > .53f) {
        hurt(h.damage, h.x, h.y);
        h.hit = true;
      }
    } else if (h.type == 0 && !h.hit) {
      h.hit = true;
      if (d < h.r + 7)
        hurt(h.damage, h.x, h.y);
      burst(h.x, h.y, h.color, 22);
      g.shakeTime = .12f;
    } else if (h.type == 2 && d < h.r)
      hurt(h.damage, h.x, h.y);
  }
  std::erase_if(g.hazards, [](auto &h) { return h.life <= 0; });
}
void updateBolts(float dt) {
  for (auto &b : g.bolts) {
    b.life -= dt;
    int steps = std::max(1, int(len(b.vx, b.vy) * dt / 5));
    for (int s = 0; s < steps && b.life > 0; s++) {
      b.x += b.vx * dt / steps;
      b.y += b.vy * dt / steps;
      if (!walk(b.x, b.y)) {
        b.life = 0;
        break;
      }
      if (b.friendly) {
        for (auto &e : g.enemies)
          if (e.alive &&
              std::find(b.hitTargets.begin(), b.hitTargets.end(), e.entityId) ==
                  b.hitTargets.end() &&
              len(b.x - e.x, b.y - e.y) < (e.boss() ? 27 : 17)) {
            b.hitTargets[b.hitCount++] = e.entityId;
            e.hitBy = b.id;
            hitEnemy(e, float(b.damage), b.vx, b.vy, false);
            if (b.pierce-- <= 0 || b.hitCount == int(b.hitTargets.size())) {
              b.life = 0;
              break;
            }
          }
        ecologyArrow(b);
      } else if (len(b.x - g.px, b.y - g.py) < 12) {
        hurt(b.damage, b.x - b.vx * .1f, b.y - b.vy * .1f);
        b.life = 0;
      }
    }
  }
  std::erase_if(g.bolts, [](auto &b) { return b.life <= 0; });
}
void collect() {
  for (size_t j = 0; j < g.drops.size();) {
    auto &d = g.drops[j];
    float dist = len(d.x - g.px, d.y - g.py);
    if (dist < 80 && !d.gear && dist > 20) {
      d.x += (g.px - d.x) * .10f;
      d.y += (g.py - d.y) * .10f;
    }
    if (dist < 30) {
      if (d.gear) {
        if (g.bag.size() < 18) {
          g.bag.push_back(d.item);
          notify(itemName(d.item) + " FOUND - OPEN BAG TO EQUIP");
        } else {
          g.gold += 15 + d.item.value * 3;
          notify("BAG FULL - ITEM SALVAGED FOR GOLD");
        }
        sfx(6);
      } else {
        g.gold += d.gold;
        floating(g.px, g.py - 24, "+" + num(d.gold) + "G", GOLD);
        sfx(3);
      }
      g.drops.erase(g.drops.begin() + j);
      save();
    } else {
      d.age += 1.f / 60;
      j++;
    }
  }
  for (auto &p : g.props)
    if (p.kind == 6 && !p.open && g.rooms[p.variant] &&
        len(p.x - g.px, p.y - g.py) < 40) {
      p.open = true;
      g.gold += 25 + g.world * 10;
      g.flasks = std::min(3, g.flasks + 1);
      g.drops.push_back({p.x + 15, p.y, 0, rollItem(), true, 0});
      burst(p.x, p.y, GOLD, 25);
      notify("SHRINE CHEST - GOLD, GEAR AND A FLASK");
      sfx(6);
      save();
    }
}
void tick(float dt) {
  if(ui7Active())ui7Step(dt);
  g.time += dt;
  frontierTick(dt);
  if (o.transition > 0)
    return;
  g.toastTime = std::max(0.f, g.toastTime - dt);
  if (g.overlay)
    return;
  if (g.scene == DEAD) {
    if (g.openWorld) {
      j.active = false;
      j.z = 0;
      j.pose = approach(j.pose, 2.f, dt * 5);
    }
    g.hurtTime = std::max(0.f, g.hurtTime - dt);
    return;
  }
  if (g.scene != PLAY && g.scene != HUB)
    return;
  g.hitstop = std::max(0.f, g.hitstop - dt);
  if (g.hitstop > 0)
    return;
  auto dec = [dt](float &v) { v = std::max(0.f, v - dt); };
  dec(g.invul);
  dec(g.hurtTime);
  dec(g.damageVignette);
  dec(g.attackCd);
  dec(g.comboTime);
  dec(g.powerCd);
  dec(g.dodgeCd);
  dec(g.wardCd);
  dec(g.healCd);
  dec(g.slash);
  dec(g.sweep);
  dec(g.dash);
  dec(g.ward);
  dec(g.shakeTime);
  dec(g.streakTime);
  if (g.streakTime <= 0)
    g.killStreak = 0;
  if (g.dodgeCharges < 2) {
    g.dodgeRecharge += dt;
    if (g.dodgeRecharge >= 3.4f) {
      g.dodgeCharges++;
      g.dodgeRecharge = 0;
    }
  } else
    g.dodgeRecharge = 0;
  survivalTick(dt);
  if (g.scene == DEAD)
    return;
  float inputLen = len(g.mx, g.my);
  float ix = g.mx, iy = g.my;
  if (inputLen > 1) {
    ix /= inputLen;
    iy /= inputLen;
  }
  if (inputLen > .1f && g.dash <= 0) {
    g.fx = ix / std::max(.001f, len(ix, iy));
    g.fy = iy / std::max(.001f, len(ix, iy));
  }
  if (g.openWorld)
    journeyPreStep(dt);
  float speed = 132 * movementFactor() * (g.openWorld ? stanceSpeed() : 1.f);
  if (g.attackTime > 0)
    speed *= g.attackWeapon == AXE ? .58f : .83f;
  float accel = inputLen > .1f ? 1150 : 1700;
  g.vx = approach(g.vx, ix * speed, accel * dt);
  g.vy = approach(g.vy, iy * speed, accel * dt);
  float ox = g.px, oy = g.py;
  if (g.dash > 0) {
    move(g.px, g.py, g.fx * 390 * dt, g.fy * 390 * dt);
    g.echoTimer -= dt;
    if (g.echoTimer <= 0) {
      g.echoes.push_back({g.px, g.py, .2f, int(g.fx < 0)});
      g.echoTimer = .04f;
    }
  } else {
    move(g.px, g.py, g.vx * dt, g.vy * dt);
    if (g.hurtTime > 0)
      move(g.px, g.py, g.hurtx * dt, g.hurty * dt);
  }
  g.hurtx *= std::exp(-12 * dt);
  g.hurty *= std::exp(-12 * dt);
  float traveled = len(g.px - ox, g.py - oy);
  g.walkPhase += traveled * .17f;
  if (g.openWorld && traveled > .1f) {
    o.stepTimer += traveled;
    if (o.stepTimer > 32) {
      o.stepTimer -= 32;
      sfx(g.world == 2 ? 18 : g.world == 3 ? 19 : 17);
    }
  }
  if (traveled > .7f && rnd(12) == 0 && g.particles.size() < 320)
    g.particles.push_back({g.px, g.py, 0, 0, 0, 8, .25f, .25f, 0xffd6cd94, 2});
  if (g.openWorld)
    journeyStrikeStep(dt);
  if (!g.openWorld && g.attackTime > 0) {
    g.attackTime -= dt;
    float elapsed = g.attackLength - g.attackTime;
    float hitAt = g.attackWeapon == AXE   ? .24f
                  : g.attackWeapon == BOW ? .22f
                                          : .10f;
    if (!g.attackHit && elapsed >= hitAt)
      attackPayload();
  }
  if (g.scene == PLAY) {
    if (g.attacking)
      attack();
    g.navTimer -= dt;
    if (g.navTimer <= 0) {
      navigation();
      g.navTimer = .25f;
    }
    updateEnemies(dt);
    updateBolts(dt);
    updateHazards(dt);
    for (int r = 0; r < 3 && !g.openWorld; r++)
      if (!g.rooms[r]) {
        bool any = false;
        for (auto &e : g.enemies)
          if (e.alive && e.room == r)
            any = true;
        if (!any) {
          g.rooms[r] = true;
          g.hp = std::min(float(maxhp()), g.hp + maxhp() * .18f);
          notify("SHRINE " + num(r + 1) + " PURIFIED - A CHEST AWAITS");
          sfx(5);
        }
      }
    if (!g.gate && g.rooms[0] && g.rooms[1] && g.rooms[2]) {
      g.gate = true;
      notify("THE GUARDIAN AWAKENS. FOLLOW THE GOLD MARKER.");
      sfx(10);
    }
    collect();
  }
  if (g.openWorld && g.scene == PLAY)
    frontierStep(dt);
  updateAnimals(dt);
  for (auto &p : g.particles) {
    p.life -= dt;
    p.x += p.vx * dt;
    p.y += p.vy * dt;
    p.z += p.vz * dt;
    p.vz -= 180 * dt;
    if (p.z < 0) {
      p.z = 0;
      p.vz *= -.3f;
    }
  }
  std::erase_if(g.particles, [](auto &p) { return p.life <= 0; });
  for (auto &f : g.floats) {
    f.life -= dt;
    f.y -= 20 * dt;
  }
  std::erase_if(g.floats, [](auto &f) { return f.life <= 0; });
  for (auto &e : g.echoes)
    e.life -= dt;
  std::erase_if(g.echoes, [](auto &e) { return e.life <= 0; });
}
} // namespace av
