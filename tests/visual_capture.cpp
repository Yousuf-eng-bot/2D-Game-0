#include "../native/engine.cpp"
#include <iostream>
using namespace av;
const std::string dest =
    (std::filesystem::temp_directory_path() / "death-world-captures").string();
void shot(const std::string &name) {
  static C buf[W * H];
  pix = buf;
  render();
  std::ofstream f(dest + "/" + name + ".ppm", std::ios::binary);
  f << "P6\n640 360\n255\n";
  for (C c : buf) {
    char b[3] = {char(c >> 16), char(c >> 8), char(c)};
    f.write(b, 3);
  }
}
int main() {
  std::filesystem::create_directories(dest);
  boot("");
  loadArtwork("android/assets/cover.bin");
  shot("01-title");
  hub();
  g.toastTime = 0;
  shot("02-hub");
  g.overlay = 4;
  shot("03-worlds");
  g.overlay = 5;
  shot("04-armory");
  g.overlay = 0;
  g.world = 0;
  expedition();
  g.toastTime = 0;
  shot("05-forest-start");
  g.px = 22 * T;
  g.py = 31 * T;
  g.camReady = false;
  g.toastTime = 0;
  for (int i = 0; i < 30; i++)
    tick(1.f / 60);
  g.hp = maxhp();
  g.hurtTime = 0;
  shot("06-forest-combat");
  g.px = g.bossX - 50;
  g.py = g.bossY + 80;
  g.camReady = false;
  g.gate = true;
  g.toastTime = 0;
  for (auto &e : g.enemies)
    if (e.boss()) {
      e.hp = e.maxhp * .6;
      beginBoss(e);
    }
  shot("07-forest-boss");
  g.world = 1;
  expedition();
  g.toastTime = 0;
  shot("08-canyon-start");
  g.px = 45 * T;
  g.py = 18 * T;
  g.camReady = false;
  shot("09-canyon");
  g.px = g.bossX - 50;
  g.py = g.bossY + 80;
  g.camReady = false;
  g.gate = true;
  g.toastTime = 0;
  for (auto &e : g.enemies)
    if (e.boss()) {
      e.hp = e.maxhp * .26f;
      beginBoss(e);
    }
  shot("10-canyon-boss");
  g.overlay = 2;
  shot("11-inventory");
  g.overlay = 3;
  shot("12-guide");
  g.overlay = 6;
  shot("13-map");
  g.overlay = 1;
  shot("14-pause");
  hub();
  g.toastTime = 0;
  g.mx = .75f;
  g.my = 0;
  for (int i = 0; i < 16; i++) {
    for (int k = 0; k < 3; k++)
      tick(1.f / 60);
    shot("walk-" + num(i));
  }
  std::cout << "Native renderer captures saved. Visual fixtures, not a "
               "gameplay playthrough.\n";
}
