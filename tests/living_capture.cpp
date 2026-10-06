// Native renderer visual fixtures. NOT a phone recording or a normal-input run.
#include "../native/engine.cpp"
#include <iostream>
using namespace av;
std::vector<C> pixels(W *H);
void advance(float seconds) {
  for (int i = 0; i < int(seconds * 60); i++)
    frame(pixels.data(), 1.f / 60);
}
void tap(float x, float y) {
  touch(0, 0, x, y);
  touch(1, 0, x, y);
  advance(.6f);
}
void ppm(const std::filesystem::path &p) {
  frame(pixels.data(), 0);
  std::ofstream f(p, std::ios::binary);
  f << "P6\n640 360\n255\n";
  for (C c : pixels) {
    char b[] = {char(c >> 16), char(c >> 8), char(c)};
    f.write(b, 3);
  }
}
void observeCamp(float hour) {
  Camp c = campAt(1, 0);
  placePlayer(c.x - 10, c.y + 1);
  o.seconds = (hour - 8) * DAY_SECONDS / 24;
  g.mx = g.my = 0;
  g.toastTime = 0;
  g.camReady = false;
}
int main(int argc, char **argv) {
  auto root = std::filesystem::temp_directory_path() /
              ("dw4-capture-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  std::filesystem::copy_file("android/assets/cover.bin", root / "cover.bin");
  boot(root.string());
  if (argc > 1 && std::string(argv[1]) == "--video") {
    std::ofstream events(argc > 2 ? argv[2] : "build/living-audio.txt");
    int lastMusic = -1, lastAmb = -1;
    std::vector<unsigned char> rgb(W * H * 3);
    for (int n = 0; n < 1080; n++) {
      if (n == 115)
        touch(0, 0, 320, 235);
      if (n == 140)
        touch(0, 0, 320, 82);
      if (n == 166) {
        submitText(1, "THE LIVING FRONTIER");
        submitText(2, "20261004");
      }
      if (n == 190)
        touch(0, 0, 320, 318);
      if (n >= 220 && n < 320 && g.scene == PLAY) {
        g.mx = 1;
        g.my = 0;
      }
      if (n == 330)
        observeCamp(11);
      if (n == 570)
        observeCamp(23);
      if (n == 690) {
        v.injury[4] = 73;
        v.bleeding[4] = .2;
        v.injury[2] = 35;
        v.selectedPart = 4;
        v.food = 37;
        v.water = 24;
        g.hp = 87;
        g.overlay = 7;
        g.toastTime = 0;
      }
      if (n == 810) {
        g.overlay = 8;
        v.journalBiome = 3;
      }
      if (n == 900) {
        g.overlay = 6;
        o.atlasLarge = true;
        g.toastTime = 0;
      }
      frame(pixels.data(), 1.f / 30);
      if (!g.overlay && n >= 330) {
        panel(203, 211, 231, 26);
        center(318, 216,
               n < 570 ? "FORMER HUMANS / WORKING DAY"
                       : "NIGHT FALLS / SLEEP ROUTINES",
               GOLD);
        center(318, 228, "NATIVE VISUAL FIXTURE", DIM);
      }
      int music = av_music(), amb = av_audio();
      if (music != lastMusic) {
        events << n / 30.0 << " music " << music << '\n';
        lastMusic = music;
      }
      if (amb != lastAmb) {
        events << n / 30.0 << " ambience " << amb << '\n';
        lastAmb = amb;
      }
      for (int sound; (sound = av_sound()) > 0;)
        events << n / 30.0 << " sound " << sound << '\n';
      for (int i = 0; i < W * H; i++) {
        rgb[i * 3] = pixels[i] >> 16;
        rgb[i * 3 + 1] = pixels[i] >> 8;
        rgb[i * 3 + 2] = pixels[i];
      }
      std::cout.write(reinterpret_cast<const char *>(rgb.data()), rgb.size());
    }
  } else {
    std::filesystem::path out = argc > 1 ? argv[1] : "build/living-captures";
    std::filesystem::create_directories(out);
    advance(4);
    ppm(out / "01-home.ppm");
    tap(320, 235);
    tap(320, 82);
    submitText(1, "THE LIVING FRONTIER");
    submitText(2, "20261004");
    ppm(out / "02-create.ppm");
    tap(320, 318);
    advance(.8);
    g.toastTime = 0;
    ppm(out / "03-origin.ppm");
    observeCamp(11);
    advance(7);
    g.toastTime = 0;
    ppm(out / "04-camp-day.ppm");
    observeCamp(23);
    advance(7);
    g.toastTime = 0;
    ppm(out / "05-camp-night.ppm");
    v.injury[4] = 73;
    v.bleeding[4] = .2;
    v.injury[2] = 35;
    v.selectedPart = 4;
    v.food = 37;
    v.water = 24;
    g.hp = 87;
    g.overlay = 7;
    ppm(out / "06-anatomy.ppm");
    g.overlay = 8;
    v.journalBiome = 3;
    ppm(out / "07-food-web.ppm");
    g.overlay = 6;
    o.atlasLarge = true;
    ppm(out / "08-region-atlas.ppm");
    g.overlay = 0;
    o.seconds = 0;
    for (int b = 0; b < 5; b++) {
      findBiome(b);
      placePlayer(o.waypointX, o.waypointY);
      g.toastTime = 0;
      g.camReady = false;
      ppm(out / ("biome-" + num(b) + ".ppm"));
    }
    std::cerr << "Native visual fixtures saved to " << out << "\n";
  }
  std::filesystem::remove_all(root);
}
