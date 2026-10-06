// Staged native renderer demonstration, NOT a phone recording or input-only
// run. Setup deliberately teleports, grants resources and sets actors/clock.
#define main old_canopy_capture_main
#include "canopy_capture.cpp"
#undef main
void resetStage() {
  clearInput();
  g.overlay = 0;
  j.active = false;
  j.queue = 0;
  j.guard = 0;
  j.stance = 0;
  j.pose = 0;
  j.z = 0;
  g.attackTime = 0;
  g.attackCd = 0;
  g.enemies.clear();
  g.animals.clear();
  g.bolts.clear();
  g.hazards.clear();
  v.stamina = 100;
  g.hp = maxhp();
  g.invul = 100;
  g.camReady = false;
  g.toastTime = 0;
}
void duel() {
  resetStage();
  placePlayer(18.5, .5);
  g.enemies.clear();
  g.animals.clear();
  g.fx = g.aimx = 1;
  g.fy = g.aimy = 0;
  g.weapon = SWORD;
  addEnemy(g.px + 48, g.py, 0, 0, false);
  auto &e = g.enemies.back();
  e.hp = e.maxhp = 600;
  e.alertTime = 20;
}
int main(int argc, char **argv) {
  auto root = std::filesystem::temp_directory_path() /
              ("journey-capture-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  std::filesystem::copy_file("android/assets/cover.bin", root / "cover.bin");
  boot(root.string());
  bool video = argc > 1 && std::string(argv[1]) == "--video";
  std::filesystem::path out = argc > 1 ? argv[1] : "build/journey-captures";
  if (!video)
    std::filesystem::create_directories(out);
  std::ofstream events(video && argc > 2 ? argv[2]
                                         : (root / "events.txt").string());
  std::vector<unsigned char> rgb(W * H * 3);
  int lastMusic = -1, lastAmb = -1;
  const char *label = "OBSIDIAN GAMES / DEATH WORLD 0.6";
  for (int n = 0; n < 1800; n++) {
    if (n == 120) {
      o.draftName = "NEW FRONTIER";
      o.draftSeed = 20261004;
      createWorld();
      label = "ARTICULATED WALK / WEIGHT TRANSFER";
    }
    if (n >= 120 && n < 270) {
      g.mx = std::cos((n - 120) * .03f);
      g.my = std::sin((n - 120) * .03f);
    }
    if (n == 270) {
      duel();
      g.attacking = true;
      label = "LIGHT COMBOS / SHARED SWEEP TIMING";
    }
    if (n == 390) {
      g.attacking = false;
      j.queue = 0;
      v.stamina = 100;
      switchWeapon(AXE);
      label = "HEAVY AXE / COMMITTED WINDUP";
    }
    if (n >= 390 && n < 510 && (n - 390) % 42 == 0)
      beginStrike(true);
    if (n == 510) {
      duel();
      g.invul = 0;
      label = "FRONTAL PARRY / COUNTER WINDOW";
    }
    if (n >= 510 && n < 660) {
      for (auto &e : g.enemies)
        if (e.wind > 0 && e.wind < .15f)
          beginGuard();
      if (j.counter > 1)
        beginStrike(true);
    }
    if (n == 660) {
      resetStage();
      placePlayer(18.5, .5);
      g.enemies.clear();
      g.animals.clear();
      label = "JUMP / HEIGHT / LANDING";
    }
    if (n >= 660 && n < 780 && (n - 660) % 40 == 0)
      jumpPlayer();
    if (n == 780) {
      resetStage();
      cycleStance();
      g.mx = .8f;
      label = "CROUCH: SLOWER / LOWER DETECTION";
    }
    if (n == 840) {
      cycleStance();
      label = "PRONE / CRAWL / NO ATTACKS";
    }
    if (n == 930) {
      resetStage();
      treeFixture();
      label = "CHOPPING / FLOATING RESOURCE DROPS";
    }
    if (chopping && treeCut(selectedTree.first, selectedTree.second)) {
      g.attacking = false;
      chopping = false;
    }
    if (n == 1080) {
      resetStage();
      j.toolTier[0] = 2;
      j.durability[0] = 240;
      for (auto &p : g.props)
        if (p.kind == 1 && fits(p.x - 43, p.y, 7) &&
            sight(p.x - 43, p.y, p.x - 19, p.y)) {
          g.px = p.x - 43;
          g.py = p.y;
          g.fx = g.aimx = 1;
          g.fy = g.aimy = 0;
          break;
        }
      label = "PICK SWING / MINING / TOOL WEAR";
    }
    if (n >= 1080 && n < 1200 && (n - 1080) % 20 == 0)
      harvest();
    if (n == 1200) {
      resetStage();
      placePlayer(22.5, .5);
      g.enemies.clear();
      g.animals.clear();
      for (int x = 19; x <= 26; x++)
        for (int y = -3; y <= 3; y++)
          if (x == 19 || x == 26 || y == -3 || y == 3) {
            Structure b;
            b.kind = (x == 22 && y == 3) ? DOOR : WALL;
            j.built[{x, y}] = b;
          }
      for (auto [x, y, kind] :
           std::vector<std::tuple<int, int, int>>{{20, -2, BED},
                                                  {24, -2, CHEST},
                                                  {24, 1, BENCH},
                                                  {20, 1, FURNACE},
                                                  {25, 2, TORCH},
                                                  {27, 2, 18}}) {
        Structure b;
        b.kind = kind;
        if (kind == FURNACE) {
          b.storage[ORE] = 1;
          b.storage[COAL] = 1;
          b.progress = 4;
        }
        if (kind == 18)
          b.progress = 178;
        j.built[{x, y}] = b;
      }
      j.stock[ORE] = 4;
      j.stock[COAL] = 3;
      w.wood = 12;
      generateWindow();
      label = "PLACEABLE BASE / FURNACE / CROPS";
    }
    if (n == 1350) {
      g.overlay = 10;
      label = "CRAFTING / STORAGE / BUILDING";
    }
    if (n == 1470) {
      resetStage();
      bool ok = false;
      for (int y = 20; y < 150 && !ok; y++)
        for (int x = 35; x < 150; x++) {
          float r = riverDistance(x, y);
          if (r > 7 && r < 9 && landHeight(x, y) >= 3 &&
              tileAt(x, y).map == 1) {
            placePlayer(x + .5, y + .5);
            ok = true;
            break;
          }
        }
      g.enemies.clear();
      g.animals.clear();
      hour(14);
      label = "TERRACES / RIVER BANKS / RAMPS";
    }
    if (n >= 1470 && n < 1650) {
      g.mx = .2f;
      g.my = 0;
    }
    if (n == 1650) {
      clearInput();
      g.overlay = 6;
      o.atlasLarge = true;
      label = "EXPLORED-ONLY ATLAS";
    }
    frame(pixels.data(), 1.f / 30);
    if (n >= 120) {
      panel(199, 335, 435, 24);
      center(416, 338, label, GOLD);
      center(416, 349, "STAGED NATIVE RENDER - NOT A PHONE CAPTURE", DIM);
    }
    int music = av_music(), amb = av_audio();
    if (music != lastMusic) {
      events << n / 30. << " music " << music << '\n';
      lastMusic = music;
    }
    if (amb != lastAmb) {
      events << n / 30. << " ambience " << amb << '\n';
      lastAmb = amb;
    }
    for (int s; (s = av_sound()) > 0;)
      events << n / 30. << " sound " << s << '\n';
    if (video) {
      for (int i = 0; i < W * H; i++) {
        rgb[i * 3] = pixels[i] >> 16;
        rgb[i * 3 + 1] = pixels[i] >> 8;
        rgb[i * 3 + 2] = pixels[i];
      }
      std::cout.write(reinterpret_cast<char *>(rgb.data()), rgb.size());
    } else if (n == 225 || n == 309 || n == 428 || n == 570 || n == 675 ||
               n == 813 || n == 879 || n == 1020 || n == 1126 || n == 1290 ||
               n == 1400 || n == 1540 || n == 1720)
      ppm(out / (num(n) + ".ppm"));
  }
  std::filesystem::remove_all(root);
}
