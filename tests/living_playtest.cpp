// Perfect-information input agent, not a human balance or phone performance
// test.
#define main classic_agent_main
#include "playthrough_test.cpp"
#undef main
int main() {
  auto root = std::filesystem::temp_directory_path() /
              ("living-agent-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  int victories = 0, total = 0;
  for (uint64_t seed : {3032026ULL, 123456789ULL})
    for (int weapon = 0; weapon < 3; weapon++) {
      boot(root.string());
      o.draftName = "INPUT AGENT";
      o.draftSeed = seed;
      g.weapon = weapon;
      createWorld();
      touch(0, 1, 140, 133); // Ordinary HUNT OFF control prioritizes combat.
      int frame = 0;
      for (; frame < 360 * 60 && g.scene == PLAY &&
             (g.wins == 0 || o.campsCleared < 2);
           frame++) {
        // A perfect-information player uses the same paused body controls as a
        // human.
        if (frame % 30 == 0) {
          for (int p = 0; p < 6; p++)
            if ((v.bleeding[p] > 0 || v.injury[p] >= 45) && v.bandages > 0) {
              touch(0, 1, 40, 133);
              const int xx[] = {164, 164, 123, 204, 151, 177},
                        yy[] = {95, 150, 156, 156, 224, 224};
              touch(0, 1, xx[p], yy[p]);
              touch(0, 1, 375, 190);
              if (v.injury[p] >= 55 && p >= 2 && v.splints > 0)
                touch(0, 1, 510, 190);
              touch(0, 1, 580, 39);
            }
          if (v.food < 50 && v.meals > 0) {
            touch(0, 1, 40, 133);
            touch(0, 1, 375, 152);
            touch(0, 1, 580, 39);
          }
          if (v.water < 45 && v.cleanWater > 0) {
            touch(0, 1, 40, 133);
            touch(0, 1, 515, 152);
            touch(0, 1, 580, 39);
          }
        }
        inputAgent(frame);
        tick(1.f / 60);
        if (!std::isfinite(g.px) || !std::isfinite(g.py) ||
            g.enemies.size() > 220)
          return 2;
      }
      bool win = g.wins > 0 && o.campsCleared >= 2;
      std::cout << "Seed " << seed << " / " << weaponName(weapon) << ": "
                << (win               ? "GOAL REACHED"
                    : g.scene == DEAD ? "DEFEAT"
                                      : "TIME LIMIT")
                << " / " << frame / 60 << "s / " << o.totalKills << " kills / "
                << o.campsCleared << " cleared camps / " << g.wins
                << " guardians / HP " << int(g.hp) << " / food " << int(v.food)
                << " / water " << int(v.water) << " / bandages " << v.bandages
                << "\n"
                << std::flush;
      victories += win;
      total++;
    }
  std::cout << "Living Wilds input-only goals: " << victories << "/" << total
            << ". Normal movement, skills, healing, body treatment, food, "
               "water and collected gear; no "
               "teleport or HP/damage grants.\n";
  std::filesystem::remove_all(root);
  return 0; // Character losses are recorded, not hidden by tuning the
            // test/game.
}
