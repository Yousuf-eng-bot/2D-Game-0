// Staged native visual fixtures. Teleports, clock/resource setup and actor
// clearing are intentional; this is NOT normal-input play or an Android phone
// recording.
#include "../native/engine.cpp"
#include <iostream>
using namespace av;
std::vector<C> pixels(W *H);
std::pair<int64_t, int64_t> selectedTree;
bool chopping = false;
void hour(float h) {
  o.seconds = (h < 8 ? h + 16 : h - 8) * 60;
  w.clockOffset = 0;
}
void observe(float h) {
  auto c = campAt(1, 0);
  placePlayer(c.x - 10, c.y + 1);
  hour(h);
  g.mx = g.my = 0;
  g.camReady = false;
  g.toastTime = 0;
  g.invul = 100;
}
void treeFixture() {
  g.mx = g.my = 0;
  g.enemies.clear();
  g.animals.clear();
  Prop tree{};
  bool ok = false;
  float d = 1e6;
  for (auto &p : g.props)
    if (p.kind == 0 && !treeCut(p) && fits(p.x - 44, p.y, 8) &&
        sight(p.x - 44, p.y, p.x - 19, p.y)) {
      float n = len(p.x - g.px, p.y - g.py);
      if (n < d) {
        d = n;
        tree = p;
        ok = true;
      }
    }
  if (ok) {
    selectedTree = treeKey(tree);
    g.px = tree.x - 44;
    g.py = tree.y;
    g.previousX = g.px;
    g.previousY = g.py;
    g.aimx = 1;
    g.aimy = 0;
    g.weapon = AXE;
    v.hunting = false;
    g.camReady = false;
    chopping = true;
    g.attacking = true;
  }
}
void ppm(const std::filesystem::path &p) {
  std::ofstream f(p, std::ios::binary);
  f << "P6\n640 360\n255\n";
  for (C c : pixels) {
    char a[] = {char(c >> 16), char(c >> 8), char(c)};
    f.write(a, 3);
  }
}
int main(int argc, char **argv) {
  auto root = std::filesystem::temp_directory_path() /
              ("earth-capture-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  std::filesystem::copy_file("android/assets/cover.bin", root / "cover.bin");
  boot(root.string());
  bool video = argc > 1 && std::string(argv[1]) == "--video",
       bench = argc > 1 && std::string(argv[1]) == "--bench";
  if (bench) {
    o.draftName = "BENCH FIXTURE";
    o.draftSeed = 20261004;
    createWorld();
    for (int low = 0; low < 2; low++)
      for (int b = 0; b < 5; b++) {
        g.lowPower = low;
        findBiome(b);
        placePlayer(o.waypointX, o.waypointY);
        g.invul = 100;
        hour(b == 3 ? 23 : 11);
        g.mx = .4f;
        g.my = .2f;
        std::vector<double> ms;
        for (int i = 0; i < 210; i++) {
          auto a = std::chrono::steady_clock::now();
          frame(pixels.data(), 1.f / 60);
          auto z = std::chrono::steady_clock::now();
          if (i >= 30)
            ms.push_back(
                std::chrono::duration<double, std::milli>(z - a).count());
        }
        std::sort(ms.begin(), ms.end());
        double total = 0;
        for (double m : ms)
          total += m;
        std::cout << "HOST ONLY biome=" << b << " battery=" << low
                  << " mean_ms=" << total / ms.size()
                  << " p95_ms=" << ms[size_t(ms.size() * .95)]
                  << " max_ms=" << ms.back() << '\n';
      }
    std::filesystem::remove_all(root);
    return 0;
  }
  std::filesystem::path out = argc > 1 ? argv[1] : "build/earth-captures";
  if (!video)
    std::filesystem::create_directories(out);
  std::ofstream events(video && argc > 2 ? argv[2]
                                         : (root / "audio.txt").string());
  std::vector<unsigned char> rgb(W * H * 3);
  int lastMusic = -1, lastAmb = -1;
  for (int n = 0; n < 1620; n++) {
    if (n == 180) {
      o.draftName = "WILD EARTH";
      o.draftSeed = 20261004;
      createWorld();
      g.toastTime = 0;
    }
    if (n >= 180 && n < 310) {
      g.mx = 1;
      g.my = 0;
    }
    if (n == 310)
      g.mx = g.my = 0;
    if (n == 330)
      treeFixture();
    if (chopping && treeCut(selectedTree.first, selectedTree.second)) {
      g.attacking = false;
      chopping = false;
    }
    if (n == 570) {
      g.attacking = false;
      w.wood = 20;
      v.fiber = 3;
      g.overlay = 9;
      g.toastTime = 0;
    }
    if (n == 690) {
      g.overlay = 0;
      observe(11);
    }
    if (n == 840)
      observe(18.5f);
    if (n == 990)
      observe(23);
    if (n == 1140) {
      g.overlay = 6;
      o.atlasLarge = true;
      g.toastTime = 0;
    }
    if (n >= 1260 && (n - 1260) % 90 == 0) {
      g.overlay = 0;
      int b = 1 + (n - 1260) / 90;
      findBiome(b);
      placePlayer(o.waypointX, o.waypointY);
      revealTerrain();
      hour(9);
      g.camReady = false;
      g.invul = 100;
      g.toastTime = 0;
    }
    frame(pixels.data(), 1.f / 30);
    if (n >= 180 && !g.overlay) {
      panel(198, 234, 270, 24);
      const char *label = n < 330    ? "NEW RANGER / GROUND / CANOPY"
                          : n < 570  ? "AXE / SAVED STUMPS / WOOD"
                          : n < 840  ? "DAYLIGHT / FORMER-HUMAN ROUTINES"
                          : n < 990  ? "DUSK / REACTIVE SHADOWS"
                          : n < 1140 ? "NIGHT / FIRE LIGHT"
                                     : "FIVE BIOMES / NEW NATURAL TEXTURES";
      center(333, 239, label, GOLD);
      center(333, 250, "STAGED NATIVE RENDER / NOT A PHONE CAPTURE", DIM);
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
    } else if (n == 150 || n == 275 || n == 360 || n == 500 || n == 620 ||
               n == 800 || n == 940 || n == 1100 || n == 1200 || n == 1300 ||
               n == 1390 || n == 1480 || n == 1570)
      ppm(out / (num(n) + ".ppm"));
  }
  std::filesystem::remove_all(root);
  return 0;
}
