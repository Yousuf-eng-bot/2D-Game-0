// Character atlas and animation tests.
//
// Guards the three things that break layered sprite characters in practice:
// a frame that does not exist, a layer that drifts off the shared anchor, and
// an animation that cannot be reached from game state.
#define DW_LEGACY_TEST_UI
#include "../native/engine.cpp"
#include <iostream>
using namespace av;
int checks = 0;
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      std::cerr << "FAIL line " << __LINE__ << ": " << #x << "\n";             \
      return 1;                                                                \
    }                                                                          \
    checks++;                                                                  \
  } while (0)

int main() {
  std::vector<C> buf(W * H);
  pix = buf.data();
  auto root = std::filesystem::temp_directory_path() /
              ("char-test-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  boot(root.string());

  auto asset = std::filesystem::path(__FILE__).parent_path().parent_path() /
               "android/assets/characters.dwa";
  CHECK(std::filesystem::exists(asset));
  CHECK(loadCharacterAtlas(asset.string()));
  bindCharacterAtlas();
  CHECK(spriteCharactersReady());

  // ---- atlas integrity ---------------------------------------------------
  CHECK(charAtlas.cellW == 48 && charAtlas.cellH == 48);
  CHECK(charAtlas.dirs == 5);
  CHECK(charAtlas.slots == CHAR_PALETTE_SLOTS);
  const char *needVariants[] = {
      "body", "body_armed", "head", "hair", "hood", "helm_steel",
      "coat_ranger", "coat_mail", "coat_bandit", "coat_elite", "pack", "belt",
      "w_dawnblade", "w_riftaxe", "w_rustaxe", "w_windbow", "w_pick"};
  for (const char *n : needVariants)
    CHECK(charAtlas.variant(n) >= 0);
  // Every animation the spec asks for must exist, with enough frames.
  struct Need { const char *name; int minFrames; };
  const Need needAnims[] = {
      {"idle", 6},   {"idle_look", 6}, {"idle_adjust", 6}, {"idle_sword", 6},
      {"walk", 6},   {"run", 6},       {"attack", 6},      {"strong", 6},
      {"chop", 6},   {"mine", 6},      {"guard", 6},       {"guard_hit", 2},
      {"dodge", 6},  {"jump", 5},      {"hurt", 3},        {"die", 6}};
  for (auto &nd : needAnims) {
    int a = charAtlas.anim(nd.name);
    CHECK(a >= 0);
    CHECK(charAtlas.animFrames[a] >= nd.minFrames);
    CHECK(charAtlas.animFps[a] > 0);
  }
  CHECK(charAtlas.animLoop[charAtlas.anim("idle")] == 1);
  CHECK(charAtlas.animLoop[charAtlas.anim("walk")] == 1);
  CHECK(charAtlas.animLoop[charAtlas.anim("attack")] == 0);

  // ---- every body frame exists, is non-empty and stays inside the cell ----
  int body = charAtlas.variant("body");
  int widest = 0, tallest = 0;
  for (size_t a = 0; a < charAtlas.animName.size(); a++)
    for (int d = 0; d < charAtlas.dirs; d++)
      for (int f = 0; f < charAtlas.animFrames[a]; f++) {
        const SpriteRect *r = charAtlas.at(body, int(a), d, f);
        CHECK(r != nullptr);
        CHECK(r->w > 0 && r->h > 0);
        CHECK(r->ox + r->w <= charAtlas.cellW);
        CHECK(r->oy + r->h <= charAtlas.cellH);
        widest = std::max<int>(widest, r->w);
        tallest = std::max<int>(tallest, r->h);
      }
  // Shared anchor sanity: no body frame may be wider than the cell or so tall
  // it would clip, which is what causes characters to hop between frames.
  CHECK(widest <= charAtlas.cellW && tallest <= charAtlas.cellH);

  // ---- layers register on the same anchor --------------------------------
  // Compare the horizontal centre of the body with the coat for every walk
  // frame; a drifting layer would show up as a large offset.
  int coat = charAtlas.variant("coat_ranger");
  int walk = charAtlas.anim("walk");
  for (int d = 0; d < charAtlas.dirs; d++)
    for (int f = 0; f < charAtlas.animFrames[walk]; f++) {
      const SpriteRect *b = charAtlas.at(body, walk, d, f);
      const SpriteRect *c = charAtlas.at(coat, walk, d, f);
      CHECK(b && c);
      int bc = b->ox + b->w / 2, cc = c->ox + c->w / 2;
      CHECK(std::abs(bc - cc) <= 6);
    }

  // ---- eight facings map onto five baked directions ----------------------
  struct FaceCase { float angle; int dir; bool mirror; };
  const FaceCase faces[] = {
      {0.f, 2, false},            // E
      {PI / 2, 0, false},         // S
      {-PI / 2, 4, false},        // N
      {PI / 4, 1, false},         // SE
      {-PI / 4, 3, false},        // NE
      {3 * PI / 4, 1, true},      // SW mirrors SE
      {PI, 2, true},              // W mirrors E
      {-3 * PI / 4, 3, true},     // NW mirrors NE
  };
  for (auto &fc : faces) {
    int d;
    bool m;
    charFacing(fc.angle, d, m);
    CHECK(d == fc.dir && m == fc.mirror);
  }

  // ---- drawing is in-bounds and actually puts pixels down ----------------
  CharOutfit kit;
  kit.coat = charIds.coatRanger;
  kit.weapon = charIds.wDawnblade;
  kit.pack = true;
  for (size_t a = 0; a < charAtlas.animName.size(); a++)
    for (int f = 0; f < charAtlas.animFrames[a]; f++) {
      CharAnimState st;
      st.anim = int(a);
      st.frame = f;
      std::fill(buf.begin(), buf.end(), 0xff000000);
      drawCharActor(kit, st, PI / 2, W / 2, H / 2, PAL_PLAYER);
      int painted = 0;
      for (C c : buf)
        if (c != 0xff000000)
          painted++;
      CHECK(painted > 60);
      // Nothing may be written outside the sprite's reasonable footprint.
      for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
          if (buf[y * W + x] != 0xff000000)
            CHECK(std::abs(x - W / 2) < 48 && y > H / 2 - 56 && y < H / 2 + 12);
    }

  // Drawing near the screen edge must clip rather than corrupt memory.
  std::fill(buf.begin(), buf.end(), 0xff000000);
  CharAnimState edge;
  edge.anim = charAtlas.anim("attack");
  edge.frame = 3;
  drawCharActor(kit, edge, PI / 2, 2, 4, PAL_PLAYER);
  drawCharActor(kit, edge, PI / 2, W - 2, H - 2, PAL_PLAYER);
  checks += 2;

  // ---- palettes are distinct (enemy readability) -------------------------
  CHECK(PAL_PLAYER.c[CS_CLOTH] != PAL_BANDIT.c[CS_CLOTH]);
  CHECK(PAL_BANDIT.c[CS_CLOTH] != PAL_ELITE.c[CS_CLOTH]);
  CHECK(PAL_ELITE.c[CS_ACCENT_H] != PAL_BANDIT.c[CS_ACCENT_H]);
  // The elite's gold must be clearly brighter than the bandit's dull trim.
  auto lum = [](C c) {
    return int((c >> 16 & 255) * 2 + (c >> 8 & 255) * 3 + (c & 255));
  };
  CHECK(lum(PAL_ELITE.c[CS_ACCENT_H]) > lum(PAL_BANDIT.c[CS_ACCENT_H]) + 200);

  // ---- animation state machine -------------------------------------------
  CharAnimState st;
  int attack = charAtlas.anim("attack");
  st.play(attack);
  CHECK(st.anim == attack && st.frame == 0 && !st.finished);
  for (int i = 0; i < 200; i++)
    st.advance(1.f / 60);
  CHECK(st.finished);
  CHECK(st.frame == charAtlas.animFrames[attack] - 1); // one-shot holds
  int idle = charAtlas.anim("idle");
  st.play(idle, true);
  for (int i = 0; i < 600; i++)
    st.advance(1.f / 60);
  CHECK(!st.finished); // looping animation never ends
  CHECK(st.frame >= 0 && st.frame < charAtlas.animFrames[idle]);

  // ---- equipment changes the outfit immediately --------------------------
  g.bag.clear();
  g.bag.push_back(Item{101, 1, 4, 1});
  g.eq[1] = 101;
  CharOutfit light = playerOutfit();
  CHECK(light.coat == charIds.coatRanger);
  g.bag.clear();
  g.bag.push_back(Item{102, 1, 9, 2});
  g.eq[1] = 102;
  CharOutfit heavy = playerOutfit();
  CHECK(heavy.coat == charIds.coatMail);
  CHECK(heavy.coat != light.coat);
  CHECK(heavy.headgear != light.headgear);
  g.weapon = 1;
  CHECK(playerOutfit().weapon == charIds.wRiftaxe);
  g.weapon = 2;
  CHECK(playerOutfit().weapon == charIds.wWindbow);
  g.weapon = 0;
  CHECK(playerOutfit().weapon == charIds.wDawnblade);

  // ---- animal atlas ------------------------------------------------------
  auto animalAsset = std::filesystem::path(__FILE__).parent_path()
                         .parent_path() / "android/assets/animals.dwa";
  CHECK(std::filesystem::exists(animalAsset));
  CHECK(loadCharacterAtlas(animalAsset.string(), animalAtlas));
  resolveAnimalIds();
  CHECK(spriteAnimalsReady());
  CHECK(animalAtlas.cellW == charAtlas.cellW);
  CHECK(animalAtlas.cellH == charAtlas.cellH);
  CHECK(animalAtlas.anchorX == charAtlas.anchorX);
  CHECK(animalAtlas.anchorY == charAtlas.anchorY);
  CHECK(animalAtlas.dirs == 5);
  const char *needAnimals[] = {"deer_doe",     "deer_buck",   "rabbit_grey",
                               "rabbit_brown", "bird_drab",   "bird_blue"};
  for (const char *n : needAnimals)
    CHECK(animalAtlas.variant(n) >= 0);
  // Spec 2.7: no animal clip may be shorter than six frames.
  for (size_t a = 0; a < animalAtlas.animName.size(); a++)
    CHECK(animalAtlas.animFrames[a] >= 6);
  const char *needAnimalAnims[] = {"idle", "graze", "head_lift", "walk",
                                   "run",  "hop",   "peck",      "flee",
                                   "hurt", "die"};
  for (const char *n : needAnimalAnims)
    CHECK(animalAtlas.anim(n) >= 0);
  // Every animal cell must exist and stay inside the shared cell.
  {
    CharSourceScope scope(animalAtlas);
    for (int v = 0; v < int(animalAtlas.variantName.size()); v++)
      for (int a = 0; a < int(animalAtlas.animName.size()); a++)
        for (int d = 0; d < animalAtlas.dirs; d++)
          for (int f = 0; f < animalAtlas.animFrames[a]; f++) {
            const SpriteRect *r = animalAtlas.at(v, a, d, f);
            CHECK(r != nullptr);
            CHECK(r->w > 0 && r->h > 0);
            CHECK(r->ox + r->w <= animalAtlas.cellW);
            CHECK(r->oy + r->h <= animalAtlas.cellH);
          }
  }
  // Species routing and behaviour selection.
  {
    Animal a{};
    a.species = DEER;
    a.id = 2;
    a.alive = true;
    CHECK(animalVariantFor(a) == animalIds.deerDoe);
    a.id = 3;
    CHECK(animalVariantFor(a) == animalIds.deerBuck);
    a.behaviour = GRAZE;
    int clip = chooseAnimalAnim(a, 0.f);
    CHECK(clip == animalIds.aGraze || clip == animalIds.aHeadLift);
    CHECK(chooseAnimalAnim(a, 1.f) == animalIds.aRun);
    a.species = HARE;
    CHECK(animalIsRabbit(a));
    CHECK(chooseAnimalAnim(a, .5f) == animalIds.aHop);
    a.species = RAVEN;
    CHECK(animalIsBird(a));
    a.behaviour = WANDER;
    CHECK(chooseAnimalAnim(a, 0.f) == animalIds.aPeck);
    a.behaviour = FLEE;
    CHECK(chooseAnimalAnim(a, 0.f) == animalIds.aFlee);
    a.species = WOLF; // not baked: keeps its original procedural art
    CHECK(animalVariantFor(a) < 0);
    CHECK(!drawSpriteAnimal(a, W / 2, H / 2, nullptr));
    // The blitter must restore the character atlas after an animal draw.
    a.species = DEER;
    a.alive = true;
    a.flash = 0;
    CHECK(drawSpriteAnimal(a, W / 2, H / 2, nullptr));
    CHECK(charSource == &charAtlas);
  }

  // ---- enemies read as enemies ------------------------------------------
  {
    Enemy e{};
    e.entityId = 7;
    e.alive = true;
    e.hp = e.maxhp = 10;
    e.kind = 0;
    CharOutfit grunt = enemyOutfit(e);
    CHECK(grunt.coat == charIds.coatBandit);
    CHECK(!grunt.pack); // the backpack is a player silhouette cue
    e.elite = true;
    CharOutfit elite = enemyOutfit(e);
    CHECK(elite.coat == charIds.coatElite);
    CHECK(elite.headgear == charIds.helm);
    CHECK(&enemyPalette(e) == &PAL_ELITE);
    e.elite = false;
    CHECK(&enemyPalette(e) == &PAL_BANDIT);
    // Player and enemy palettes must differ in the cloth ramp, which is what
    // makes them separable at a distance.
    for (int i = CS_CLOTH_S; i <= CS_CLOTH_H; i++)
      CHECK(PAL_PLAYER.c[i] != PAL_BANDIT.c[i]);
    e.stun = .2f;
    CHECK(chooseEnemyAnim(e) == charIds.aHurt);
    e.stun = 0;
    e.wind = .2f;
    e.windMax = .3f;
    CHECK(chooseEnemyAnim(e) == charIds.aAttack);
    e.wind = 0;
    e.alive = false;
    CHECK(chooseEnemyAnim(e) == charIds.aDie);
  }

  // ---- hit reaction, lunge and knockback --------------------------------
  {
    int x = 100, y = 100;
    g.hurtTime = 0;
    charKnockback(g.hurtTime, 1, 0, x, y);
    CHECK(x == 100 && y == 100); // no hit, no shove
    charKnockback(.28f, 1, 0, x, y);
    CHECK(x > 100 && x - 100 <= 6); // spec 2.4: 4-6 px directional knockback
    playerLunge = 0;
    j.active = true;
    j.heavy = false;
    j.struck.clear();
    playerStruckSeen = 0;
    updateHitLunge(.016f);
    CHECK(playerLunge == 0.f);
    j.struck.insert(1);
    updateHitLunge(.016f);
    CHECK(playerLunge > 0.f); // landing a hit lunges
    int lx = 50, ly = 50;
    charLunge(0.f, lx, ly);
    CHECK(lx > 50);
    playerLunge = 0;
    j.struck.clear();
    j.active = false;
  }

  // ---- a missing atlas falls back instead of crashing --------------------
  CHECK(!loadCharacterAtlas((root / "no-such-file.dwa").string()));
  CHECK(spriteCharactersReady()); // previous atlas still valid

  std::filesystem::remove_all(root);
  std::cout << "characters: " << checks << " assertions pass\n";
  return 0;
}
