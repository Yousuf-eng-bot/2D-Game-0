#pragma once
#include "state.hpp"
namespace av {
uint32_t checksum(const std::string &s) {
  uint32_t h = 2166136261u;
  for (unsigned char c : s) {
    h ^= c;
    h *= 16777619u;
  }
  return h;
}
std::string encode() {
  std::ostringstream s;
  s << "DEATH2 " << g.level << ' ' << g.xp << ' ' << g.gold << ' ' << g.upgrade
    << ' ' << g.wins << ' ' << g.run << ' ' << g.uid << ' ' << g.muted << ' '
    << g.shake << ' ' << g.eq[0] << ' ' << g.eq[1] << ' ' << g.eq[2] << ' '
    << g.bag.size() << ' ' << g.weapon << ' '
    << (g.openWorld ? g.world % 2 : g.world) << ' ' << g.worldWins[0] << ' '
    << g.worldWins[1] << ' ' << g.difficulty << ' ' << g.lowPower << '\n';
  for (auto &i : g.bag)
    s << i.id << ' ' << i.slot << ' ' << i.value << ' ' << i.rarity << '\n';
  return s.str();
}
bool decode(const std::string &s) {
  std::istringstream f(s);
  std::string magic;
  int lv, xp, gold, up, wins, run, uid, mute, shake, eq[3], count,
      weapon = 0, world = 0, w0 = 0, w1 = 0, diff = 0, low = 0;
  if (!(f >> magic >> lv >> xp >> gold >> up >> wins >> run >> uid >> mute >>
        shake >> eq[0] >> eq[1] >> eq[2] >> count))
    return false;
  if (magic != "ASHEN1" && magic != "DEATH2")
    return false;
  if (magic == "DEATH2" && !(f >> weapon >> world >> w0 >> w1 >> diff >> low))
    return false;
  if (lv < 1 || lv > 10 || xp < 0 || xp > 100000 || gold < 0 ||
      gold > 10000000 || up < 0 || up > 50 || wins < 0 || wins > 10000 ||
      run < 0 || run > 1000000 || count < 3 || count > 18 || uid < 4 ||
      uid > 10000000 || mute < 0 || mute > 1 || shake < 0 || shake > 1 ||
      weapon < 0 || weapon > 2 || world < 0 || world > 1 || w0 < 0 || w1 < 0 ||
      w0 > 10000 || w1 > 10000 || diff < 0 || diff > 1 || low < 0 || low > 1)
    return false;
  std::vector<Item> b;
  for (int j = 0; j < count; j++) {
    Item i;
    if (!(f >> i.id >> i.slot >> i.value >> i.rarity) || i.id < 1 ||
        i.id >= uid || i.slot < 0 || i.slot > 2 || i.value < 0 ||
        i.value > 100 || i.rarity < 0 || i.rarity > 3)
      return false;
    for (auto &old : b)
      if (old.id == i.id)
        return false;
    b.push_back(i);
  }
  for (int j = 0; j < 3; j++) {
    bool found = false;
    for (auto &i : b)
      if (i.id == eq[j] && i.slot == j)
        found = true;
    if (!found)
      return false;
  }
  g.level = lv;
  g.xp = xp;
  g.gold = gold;
  g.upgrade = up;
  g.wins = wins;
  g.run = run;
  g.uid = uid;
  g.muted = mute;
  g.shake = shake;
  g.bag = b;
  std::copy(eq, eq + 3, g.eq);
  g.weapon = weapon;
  g.world = world;
  g.worldWins[0] = magic == "ASHEN1" ? wins : w0;
  g.worldWins[1] = w1;
  g.difficulty = diff;
  g.lowPower = low;
  g.loaded = true;
  g.migrated = magic == "ASHEN1";
  return true;
}
bool loadOne(const std::string &p) {
  std::ifstream f(p);
  std::string sig;
  if (!std::getline(f, sig))
    return false;
  std::ostringstream s;
  s << f.rdbuf();
  std::string body = s.str();
  if (body.size() > 12000)
    return false;
  try {
    if (std::stoull(sig) != checksum(body))
      return false;
  } catch (...) {
    return false;
  }
  return decode(body);
}
void save() {
  saveSettings();
  if (g.openWorld) {
    saveOpenWorld();
    return;
  }
  if (g.scene >= SPLASH)
    return;
  if (g.path.empty())
    return;
  std::string body = encode(), p = g.path + "/progress.sav";
  {
    std::ofstream f(p + ".tmp", std::ios::trunc);
    if (!f)
      return;
    f << checksum(body) << '\n' << body;
    f.flush();
    if (!f)
      return;
  }
  std::error_code ec;
  if (std::filesystem::exists(p)) {
    if (g.migrated && !std::filesystem::exists(g.path + "/legacy-progress.sav"))
      std::filesystem::copy_file(p, g.path + "/legacy-progress.sav", ec);
    std::filesystem::copy_file(
        p, p + ".bak", std::filesystem::copy_options::overwrite_existing, ec);
  }
  if (std::rename((p + ".tmp").c_str(), p.c_str()) == 0)
    g.loaded = true;
}
void loadArtwork(const std::string &file) {
  std::ifstream f(file, std::ios::binary);
  if (!f)
    return;
  std::vector<uint16_t> r(W * H);
  f.read(reinterpret_cast<char *>(r.data()), r.size() * 2);
  if (f.gcount() != W * H * 2)
    return;
  cover.resize(W * H);
  for (int i = 0; i < W * H; i++) {
    uint16_t v = r[i];
    int R = (v >> 11) * 255 / 31, G = ((v >> 5) & 63) * 255 / 63,
        B = (v & 31) * 255 / 31;
    cover[i] = 0xff000000 | R << 16 | G << 8 | B;
  }
}
} // namespace av
