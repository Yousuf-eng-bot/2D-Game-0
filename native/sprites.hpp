#pragma once
// Runtime loader and blitter for the baked character atlas
// (`android/assets/characters.dwa`, produced by tools/build_character_atlas.py).
//
// Why this format: the engine has no PNG decoder and must stay dependency
// free, so frames are stored as trimmed, row-RLE *palette indices*. That gives
// three things the character upgrade needs:
//   * layers composite at runtime, so equipment changes show immediately;
//   * one baked actor can be recoloured per enemy type from a palette, which
//     is the enemy colour-coding without any extra frames;
//   * every frame shares one cell and one anchor, so nothing can jitter.
//
// If the atlas is missing the engine keeps the previous procedural renderers.
// That fallback is deliberate: it preserves the established look on any build
// where the asset was not shipped.
#include "state.hpp"

namespace av {

constexpr int CHAR_PALETTE_SLOTS = 24;
// Semantic palette slots, matching tools/charparts.py.
enum CharSlot {
  CS_CLEAR = 0,
  CS_OUTLINE = 1,
  CS_SKIN_S = 2, CS_SKIN = 3, CS_SKIN_H = 4,
  CS_CLOTH_S = 5, CS_CLOTH = 6, CS_CLOTH_H = 7,
  CS_LEATHER_S = 8, CS_LEATHER = 9, CS_LEATHER_H = 10,
  CS_METAL_S = 11, CS_METAL = 12, CS_METAL_H = 13,
  CS_ACCENT_S = 14, CS_ACCENT = 15, CS_ACCENT_H = 16,
  CS_HAIR_S = 17, CS_HAIR = 18, CS_HAIR_H = 19,
  CS_FUR_S = 20, CS_FUR = 21, CS_FUR_H = 22,
  CS_FLASH = 23,
};

struct CharPalette {
  C c[CHAR_PALETTE_SLOTS];
};

struct SpriteRect {
  uint8_t ox = 0, oy = 0, w = 0, h = 0;
  uint32_t off = 0, len = 0;
};

struct CharAtlas {
  bool ready = false;
  int cellW = 0, cellH = 0, anchorX = 0, anchorY = 0, slots = 0;
  int dirs = 0;
  std::vector<std::string> variantName, variantSlot, animName;
  std::vector<int> animFrames, animLoop, animFps, animOffset;
  int framesPerVariant = 0;
  std::vector<SpriteRect> index;
  std::vector<uint8_t> blob;

