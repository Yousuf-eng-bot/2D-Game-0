// Staged renderer demonstration. Teleports/clock setup/invulnerability are
// intentional.
#ifdef BASELINE_ENGINE
#include BASELINE_ENGINE
#else
#include "../native/engine.cpp"
#endif
#include <iostream>
using namespace av;
std::vector<C> pixels(W *H);
std::pair<int64_t, int64_t> scenic(int b) {
  findBiome(b);
  int64_t ox = o.waypointX, oy = o.waypointY, bx = ox, by = oy;
  float best = -1e9;
  for (int y = -96; y <= 96; y += 12)
    for (int x = -96; x <= 96; x += 12) {
      int64_t px = ox + x, py = oy + y;
      auto t = tileAt(px, py);
      if (t.map != 1 || t.biome != b || t.ground != 0)
        continue;
      int water = 0, low = 9, high = 0, trees = 0;
      for (int yy = -8; yy <= 8; yy += 4)
        for (int xx = -12; xx <= 12; xx += 4) {
          auto q = tileAt(px + xx, py + yy);
          water += q.map == 4;
          trees += q.map == 2;
          int ht = landHeight(px + xx, py + yy);
          low = std::min(low, ht);
          high = std::max(high, ht);
        }
      float score =
          water > 15 ? -30
                     : water * 3 + (high - low) * 6 + landHeight(px, py) * 5 +
                           std::min(trees, 9) * 2 - std::abs(x) * .005f;
      if (score > best) {
        best = score;
        bx = px;
        by = py;
      }
    }
  return {bx, by};
}
void stage(int b, float hour) {
  auto xy = scenic(b);
  placePlayer(xy.first + .5, xy.second + .5);
  o.waypoint = false;
  g.camReady = false;
  g.toastTime = 0;
  g.invul = 100;
  g.overlay = 0;
  g.mx = g.my = 0;
  g.hp = maxhp();
  clearInput();
  w.clockOffset = std::fmod(double(hour - 8) * 60 - o.seconds + 14400., 1440.);
  u.tips = false;
  motionBlur = false;
  g.shake = false;
}
void savePpm(std::filesystem::path p) {
  std::ofstream f(p, std::ios::binary);
  f << "P6\n640 360\n255\n";
  for (C c : pixels) {
    char rgb[] = {char(c >> 16), char(c >> 8), char(c)};
    f.write(rgb, 3);
  }
}
int main(int argc, char **argv) {
  auto root = std::filesystem::temp_directory_path() /
              ("vista-capture-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  std::filesystem::copy_file("android/assets/cover.bin", root / "cover.bin");
  boot(root.string());
  o.draftName = "VISTA 0.8";
  o.draftSeed = 20261004;
  createWorld();
  bool video = argc > 1 && std::string(argv[1]) == "--video";
  std::filesystem::path out = argc > 1 ? argv[1] : "build/vista-captures";
  if (!video) {
    std::filesystem::create_directories(out);
    const char *names[] = {"forest", "desert", "snow", "swamp", "volcanic"};
    for (int b = 0; b < 5; b++) {
      stage(b, b == 3 ? 7.f : 11.f);
      frame(pixels.data(), 0);
      savePpm(out / (std::string(names[b]) + ".ppm"));
      std::cerr << names[b] << " " << globalX() << "," << globalY() << "\n";
    }
    placePlayer(0, 0);
    o.waypoint = false;
    g.camReady = false;
    w.clockOffset = 0;
    o.seconds = 180;
    g.toastTime = 0;
    frame(pixels.data(), 0);
    savePpm(out / "camp-day.ppm");
    w.clockOffset = 660;
    frame(pixels.data(), 0);
    savePpm(out / "camp-night.ppm");
#ifndef BASELINE_ENGINE
    stage(0, 11);
    vistaQuality = 0;
    frame(pixels.data(), 0);
    savePpm(out / "forest-low.ppm");
    vistaQuality = 2;
    frame(pixels.data(), 0);
    savePpm(out / "forest-high.ppm");
#endif
  } else {
    std::ofstream events(argv[2]);
    std::vector<unsigned char> rgb(W * H * 3);
    int lm = -1, la = -1;
    const char *names[] = {
        "FOREST / LAYERED FOLIAGE", "DESERT / MATERIALS & DEPTH",
        "SNOW / TERRACES & SOFT SHADOWS", "SWAMP / WATER & UNDERSTORY",
        "VOLCANIC / ROCK & EMBERS"};
    for (int n = 0; n < 1800; n++) {
      int segment = n / 300;
      const char *label =
          segment < 5 ? names[segment] : "CAMP / OCCLUDED FIRELIGHT";
      if (n % 300 == 0) {
        if (segment < 5)
          stage(segment, segment == 3 ? 6.5f : 11.f);
        else {
          placePlayer(0, 0);
          g.camReady = false;
          w.clockOffset = std::fmod(840. - o.seconds + 14400., 1440.);
          g.toastTime = 0;
          o.waypoint = false;
        }
      }
      g.invul = 100;
      g.mx = (n % 300 > 50 && n % 300 < 230) ? .25f : 0;
      g.my = 0;
      frame(pixels.data(), 1.f / 30);
      uRound(138, 334, 364, 25, UBG, 240, 3);
      uCenter(320, 333, label, UINK, 10);
      uCenter(320, 346, "STAGED NATIVE PREVIEW / NOT PHONE FOOTAGE", UMUTED,
              10);
      int m = av_music(), a = av_audio();
      if (m != lm) {
        events << n / 30. << " music " << m << '\n';
        lm = m;
      }
      if (a != la) {
        events << n / 30. << " ambience " << a << '\n';
        la = a;
      }
      for (int s; (s = av_sound()) > 0;)
        events << n / 30. << " sound " << s << '\n';
      for (int i = 0; i < W * H; i++) {
        rgb[i * 3] = pixels[i] >> 16;
        rgb[i * 3 + 1] = pixels[i] >> 8;
        rgb[i * 3 + 2] = pixels[i];
      }
      std::cout.write(reinterpret_cast<char *>(rgb.data()), rgb.size());
    }
  }
  std::filesystem::remove_all(root);
  return 0;
}
