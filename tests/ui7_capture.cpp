// Staged screenshots/preview, not a physical phone or input-only playthrough.
#include "../native/engine.cpp"
#include <iostream>
using namespace av;
std::filesystem::path captureDir = "build/ui7-captures";
std::vector<C> pixels(W *H);
void shot(const std::string &name) {
  frame(pixels.data(), 0);
  std::ofstream out((captureDir / (name + ".ppm")).string(), std::ios::binary);
  out << "P6\n640 360\n255\n";
  for (C c : pixels) {
    char p[] = {char(c >> 16), char(c >> 8), char(c)};
    out.write(p, 3);
  }
}
int main(int argc, char **argv) {
  if (argc > 1)
    captureDir = argv[1];
  std::filesystem::create_directories(captureDir);
  auto root = std::filesystem::temp_directory_path() / "ui7-capture";
  std::filesystem::create_directories(root);
  std::filesystem::copy_file("android/assets/cover.bin", root / "cover.bin",
                             std::filesystem::copy_options::overwrite_existing);
  boot(root.string());
  g.scene = HOME;
  o.transition = 0;
  shot("home-bn");
  o.draftName = "NEW HORIZON 5";
  o.draftSeed = 20261004;
  createWorld();
  o.seconds = 180;
  g.toastTime = 0;
  g.invul = 100;
  u.tips = false;
  g.overlay = 0;
  shot("simple-bn");
  u.layout = 1;
  shot("combat-bn");
  u.layout = 2;
  shot("left-bn");
  u.layout = 3;
  shot("large-bn");
  u.layout = 0;
  uOpen(12);
  u.settingsTab = 0;
  shot("layouts-bn");
  u.settingsTab = 1;
  shot("settings-bn");
  u.settingsTab = 2;
  shot("language-bn");
  uOpen(10);
  shot("craft-bn");
  u.tab = 1;
  shot("build-bn");
  u.tab = 2;
  shot("materials-bn");
  u.tab = 4;
  shot("tools-bn");
  uOpen(7);
  shot("body-bn");
  u.tab = 1;
  shot("injuries-bn");
  uOpen(2);
  shot("bag-bn");
  uOpen(5);
  shot("weapons-bn");
  uOpen(11);
  shot("quick-bn");
  uOpen(3);
  shot("help-bn");
  uOpen(6);
  shot("map-bn");
  uOpen(1);
  shot("pause-bn");
  u.language = 1;
  shot("pause-en");
  uOpen(10);
  shot("craft-en");
  g.overlay = 0;
  shot("simple-en");
  g.openWorld = false;
  g.scene = WORLDS;
  u.language = 0;
  scanWorlds();
  shot("worlds-bn");
  u.language = 0;
  g.scene = CREATE;
  shot("create-bn");
  std::filesystem::remove_all(root);
  return 0;
}
