#pragma once
#include "ui7_fonts.hpp"
namespace av {
struct UIHit {
  int action, arg;
  float x, y, w, h, r;
};
struct UIControl {
  float x, y, r;
  int action;
};
struct ClearUI {
#ifdef DW_LEGACY_TEST_UI
  bool enabled = false;
#else
  bool enabled = true;
#endif
  int language = 0, layout = 0, size = 1, opacity = 1, textSize = 0;
  bool labels = true, tips = true, custom = false;
  int tab = 0, page = 0, settingsTab = 0, returnOverlay = 0, helpPage = 0,
      drag = -1, dragFinger = -1, mineFinger = -1, chopFinger = -1;
  bool oldHunt = true;
  float notice = 0;
  std::string message;
  std::array<std::pair<float, float>, 8> offsets{}, draft{};
  std::vector<UIHit> hits;
  std::vector<int> history;
} u;
uint64_t uMapRevision = 0;
constexpr C UBG = 0xff101c22, UCARD = 0xff1b3035, ULINE = 0xff3e5859,
            UINK = 0xffecf0e5, UMUTED = 0xffb1c3be, UACCENT = 0xffd9bd83,
            UACTIVE = 0xff28534e;
bool ui7Active() { return u.enabled; }
int uSize(int n) {
  return n <= 10 ? 10 : n <= 14 ? 14 : n <= 16 ? 16 : n <= 20 ? 20 : 28;
}
std::string uLower(std::string s) {
  for (char &c : s)
    if (c >= 'A' && c <= 'Z')
      c += 32;
  return s;
}
const std::map<std::string, std::string> &uDictionary() {
  static std::map<std::string, std::string> m = []() {
    std::map<std::string, std::string> a;
    for (auto &p : uiTranslations)
      a[uLower(p.first)] = p.second;
    return a;
  }();
  return m;
}
std::string uTranslate(const std::string &s) {
  if (u.language == 0) {
    auto it = uDictionary().find(uLower(s));
    if (it != uDictionary().end())
      return it->second;
  }
  return s;
}
const UIFontEntry *uFont(const std::string &s, int size) {
  static std::map<std::pair<int, std::string>, const UIFontEntry *> f = []() {
    std::map<std::pair<int, std::string>, const UIFontEntry *> a;
    for (auto &e : uiFontEntries)
      a[{e.size, e.key}] = &e;
    return a;
  }();
  auto it = f.find({uSize(size), s});
  if (it == f.end() && s.size() > 1)
    it = f.find({14, s});
  return it == f.end() ? nullptr : it->second;
}
void uPixel(int x, int y, C c, int a) {
  if (x < 0 || x >= W || y < 0 || y >= H || !pix)
    return;
  if(uiOverlayTarget)trackUI(x,y,c,a);
  C p = pix[y * W + x];
  int r = ((p >> 16 & 255) * (255 - a) + (c >> 16 & 255) * a) / 255,
      b = ((p & 255) * (255 - a) + (c & 255) * a) / 255,
      gr = ((p >> 8 & 255) * (255 - a) + (c >> 8 & 255) * a) / 255;
  pix[y * W + x] = 0xff000000 | r << 16 | gr << 8 | b;
}
void uMask(int x, int y, const UIFontEntry &e, C color, int clipWidth = 640) {
  int p = 0;
  for (int i = e.offset; i < e.offset + e.length; i += 2) {
    int n = uiFontBytes[i], a = uiFontBytes[i + 1];
    if (a)
      for (int k = 0; k < n; k++) {
        int idx = p + k;
        if (idx % e.w < clipWidth)
          uPixel(x + idx % e.w, y + idx / e.w, color, a);
      }
    p += n;
  }
}
int uWidth(const std::string &raw, int size = 14) {
  auto s = uTranslate(raw);
  if (auto *f = uFont(s, size))
    return f->w;
  if (s.find_first_of("\xE0\xE1\xE2") != std::string::npos) {
    std::istringstream words(s);
    std::string word;
    int width = 0;
    while (words >> word) {
      if (auto *f = uFont(word, size))
        width += f->w + 4;
      else {
        for (unsigned char c : word)
          if (auto *f = uFont(std::string(1, c), size))
            width += f->w;
        width += 4;
      }
    }
    return std::max(0, width - 4);
  }
  int w = 0;
  for (unsigned char c : s) {
    auto *f = uFont(std::string(1, c), size);
    if (f)
      w += f->w;
  }
  return w;
}
void uText(int x, int y, const std::string &raw, C color = UINK, int size = 14,
           int maxWidth = 620) {
  auto s = uTranslate(raw);
  if (auto *f = uFont(s, size)) {
    if (f->w > maxWidth && size > 10) {
      uText(x, y, raw, color, size > 14 ? 14 : 10, maxWidth);
      return;
    }
    uMask(x, y, *f, color, maxWidth);
    return;
  }
  if (s.find_first_of("\xE0\xE1\xE2") != std::string::npos) {
    std::istringstream words(s);
    std::string word;
    int cx = x;
    while (words >> word) {
      int w = uWidth(word, size);
      if (cx + w > x + maxWidth)
        break;
      if (auto *f = uFont(word, size))
        uMask(cx, y, *f, color, maxWidth - (cx - x));
      else if (word.find_first_of("\xE0\xE1\xE2") == std::string::npos)
        uText(cx, y, word, color, size, maxWidth - (cx - x));
      cx += w + 4;
    }
    return;
  }
  int start = x;
  for (unsigned char c : s) {
    auto *f = uFont(std::string(1, c), size);
    if (!f)
      continue;
    if (x + f->w > start + maxWidth)
      break;
    uMask(x, y, *f, color);
    x += f->w;
  }
}
void uCenter(int x, int y, const std::string &s, C color = UINK,
             int size = 14) {
  uText(x - uWidth(s, size) / 2, y, s, color, size);
}
int ui7TextWidth(const std::string &s, int sc) {
  return uWidth(s, std::min(28, sc * 10));
}
void ui7WorldText(int x, int y, const std::string &s, C c, int sc) {
  uText(x, y, s, c, std::min(28, sc * 10));
}
void uRound(int x, int y, int w, int h, C color, int alpha = 255,
            int radius = 9) {
  for (int yy = 0; yy < h; yy++)
    for (int xx = 0; xx < w; xx++) {
      int dx = xx < radius        ? radius - xx
               : xx >= w - radius ? xx - (w - radius - 1)
                                  : 0,
          dy = yy < radius        ? radius - yy
               : yy >= h - radius ? yy - (h - radius - 1)
                                  : 0;
      if (dx * dx + dy * dy <= radius * radius)
        uPixel(x + xx, y + yy, color, alpha);
    }
}
void uIcon(int x, int y, int kind, C c = UINK) {
  switch (kind) {
  case 1:
    thickLine(x - 8, y + 9, x + 9, y - 10, c, 2);
    line(x - 10, y + 2, x - 2, y + 10, c);
    line(x + 9, y - 10, x + 8, y - 4, c);
    break;
  case 2:
    line(x - 10, y + 7, x, y - 8, c);
    line(x, y - 8, x + 10, y + 7, c);
    line(x - 11, y + 11, x + 11, y + 11, c);
    break;
  case 3:
    circle(x, y, 9, c);
    line(x - 6, y, x + 6, y, c);
    line(x, y - 6, x, y + 6, c);
    break;
  case 4:
    for (int a = -1; a <= 1; a++)
      circle(x + a * 8, y, 2, c, true);
    break;
  case 5:
    thickLine(x - 7, y + 10, x + 7, y - 9, c, 2);
    poly({{x + 1, y - 11}, {x + 12, y - 8}, {x + 10, y + 1}, {x + 2, y - 2}},
         c);
    break;
  case 6:
    poly({{x - 9, y - 10},
          {x + 9, y - 10},
          {x + 8, y + 3},
          {x, y + 12},
          {x - 8, y + 3}},
         c);
    line(x, y - 7, x, y + 7, UCARD);
    break;
  case 7:
    line(x - 10, y + 7, x, y - 8, c);
    line(x, y - 8, x + 10, y + 7, c);
    line(x, y - 8, x, y + 12, c);
    break;
  case 9:
    line(x - 9, y, x + 9, y, c);
    line(x, y - 9, x, y + 9, c);
    break;
  case 10:
    box(x - 10, y - 9, 21, 18, UCARD, c);
    line(x - 3, y - 9, x - 3, y + 9, c);
    line(x + 4, y - 9, x + 4, y + 9, c);
    break;
  case 11:
    rect(x - 7, y - 9, 4, 18, c);
    rect(x + 3, y - 9, 4, 18, c);
    break;
  case 12:
    line(x - 6, y - 6, x + 6, y + 6, c);
    line(x + 6, y - 6, x - 6, y + 6, c);
    break;
  case 16:
    line(x - 9, y - 8, x + 9, y - 8, c);
    rect(x - 3, y - 12, 6, 2, c);
    line(x - 6, y - 6, x - 5, y + 10, c);
    line(x + 6, y - 6, x + 5, y + 10, c);
    line(x - 5, y + 10, x + 5, y + 10, c);
    break;
  default:
    circle(x, y, 8, c);
  }
}
void uButton(int action, const std::string &label, int x, int y, int w,
             int h = 44, int arg = 0, bool selected = false) {
  uRound(x, y, w, h, selected ? UACTIVE : UCARD);
  if (selected)
    rect(x + 12, y + h - 3, w - 24, 2, UACCENT);
  int font = u.textSize ? 16 : 14;
  if (uWidth(label, font) > w - 16)
    font = 14;
  uText(x + std::max(8, (w - uWidth(label, font)) / 2), y + (h - 21) / 2, label,
        selected ? UACCENT : UINK, font, w - 16);
  u.hits.push_back({action, arg, float(x), float(y), float(w), float(h), 0});
}
void uIconButton(int action, int icon, int x, int y, int arg = 0) {
  uRound(x, y, 44, 44, UCARD, 225);
  uIcon(x + 22, y + 22, icon);
  u.hits.push_back({action, arg, float(x), float(y), 44, 44, 0});
}
void uNotice(const std::string &s) {
  u.message = s;
  u.notice = 3;
}
bool uSave() {
  if (g.path.empty())
    return false;
  std::ostringstream s;
  s << "DWUI1 " << u.language << ' ' << u.layout << ' ' << u.size << ' '
    << u.opacity << ' ' << u.textSize << ' ' << u.labels << ' ' << u.tips << ' '
    << u.custom << '\n';
  for (auto p : u.offsets)
    s << p.first << ' ' << p.second << '\n';
  bool ok = atomicWorld(g.path + "/ui7.cfg", s.str());
  if (!ok)
    uNotice("Settings could not be saved");
  return ok;
}
std::array<UIControl, 8> uControls(bool draft = false) {
  std::array<UIControl, 8> a{{{574, 285, 36, 1},
                              {492, 313, 26, 2},
                              {493, 240, 27, 3},
                              {579, 199, 23, 4},
                              {493, 236, 26, 5},
                              {574, 207, 25, 6},
                              {416, 315, 25, 7},
                              {74, 284, 44, 8}}};
  if (u.layout == 1) {
    a[2] = {415, 243, 25, 3};
    a[3] = {579, 136, 23, 4};
  }
  if (u.layout == 3) {
    a[0] = {571, 285, 41, 1};
    a[1] = {473, 313, 29, 2};
    a[2] = {475, 236, 30, 3};
    a[3] = {579, 190, 24, 4};
  }
  float scale = u.size == 0 ? .9f : u.size == 2 ? 1.12f : 1;
  for (int i = 0; i < 8; i++) {
    if (i != 7)
      a[i].r = std::max(22.f, a[i].r * scale);
    if (u.layout == 2)
      a[i].x = W - a[i].x;
    auto d = draft ? u.draft[i] : u.offsets[i];
    if (u.custom || draft) {
      a[i].x += d.first;
      a[i].y += d.second;
    }
  }
  return a;
}
bool uVisible(int i) { return i < 4 || i == 7 || u.layout == 1; }
bool uValidLayout(bool draft = false) {
  auto a = uControls(draft);
  for (int i = 0; i < 8; i++)
    if (uVisible(i)) {
      auto b = a[i];
      if (!std::isfinite(b.x) || !std::isfinite(b.y) || b.x - b.r < 8 ||
          b.x + b.r > W - 8 || b.y - b.r < 96 || b.y + b.r > H - 6)
        return false;
      if (b.x > 200 && b.x < 400 && b.y < 230)
        return false;
      for (int k = 0; k < i; k++)
        if (uVisible(k) && len(b.x - a[k].x, b.y - a[k].y) < b.r + a[k].r + 6)
          return false;
    }
  return true;
}
void ui7Boot() {
  vistaLoad();
  bool enabled = u.enabled;
  u = ClearUI{};
  u.enabled = enabled;
  std::string s;
  if ((readChecked(g.path + "/ui7.cfg", s) ||
       readChecked(g.path + "/ui7.cfg.bak", s)) &&
      s.size() < 4096) {
    std::istringstream in(s);
    std::string tag;
    int lang, layout, size, alpha, text, labels, tips, custom;
    std::array<std::pair<float, float>, 8> offsets{};
    bool ok = bool(in >> tag >> lang >> layout >> size >> alpha >> text >>
                   labels >> tips >> custom) &&
              tag == "DWUI1" && lang >= 0 && lang <= 1 && layout >= 0 &&
              layout <= 3 && size >= 0 && size <= 2 && alpha >= 0 &&
              alpha <= 2 && text >= 0 && text <= 1 && labels >= 0 &&
              labels <= 1 && tips >= 0 && tips <= 1 && custom >= 0 &&
              custom <= 1;
    for (auto &p : offsets)
      ok = bool(in >> p.first >> p.second) && ok && std::isfinite(p.first) &&
           std::isfinite(p.second) && std::abs(p.first) < 640 &&
           std::abs(p.second) < 360;
    in >> std::ws;
    ok = ok && in.eof();
    if (ok) {
      u.language = lang;
      u.layout = layout;
      u.size = size;
      u.opacity = alpha;
      u.textSize = text;
      u.labels = labels;
      u.tips = tips;
      u.custom = custom;
      u.offsets = offsets;
      if (!uValidLayout()) {
        u.custom = false;
        u.offsets = {};
        u.size = 1;
      }
    }
  }
}
void ui7InputClear() {
  if (u.chopFinger >= 0)
    v.hunting = u.oldHunt;
  if (u.enabled)
    j.toolAnim = 0;
  u.chopFinger = u.mineFinger = -1;
  u.drag = u.dragFinger = -1;
}
void ui7Step(float dt) {
  u.notice = std::max(0.f, u.notice - dt);
  if (u.mineFinger >= 0 && g.scene == PLAY && !g.overlay && g.hp > 0)
    harvest();
  if (g.openWorld && g.scene == PLAY && !g.overlay && o.waypoint &&
      std::hypot(o.waypointX - globalX(), o.waypointY - globalY()) < 3) {
    o.waypoint = false;
    uNotice("Destination reached");
  }
}
int uContext() {
  if (!g.openWorld)
    return 0;
  for (auto &[k, b] : j.built)
    if ((b.kind == DOOR || b.kind == BED || b.kind == 18) &&
        std::hypot(k.first + .5 - globalX(), k.second + .5 - globalY()) * T <
            43)
      return 1;
  float d = 64;
  int kind = 0;
  for (auto &p : g.props)
    if (p.kind == 0 && !treeCut(p) || p.kind == 1) {
      float dist = len(p.x - g.px, p.y - g.py);
      if (dist < d && dist > 0 &&
          sight(g.px, g.py, p.x - (p.x - g.px) * 19 / dist,
                p.y - (p.y - g.py) * 19 / dist)) {
        d = dist;
        kind = p.kind == 0 ? 3 : 2;
      }
    }
  return kind;
}
std::string uActionLabel(int n) {
  if (n == 1)
    return g.openWorld && j.stance == 2 ? "Stand" : "Attack";
  if (n == 2)
    return "Dodge";
  if (n == 3) {
    if (!g.openWorld)
      return "Strong hit";
    int c = uContext();
    return c == 1 ? "Use" : c == 2 ? "Mine" : c == 3 ? "Chop" : "Jump";
  }
  if (n == 4)
    return "More";
  if (n == 5)
    return "Strong hit";
  if (n == 6)
    return g.openWorld ? "Guard" : "Ward";
  return "Jump";
}
void uDrawControl(UIControl b, bool edit = false) {
  int alpha = u.opacity == 0 ? 85 : u.opacity == 2 ? 225 : 150;
  bool pressed=mediumEnabled()&&((b.action==1&&g.attacking)||(b.action==2&&g.dash>0)||(b.action==3&&(u.mineFinger>=0||u.chopFinger>=0))||(b.action==6&&j.guard>0));
  for (int y = -int(b.r); y <= b.r; y++)
    for (int x = -int(b.r); x <= b.r; x++) {
      float d = x * x + y * y;
      if (d <= b.r * b.r)
        uPixel(int(b.x) + x, int(b.y) + y, pressed?UACTIVE:UCARD, alpha);
    }
  circle(b.x, b.y, b.r, pressed?UACCENT:ULINE);
  if(mediumEnabled()) {
    float cd=b.action==2?g.dodgeCd/.34f:b.action==1?g.attackCd:.0f;
    if(cd>0)arc(int(b.x),int(b.y),int(b.r)-1,-PI/2,-PI/2+2*PI*clamp(cd,0,1),GOLD,2);
  }
  if (b.action == 1)
    circle(b.x, b.y, b.r - 3, UACCENT);
  if (b.action == 8) {
    circle(b.x, b.y, 18, UACCENT);
    circle(b.x + g.mx * 22, b.y + g.my * 22, 14, UACTIVE, true);
    if (u.labels)
      uCenter(b.x, b.y + b.r - 15, "Move", UINK, 10);
  } else {
    uIcon(b.x, b.y - (u.labels ? 8 : 0),
          b.action == 3 ? (uContext() == 0   ? 7
                           : uContext() == 3 ? 5
                                             : 3)
                        : b.action);
    if (u.labels) {
      auto label = uActionLabel(b.action);
      int ww = uWidth(label, 14);
      uRound(b.x - ww / 2 - 3, b.y + 4, ww + 6, 21, UBG, 190, 4);
      uCenter(b.x, b.y + 6, label, UINK, 14);
    }
    if (b.action == 2) {
      uRound(b.x + b.r - 14, b.y - b.r, 18, 18, UBG, 230, 5);
      uText(b.x + b.r - 10, b.y - b.r, num(g.dodgeCharges), UACCENT, 10);
    }
  }
  u.hits.push_back({edit ? 800 + b.action : b.action, 0, b.x - b.r, b.y - b.r,
                    b.r * 2, b.r * 2, b.r});
}
void ui7Hud() {
  if(graphicsController.settings().requested==gfx::GraphicsQuality::Medium){
    uRound(220,60,292,22,UBG,240,4);
    uText(231,62,mediumEnabled()?"Medium":"Low",mediumEnabled()?UACCENT:GOLD,10,68);
    uText(301,62,mediumEnabled()?(graphicsCapLabel()):graphicsReason(),UMUTED,10,203);
  }

  if (g.overlay || g.scene == DEAD || g.scene == WIN)
    return;
  uRound(12, 12, 163, 50, UBG, mediumEnabled()?255:208);
  uText(22, 15, num(int(g.hp)) + " / " + num(maxhp()), UINK, 14);
  uText(120, 17, "Lv " + num(g.level), UACCENT, 10);
  uRound(22, 40, 140, 6, ULINE, 255, 3);
  if(mediumEnabled()) {
    static float hpLag=0;static std::string world;
    if(world!=o.id||hpLag<g.hp){hpLag=g.hp;world=o.id;}
    hpLag=std::max(g.hp,hpLag-renderDt*maxhp()*.38f);
    uRound(22,40,std::max(5,int(140*hpLag/maxhp())),6,0xffd4b976,255,3);
  }
  uRound(22, 40, std::max(5, int(140 * g.hp / maxhp())), 6, RED, 255, 3);
  if (g.openWorld) {
    uRound(22, 51, 140, 3, ULINE, 255, 1);
    rect(22, 51, int(140 * v.stamina / 100), 3, UACCENT);
    if (v.food < 35 || v.water < 35) {
      uRound(12, 66, 163, 22, UBG, 210);
      uText(20, 66, v.food < 35 ? "Food" : "Water", GOLD, 14);
    }
  }
  u.hits.push_back(
      {g.openWorld ? 107 : 102, 0, 12, 12, 163,
       float(g.openWorld && (v.food < 35 || v.water < 35) ? 76 : 52), 0});
  uIconButton(106, 10, 529, 12);
  uIconButton(101, 11, 580, 12);
  if (g.openWorld) {
    if(mediumEnabled())uRound(215,9,303,49,UBG,255,4);
    uCenter(340, mediumEnabled()?10:16, o.name, mediumEnabled()?UINK:UMUTED, mediumEnabled()?14:10);
    uCenter(340, 31,
            "Day " + num(int(o.seconds / 1440) + 1) + "  " +
                num(int(worldHour())) + ":" +
                (int(worldHour() * 60) % 60 < 10 ? "0" : "") +
                num(int(worldHour() * 60) % 60),
            mediumEnabled()?UACCENT:UMUTED, mediumEnabled()?14:10);
    if (v.task > 0 || j.restTime > 0) {
      uRound(219, 63, 202, 25, UBG, 225);
      uCenter(320, 65,
              v.task == 1   ? "COOKING MEAT..."
              : v.task == 2 ? "BOILING WATER..."
                            : "Resting",
              UACCENT, 14);
    }
  }
  bool bossNear = false;
  for (auto &e : g.enemies)
    if (e.alive && e.boss() && len(e.x - g.px, e.y - g.py) < 350)
      bossNear = true;
  if (g.openWorld && o.waypoint && !bossNear) {
    double dx = o.waypointX - globalX(), dy = o.waypointY - globalY(),
           d = std::max(1., std::hypot(dx, dy));
    dx /= d;
    dy /= d;
    poly({{279 + int(dx * 9), 56 + int(dy * 9)},
          {279 - int(dx * 5 + dy * 5), 56 - int(dy * 5 - dx * 5)},
          {279 - int(dx * 5 - dy * 5), 56 - int(dy * 5 + dx * 5)}},
         UACCENT);
    uText(297, 49, std::to_string(int64_t(d)) + " tiles", UACCENT, 10);
  }
  auto controls = uControls();
  for (int i = 0; i < 8; i++)
    if (uVisible(i) && !(u.layout == 1 && i == 2 && uContext() == 0) &&
        !(!g.openWorld && i == 6))
      uDrawControl(controls[i]);
  if (u.tips && g.openWorld && o.seconds < 55) {
    const char *tips[] = {uControls()[7].x > 320 ? "Move with the right thumb"
                                                 : "Move with the left thumb",
                          "Hold attack; save some stamina",
                          "Use changes near trees and rocks",
                          "More opens tools and other actions"};
    int t = std::min(3, int(o.seconds / 14));
    uRound(194, 77, 328, 29, UBG, 220);
    uCenter(358, 81, tips[t], UMUTED, 14);
    int idx = t == 0 ? 7 : t == 1 ? 0 : t == 2 ? 2 : 3;
    circle(controls[idx].x, controls[idx].y, controls[idx].r + 3, UACCENT);
  }
  if (g.openWorld && j.active) {
    int x = sx(g.px), y = sy(g.py) + 14;
    rect(x - 24, y, 48, 2, ULINE);
    rect(x - 24, y, int(48 * j.elapsed / g.attackLength), 2, UACCENT);
  }
  for (auto &e : g.enemies)
    if (e.alive && e.boss() && len(e.x - g.px, e.y - g.py) < 350) {
      uRound(227, 51, 180, 5, ULINE);
      uRound(227, 51, std::max(5, int(180 * e.hp / e.maxhp)), 5, RED);
      break;
    }
  if (!g.openWorld && g.scene == HUB)
    uButton(700, "Enter expedition", 219, 286, 210, 48);
  if (!g.openWorld && g.bossKilled)
    uButton(706, "Guardian defeated", 224, 77, 208, 44);
}
void uOpen(int overlay) {
  if (overlay == 6)
    uMapRevision++;
  clearInput();
  if (g.overlay && g.overlay != overlay)
    u.history.push_back(g.overlay);
  else if (!g.overlay)
    u.history.clear();
  u.hits.clear();
  g.overlay = overlay;
  u.page = 0;
  u.tab = 0;
}
void uClose() {
  clearInput();
  if (u.history.empty())
    g.overlay = 0;
  else {
    g.overlay = u.history.back();
    u.history.pop_back();
  }
  u.hits.clear();
  u.page = 0;
  u.tab = 0;
}
void uHeader(const std::string &title) {
  uRound(8, 8, 624, 344, UBG, 250, 14);
  uText(26, 18, title, UINK, 20);
  uIconButton(900, 12, 580, 14);
  line(26, 65, 614, 65, ULINE);
}
std::string uMaterial(int k) {
  static const char *names[] = {
      "Log",   "Stick", "Stone",  "Coal", "Iron ore",  "Iron ingot",
      "Plank", "Torch", "Wall",   "Door", "Workbench", "Furnace",
      "Chest", "Bed",   "Bridge", "Seed", "Grain",     "Dirt"};
  return k >= 0 && k < MATERIALS ? names[k] : "Seed";
}
std::string uRecipe(int k) {
  static const char *names[] = {
      "Planks",    "Sticks",    "Workbench",     "Wood pick", "Stone pick",
      "Iron pick", "Stone axe", "Iron axe",      "Torches",   "Wood walls",
      "Door",      "Furnace",   "Storage chest", "Bed",       "Bridges",
      "Bread",     "Charcoal"};
  return names[std::clamp(k, 0, 16)];
}
void uTabs(const std::vector<std::string> &names, int current, int code = 210) {
  int w = (588 - int(names.size() - 1) * 6) / int(names.size());
  for (int i = 0; i < int(names.size()); i++)
    uButton(code, names[i], 26 + i * (w + 6), 76, w, 44, i, current == i);
}
void uMap() {
  uHeader("Map");
  uRound(26, 80, 346, 251, 0xff060d12);
  if (g.openWorld) {
    static const double scales[] = {.125, .25, .5, 1, 4, 16, 64, 128, 256, 512};
    double scale = scales[std::clamp(w.mapZoom, 0, 9)], px = globalX(),
           py = globalY();
    static std::array<C, 346 * 251> cache{};
    static std::string lastId;
    static uint64_t lastRevision = 0;
    static int zoom = -1;
    static double lastX = 1e30, lastY = 1e30;
    if (lastId != o.id || lastRevision != uMapRevision || zoom != w.mapZoom ||
        lastX != px || lastY != py || w.atlasDirty) {
      for (auto &[k, bits] : w.explored) {
        if (k.first * 32 < px - 175 * scale - 32 ||
            k.first * 32 > px + 175 * scale ||
            k.second * 32 < py - 120 * scale - 32 ||
            k.second * 32 > py + 120 * scale)
          continue;
        for (int a = 0; a < 16; a++) {
          auto z = bits[a];
          while (z) {
            int bit = std::countr_zero(z), n = a * 64 + bit;
            z &= z - 1;
            int64_t x = k.first * 32 + n % 32, y = k.second * 32 + n / 32;
            int xx = 199 + int(std::floor((x - px) / scale)),
                yy = 205 + int(std::floor((y - py) / scale)),
                size = std::max(1, int(1 / scale));
            if (xx + size < 29 || xx >= 369 || yy + size < 83 || yy >= 328)
              continue;
            C color = earthColor(biomeAt(x, y));
            if (scale <= 1) {
              auto t = tileAt(x, y);
              if (t.map == 4)
                color = t.biome == 4 ? 0xffc87952 : 0xff56858a;
              else if (t.map == 3)
                color = 0xffa5a593;
              else if (t.map == 5)
                color = 0xffbe9770;
              else if (t.ground >= 2)
                color = 0xffc6b58b;
              else if (t.map == 2)
                color = mix(color, INK, 50);
            }
            int ax = std::max(29, xx), ay = std::max(83, yy),
                bx = std::min(369, xx + size), by = std::min(328, yy + size);
            rect(ax, ay, bx - ax, by - ay, color);
          }
        }
      }
      for (int y = 0; y < 251; y++)
        for (int x = 0; x < 346; x++)
          cache[y * 346 + x] = pix[(y + 80) * W + x + 26];
      lastId = o.id;
      lastRevision = uMapRevision;
      zoom = w.mapZoom;
      lastX = px;
      lastY = py;
      w.atlasDirty = false;
    } else
      for (int y = 0; y < 251; y++)
        for (int x = 0; x < 346; x++)
          pix[(y + 80) * W + x + 26] = cache[y * 346 + x];
    if (o.waypoint && exploredAt(o.waypointX, o.waypointY)) {
      int xx = 199 + int(std::clamp((o.waypointX - px) / scale, -164., 164.)),
          yy = 205 + int(std::clamp((o.waypointY - py) / scale, -116., 117.));
      circle(xx, yy, 5, UACCENT);
      line(xx - 7, yy, xx + 7, yy, UACCENT);
      line(xx, yy - 7, xx, yy + 7, UACCENT);
    }
    circle(199, 205, 4, UACCENT, true);
    uText(390, 78, "Dark areas are unexplored", UMUTED, 10, 218);
    static const char *names[] = {"Forest", "Desert", "Snow", "Swamp",
                                  "Volcanic"};
    for (int b = 0; b < 5; b++)
      uButton(530, names[b], 390 + (b % 2) * 115, 108 + (b / 2) * 61, 105, 50,
              b);
    uButton(531, "+", 390, 296, 103, 44, 1);
    uButton(531, "-", 503, 296, 107, 44, -1);
  } else {
    for (int y = 0; y < MH; y++)
      for (int x = 0; x < MW; x++)
        if (g.map[y * MW + x])
          rect(52 + x * 4, 97 + y * 4, 4, 4,
               g.map[y * MW + x] == 1 ? TEAL : EDGE);
    circle(52 + g.px / T * 4, 97 + g.py / T * 4, 4, GOLD, true);
  }
}
void uSettings() {
  uHeader("Settings");
  uTabs({"Layout", "Appearance", "Language", "Graphics"}, u.settingsTab, 220);
  if (u.settingsTab == 0) {
    static const char *names[] = {"Simple", "Combat", "Left-handed",
                                  "Large buttons"};
    static const char *descriptions[] = {
        "Fewer buttons, more space", "All combat actions nearby",
        "Attack left, move right", "Larger targets, easy to tap"};
    for (int i = 0; i < 4; i++) {
      int x = 26 + (i % 2) * 300, y = 128 + (i / 2) * 78;
      uRound(x, y, 288, 68, u.layout == i ? UACTIVE : UCARD);
      uText(x + 14, y + 8, names[i], u.layout == i ? UACCENT : UINK, 16);
      uText(x + 14, y + 37, descriptions[i], UMUTED, 10, 260);
      u.hits.push_back({400, i, float(x), float(y), 288, 68, 0});
      circle(x + 267, y + 22, 6, u.layout == i ? UACCENT : ULINE,
             u.layout == i);
    }
    uButton(401, "Edit positions", 26, 292, 288, 44);
    uButton(409, u.tips ? "Hide tips" : "Show tips", 326, 292, 288, 44);
  } else if (u.settingsTab == 1) {
    const char *names[] = {"Button size",   "Opacity",     "Text size",
                           "Labels",        "Sound",       "Screen shake",
                           "Battery saver", "Motion blur", "Graphics quality",
                           "Focus effect"};
    for (int row = 0; row < 3; row++) {
      int k = u.page * 3 + row;
      if (k > 9)
        break;
      int y = 130 + row * 55;
      uText(40, y + 8, names[k], UINK, u.textSize ? 16 : 14);
      std::string value = k == 0   ? (u.size == 0   ? "Small"
                                      : u.size == 1 ? "Normal"
                                                    : "Large")
                          : k == 1 ? (u.opacity == 0   ? "33%"
                                      : u.opacity == 1 ? "59%"
                                                       : "88%")
                          : k == 2 ? (u.textSize ? "Large" : "Normal")
                          : k == 3 ? (u.labels ? "On" : "Off")
                          : k == 4 ? (g.muted ? "Off" : "On")
                          : k == 5 ? (g.shake ? "On" : "Off")
                          : k == 6 ? (g.lowPower ? "On" : "Off")
                          : k == 7 ? (motionBlur ? "On" : "Off")
                          : k == 8 ? "Open"
                                   : (vistaFocus ? "On" : "Off");
      uButton(402, value, 369, y, 231, 44, k, true);
    }
    uButton(230, "Previous", 26, 296, 130, 44, -1);
    uButton(230, "Next", 484, 296, 130, 44, 1);
    uCenter(320, 303, num(u.page + 1) + " / 4", UMUTED, 14);
  } else if (u.settingsTab == 3) {
    bool requested=graphicsController.settings().requested==gfx::GraphicsQuality::Medium;
    uButton(410,"Low",26,126,288,58,0,!requested);
    uButton(410,"Medium",326,126,288,58,1,requested);
    uText(40,187,"Original look preserved",UMUTED,10,274);
    uText(340,187,"New art and normal lighting",UMUTED,10,274);
    uRound(26,207,588,46,UBG,255);
    uText(40,208,"Active graphics",UMUTED,10,160);
    uText(192,208,mediumEnabled()?"Medium":"Low",mediumEnabled()?UACCENT:GOLD,10,100);
    uText(315,208,graphicsCapLabel(),UMUTED,10,260);
    uCenter(320,226,graphicsReason(),mediumEnabled()?UACCENT:GOLD,10);
    uButton(411,graphicsController.settings().frameCap==60?"60 FPS target":"30 FPS target",26,256,282,44);
    uButton(412,graphicsController.settings().automaticFallback?"Auto fallback: On":"Auto fallback: Off",326,256,288,44);
    uText(40,308,gfx::recommend(graphicsController.device())==gfx::GraphicsQuality::Medium?"Suggested: Medium":"Suggested: Low",UMUTED,10,230);
    uText(296,308,"System thermal",UMUTED,10,120);
    uText(421,308,num(platformThermal),UINK,10,24);
    uText(447,308,gfx::androidThermalName(platformThermal),UINK,10,160);
    uText(40,330,"Thermal protection stays on",UMUTED,10,548);
  } else {
    uText(40, 131, "Settings apply to every world", UMUTED, 14);
    uButton(403, "Bangla", 40, 178, 270, 60, 0, u.language == 0);
    uButton(403, "English", 330, 178, 270, 60, 1, u.language == 1);
    uText(40, 266, "Bigger text in menus", UMUTED, 14);
    uButton(404, "Large", 418, 256, 182, 44, 0, u.textSize);
  }
}
void uCraft() {
  uHeader("Crafting");
  uTabs({"Recipes", "Building", "Materials", "Storage", "Tools"}, u.tab);
  if (u.tab == 0) {
    for (int row = 0; row < 3; row++) {
      int k = u.page * 3 + row;
      if (k >= int(recipes().size()))
        break;
      auto &r = recipes()[k];
      int y = 128 + row * 53;
      uRound(26, y, 588, 48, UCARD);
      uText(40, y + 1, uRecipe(k), UINK, u.textSize ? 16 : 14);
      int x = 40;
      for (int n = 0; n < 3; n++)
        if (r.amount[n]) {
          uText(x, y + 26, uMaterial(r.kind[n]), UMUTED, 10);
          x += uWidth(uMaterial(r.kind[n]), 10) + 5;
          auto qty = num(r.amount[n]);
          uText(x, y + 26, qty,
                materialCount(r.kind[n]) >= r.amount[n] ? TEAL : RED, 10);
          x += uWidth(qty, 10) + 15;
        }
      bool ready = true;
      for (int a = 0; a < 3; a++)
        if (materialCount(r.kind[a]) < r.amount[a])
          ready = false;
      if (r.station && !nearbyStructure(r.station))
        ready = false;
      uButton(300,
              ready ? "Craft"
              : r.station && !nearbyStructure(r.station)
                  ? r.station == BENCH ? "Need workbench" : "Need furnace"
                  : "Need materials",
              449, y, 165, 48, k, ready);
    }
    uButton(230, "Previous", 26, 294, 130, 44, -1);
    uButton(230, "Next", 484, 294, 130, 44, 1);
    uCenter(320, 300, num(u.page + 1) + " / 6", UMUTED, 14);
  } else if (u.tab == 1) {
    static const int kinds[] = {BENCH, FURNACE, CHEST,  BED, WALL,
                                DOOR,  TORCH,   BRIDGE, DIRT};
    for (int row = 0; row < 6; row++) {
      int n = u.page * 6 + row;
      if (n >= 9)
        break;
      int k = kinds[n], x = 26 + (row % 3) * 199, y = 128 + (row / 3) * 61;
      uButton(301, uMaterial(k), x, y, 190, 52, k, j.selectedBuild == k);
      uText(x + 170, y + 33, num(materialCount(k)), UACCENT, 10, 18);
    }
    uButton(302, "Place", 26, 252, 190, 44);
    uButton(303, "Reclaim", 225, 252, 190, 44);
    uButton(304, "Plant", 424, 252, 190, 44);
    uButton(230, "Previous", 26, 300, 130, 44, -1);
    uButton(230, "Next", 484, 300, 130, 44, 1);
    uCenter(320, 311, "Face a clear tile", UMUTED, 10);
  } else if (u.tab == 2) {
    for (int r = 0; r < 12; r++) {
      int k = u.page * 12 + r;
      if (k >= MATERIALS)
        break;
      int x = 26 + (r % 3) * 199, y = 129 + (r / 3) * 38;
      uRound(x, y, 190, 32, UCARD);
      uText(x + 9, y + 4, uMaterial(k), UINK, 14, 139);
      uText(x + 149, y + 4, num(materialCount(k)), UACCENT, 14, 34);
    }
    uButton(230, "Previous", 26, 294, 130, 44, -1);
    uButton(230, "Next", 484, 294, 130, 44, 1);
    uCenter(320, 301, num(occupiedSlots()) + " / 24", UMUTED, 14);
  } else if (u.tab == 4) {
    for (int slot = 0; slot < 2; slot++) {
      int x = 26 + slot * 304, tier = j.toolTier[slot];
      uRound(x, 135, 284, 186, UCARD);
      std::string name =
          slot == 0 ? (tier > 0 ? uRecipe(std::clamp(tier + 2, 3, 5)) : "Mine")
                    : (tier >= 2 ? uRecipe(std::clamp(tier + 4, 6, 7)) : "Axe");
      uText(x + 16, 149, name, UINK, 20);
      uText(x + 16, 190, "Level", UMUTED, 14);
      uText(x + 200, 190, num(tier) + " / 3", UACCENT, 14);
      uText(x + 16, 220, "Durability", UMUTED, 14);
      uText(x + 200, 220, num(j.durability[slot]), UACCENT, 14);
      uButton(210, "Recipes", x + 16, 264, 252, 44, 0);
    }
  } else {
    uButton(305, "Store all", 26, 134, 284, 58);
    uButton(306, "Take all", 330, 134, 284, 58);
    uButton(307, "Smelt ore", 26, 205, 588, 48);
    uText(42, 266, "Nearby furnace", UMUTED, 14);
    uText(42, 294, "Iron ore", UACCENT, 14);
    uText(155, 294, num(j.stock[ORE]) + " + " + num(j.stock[COAL]), UINK, 14);
    uText(240, 294, "Coal", UACCENT, 14);
  }
}
void uBody() {
  uHeader("Body and supplies");
  uTabs({"Supplies", "Injuries", "Advanced"}, u.tab);
  if (u.tab == 0) {
    uText(40, 120, "Food", UMUTED, 14);
    uText(125, 120, num(int(v.food)) + "%", UACCENT, 14);
    uText(240, 120, "Water", UMUTED, 14);
    uText(323, 120, num(int(v.water)) + "%", UACCENT, 14);
    uText(445, 120, "Health", UMUTED, 14);
    uText(524, 120, num(int(g.hp)), UACCENT, 14);
    for (int i = 0; i < 3; i++) {
      int x = i == 0 ? 40 : i == 1 ? 240 : 445;
      float value = i == 0   ? v.food
                    : i == 1 ? v.water
                             : 100.f * g.hp / std::max(1, maxhp());
      rect(x, 146, 150, 4, ULINE);
      rect(x, 146, int(150 * std::clamp(value / 100.f, 0.f, 1.f)), 4,
           i == 0   ? GOLD
           : i == 1 ? TEAL
                    : RED);
    }
    const char *labels[] = {"Eat",  "Drink",         "Bandage", "Splint",
                            "Cook", "Refill / boil", "Forage",  "Rest"};
    int counts[] = {v.meals,   v.cleanWater, v.bandages, v.splints,
                    v.rawMeat, v.dirtyWater, -1,         -1};
    for (int a = 0; a < 8; a++) {
      int x = 26 + (a % 2) * 300, y = 154 + (a / 2) * 48;
      uButton(500, labels[a], x, y, 288, 44, a);
      if (counts[a] >= 0)
        uText(x + 248, y + 13, num(counts[a]), UACCENT, 14, 33);
    }
  } else if (u.tab == 1) {
    const char *names[] = {"Head",      "Torso",    "Left arm",
                           "Right arm", "Left leg", "Right leg"};
    for (int a = 0; a < 6; a++) {
      int x = 26 + (a % 3) * 199, y = 129 + (a / 3) * 63;
      uButton(501, names[a], x, y, 190, 56, a, v.selectedPart == a);
      rect(x + 14, y + 44, 162, 4, ULINE);
      rect(x + 14, y + 44, int(162 * v.injury[a] / 100), 4, RED);
      if (v.bleeding[a] > 0)
        circle(x + 173, y + 14, 4, RED, true);
    }
    uButton(500, "Bandage", 26, 266, 284, 58, 2);
    uButton(500, "Splint", 330, 266, 284, 58, 3);
  } else {
    uButton(502, "Forge", 26, 132, 284, 58, 0);
    uButton(502, "Splint", 330, 132, 284, 58, 1);
    uButton(502, "Campfire", 26, 204, 284, 58, 2);
    uButton(108, "Journal", 330, 204, 284, 58);
    uText(40, 291, "Meals", UMUTED, 14);
    uText(133, 291, num(v.meals), UACCENT, 14);
    uText(210, 291, "Clean water", UMUTED, 14);
    uText(380, 291, num(v.cleanWater), UACCENT, 14);
  }
}
void uBag() {
  uHeader("Equipment");
  uText(331, 26, "Coins", UMUTED, 14);
  uText(438, 26, num(g.gold), UACCENT, 14);
  for (int row = 0; row < 3; row++) {
    int k = u.page * 3 + row;
    if (k >= int(g.bag.size()))
      break;
    auto item = g.bag[k];
    int y = 82 + row * 65;
    uRound(26, y, 588, 58, UCARD);
    uText(42, y + 3, itemName(item), rarityColor(item.rarity), 14, 240);
    uText(42, y + 29, "+" + num(item.value), UMUTED, 14);
    uButton(510, equipped(item) ? "Equipped" : "Equip", 299, y + 7, 150, 44, k,
            equipped(item));
    uButton(511, "Salvage", 459, y + 7, 145, 44, k);
  }
  uButton(230, "Previous", 26, 296, 130, 44, -1);
  uButton(230, "Next", 484, 296, 130, 44, 1);
  uCenter(320, 303, num(u.page + 1), UMUTED, 14);
}
void uWeapons() {
  uHeader("Weapons");
  const char *names[] = {"Sword", "Axe", "Bow"};
  const char *desc[] = {"Quick strikes", "Slow, powerful hits",
                        "Attack from a distance"};
  for (int k = 0; k < 3; k++) {
    int x = 26 + k * 199;
    uRound(x, 88, 190, 176, g.weapon == k ? UACTIVE : UCARD);
    uIcon(x + 95, 124, k == 1 ? 5 : 1, UACCENT);
    uCenter(x + 95, 153, names[k], UINK, 20);
    uText(x + 12, 191, desc[k], UMUTED, 10, 166);
    uButton(520, "Equip", x + 12, 211, 166, 44, k, g.weapon == k);
  }
  if (g.openWorld) {
    uText(44, 281, "Hunting", UMUTED, 16);
    uButton(521, v.hunting ? "On" : "Off", 344, 274, 254, 48);
  } else
    uButton(900, "Back", 203, 277, 234, 48);
}
void uHelp() {
  uHeader("How to play");
  uTabs({"Learn the controls", "Survival basics"}, u.helpPage, 222);
  const char *first[] = {uControls()[7].x > 320 ? "Move with the right thumb"
                                                : "Move with the left thumb",
                         "Hold attack; save some stamina",
                         "Use changes near trees and rocks",
                         "More opens tools and other actions",
                         "Tap health bars for food and injuries",
                         "Choose a layout in Settings"};
  const char *second[] = {"Gather wood, then craft a workbench",
                          "Eat, drink and treat bleeding",
                          "Build a bed to set your return point",
                          "Unexplored places stay black",
                          "Game is paused in menus",
                          "Each world saves separately"};
  for (int r = 0; r < 6; r++) {
    uText(34, 128 + r * 33, num(r + 1), UACCENT, 14);
    uText(62, 128 + r * 33, u.helpPage ? second[r] : first[r], UINK,
          u.textSize ? 16 : 14, 535);
  }
}
void uQuick() {
  uHeader("Actions");
  if (!g.openWorld) {
    const char *names[] = {"Power skill", "Ward",        "Heal",
                           "Weapons",     "Equipment",   "Map",
                           "Settings",    "How to play", "Pause"};
    int actions[] = {605, 606, 612, 105, 102, 106, 112, 103, 101};
    for (int k = 0; k < 9; k++)
      uButton(actions[k], names[k], 26 + (k % 3) * 199, 84 + (k / 3) * 82, 190,
              64);
    return;
  }
  const char *names[] = {"Jump",
                         "Guard",
                         "Heavy",
                         "Weapons",
                         j.stance == 0   ? "Crouch"
                         : j.stance == 1 ? "Prone"
                                         : "Stand",
                         v.sprint ? "Walk" : "Sprint",
                         "Crafting",
                         "Body and supplies",
                         "Heal",
                         "Mine",
                         "Bag",
                         "Building"};
  int actions[] = {607, 606, 605, 105, 610, 611, 110, 107, 612, 613, 102, 614};
  for (int k = 0; k < 12; k++)
    uButton(actions[k], names[k], 26 + (k % 3) * 199, 82 + (k / 3) * 64, 190,
            54);
}
void uPause() {
  uHeader("Paused");
  const char *names[] = {"Resume",    "Controls", "Settings", "How to play",
                         "Equipment", "Map",      "Feedback", "Save and exit"};
  int codes[] = {901, 112, 112, 103, 102, 106, 630, 631};
  for (int a = 0; a < 8; a++)
    uButton(codes[a], names[a], 26 + (a % 2) * 300, 82 + (a / 2) * 64, 288, 54,
            a == 2 ? 1 : 0);
}
void uJournal() {
  uHeader("Journal");
  uText(38, 93, "Predators and prey share this world", UINK, 16);
  uText(38, 126, "Camp creatures have daily routines", UMUTED, 14);
  int row = 0;
  for (auto &a : g.animals) {
    if (row >= 5)
      break;
    uText(40, 167 + row * 31, speciesName(a.species), UACCENT, 14);
    uText(269, 167 + row * 31, behaviourName(a.behaviour), UMUTED, 14);
    row++;
  }
  if (!row)
    uText(40, 179, "No items here", UMUTED, 14);
}
void uEdit() {
  uRound(8, 8, 624, 87, UBG, 242);
  uText(24, 16, "Drag controls to move them", UINK, 16);
  uButton(405, "Reset", 300, 43, 94, 44);
  uButton(406, "Apply", 403, 43, 99, 44);
  uButton(407, "Cancel", 511, 43, 104, 44);
  auto a = uControls(true);
  for (int i = 0; i < 8; i++)
    if (uVisible(i))
      uDrawControl(a[i], true);
}
void uMessageBox(const std::string &raw) {
  std::string message = raw;
  if (raw.rfind("CRAFTED ", 0) == 0)
    message = uTranslate("Made") + " " + uTranslate(raw.substr(8));
  if (raw.rfind("PLACED ", 0) == 0)
    message = uTranslate("Building") + " / " + uTranslate(raw.substr(7));
  if (raw.size() > 9 && raw.substr(raw.size() - 9) == " EQUIPPED")
    message = raw.substr(0, raw.size() - 9) + " / " + uTranslate("Equipped");
  if (message.rfind("NEW WORLD", 0) == 0)
    message = "Tap health bars for food and injuries";
  bool menu = g.overlay && g.overlay != 13 || g.scene == WORLDS ||
              g.scene == CREATE || g.scene == DELETE_WORLD || g.scene == DEAD ||
              g.scene == WIN;
  int x = menu ? 257 : 64, y = menu ? 16 : 71, w = menu ? 310 : 510;
  uRound(x - 8, y - 5, w + 16, menu ? 40 : 31, UBG, 248);
  std::istringstream words(uTranslate(message));
  std::string word;
  int cx = x, line = 0;
  while (words >> word) {
    int ww = uWidth(word, 10);
    if (cx + ww > x + w) {
      line++;
      cx = x;
    }
    if (line > 1)
      break;
    uText(cx, y + line * 15, word, UACCENT, 10, w - (cx - x));
    cx += ww + 4;
  }
}
void uFront() {
  panorama(true);
  if (g.scene == SPLASH) {
    crystal(320, 120, clamp(o.screenTime / 1.2f, 0, 1), 29);
    uCenter(320, 215, "OBSIDIAN GAMES", UINK, 28);
    uCenter(320, 262, "OBSIDIAN SYNDICATE", UMUTED, 14);
    return;
  }
  if (g.scene == HOME || g.scene == TITLE) {
    uText(40, 29, "OBSIDIAN GAMES", UACCENT, 14);
    uText(40, 70, "DEATH WORLD", UINK, 28);
    uText(40, 117, "Explore. Build. Survive.", UMUTED, 16);
    uButton(640, "Your worlds", 40, 177, 302, 61, 0, true);
    uButton(112, "Settings", 40, 253, 146, 46);
    uButton(103, "How to play", 196, 253, 146, 46);
    uButton(642, "Classic", 390, 253, 107, 46);
    uButton(630, "Feedback", 507, 253, 107, 46);
    uText(40, 326, "0.8 / CLEAR PLAY", UMUTED, 10);
    return;
  }
  if (g.scene == WORLDS) {
    uHeader("Your worlds");
    uButton(641, "New world", 26, 77, 588, 46, 0, true);
    if (o.worlds.empty()) {
      uCenter(320, 175, "Create your first world", UACCENT, 20);
      uCenter(320, 215, "Each world saves separately", UMUTED, 14);
    }
    for (int row = 0; row < 2; row++) {
      int index = o.listPage * 2 + row;
      if (index >= int(o.worlds.size()))
        break;
      auto &r = o.worlds[index];
      int y = 136 + row * 76;
      uRound(26, y, 530, 64, UCARD);
      uText(42, y + 4, r.name, UINK, 16, 498);
      uText(42, y + 34,
            "Lv " + num(r.level) + "  |  " + num(int(r.seconds / 60)) +
                " min  |  " + std::to_string(r.seed),
            UMUTED, 10, 495);
      u.hits.push_back({643, index, 26, float(y), 530, 64, 0});
      uIconButton(644, 16, 566, y + 10, index);
    }
    uButton(645, "Previous", 26, 296, 130, 44, -1);
    uButton(645, "Next", 484, 296, 130, 44, 1);
    uCenter(320, 304,
            num(o.listPage + 1) + " / " +
                num(std::max(1, (int(o.worlds.size()) + 1) / 2)),
            UMUTED, 14);
    return;
  }
  if (g.scene == CREATE) {
    uHeader("Create world");
    uText(40, 77, "World name", UMUTED, 14);
    uButton(646, o.draftName, 40, 102, 560, 44, 1);
    uText(40, 154, "Seed number", UMUTED, 14);
    uButton(646, std::to_string(o.draftSeed), 40, 179, 353, 44, 2);
    uButton(647, "Random seed", 405, 179, 195, 44);
    uButton(648, g.difficulty ? "Veteran" : "Adventurer", 40, 240, 272, 44);
    const char *weapons[] = {"Sword", "Axe", "Bow"};
    uButton(649, weapons[g.weapon], 328, 240, 272, 44);
    uButton(650, "Cancel", 40, 296, 150, 44);
    uButton(651, "Start adventure", 202, 296, 398, 44, 0, true);
    return;
  }
  if (g.scene == DELETE_WORLD) {
    uHeader("Delete world?");
    if (o.selected >= 0 && o.selected < int(o.worlds.size()))
      uCenter(320, 124, o.worlds[o.selected].name, UACCENT, 20);
    uCenter(320, 177, "This cannot be undone", RED, 16);
    uButton(650, "Keep world", 40, 253, 272, 60);
    uButton(652, "Delete", 328, 253, 272, 60);
    return;
  }
  if (g.scene == LOADING) {
    crystal(320, 120, 1, 25);
    uCenter(320, 220, "Loading your world", UINK, 20);
  }
}
void uClassicWorlds() {
  uHeader("Classic expedition");
  uButton(701, "Forest realm", 40, 104, 272, 120, 0);
  uButton(701, "Ember realm", 328, 104, 272, 120, 1);
  uButton(648, g.difficulty ? "Veteran" : "Adventurer", 177, 272, 286, 50);
}
void uResult() {
  uHeader(g.scene == WIN ? "Guardian defeated" : "You have fallen");
  uCenter(320, 139, "Your progress is saved", UMUTED, 16);
  uButton(702,
          g.openWorld ? (j.home ? "Return to bed" : "Return to camp")
                      : "Return to camp",
          138, 218, 364, 68, 0, true);
}
void ui7Render() {
  u.hits.clear();
  if (g.scene >= SPLASH || g.scene == TITLE)
    uFront();
  else
    worldScene();
  if (g.overlay) {
    u.hits.clear();
    shade(0, 0, W, H, UBG, 120);
    switch (g.overlay) {
    case 1:
      uPause();
      break;
    case 2:
      uBag();
      break;
    case 3:
      uHelp();
      break;
    case 4:
      uClassicWorlds();
      break;
    case 5:
      uWeapons();
      break;
    case 6:
      uMap();
      break;
    case 7:
      uBody();
      break;
    case 8:
      uJournal();
      break;
    case 9:
    case 12:
      uSettings();
      break;
    case 10:
      uCraft();
      break;
    case 11:
      uQuick();
      break;
    case 13:
      uEdit();
      break;
    default:
      uPause();
    }
  } else if (g.scene == DEAD || g.scene == WIN) {
    u.hits.clear();
    shade(0, 0, W, H, UBG, 150);
    uResult();
  }
  if (u.notice > 0)
    uMessageBox(u.message);
  else if (g.toastTime > 0)
    uMessageBox(g.toast);
  if (o.transition > 0) {
    float a = o.transition > .22f ? (.44f - o.transition) / .22f
                                  : o.transition / .22f;
    shade(0, 0, W, H, UBG, int(clamp(a, 0, 1) * 255));
  }
}
void uPlayAction(int a, int finger) {
  if (g.scene != PLAY && g.scene != HUB)
    return;
  if (a == 1) {
    if (g.openWorld && j.stance == 2) {
      cycleStance();
      return;
    }
    g.attackFinger = finger;
    g.attacking = true;
    attack();
  } else if (a == 2)
    skill(1);
  else if (a == 5) {
    if (g.openWorld)
      beginStrike(true);
    else
      skill(0);
  } else if (a == 6) {
    if (g.openWorld) {
      beginGuard();
      j.guardFinger = finger;
    } else
      skill(2);
  } else if (a == 7) {
    if (g.openWorld)
      jumpPlayer();
    else
      skill(1);
  } else if (a == 3) {
    int context = uContext();
    if (context == 1) {
      float best = 43;
      for (auto &[k, b] : j.built) {
        float dx = (k.first + .5 - globalX()) * T,
              dy = (k.second + .5 - globalY()) * T, d = len(dx, dy);
        if ((b.kind == DOOR || b.kind == BED || b.kind == 18) && d < best &&
            d > 0) {
          best = d;
          g.fx = dx / d;
          g.fy = dy / d;
        }
      }
      journeyInteract();
    } else if (context == 2 || context == 3) {
      float best = 65;
      for (auto &p : g.props)
        if (p.kind == (context == 2 ? 1 : 0)) {
          float dx = p.x - g.px, dy = p.y - g.py, d = len(dx, dy);
          if (d < best && d > 0 &&
              sight(g.px, g.py, p.x - dx * 19 / d, p.y - dy * 19 / d)) {
            best = d;
            g.fx = g.aimx = dx / d;
            g.fy = g.aimy = dy / d;
          }
        }
      if (context == 2) {
        u.mineFinger = finger;
        harvest();
      } else {
        switchWeapon(AXE);
        u.oldHunt = v.hunting;
        v.hunting = false;
        u.chopFinger = finger;
        g.attackFinger = finger;
        g.attacking = true;
        attack();
      }
    } else if (g.openWorld)
      jumpPlayer();
    else
      skill(0);
  }
}
void uAction(int a, int arg, int finger) {
  if (a >= 100)
    sfx(12);
  if (a >= 100 && a <= 113) {
    uOpen(a - 100);
    if (a == 112)
      u.settingsTab = arg;
    return;
  }
  if (a >= 1 && a <= 7) {
    if (a == 4)
      uOpen(11);
    else
      uPlayAction(a, finger);
    return;
  }
  if (a == 900) {
    if (g.overlay)
      uClose();
    else
      ui7Back();
  } else if (a == 901) {
    clearInput();
    g.overlay = 0;
    u.history.clear();
  } else if (a == 210) {
    u.tab = arg;
    u.page = 0;
  } else if (a == 220) {
    u.settingsTab = arg;
    u.page = 0;
  } else if (a == 222)
    u.helpPage = arg;
  else if (a == 230) {
    int maxPage = g.overlay == 10  ? (u.tab == 0                 ? 5
                                      : u.tab == 1 || u.tab == 2 ? 1
                                                                 : 0)
                  : g.overlay == 2 ? std::max(0, (int(g.bag.size()) - 1) / 3)
                                   : 3;
    u.page = std::clamp(u.page + arg, 0, maxPage);
  } else if (a == 300) {
    craftRecipe(arg);
  } else if (a == 301) {
    j.selectedBuild = arg;
  } else if (a == 302 || a == 303 || a == 304) {
    g.overlay = 0;
    u.history.clear();
    clearInput();
    if (a == 302)
      placeStructure();
    if (a == 303)
      reclaimStructure();
    if (a == 304)
      plantSeed();
  } else if (a == 305)
    storeMaterials(false);
  else if (a == 306)
    storeMaterials(true);
  else if (a == 307)
    smeltOre();
  else if (a == 400) {
    clearInput();
    u.layout = arg;
    u.offsets = {};
    u.custom = false;
    if (!uValidLayout())
      u.size = 1;
    uSave();
  } else if (a == 401) {
    u.draft = u.offsets;
    uOpen(13);
  } else if (a == 402) {
    if (arg == 0) {
      int old = u.size;
      u.size = (u.size + 1) % 3;
      if (!uValidLayout()) {
        u.size = old;
        uNotice("Keep buttons apart");
      }
    }
    if (arg == 1)
      u.opacity = (u.opacity + 1) % 3;
    if (arg == 2)
      u.textSize = 1 - u.textSize;
    if (arg == 3)
      u.labels = !u.labels;
    if (arg == 4) {
      g.muted = !g.muted;
      g.sfxCount = 0;
    }
    if (arg == 5)
      g.shake = !g.shake;
    if (arg == 6)
      g.lowPower = !g.lowPower;
    if (arg == 7)
      motionBlur = !motionBlur;
    if (arg == 8) {
      u.settingsTab=3; u.page=0;
    }
    if (arg == 9) {
      vistaFocus = !vistaFocus;
      vistaSave();
      graphicsController.setLegacyAppearance({vistaQuality,vistaFocus});
      graphicsSave();
    }
    uSave();
    saveSettings();
  } else if (a == 410 || a == 411 || a == 412) {
    clearInput();j.guardFinger=-1;j.guard=0;j.queue=0;
    if(a==410)graphicsController.request(arg?gfx::GraphicsQuality::Medium:gfx::GraphicsQuality::Low);
    if(a==411)graphicsController.setFrameCap(graphicsController.settings().frameCap==60?30:60);
    if(a==412)graphicsController.setAutomaticFallback(!graphicsController.settings().automaticFallback);
    if(!graphicsSave())uNotice("Settings could not be saved");
  } else if (a == 403) {
    u.language = arg;
    uSave();
  } else if (a == 404) {
    u.textSize = !u.textSize;
    uSave();
  } else if (a == 405)
    u.draft = {};
  else if (a == 406) {
    if (uValidLayout(true)) {
      u.offsets = u.draft;
      u.custom = true;
      if (uSave()) {
        uClose();
        uNotice("Layout saved");
      }
    } else
      uNotice("Keep buttons apart and the center clear");
  } else if (a == 407)
    uClose();
  else if (a == 409) {
    u.tips = !u.tips;
    uSave();
  } else if (a == 500) {
    if (arg == 0)
      eatMeal();
    if (arg == 1)
      drinkWater();
    if (arg == 2)
      bandagePart();
    if (arg == 3)
      splintPart();
    if (arg == 4)
      beginTask(1);
    if (arg == 5) {
      if (nearFire() && v.dirtyWater > 0)
        beginTask(2);
      else
        fillWater();
    }
    if (arg == 6)
      forage();
    if (arg == 7)
      beginTask(3);
  } else if (a == 501)
    v.selectedPart = arg;
  else if (a == 502) {
    if (arg == 1)
      craftSplint();
    if (arg == 2)
      buildCampfire();
    if (arg == 0) {
      if (g.openWorld && (std::abs(globalX()) >= 5 || std::abs(globalY()) >= 5))
        uNotice("Forge at origin camp");
      else {
        int cost = 20 + g.upgrade * 15;
        if (g.gold >= cost && g.upgrade < 50) {
          g.gold -= cost;
          g.upgrade++;
          o.dirty = true;
          save();
          uNotice("Damage increased");
        } else
          uNotice("Need coins");
      }
    }
  } else if (a == 510 && arg >= 0 && arg < int(g.bag.size())) {
    auto item = g.bag[arg];
    g.eq[item.slot] = item.id;
    g.hp = std::min(g.hp, float(maxhp()));
    save();
  } else if (a == 511 && arg >= 0 && arg < int(g.bag.size()) &&
             !equipped(g.bag[arg])) {
    g.gold += 10 + g.bag[arg].value * 3;
    g.bag.erase(g.bag.begin() + arg);
    u.page = std::min(u.page, std::max(0, (int(g.bag.size()) - 1) / 3));
    save();
  } else if (a == 520) {
    switchWeapon(arg);
    clearInput();
    g.overlay = 0;
    u.history.clear();
  } else if (a == 521)
    v.hunting = !v.hunting;
  else if (a == 530)
    knownBiomeWaypoint(arg);
  else if (a == 531) {
    w.mapZoom = std::clamp(w.mapZoom - arg, 0, 9);
    w.atlasDirty = true;
  } else if (a >= 605 && a <= 607) {
    g.overlay = 0;
    u.history.clear();
    clearInput();
    uPlayAction(a - 600, finger);
  } else if (a == 610) {
    g.overlay = 0;
    u.history.clear();
    cycleStance();
  } else if (a == 611) {
    v.sprint = !v.sprint;
    g.overlay = 0;
    u.history.clear();
  } else if (a == 612) {
    g.overlay = 0;
    u.history.clear();
    if (g.flasks <= 0)
      uNotice("No flask left");
    else
      skill(3);
  } else if (a == 613) {
    g.overlay = 0;
    u.history.clear();
    u.mineFinger = finger;
    harvest();
  } else if (a == 614) {
    uOpen(10);
    u.tab = 1;
  } else if (a == 630)
    o.textRequest = 3;
  else if (a == 631) {
    uSave();
    u.history.clear();
    if (g.openWorld)
      leaveFrontier();
    else {
      save();
      g.overlay = 0;
      startTransition(HOME);
    }
  } else if (a == 640) {
    scanWorlds();
    startTransition(WORLDS);
  } else if (a == 641) {
    o.draftName = "NEW HORIZON " + num(o.worlds.size() + 1);
    o.draftSeed = entropy();
    startTransition(CREATE);
  } else if (a == 642) {
    std::string path = g.path;
    clearInput();
    g = State{};
    g.path = path;
    if (!loadOne(path + "/progress.sav"))
      loadOne(path + "/progress.sav.bak");
    g.openWorld = false;
    loadSettings();
    hub();
  } else if (a == 643 && arg >= 0 && arg < int(o.worlds.size())) {
    o.selected = arg;
    o.pendingCreate = false;
    startTransition(LOADING);
  } else if (a == 644 && arg >= 0 && arg < int(o.worlds.size())) {
    o.selected = arg;
    startTransition(DELETE_WORLD);
  } else if (a == 645)
    o.listPage = std::clamp(o.listPage + arg, 0,
                            std::max(0, (int(o.worlds.size()) - 1) / 2));
  else if (a == 646)
    o.textRequest = arg;
  else if (a == 647)
    o.draftSeed = entropy();
  else if (a == 648)
    g.difficulty = 1 - g.difficulty;
  else if (a == 649)
    g.weapon = (g.weapon + 1) % 3;
  else if (a == 650)
    startTransition(WORLDS);
  else if (a == 651) {
    o.pendingCreate = true;
    startTransition(LOADING);
  } else if (a == 652 && o.selected >= 0 && o.selected < int(o.worlds.size())) {
    auto id = o.worlds[o.selected].id;
    if (validId(id)) {
      std::error_code ec;
      std::filesystem::remove_all(
          std::filesystem::path(worldFile(id)).parent_path(), ec);
      if (ec)
        uNotice("DELETE FAILED - SAVE KEPT");
    }
    scanWorlds();
    startTransition(WORLDS);
  } else if (a == 700)
    uOpen(4);
  else if (a == 701) {
    g.world = arg;
    expedition();
  } else if (a == 702) {
    u.history.clear();
    g.overlay = 0;
    if (g.openWorld)
      respawnFrontier();
    else
      hub();
  } else if (a == 706) {
    for (auto &d : g.drops) {
      if (d.gear && g.bag.size() < 18)
        g.bag.push_back(d.item);
      else
        g.gold += d.gear ? 15 + d.item.value * 3 : d.gold;
    }
    g.drops.clear();
    g.scene = WIN;
    clearInput();
    save();
  }
  u.hits.clear();
}
bool ui7Back() {
  clearInput();
  if (g.overlay) {
    uClose();
    return true;
  }
  if (g.scene == PLAY || g.scene == HUB) {
    uOpen(1);
    return true;
  }
  if (g.scene == WORLDS || g.scene == SPLASH) {
    startTransition(HOME);
    return true;
  }
  if (g.scene == CREATE || g.scene == DELETE_WORLD) {
    startTransition(WORLDS);
    return true;
  }
  return true;
}
bool ui7Touch(int action, int id, float x, float y) {
  if (action == 3) {
    clearInput();
    return true;
  }
  if (action == 1) {
    if (id == u.chopFinger) {
      v.hunting = u.oldHunt;
      u.chopFinger = -1;
    }
    if (id == u.mineFinger)
      u.mineFinger = -1;
    if (id == g.attackFinger) {
      g.attackFinger = -1;
      g.attacking = false;
    }
    if (id == j.guardFinger) {
      j.guardFinger = -1;
      j.guard = 0;
    }
    if (id == g.joystick) {
      g.joystick = -1;
      g.mx = g.my = 0;
    }
    if (id == u.dragFinger) {
      u.drag = -1;
      u.dragFinger = -1;
    }
    return true;
  }
  if (action == 2) {
    if (id == u.dragFinger && u.drag >= 0) {
      auto a = uControls(true);
      auto &b = a[u.drag];
      float xx = clamp(x, b.r + 8, W - b.r - 8),
            yy = clamp(y, b.r + 96, H - b.r - 6);
      u.draft[u.drag].first += xx - b.x;
      u.draft[u.drag].second += yy - b.y;
    } else if (id == g.joystick) {
      float dx = (x - g.joyx) / 32, dy = (y - g.joyy) / 32, d = len(dx, dy);
      g.mx = d > .12f ? dx / std::max(1.f, d) : 0;
      g.my = d > .12f ? dy / std::max(1.f, d) : 0;
    }
    return true;
  }
  if (action != 0 || o.transition > 0)
    return true;
  if (g.scene == SPLASH) {
    startTransition(HOME);
    return true;
  }
  for (auto it = u.hits.rbegin(); it != u.hits.rend(); ++it) {
    auto b = *it;
    bool inside = b.r > 0
                      ? len(x - (b.x + b.r), y - (b.y + b.r)) <= b.r
                      : x >= b.x && x < b.x + b.w && y >= b.y && y < b.y + b.h;
    if (!inside)
      continue;
    if (b.action >= 801 && b.action <= 808) {
      u.drag = b.action - 801;
      u.dragFinger = id;
      return true;
    }
    if (b.action == 8) {
      if (g.joystick < 0) {
        g.joystick = id;
        g.joyx = b.x + b.r;
        g.joyy = b.y + b.r;
      }
      return true;
    }
    uAction(b.action, b.arg, id);
    return true;
  }
  return true;
}
} // namespace av
