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
void ppm(const std::string &path) {
  frame(pixels.data(), 0);
  std::ofstream f(path, std::ios::binary);
  f << "P6\n640 360\n255\n";
  for (C c : pixels) {
    char b[] = {char(c >> 16), char(c >> 8), char(c)};
    f.write(b, 3);
  }
}
int main(int argc, char **argv) {
  bool video = argc > 1 && std::string(argv[1]) == "--video";
  auto root = std::filesystem::temp_directory_path() /
              ("dw3-capture-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  std::filesystem::copy_file("android/assets/cover.bin", root / "cover.bin");
  boot(root.string());
  if (video) {
    std::ofstream events(argc > 2 ? argv[2] : "build/audio-events.txt");
    int lastMusic = -1, lastAmb = -1;
    std::vector<unsigned char> rgb(W * H * 3);
    for (int n = 0; n < 660; n++) {
      if (n == 210)
        touch(0, 0, 320, 235);
      if (n == 270)
        touch(0, 0, 320, 82);
      if (n == 295) {
        submitText(1, "THE FIRST FRONTIER");
        submitText(2, "3032026");
      }
      if (n == 385)
        touch(0, 0, 320, 318);
      if (n > 430 && g.scene == PLAY) {
        g.mx = 1;
        g.my = 0;
      }
      frame(pixels.data(), 1.f / 30);
      if (n > 590 && g.scene == PLAY) {
        g.overlay = 6;
        frame(pixels.data(), 0);
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
      for (int sound = av_sound(); sound; sound = av_sound())
        events << n / 30.0 << " sound " << sound << '\n';
      shade(0, 349, W, 11, INK, 190);
      text(10, 351, "NATIVE RENDERER PREVIEW / NOT A PHONE RECORDING", DIM);
      for (size_t i = 0; i < pixels.size(); i++) {
        rgb[i * 3] = pixels[i] >> 16;
        rgb[i * 3 + 1] = pixels[i] >> 8;
        rgb[i * 3 + 2] = pixels[i];
      }
      std::cout.write(reinterpret_cast<char *>(rgb.data()), rgb.size());
    }
  } else {
    std::filesystem::path out = argc > 1 ? argv[1] : "build/frontier-captures";
    std::filesystem::create_directories(out);
    advance(1.9f);
    ppm((out / "01-studio.ppm").string());
    advance(2.5f);
    ppm((out / "02-home.ppm").string());
    tap(320, 235);
    ppm((out / "03-empty-library.ppm").string());
    tap(320, 82);
    submitText(1, "THE FIRST FRONTIER");
    submitText(2, "3032026");
    ppm((out / "04-create.ppm").string());
    tap(320, 318);
    advance(.8f);
    g.toastTime = 0;
    ppm((out / "05-origin.ppm").string());
    for (int b = 0; b < 5; b++) {
      int bx = 0, by = 0;
      double best = 1e20;
      for (int y = -240; y <= 240; y += 8)
        for (int x = -240; x <= 240; x += 8) {
          if (biomeAt(x, y) != b || biomeAt(x + 12, y) != b ||
              biomeAt(x - 12, y) != b || biomeAt(x, y + 12) != b ||
              biomeAt(x, y - 12) != b)
            continue;
          double d = double(x) * x + double(y) * y;
          if (d < best) {
            best = d;
            bx = x;
            by = y;
          }
        }
      placePlayer(bx, by);
      g.toastTime = 0;
      g.camReady = false;
      ppm((out / ("biome-" + num(b) + ".ppm")).string());
    }
    Camp c = campAt(1, 0);
    placePlayer(c.x + .5, c.y + 4);
    advance(.45f);
    g.toastTime = 0;
    ppm((out / "06-stronghold.ppm").string());
    g.overlay = 6;
    ppm((out / "07-atlas.ppm").string());
    g.overlay = 0;
    flushOpenWorld();
    o.draftName = "FROSTFALL EXPEDITION";
    o.draftSeed = 928117;
    createWorld();
    o.draftName = "EMBER VALE";
    o.draftSeed = 775511;
    createWorld();
    leaveFrontier();
    advance(.6f);
    g.toastTime = 0;
    ppm((out / "08-world-library.ppm").string());
    std::cerr << "Saved native visual fixtures to " << out << "\n";
  }
  std::filesystem::remove_all(root);
}
