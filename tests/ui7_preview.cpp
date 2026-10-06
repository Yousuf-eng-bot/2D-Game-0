// Staged native renderer UI preview, NOT physical phone footage or a
// performance test.
#define main screenshot_capture_main
#include "ui7_capture.cpp"
#undef main
int main(int argc, char **argv) {
  auto root = std::filesystem::temp_directory_path() /
              ("ui7-preview-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  std::filesystem::copy_file("android/assets/cover.bin", root / "cover.bin");
  boot(root.string());
  g.scene = HOME;
  o.transition = 0;
  std::ofstream events(argc > 2 ? argv[2] : (root / "events.txt").string());
  std::vector<unsigned char> rgb(W * H * 3);
  int lastMusic = -1, lastAmb = -1;
  for (int n = 0; n < 1800; n++) {
    if (n == 120) {
      o.draftName = "CLEAR PLAY 0.7";
      o.draftSeed = 20261004;
      createWorld();
      u.tips = false;
      g.toastTime = 0;
      g.invul = 100;
      w.wood = 12;
    }
    if (n >= 120 && n < 300) {
      g.mx = .45;
      g.my = 0;
    }
    if (n == 300)
      uOpen(11);
    if (n == 420)
      uOpen(10);
    if (n == 492)
      uAction(300, 0, 0);
    if (n == 540) {
      u.tab = 1;
      j.stock[BENCH] = 1;
      j.stock[WALL] = 8;
      j.stock[DOOR] = 1;
    }
    if (n == 660) {
      uOpen(12);
      u.settingsTab = 0;
    }
    if (n == 750)
      uAction(400, 1, 0);
    if (n == 780) {
      g.overlay = 0;
      u.history.clear();
      g.attacking = true;
    }
    if (n == 960) {
      clearInput();
      u.layout = 2;
      g.mx = -.4;
    }
    if (n == 1080) {
      clearInput();
      u.layout = 3;
      g.mx = .3;
    }
    if (n == 1200) {
      clearInput();
      u.draft = u.offsets;
      uOpen(13);
    }
    if (n >= 1220 && n < 1260)
      u.draft[0].first = -float(n - 1220) / 4;
    if (n == 1320) {
      uAction(406, 0, 0);
      u.language = 1;
      uOpen(7);
    }
    if (n == 1440) {
      u.language = 0;
      uOpen(3);
    }
    if (n == 1560) {
      uOpen(10);
      u.tab = 2;
    }
    if (n == 1680) {
      u.layout = 0;
      u.custom = false;
      u.offsets = {};
      u.size = 1;
      u.language = 0;
      u.history.clear();
      g.overlay = 0;
      clearInput();
      g.mx = .4;
    }
    frame(pixels.data(), 1.f / 30);
    uRound(159, 346, 322, 14, UBG, 235, 3);
    uCenter(320, 345, "STAGED NATIVE UI PREVIEW / NOT A PHONE RECORDING",
            UMUTED, 10);
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
    for (int i = 0; i < W * H; i++) {
      rgb[i * 3] = pixels[i] >> 16;
      rgb[i * 3 + 1] = pixels[i] >> 8;
      rgb[i * 3 + 2] = pixels[i];
    }
    std::cout.write(reinterpret_cast<char *>(rgb.data()), rgb.size());
  }
  std::filesystem::remove_all(root);
}