  int variant(const std::string &n) const {
    for (size_t i = 0; i < variantName.size(); i++)
      if (variantName[i] == n)
        return int(i);
    return -1;
  }
  int anim(const std::string &n) const {
    for (size_t i = 0; i < animName.size(); i++)
      if (animName[i] == n)
        return int(i);
    return -1;
  }
  const SpriteRect *at(int v, int a, int d, int f) const {
    if (!ready || v < 0 || a < 0 || d < 0 || d >= dirs || f < 0 ||
        v >= int(variantName.size()) || a >= int(animName.size()) ||
        f >= animFrames[a])
      return nullptr;
    size_t i = size_t(v) * framesPerVariant + animOffset[a] +
               size_t(d) * animFrames[a] + f;
    return i < index.size() ? &index[i] : nullptr;
  }
} charAtlas;

namespace detail {
inline bool rd(std::istream &f, void *p, size_t n) {
  f.read(reinterpret_cast<char *>(p), std::streamsize(n));
  return f.good() || f.gcount() == std::streamsize(n);
}
inline std::string rdStr(std::istream &f) {
  uint8_t n = 0;
  if (!rd(f, &n, 1))
    return {};
  std::string s(n, '\0');
  if (n)
    rd(f, s.data(), n);
  return s;
}
template <class T> inline T rdVal(std::istream &f) {
  T v{};
  rd(f, &v, sizeof(T));
  return v;
}
} // namespace detail

bool loadCharacterAtlas(const std::string &file) {
  using namespace detail;
  CharAtlas a;
  std::ifstream f(file, std::ios::binary);
  if (!f)
    return false;
  char magic[4];
  if (!rd(f, magic, 4) || std::memcmp(magic, "DWCA", 4) != 0)
    return false;
  uint16_t version = rdVal<uint16_t>(f);
  if (version != 1)
    return false;
  a.cellW = rdVal<uint16_t>(f);
  a.cellH = rdVal<uint16_t>(f);
  a.anchorX = rdVal<int16_t>(f);
  a.anchorY = rdVal<int16_t>(f);
  a.slots = rdVal<uint16_t>(f);
  int nv = rdVal<uint16_t>(f), na = rdVal<uint16_t>(f);
  a.dirs = rdVal<uint16_t>(f);
  if (a.cellW <= 0 || a.cellW > 128 || a.cellH <= 0 || a.cellH > 128 ||
      a.slots != CHAR_PALETTE_SLOTS || nv <= 0 || nv > 128 || na <= 0 ||
      na > 64 || a.dirs <= 0 || a.dirs > 8)
    return false;
  for (int i = 0; i < nv; i++) {
    a.variantName.push_back(rdStr(f));
    a.variantSlot.push_back(rdStr(f));
  }
  int total = 0;
  for (int i = 0; i < na; i++) {
    a.animName.push_back(rdStr(f));
    int fr = rdVal<uint8_t>(f), lp = rdVal<uint8_t>(f), fps = rdVal<uint8_t>(f);
    if (fr <= 0 || fr > 64 || fps <= 0)
      return false;
    a.animFrames.push_back(fr);
    a.animLoop.push_back(lp);
    a.animFps.push_back(fps);
    a.animOffset.push_back(total);
    total += fr * a.dirs;
  }
  a.framesPerVariant = total;
  a.index.resize(size_t(nv) * total);
  for (auto &r : a.index) {
    r.ox = rdVal<uint8_t>(f);
    r.oy = rdVal<uint8_t>(f);
    r.w = rdVal<uint8_t>(f);
    r.h = rdVal<uint8_t>(f);
    r.off = rdVal<uint32_t>(f);
    r.len = rdVal<uint32_t>(f);
  }
  uint32_t blobLen = rdVal<uint32_t>(f);
  if (!f || blobLen > (64u << 20))
    return false;
  a.blob.resize(blobLen);
  if (blobLen)
    f.read(reinterpret_cast<char *>(a.blob.data()), blobLen);
  if (f.gcount() != std::streamsize(blobLen))
    return false;
  for (const auto &r : a.index)
    if (size_t(r.off) + r.len > a.blob.size() || r.ox + r.w > a.cellW ||
        r.oy + r.h > a.cellH)
      return false;
  a.ready = true;
  charAtlas = std::move(a);
  return true;
}

// Eight facing directions from five baked ones plus a horizontal mirror.
inline void charFacing(float angle, int &dir, bool &mirror) {
  int s = int(std::lround(angle / (PI / 4))) & 7; // 0=E 1=SE 2=S 3=SW ...
  static const int map[8] = {2, 1, 0, 1, 2, 3, 4, 3};
  static const bool mir[8] = {false, false, false, true,
                              true,  true,  false, false};
  dir = map[s];
  mirror = mir[s];
}

// Blit one layer. (px,py) is the anchor: centre of the feet, in screen pixels.
void drawCharLayer(int variant, int anim, int dir, int frame, bool mirror,
                   int px, int py, const CharPalette &pal, int flash = 0,
                   int alpha = 255) {
  const SpriteRect *r = charAtlas.at(variant, anim, dir, frame);
  if (!r || !r->w || alpha <= 0)
    return;
  const uint8_t *d = charAtlas.blob.data() + r->off;
  const uint8_t *end = d + r->len;
  int baseX = px - charAtlas.anchorX, baseY = py - charAtlas.anchorY;
  for (int row = 0; row < r->h; row++) {
    int col = 0;
    int y = baseY + r->oy + row;
    while (col < r->w && d + 1 < end + 1 && d + 1 <= end - 1) {
      int run = *d++, idx = *d++;
      if (idx) {
        C col32 = pal.c[idx];
        if (flash > 0)
          col32 = mix(col32, WHITE, std::min(255, flash));
        for (int k = 0; k < run && col + k < r->w; k++) {
          int cx = r->ox + col + k;
          int x = baseX + (mirror ? (charAtlas.cellW - 1 - cx) : cx);
          if (x < 0 || x >= W || y < 0 || y >= H)
            continue;
          if (alpha >= 255)
            pix[y * W + x] = col32;
          else
            pix[y * W + x] = mix(pix[y * W + x], col32, alpha);
        }
      }
      col += run;
    }
  }
}

} // namespace av
