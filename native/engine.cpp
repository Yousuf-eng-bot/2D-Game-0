// Death World 0.7: C++ owns the game. Java is only the Android platform bridge.
#include "frontier_ui.hpp"
#include "ui7.hpp"
namespace av {
bool inside(float x, float y, int a, int b, int w, int h) {
  return x >= a && x < a + w && y >= b && y < b + h;
}
bool at(float x, float y, int a, int b, int r) {
  return len(x - a, y - b) <= r;
}
void joystick(float x, float y) {
  float dx = (x - g.joyx) / 35, dy = (y - g.joyy) / 35, d = len(dx, dy);
  if (d <= .12f)
    g.mx = g.my = 0;
  else {
    float magnitude = clamp((d - .12f) / .88f, 0, 1);
    g.mx = dx / d * magnitude;
    g.my = dy / d * magnitude;
  }
}
void touch(int action, int id, float x, float y) {
  if(ui7Active()){ui7Touch(action,id,x,y);return;}
  if (action == 3) {
    j.guardFinger = -1;
    j.guard = 0;
    j.queue = 0;
    clearInput();
    return;
  }
  if (action == 1) {
    if (id == j.guardFinger) {
      j.guardFinger = -1;
      j.guard = 0;
    }
    if (id == g.joystick) {
      g.joystick = -1;
      g.mx = g.my = 0;
    }
    if (id == g.attackFinger) {
      g.attackFinger = -1;
      g.attacking = false;
    }
    return;
  }
  if (action == 2) {
    if (id == g.joystick)
      joystick(x, y);
    return;
  }
  if (action != 0)
    return;
  if (journeyTouch(x, y, id))
    return;
  if (frontierTouch(x, y))
    return;
  if (g.overlay == 3) {
    if (inside(x, y, 220, 297, 200, 28))
      g.overlay = 0;
    return;
  }
  if (g.overlay == 5) {
    if (inside(x, y, 565, 35, 28, 25)) {
      g.overlay = 0;
      return;
    }
    for (int w = 0; w < 3; w++)
      if (inside(x, y, 48 + w * 184, 87, 176, 229)) {
        switchWeapon(w);
        g.overlay = 0;
        return;
      }
    return;
  }
  if (g.overlay == 4) {
    if (inside(x, y, 564, 34, 25, 25)) {
      g.overlay = 0;
      return;
    }
    if (inside(x, y, 205, 301, 230, 25)) {
      g.difficulty = 1 - g.difficulty;
      save();
      return;
    }
    for (int r = 0; r < 2; r++)
      if (inside(x, y, r ? 335 : 51, 87, 254, 203)) {
        g.world = r;
        expedition();
        return;
      }
    return;
  }
  if (g.overlay == 6) {
    if (inside(x, y, 541, 26, 29, 25))
      g.overlay = 0;
    return;
  }
  if (g.overlay == 1) {
    if (inside(x, y, 204, 79, 232, 31))
      g.overlay = 0;
    else if (inside(x, y, 204, 119, 112, 29)) {
      g.muted = !g.muted;
      g.sfxCount = 0;
      save();
    } else if (inside(x, y, 324, 119, 112, 29)) {
      g.shake = !g.shake;
      save();
    } else if (inside(x, y, 204, 158, 232, 29)) {
      g.lowPower = !g.lowPower;
      save();
    } else if (inside(x, y, 204, 196, 112, 29))
      g.overlay = 5;
    else if (inside(x, y, 324, 196, 112, 29))
      g.overlay = 3;
    else if (inside(x, y, 204, 235, 232, 31)) {
      save();
      g.overlay = 0;
      startTransition(HOME);
    }
    return;
  }
  if (g.overlay == 2) {
    if (inside(x, y, 500, 33, 30, 24)) {
      g.overlay = 0;
      return;
    }
    for (int r = 0; r < 5; r++)
      if (inside(x, y, 111, 85 + r * 32, 418, 29)) {
        int n = g.page * 5 + r;
        if (n < int(g.bag.size()))
          g.selected = n;
        return;
      }
    if (inside(x, y, 111, 278, 98, 29))
      g.page = std::max(0, g.page - 1);
    if (inside(x, y, 431, 278, 98, 29))
      g.page = std::min((int(g.bag.size()) - 1) / 5, g.page + 1);
    if (g.selected >= 0 && g.selected < int(g.bag.size())) {
      auto i = g.bag[g.selected];
      if (inside(x, y, 217, 278, 99, 29) && !equipped(i)) {
        g.eq[i.slot] = i.id;
        g.hp = std::min(g.hp, float(maxhp()));
        sfx(12);
        save();
      }
      if (inside(x, y, 324, 278, 99, 29) && !equipped(i)) {
        g.gold += 10 + i.value * 3;
        g.bag.erase(g.bag.begin() + g.selected);
        g.selected = 0;
        g.page = std::min(g.page, (int(g.bag.size()) - 1) / 5);
        sfx(3);
        save();
      }
    }
    return;
  }
  if (g.scene == TITLE) {
    if (inside(x, y, 32, 252, 241, 39))
      hub();
    if (inside(x, y, 32, 302, 113, 28))
      g.overlay = 3;
    if (inside(x, y, 156, 302, 117, 28))
      g.overlay = 5;
    return;
  }
  if (g.scene == DEAD || g.scene == WIN) {
    if (inside(x, y, 185, 217, 270, 37))
      hub();
    return;
  }
  if (inside(x, y, 435, 10, 43, 27)) {
    clearInput();
    g.overlay = 2;
    return;
  }
  if (inside(x, y, 483, 10, 44, 27)) {
    clearInput();
    g.overlay = 1;
    return;
  }
  if (inside(x, y, 145, 317, 117, 28)) {
    clearInput();
    g.overlay = 5;
    return;
  }
  if (g.scene == HUB) {
    if (inside(x, y, 281, 303, 142, 41)) {
      clearInput();
      g.overlay = 4;
      return;
    }
    if (inside(x, y, 444, 310, 182, 34)) {
      int cost = 20 + g.upgrade * 15;
      if (g.gold >= cost && g.upgrade < 50) {
        g.gold -= cost;
        g.upgrade++;
        sfx(8);
        save();
        notify("FORGED - ALL THREE WEAPONS GAIN +3 ATTACK");
      } else
        notify("HUNT MONSTERS AND OPEN SHRINE CHESTS FOR GOLD");
      return;
    }
  }
  if (g.scene == PLAY) {
    if (inside(x, y, 542, 5, 95, 74)) {
      clearInput();
      g.overlay = 6;
      return;
    }
    if (g.bossKilled && inside(x, y, 278, 308, 146, 35)) {
      for (auto &d : g.drops) {
        if (d.gear) {
          if (g.bag.size() < 18)
            g.bag.push_back(d.item);
          else
            g.gold += 15 + d.item.value * 3;
        } else
          g.gold += d.gold;
      }
      g.drops.clear();
      g.scene = WIN;
      clearInput();
      save();
      return;
    }
    if (at(x, y, 587, 293, 36)) {
      g.attackFinger = id;
      g.attacking = true;
      attack();
      return;
    }
    if (at(x, y, 518, 263, 28)) {
      skill(0);
      return;
    }
    if (at(x, y, 519, 325, 25)) {
      skill(1);
      return;
    }
    if (at(x, y, 585, 218, 25)) {
      skill(2);
      return;
    }
    if (at(x, y, 462, 324, 24)) {
      skill(3);
      return;
    }
  }
  if (x < 170 && y > 198 && g.joystick < 0) {
    g.joystick = id;
    g.joyx = clamp(x, 45, 126);
    g.joyy = clamp(y, 240, 307);
    joystick(x, y);
  }
}
void boot(const std::string &path) {
  g = State{};
  o = Frontier{};
  v = Survival{};
  w = Wildlands{};
  j = Journey{};
  motionBlur = true;
  g.path = path;
  if (!path.empty()) {
    if (!loadOne(path + "/progress.sav"))
      loadOne(path + "/progress.sav.bak");
    loadArtwork(path + "/cover.bin");
    if (!loadCharacterAtlas(path + "/characters.dwa"))
      loadCharacterAtlas("android/assets/characters.dwa");
    bindCharacterAtlas();
  }
  loadSettings();
  ui7Boot();
  graphicsBoot();
  g.scene = SPLASH;
  o.draftSeed = entropy();
  scanWorlds();
  sfx(16);
}
void frame(C *out, float dt) {
  graphicsStartFrame(dt);
  pix = out;
  dt = clamp(dt, 0, .1f);
  renderDt = dt;
  g.acc += dt;
  int steps = 0;
  while (g.acc >= 1.f / 60 && steps++ < 6) {
    g.previousX = g.px;
    g.previousY = g.py;
    for (auto &e : g.enemies) {
      e.previousX = e.x;
      e.previousY = e.y;
    }
    for (auto &a : g.animals) {
      a.previousX = a.x;
      a.previousY = a.y;
    }
    tick(1.f / 60);
    g.acc -= 1.f / 60;
  }
  updatePlayerAnim(dt);
  // Render-only interpolation never writes back into physics or persistent
  // position.
  bool smooth = g.openWorld && g.scene == PLAY && !g.overlay && dt > 0;
  float px = g.px, py = g.py, alpha = clamp(g.acc * 60, 0, 1);
  std::vector<std::pair<float, float>> enemies, animals;
  auto blend = [&](float &x, float &y, float oldx, float oldy) {
    if (len(x - oldx, y - oldy) < 30) {
      x = oldx + (x - oldx) * alpha;
      y = oldy + (y - oldy) * alpha;
    }
  };
  if (smooth) {
    blend(g.px, g.py, g.previousX, g.previousY);
    for (auto &e : g.enemies) {
      enemies.push_back({e.x, e.y});
      blend(e.x, e.y, e.previousX, e.previousY);
    }
    for (auto &a : g.animals) {
      animals.push_back({a.x, a.y});
      blend(a.x, a.y, a.previousX, a.previousY);
    }
  }
  render();
  mediumFinishFrame();
  if (smooth) {
    g.px = px;
    g.py = py;
    for (size_t i = 0; i < enemies.size(); i++) {
      g.enemies[i].x = enemies[i].first;
      g.enemies[i].y = enemies[i].second;
    }
    for (size_t i = 0; i < animals.size(); i++) {
      g.animals[i].x = animals[i].first;
      g.animals[i].y = animals[i].second;
    }
  }
}
void back() {
  if(ui7Active()&&ui7Back())return;
  clearInput();
  if (g.overlay)
    g.overlay = 0;
  else if (g.scene == PLAY || g.scene == HUB)
    g.overlay = 1;
  else if (g.scene == SPLASH || g.scene == WORLDS)
    startTransition(HOME);
  else if (g.scene == CREATE || g.scene == DELETE_WORLD)
    startTransition(WORLDS);
}
void suspend() {
  j.guardFinger = -1;
  j.guard = 0;
  j.queue = 0;
  clearInput();
  g.vx = g.vy = 0;
  g.acc = 0;
  if (g.scene == PLAY)
    g.overlay = 1;
  save();
  if (g.openWorld)
    flushOpenWorld();
}
void submitText(int kind, const std::string &s) {
  if (kind == 1)
    o.draftName = cleanName(s);
  if (kind == 2) {
    try {
      size_t n = 0;
      uint64_t seed = std::stoull(s, &n);
      if (n == s.size() && !s.empty() && s[0] != '-')
        o.draftSeed = seed;
      else
        notify("ENTER AN UNSIGNED WHOLE NUMBER");
    } catch (...) {
      notify("INVALID SEED - CURRENT SEED KEPT");
    }
  }
}
} // namespace av
extern "C" {
void av_boot(const char *p) { av::boot(p ? p : ""); }
void av_frame(uint32_t *p, float dt) { av::frame(p, dt); }
void av_touch(int a, int id, float x, float y) { av::touch(a, id, x, y); }
void av_back() { av::back(); }
void av_suspend() { av::suspend(); }
int av_sound() {
  if (!av::g.sfxCount)
    return 0;
  int s = av::g.sfxQueue[0];
  for (int i = 1; i < av::g.sfxCount; i++)
    av::g.sfxQueue[i - 1] = av::g.sfxQueue[i];
  av::g.sfxCount--;
  return s;
}
int av_audio() {
  if (av::g.muted || av::g.scene >= av::SPLASH)
    return 0;
  return av::g.scene == av::HUB ? 1 : av::g.world + 1;
}
int av_music() {
  return av::g.muted || av::g.scene == av::SPLASH ? 0
         : av::g.scene >= av::HOME                ? 1
                                                  : av::g.world + 2;
}
int av_text_request() {
  int r = av::o.textRequest;
  av::o.textRequest = 0;
  return r;
}
void av_submit_text(int kind, const char *s) {
  av::submitText(kind, s ? s : "");
}
const char *av_text_value(int kind) {
  static std::string v;
  v = kind == 100 ? (av::u.language==0?"bn":"en") : kind == 1 ? av::o.draftName : std::to_string(av::o.draftSeed);
  return v.c_str();
}
const char *av_report() {
  av::graphicsApplySafety();
  static std::string s;
  s = "DEATH WORLD 0.9.2 / THERMAL FIX\nGenerator: " +
      std::to_string(av::o.generator) + "\nWorld: " + av::o.name +
      "\nSeed: " + std::to_string(av::o.seed) +
      "\nPosition: " + std::to_string(int64_t(av::globalX())) + ", " +
      std::to_string(int64_t(av::globalY())) +
      "\nLevel: " + std::to_string(av::g.level) +
      " / weapon: " + av::weaponName(av::g.weapon) +
      "\nFrame cap: " + std::to_string(av::graphicsController.decision().frameCap) +
      " FPS (not measured)" +
      "\nRequested graphics: " + (av::graphicsController.settings().requested==av::gfx::GraphicsQuality::Medium?"Medium":"Low") +
      "\nActive graphics: " + (av::mediumEnabled()?"Medium":"Low") +
      "\nGraphics status: " + av::graphicsReason() +
      "\nUI frame interval estimate (ms): " + std::to_string(av::mediumFrameMs) +
      "\nRAM MiB: " + std::to_string(av::graphicsController.device().ramMiB) +
      "\nBattery saving (game/system): " + std::to_string(av::g.lowPower) + "/" + std::to_string(av::platformBatterySaver) +
      "\nThermal status: " + std::to_string(av::platformThermal) + " (" + av::gfx::androidThermalName(av::platformThermal) + ")" +
      "\nApplied thermal enum: " + std::to_string(int(av::graphicsController.thermal())) +
      "\nProfile revision: " + std::to_string(av::platformProfileRevision) +
      "\nCooling effects budget: " + std::to_string(av::graphicsController.reducedEffects()) +
      "\nMemory pressure: " + std::to_string(av::platformLowMemory) +
      "\n" + av::graphicsBackendInfo() +
      "\nFeedback: controls / music / difficulty / bugs";
  return s.c_str();
}

int av_rate() { av::graphicsApplySafety(); return av::graphicsController.decision().frameCap; }
int av_haptic() {
  int h = av::g.haptic;
  av::g.haptic = 0;
  return h;
}
}
#ifdef __ANDROID__
#include <jni.h>
extern "C" JNIEXPORT void JNICALL
Java_com_ashenveil_game_MainActivity_boot(JNIEnv *e, jclass, jstring p) {
  const char *s = e->GetStringUTFChars(p, nullptr);
  av_boot(s);
  e->ReleaseStringUTFChars(p, s);
}
extern "C" JNIEXPORT void JNICALL Java_com_ashenveil_game_MainActivity_frame(
    JNIEnv *e, jclass, jintArray a, jfloat dt) {
  static uint32_t buffer[640 * 360];
  av_frame(buffer, dt);
  e->SetIntArrayRegion(a, 0, 640 * 360, reinterpret_cast<jint *>(buffer));
}
extern "C" JNIEXPORT void JNICALL Java_com_ashenveil_game_MainActivity_touch(
    JNIEnv *, jclass, jint a, jint id, jfloat x, jfloat y) {
  av_touch(a, id, x, y);
}
extern "C" JNIEXPORT void JNICALL
Java_com_ashenveil_game_MainActivity_suspend(JNIEnv *, jclass) {
  av_suspend();
}
extern "C" JNIEXPORT void JNICALL
Java_com_ashenveil_game_MainActivity_back(JNIEnv *, jclass) {
  av_back();
}
extern "C" JNIEXPORT jint JNICALL
Java_com_ashenveil_game_MainActivity_sound(JNIEnv *, jclass) {
  return av_sound();
}
extern "C" JNIEXPORT jint JNICALL
Java_com_ashenveil_game_MainActivity_rate(JNIEnv *, jclass) {
  return av_rate();
}
extern "C" JNIEXPORT jint JNICALL
Java_com_ashenveil_game_MainActivity_haptic(JNIEnv *, jclass) {
  return av_haptic();
}
extern "C" JNIEXPORT jint JNICALL
Java_com_ashenveil_game_MainActivity_audio(JNIEnv *, jclass) {
  return av_audio();
}
extern "C" JNIEXPORT jint JNICALL
Java_com_ashenveil_game_MainActivity_music(JNIEnv *, jclass) {
  return av_music();
}
extern "C" JNIEXPORT jint JNICALL
Java_com_ashenveil_game_MainActivity_textRequest(JNIEnv *, jclass) {
  return av_text_request();
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_ashenveil_game_MainActivity_textValue(JNIEnv *e, jclass, jint kind) {
  return e->NewStringUTF(av_text_value(kind));
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_ashenveil_game_MainActivity_report(JNIEnv *e, jclass) {
  return e->NewStringUTF(av_report());
}
extern "C" JNIEXPORT void JNICALL
Java_com_ashenveil_game_MainActivity_submitText(JNIEnv *e, jclass, jint kind,
                                                jstring v) {
  const char *s = e->GetStringUTFChars(v, nullptr);
  av_submit_text(kind, s);
  e->ReleaseStringUTFChars(v, s);
}
#endif

#include "gpu_medium.hpp"
#ifdef __ANDROID__
extern "C" JNIEXPORT jboolean JNICALL
Java_com_ashenveil_game_MainActivity_gpuAttach(JNIEnv *env,jclass,jobject surface) {
  ANativeWindow *window=ANativeWindow_fromSurface(env,surface);
  return window&&av::mediumGPU.create(window)?JNI_TRUE:JNI_FALSE;
}
extern "C" JNIEXPORT void JNICALL
Java_com_ashenveil_game_MainActivity_gpuDetach(JNIEnv *,jclass) { av::mediumGPU.destroy(); }
extern "C" JNIEXPORT jboolean JNICALL
Java_com_ashenveil_game_MainActivity_gpuPresent(JNIEnv *,jclass,jfloat dt,jint width,jint height,jfloat ox,jfloat oy,jfloat scale) {
  static std::vector<av::C> buffer(av::W*av::H);
  av::frame(buffer.data(),dt);
  if(av::mediumGPU.draw(buffer.data(),width,height,ox,oy,scale))return JNI_TRUE;
  av::mediumGPU.destroy();return JNI_FALSE;
}
extern "C" JNIEXPORT void JNICALL
Java_com_ashenveil_game_MainActivity_deviceProfile(JNIEnv *,jclass,jlong ram,jboolean lowRam,jboolean saver,jint thermal,jboolean pressure) {
  av::graphicsPlatformProfile(uint64_t(ram),bool(lowRam),bool(saver),int(thermal),bool(pressure));
}
#endif

namespace av {
std::string graphicsBackendInfo(){
#if defined(__ANDROID__) || defined(MEDIUM_GL_HOST)
  return "GPU ready: "+std::to_string(mediumGPU.ready)+"\nGPU: "+mediumGPU.driver+
    "\nLighting target: 640x360\nGPU error: "+mediumGPU.error;
#else
  return "Host CPU test (no Android/EGL backend)";
#endif
}
}
