#pragma once
#include "graphics_quality.hpp"
namespace av {
namespace gfx = graphics;
gfx::QualityController graphicsController;
int platformThermal = 0;
bool platformLowMemory = false, platformBatterySaver = false;
std::string graphicsBackendInfo();
bool mediumWorldCaptured = false;
std::vector<C> mediumAlbedo(W * H), mediumNormals(W * H), mediumOverlay(W * H);
float mediumCameraZoom=1.04f;
float mediumFrameMs = 0, mediumPeakMs = 0;
uint64_t mediumFrameCount = 0;
inline bool mediumEnabled() {
  return graphicsController.decision().effective == gfx::GraphicsQuality::Medium;
}
void graphicsBoot() {
  auto loaded = gfx::loadSettings([](const char *name, std::string &raw) {
    return readChecked(g.path + "/" + name, raw);
  }, {vistaQuality, vistaFocus});
  auto device = graphicsController.device();
  graphicsController = gfx::QualityController(loaded.settings);
  graphicsController.setDevice(device);
  // The old appearance is authoritative only for migration; once imported,
  // the preserved appearance belongs to Low and is not modified by Medium.
  vistaQuality = loaded.settings.legacy.vistaQuality;
  vistaFocus = loaded.settings.legacy.focus;
}
bool graphicsSave() {
  return gfx::saveSettings(graphicsController.settings(), [](const char *name, const std::string &raw) {
    return atomicWorld(g.path + "/" + name, raw);
  });
}
void mediumReleaseCaches();
void graphicsStartFrame(float dt) {
  static bool trimmed=false;
  if(platformLowMemory&&!trimmed){mediumReleaseCaches();trimmed=true;}
  if(!platformLowMemory)trimmed=false;
  mediumWorldCaptured = false;
  surfaceNormals = nullptr;
  uiOverlayTarget = nullptr;
  graphicsController.setSafety(g.lowPower || platformBatterySaver,
    platformThermal >= 3 ? gfx::ThermalState::Severe : platformThermal >= 2 ? gfx::ThermalState::Moderate : gfx::ThermalState::Normal,
    platformLowMemory);
  graphicsController.observeFrame(dt * 1000., g.scene == PLAY && !g.overlay,
    g.scene == LOADING || o.transition > 0);
  if (dt > 0 && dt < 1) {
    mediumFrameMs += (dt * 1000 - mediumFrameMs) * .03f;
    mediumPeakMs = std::max(mediumPeakMs, dt * 1000);
    mediumFrameCount++;
  }
}
void mediumBeginWorld() {
  if (!mediumEnabled()) return;
  bool combat=false;
  for(const auto&e:g.enemies)if(e.alive&&e.alertTime>0&&len(e.x-g.px,e.y-g.py)<300){combat=true;break;}
  mediumCameraZoom+=((combat?1.f:1.075f)-mediumCameraZoom)*(1-std::exp(-2.5f*renderDt));
  std::fill(mediumNormals.begin(), mediumNormals.end(), 0x008080ff);
  surfaceNormals = mediumNormals.data();
}
void mediumCaptureWorld() {
  if (!mediumEnabled()) return;
  std::copy(pix, pix + W * H, mediumAlbedo.begin());
  surfaceNormals = nullptr;
  mediumWorldCaptured = true;
  std::fill(mediumOverlay.begin(),mediumOverlay.end(),0);
  uiOverlayTarget=mediumOverlay.data();
}
void mediumFinishFrame() {
  surfaceNormals = nullptr;
  uiOverlayTarget = nullptr;
}
const char *graphicsReason() {
  using R = gfx::FallbackReason;
  switch (graphicsController.decision().reason) {
    case R::UnsupportedGpu: return "GPU not supported";
    case R::RendererNotReady: return "Renderer unavailable";
    case R::MemoryPressure: return "Memory pressure";
    case R::ThermalPressure: return "Device is hot";
    case R::BatterySaver: return "Medium: battery 30 cap";
    case R::AdaptiveFrameCap: return "Medium: adaptive 30 cap";
    case R::SustainedFrameDrops: return "Frame drops: using Low";
    default: return mediumEnabled() ? "Medium active" : "Original look preserved";
  }
}
} // namespace av
