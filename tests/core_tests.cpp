// Historical coordinate adapter; new UI is exercised separately in ui7_tests.
#define DW_LEGACY_TEST_UI
#include "../native/engine.cpp"
#include <chrono>
#include <iostream>
#include <set>
using namespace av;
int checks = 0;
#define CHECK(a)                                                               \
  do {                                                                         \
    if (!(a)) {                                                                \
      std::cerr << "FAIL line " << __LINE__ << ": " << #a << "\n";             \
      return 1;                                                                \
    }                                                                          \
    checks++;                                                                  \
  } while (0)
void fresh(int world = 0) {
  boot("");
  g.world = world;
  expedition();
}
void advance(float s) {
  for (int i = 0; i < int(s * 60); i++)
    tick(1.f / 60);
}
void arena() {
  fresh();
  g.enemies.clear();
  g.props.clear();
  patch(8, 41, 12);
  std::fill(g.rooms, g.rooms + 3, true);
  g.gate = true;
}
int main() {
  std::cout << "Death World 0.2 core regression checks\n";
  std::string legacy =
      "ASHEN1 4 25 275 3 2 9 4 0 1 1 2 3 3\n1 0 2 1\n2 1 3 2\n3 2 3 3\n";
  boot("");
  CHECK(decode(legacy));
  CHECK(g.migrated && g.gold == 275 && g.level == 4 && g.upgrade == 3 &&
        g.wins == 2);
  CHECK(g.bag.size() == 3 && g.eq[1] == 2 && equippedValue(0) == 2);
  CHECK(g.worldWins[0] == 2);
  std::string saved = encode();
  boot("");
  CHECK(decode(saved));
  CHECK(g.gold == 275 && g.bag[2].value == 3 && g.weapon == 0);
  CHECK(!decode("DEATH2 -1 bad"));
  CHECK(encode() == saved);
  std::string path =
      (std::filesystem::temp_directory_path() / "death-world-save-test")
          .string();
  std::filesystem::create_directories(path);
  {
    std::ofstream f(path + "/progress.sav");
    f << checksum(legacy) << '\n' << legacy;
  }
  boot(path);
  hub();
  CHECK(std::filesystem::exists(path + "/legacy-progress.sav"));
  g.gold = 300;
  save();
  g.gold = 310;
  save();
  {
    std::ofstream f(path + "/progress.sav");
    f << "corrupt";
  }
  boot(path);
  CHECK(g.loaded && g.gold == 300);
  std::cout << "PASS old save migration, new round trip, immutable legacy "
               "backup, corrupt-save recovery\n";
  for (int world = 0; world < 2; world++)
    for (int seed = 0; seed < 100; seed++) {
      boot("");
      g.world = world;
      g.run = seed;
      expedition();
      navigation();
      CHECK(fits(g.px, g.py));
      CHECK(g.enemies.size() > 25);
      CHECK(g.animals.size() > 5);
      for (auto &e : g.enemies) {
        CHECK(fits(e.x, e.y, e.boss() ? 14 : 7));
        CHECK(g.distance[int(e.y / T) * MW + int(e.x / T)] >= 0);
      }
      for (int r = 0; r < 3; r++)
        CHECK(g.distance[int(g.roomY[r] / T) * MW + int(g.roomX[r] / T)] >= 0);
      CHECK(g.distance[int(g.bossY / T) * MW + int(g.bossX / T)] >= 0);
    }
  std::cout << "PASS both worlds / 200 encounter seeds / walkable monster and "
               "boss routes\n";
  arena();
  float x = g.px, y = g.py;
  move(x, y, -9999, 0);
  CHECK(fits(x, y));
  CHECK(x >= 2 * T);
  g.mx = g.my = 1;
  float ox = g.px, oy = g.py;
  advance(.5f);
  CHECK(len(g.px - ox, g.py - oy) < 67);
  g.mx = g.my = 0;
  advance(.3f);
  CHECK(std::abs(g.vx) < .1f && std::abs(g.vy) < .1f);
  arena();
  touch(0, 7, 76, 282);
  touch(2, 7, 110, 282);
  touch(0, 8, 587, 293);
  CHECK(g.mx > .8f && g.attacking);
  touch(1, 8, 587, 293);
  CHECK(g.mx > .8f && !g.attacking);
  touch(3, 0, 0, 0);
  CHECK(g.mx == 0 && g.joystick == -1);
  touch(0, 7, 76, 282);
  touch(2, 7, 100, 290);
  suspend();
  CHECK(g.overlay == 1 && g.mx == 0 && g.vx == 0);
  float px = g.px;
  advance(1);
  CHECK(g.px == px);
  std::cout << "PASS analog acceleration, diagonal normalization, braking, "
               "independent touch, lifecycle pause\n";
  arena();
  addEnemy(g.px + 35, g.py, 0, 0);
  float eh = g.enemies[0].hp;
  attack();
  CHECK(g.enemies[0].hp == eh);
  tick(.04f);
  CHECK(g.enemies[0].hp == eh);
  tick(.07f);
  CHECK(g.enemies[0].hp < eh);
  float after = g.enemies[0].hp;
  attackPayload();
  CHECK(g.enemies[0].hp == after);
  float remaining = g.attackCd;
  switchWeapon(AXE);
  CHECK(g.weapon == AXE && g.attackTime == 0 && g.attackCd >= remaining);
  attack();
  CHECK(g.attackTime == 0);
  arena();
  g.weapon = BOW;
  addEnemy(g.px + 160, g.py, 0, 0);
  float old = g.enemies[0].hp;
  attack();
  advance(.7f);
  CHECK(g.enemies[0].hp < old);
  CHECK(g.attackWeapon == BOW);
  arena();
  g.weapon = AXE;
  addEnemy(g.px + 55, g.py, 0, 0);
  old = g.enemies[0].hp;
  attack();
  tick(.1f);
  CHECK(g.enemies[0].hp == old);
  tick(.15f);
  CHECK(g.enemies[0].hp < old - damage());
  std::cout << "PASS animation-timed sword/axe hits, once-per-swing payloads, "
               "ranged arrows, switch cooldown protection\n";
  // Two overlapping piercing shots must each hit a target only once, even
  // when their updates interleave. The same arrows can hit a second target.
  arena();
  addEnemy(g.px + 45, g.py, 0, 0);
  g.enemies[0].hp = g.enemies[0].maxhp = 10000;
  for (int i = 0; i < 2; i++) {
    fireArrow(g.enemies[0].x, g.enemies[0].y, 0, 10, 1);
    g.bolts.back().vx = 0;
  }
  updateBolts(.01f);
  CHECK(g.bolts.size() == 2);
  float once = g.enemies[0].hp;
  CHECK(once < 10000);
  updateBolts(.01f);
  CHECK(g.enemies[0].hp == once && g.bolts.size() == 2);
  addEnemy(g.enemies[0].x, g.enemies[0].y, 0, 0);
  g.enemies[1].hp = g.enemies[1].maxhp = 10000;
  updateBolts(.01f);
  CHECK(g.enemies[0].hp == once);
  CHECK(g.enemies[1].hp < 10000 && g.bolts.empty());
  std::cout << "PASS overlapping piercing arrows hit each target once\n";
  arena();
  g.hp = 70;
  skill(3);
  CHECK(g.hp == 120 && g.flasks == 2);
  g.hp = 60;
  skill(3);
  CHECK(g.hp == 60);
  skill(1);
  CHECK(g.dodgeCharges == 1 && g.invul > 0);
  hurt(30, g.px - 10, g.py);
  CHECK(g.hp == 60);
  g.dodgeCd = 0;
  skill(1);
  CHECK(g.dodgeCharges == 0);
  g.dodgeCd = 0;
  skill(1);
  CHECK(g.dodgeCharges == 0);
  advance(3.5f);
  CHECK(g.dodgeCharges == 1);
  g.invul = 0;
  skill(2);
  hurt(20, g.px - 10, g.py);
  CHECK(g.hp == 47);
  CHECK(g.hurtTime > 0 && g.haptic == 1 && g.damageVignette > 0);
  std::cout << "PASS two rechargeable dodges, flask limits, ward mitigation "
               "and hurt feedback\n";
  for (int world = 0; world < 2; world++) {
    fresh(world);
    auto &boss = g.enemies.back();
    float hp = boss.hp;
    hitEnemy(boss, 9999, 1, 0, true);
    CHECK(boss.hp == hp);
    g.gate = true;
    g.px = boss.x + 80;
    g.py = boss.y + 50;
    g.enemies.erase(g.enemies.begin(), g.enemies.end() - 1);
    std::set<int> patterns;
    g.invul = 10000;
    g.enemies[0].hp = g.enemies[0].maxhp * .29f;
    for (int i = 0; i < 60 * 40; i++) {
      auto &e = g.enemies[0];
      if (e.state == 1) {
        patterns.insert(e.pattern);
        CHECK(e.windMax >= .7f);
      }
      tick(1.f / 60);
    }
    CHECK(g.enemies[0].phase == 3);
    CHECK(patterns.size() >= 4);
    CHECK(g.enemies.size() > 1);
  }
  std::cout << "PASS shielded guardians, three phases, at least four attack "
               "patterns per boss, deferred summons\n";
  arena();
  addEnemy(g.px + 50, g.py, 8, 4);
  g.gate = true;
  g.enemies[0].hp = g.enemies[0].maxhp = 20000;
  for (int i = 0; i < 13; i++)
    hitEnemy(g.enemies[0], 1, 1, 0, true);
  CHECK(g.enemies[0].state == 4 && g.enemies[0].resist > 0);
  float stagger = g.enemies[0].stagger;
  hitEnemy(g.enemies[0], 1, 1, 0, true);
  CHECK(g.enemies[0].stagger - stagger < 2);
  // Regression: an arrow killing a boss must never invalidate the bolt
  // iteration.
  arena();
  addEnemy(g.px + 40, g.py, 8, 4);
  g.gate = true;
  g.enemies[0].hp = 1;
  fireArrow(g.px, g.py, 0, 20);
  advance(.2f);
  CHECK(g.bossKilled && g.wins == 1);
  int count = g.drops.size();
  hitEnemy(g.enemies[0], 500, 1, 0, true);
  CHECK(int(g.drops.size()) == count);
  touch(0, 1, 330, 320);
  CHECK(g.scene == WIN);
  touch(0, 1, 320, 230);
  CHECK(g.scene == HUB && g.hp == maxhp());
  std::cout << "PASS boss stagger resistance, projectile-kill iterator safety, "
               "one-time reward and victory flow\n";
  arena();
  g.bag.push_back({g.uid++, 0, 4, 2});
  g.overlay = 2;
  g.selected = 3;
  old = damage();
  touch(0, 0, 255, 290);
  CHECK(damage() == old + 12);
  int size = g.bag.size();
  touch(0, 0, 350, 290);
  CHECK(int(g.bag.size()) == size);
  g.selected = 0;
  touch(0, 0, 350, 290);
  CHECK(int(g.bag.size()) == size - 1);
  for (int i = 0; i < 500; i++) {
    auto item = rollItem();
    CHECK(item.value > 0 && item.slot >= 0 && item.slot < 2 &&
          item.rarity >= 1 && item.rarity <= 2);
  }
  std::cout << "PASS cross-weapon equipment bonuses, equipped-item protection, "
               "loot constraints\n";
  // Animation fixture: same position, different stride phases must change
  // rendered pixels.
  hub();
  static C buf[W * H];
  pix = buf;
  std::set<uint32_t> anim;
  for (int i = 0; i < 8; i++) {
    rect(0, 0, W, H, INK);
    g.walkPhase = i * PI / 4;
    heroArt(320, 180, g.walkPhase, true);
    uint32_t h = 0;
    for (C c : buf)
      h = hash(h ^ c);
    anim.insert(h);
  }
  CHECK(anim.size() >= 6);
  for (int i = 0; i < 3000; i++) {
    if (g.scene == HUB || g.scene == DEAD || g.scene == WIN) {
      g.world = i % 2;
      expedition();
    }
    if (i % 90 == 0) {
      g.mx = (rnd(200) - 100) / 100.f;
      g.my = (rnd(200) - 100) / 100.f;
      g.attacking = true;
      skill(rnd(4));
    }
    if (i % 350 == 0)
      switchWeapon((g.weapon + 1) % 3);
    frame(buf, 1.f / 60);
    CHECK(std::isfinite(g.px) && std::isfinite(g.py));
    CHECK(g.particles.size() <= 320);
    CHECK(g.bolts.size() <= 180);
    CHECK(g.hazards.size() <= 40);
  }
  std::cout << "PASS distinct locomotion poses and 3,000-frame "
               "sanitizer/render smoke\n";
  std::cout << "ALL PASSED: " << checks << " assertions\n";
  return 0;
}
