// Perfect-information input agent, not a human balance or phone performance
// test.
#define main classic_agent_main
#include "playthrough_test.cpp"
#undef main
int main() {
  auto root = std::filesystem::temp_directory_path() /
              ("frontier-agent-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  int victories = 0, total = 0;
  for (uint64_t seed : {3032026ULL, 123456789ULL})
    for (int weapon = 0; weapon < 3; weapon++) {
      boot(root.string());
      o.draftName = "INPUT AGENT";
      o.draftSeed = seed;
      g.weapon = weapon;
      createWorld();
      int frame = 0;
      for (; frame < 240 * 60 && g.scene == PLAY &&
             (g.wins == 0 || o.campsCleared < 2);
           frame++) {
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
                << " guardians / HP " << int(g.hp) << "\n"
                << std::flush;
      victories += win;
      total++;
    }
  std::cout << "Frontier input-only goals: " << victories << "/" << total
            << ". Normal movement, skills, healing and collected gear; no "
               "teleport or HP/damage grants.\n";
  std::filesystem::remove_all(root);
  return 0; // Character losses are recorded, not hidden by tuning the
            // test/game.
}
