// Policy/configuration foundation tests, NOT an Android/GPU integration test.
#include "../native/engine.cpp"
#include "../native/graphics_quality.hpp"
#include <iostream>
#include <limits>
#include <random>
#include <tuple>
namespace gfx = av::graphics;
using Q = gfx::GraphicsQuality;
using R = gfx::FallbackReason;
int checks = 0;
#define CHECK(x) do { ++checks; if (!(x)) { std::cerr << "FAIL " << __LINE__ << ": " #x "\n"; return 1; } } while (0)

int main() {
  auto root = std::filesystem::temp_directory_path() /
              ("graphics-policy-" + av::hexId(av::entropy()));
  std::filesystem::create_directories(root);
  auto read = [&](const char *name, std::string &out) {
    return av::readChecked((root / name).string(), out);
  };
  auto write = [&](const char *name, const std::string &value) {
    return av::atomicWorld((root / name).string(), value);
  };
  auto loaded = gfx::loadSettings(read, {2, false});
  CHECK(loaded.origin == gfx::ConfigOrigin::Legacy);
  CHECK(loaded.settings.requested == Q::Low);
  CHECK(loaded.settings.legacy.vistaQuality == 2 && !loaded.settings.legacy.focus);
  CHECK(!std::filesystem::exists(root / "graphics.cfg"));
  CHECK(gfx::loadSettings(read, {-1, false}).settings.legacy.vistaQuality == 1);

  for (int q = 0; q < 2; ++q)
    for (bool adaptive : {false, true})
      for (int fps : {30, 60})
        for (int legacy = 0; legacy < 3; ++legacy)
          for (bool focus : {false, true}) {
            gfx::Settings s{Q(q), adaptive, fps, {legacy, focus}}, decoded;
            CHECK(gfx::valid(s));
            CHECK(gfx::decodeSettings(gfx::encodeSettings(s), decoded));
            CHECK(s == decoded);
            CHECK(gfx::saveSettings(s, write));
            auto recovered = gfx::loadSettings(read, {1, true});
            CHECK(recovered.settings == s);
            CHECK(recovered.origin == gfx::ConfigOrigin::Primary);
          }
  const gfx::Settings saved{Q::Medium, true, 30, {1, false}};
  CHECK(gfx::saveSettings(saved, write));
  CHECK(write("graphics.cfg", "DWGFX1 2 1 60 1 1\n"));
  loaded = gfx::loadSettings(read, {0, true});
  CHECK(loaded.origin == gfx::ConfigOrigin::Backup && loaded.settings == saved);
  // A checksum failure must also use the checked backup.
  { std::ofstream f(root / "graphics.cfg"); f << "corrupt"; }
  loaded = gfx::loadSettings(read, {0, true});
  CHECK(loaded.origin == gfx::ConfigOrigin::Backup && loaded.settings == saved);
  std::filesystem::remove(root / "graphics.cfg.bak");
  loaded = gfx::loadSettings(read, {0, true});
  CHECK(loaded.origin == gfx::ConfigOrigin::Legacy);
  CHECK(loaded.settings.requested == Q::Low && loaded.settings.legacy.vistaQuality == 0);
  CHECK(!gfx::saveSettings(saved, [](auto, auto) { return false; }));
  gfx::Settings invalid = saved;
  invalid.frameCap = 120;
  bool wrote = false;
  CHECK(!gfx::saveSettings(invalid, [&](auto, auto) { wrote = true; return true; }));
  CHECK(!wrote);
  for (const std::string raw : {
           "", "DWGFX1", "DWGFX9 0 1 60 1 1", "DWGFX1 -1 1 60 1 1",
           "DWGFX1 2 1 60 1 1", "DWGFX1 1 2 60 1 1", "DWGFX1 1 1 45 1 1",
           "DWGFX1 1 1 60 3 1", "DWGFX1 1 1 60 1 2", "DWGFX1 1 1 60 1 1 extra",
           "DWGFX1 99999999999999999 1 60 1 1", "DWGFX1 1 1 60 1"}) {
    auto decoded = saved;
    CHECK(!gfx::decodeSettings(raw, decoded));
    CHECK(decoded == saved);
  }
  std::mt19937 fuzz(5012026);
  for (int n = 0; n < 10000; ++n) {
    std::string raw(size_t(fuzz() % 300), ' ');
    for (char &c : raw) c = char(fuzz() % 128);
    auto decoded = saved;
    bool ok = gfx::decodeSettings(raw, decoded);
    CHECK(ok ? gfx::valid(decoded) : decoded == saved);
  }
  std::cout << "PASS strict config, checked atomic persistence, backup/migration, 10000 malformed-input probes\n";

  gfx::DeviceProfile ready{8192, 3, 4096, false, true, true};
  CHECK(gfx::recommend({}) == Q::Low);
  CHECK(gfx::recommend(ready) == Q::Medium);
  auto device = ready;
  device.lowRam = true;
  CHECK(gfx::recommend(device) == Q::Low);
  device = ready; device.ramMiB = 4096;
  CHECK(gfx::recommend(device) == Q::Low);
  device = ready; device.glesMajor = 2;
  CHECK(!gfx::mediumReady(device) && gfx::recommend(device) == Q::Low);
  device = ready; device.maxTextureSize = 1024;
  CHECK(!gfx::supportedGpu(device));
  device = ready; device.mediumAssetsReady = false;
  CHECK(!gfx::mediumReady(device));
  device = ready; device.shaderStartupPassed = false;
  CHECK(!gfx::mediumReady(device));

  gfx::QualityController c;
  CHECK(c.decision().effective == Q::Low && c.decision().frameCap == 60);
  CHECK(!c.request(Q(2)));
  CHECK(!c.setFrameCap(120));
  CHECK(c.request(Q::Medium));
  CHECK(c.settings().requested == Q::Medium);
  CHECK(c.decision().reason == R::UnsupportedGpu);
  c.setDevice(device);
  CHECK(c.decision().reason == R::RendererNotReady);
  c.setDevice(ready);
  CHECK(c.decision().effective == Q::Medium && c.decision().reason == R::None);
  CHECK(c.features().normalLighting && c.features().actorAtlases);
  CHECK(c.features().pointLights == 4 && c.features().particles == 384);
  for(int raw=0;raw<=6;raw++) {
    auto t=gfx::thermalFromAndroid(raw);
    CHECK(t==(raw>=4?gfx::ThermalState::Critical:raw==3?gfx::ThermalState::Severe:raw==2?gfx::ThermalState::Moderate:gfx::ThermalState::Normal));
    c.setSafety(false,t,false);
    CHECK(c.decision().effective==(raw>=4?Q::Low:Q::Medium));
    CHECK(c.decision().frameCap==(raw>=3?20:raw==2?30:60));
  }
  CHECK(gfx::thermalFromAndroid(-1)==gfx::ThermalState::Unknown);
  CHECK(gfx::thermalFromAndroid(7)==gfx::ThermalState::Unknown);
  c.setSafety(false, gfx::ThermalState::Moderate, false);
  CHECK(c.decision().effective==Q::Medium && c.decision().frameCap==30);
  CHECK(c.decision().reason==R::ThermalBudget && c.features().normalLighting);
  c.setSafety(false, gfx::ThermalState::Severe, false);
  CHECK(c.decision().reason==R::ThermalBudget && c.decision().frameCap==20);
  CHECK(c.settings().requested == Q::Medium);
  CHECK(c.features().normalLighting && c.features().newHud && !c.features().bloom && c.features().pointLights==1);
  c.setAutomaticFallback(false);
  CHECK(c.decision().effective==Q::Medium && c.decision().frameCap==20);
  c.setSafety(false,gfx::ThermalState::Critical,false);
  CHECK(c.decision().effective==Q::Low && c.decision().reason==R::ThermalPressure && c.decision().frameCap==20);
  CHECK(!c.features().normalLighting);
  c.setSafety(false, gfx::ThermalState::Unknown, true);
  CHECK(c.decision().reason == R::MemoryPressure);
  c.setSafety(true, gfx::ThermalState::Normal, false);
  CHECK(c.decision().reason == R::BatterySaver && c.decision().frameCap == 30);
  CHECK(c.decision().effective == Q::Medium && c.features().normalLighting);
  c.setSafety(false, gfx::ThermalState::Normal, false);
  CHECK(c.decision().effective == Q::Medium);
  CHECK(c.setFrameCap(30) && c.decision().frameCap == 30);
  c.setAutomaticFallback(true);
  for (int i = 0; i < 1500; ++i) c.observeFrame(1000.0 / 30);
  CHECK(c.decision().effective == Q::Medium); // normal 30 FPS is not a 60 FPS failure
  CHECK(c.setFrameCap(60));
  for (int i = 0; i < 1200; ++i) c.observeFrame(i % 120 == 0 ? 80 : 1000.0 / 60);
  CHECK(c.decision().effective == Q::Medium); // sparse spikes do not demote
  for (int i = 0; i < 120; ++i) c.observeFrame(35);
  CHECK(c.decision().effective == Q::Medium); // fewer than 3 consecutive windows
  for (int i = 0; i < 120; ++i) c.observeFrame(35);
  CHECK(c.decision().effective == Q::Medium);
  CHECK(c.decision().reason == R::AdaptiveFrameCap && c.decision().frameCap == 30);
  // A phone that sustains ~28-30 FPS must retain Medium, not be judged at 60.
  for (int i = 0; i < 1800; ++i) c.observeFrame(35);
  CHECK(c.decision().effective == Q::Medium && c.decision().frameCap == 30);
  // A second, substantially longer failure at the 30 cap still protects slow devices.
  for (int i = 0; i < 400; ++i) c.observeFrame(60);
  CHECK(c.decision().reason == R::SustainedFrameDrops);
  CHECK(c.settings().requested == Q::Medium && c.settings().frameCap == 60);
  for (int i = 0; i < 1200; ++i) c.observeFrame(10);
  CHECK(c.decision().effective == Q::Low); // no Low/Medium oscillation
  CHECK(c.request(Q::Medium));
  CHECK(c.decision().effective == Q::Medium);
  for (int i = 0; i < 600; ++i) c.observeFrame(100, true, true);
  CHECK(c.decision().effective == Q::Medium); // loading excluded
  for (int i = 0; i < 600; ++i) c.observeFrame(100, false, false);
  CHECK(c.decision().effective == Q::Medium); // inactive/background excluded
  c.observeFrame(std::numeric_limits<double>::quiet_NaN());
  c.observeFrame(std::numeric_limits<double>::infinity());
  c.observeFrame(-12);
  c.observeFrame(0);
  c.observeFrame(12000);
  CHECK(c.decision().effective == Q::Medium);
  c.setAutomaticFallback(false);
  for (int i = 0; i < 600; ++i) c.observeFrame(50);
  CHECK(c.decision().effective == Q::Medium);
  c.setAutomaticFallback(true);
  for (int i = 0; i < 700; ++i) c.observeFrame(50);
  CHECK(c.decision().reason == R::SustainedFrameDrops);
  CHECK(c.request(Q::Low));
  CHECK(c.decision().reason == R::UserLow && !c.features().newHud);
  c.setSafety(true, gfx::ThermalState::Normal, false);
  CHECK(c.decision().frameCap == 30);
  std::cout << "PASS readiness gate, recommendation, 30/60 caps, mandatory heat/memory safety, Medium battery cap, staged 60-to-30-to-Low fallback and manual retry\n";

  // Android's raw 3 -> 2 transition must update the applied decision without
  // waiting for a subsequent graphicsStartFrame. This is the real JNI helper.
  av::graphicsController.setDevice(ready);
  av::graphicsController.request(Q::Medium);
  av::graphicsController.setAutomaticFallback(false);
  for(int raw:{3,2,4,2,5,0,6,1}) {
    av::graphicsPlatformProfile(5500,false,false,raw,false);
    CHECK(av::platformThermal==raw);
    CHECK(av::graphicsController.thermal()==gfx::thermalFromAndroid(raw));
    CHECK(av::mediumEnabled()==(raw<4));
    CHECK(av_rate()==(raw>=3?20:raw==2?30:60));
    auto report=std::string(av_report());
    CHECK(report.find("Thermal status: "+std::to_string(raw)+" (")!=std::string::npos);
    CHECK(report.find(raw<4?"Active graphics: Medium":"Active graphics: Low")!=std::string::npos);
  }
  av::g.lowPower=true;
  av::graphicsPlatformProfile(5500,false,true,3,false);
  CHECK(av_rate()==20); // a battery flag must never raise a stricter thermal cap
  av::graphicsPlatformProfile(5500,false,true,2,false);
  CHECK(av::mediumEnabled()&&av_rate()==30);
  av::g.lowPower=false;
  av::graphicsPlatformProfile(5500,false,false,0,false);
  std::cout<<"PASS raw Android thermal 0-6 mapping, immediate 3/4/5/6-to-2 recovery, aligned report and cap\n";

  // Pure-policy requests cannot change gameplay. This is not yet a live GL
  // renderer-switch test: engine/Android integration has deliberately not run.
  av::boot((root / "game").string());
  av::o.draftName = "POLICY BOUNDARY";
  av::o.draftSeed = 20261005;
  av::createWorld();
  auto state = std::make_tuple(av::g.px, av::g.py, av::g.hp, av::g.level,
                              av::rng, av::o.seed, av::o.id, av::j.stock);
  auto map = av::g.map;
  auto heights = av::j.heights;
  for (int i = 0; i < 1000; ++i) {
    CHECK(c.request(i % 2 ? Q::Medium : Q::Low));
    CHECK(c.features().normalLighting == bool(i % 2)); // saver caps FPS, not Medium art
  }
  CHECK(state == std::make_tuple(av::g.px, av::g.py, av::g.hp, av::g.level,
                                av::rng, av::o.seed, av::o.id, av::j.stock));
  CHECK(map == av::g.map && heights == av::j.heights);
  std::filesystem::remove_all(root);
  std::cout << "PASS " << checks << " policy/config assertions; NOT a GPU/device validation\n";
}
