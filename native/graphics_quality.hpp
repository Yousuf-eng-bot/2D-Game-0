#pragma once
// Central Low/Medium policy/configuration, independent of simulation and GL.
// No world state, RNG, GL calls or Android dependency. Owner-thread only.
#include <cmath>
#include <cstdint>
#include <sstream>
#include <string>

namespace av::graphics {
enum class GraphicsQuality : uint8_t { Low = 0, Medium = 1 };
enum class ThermalState : uint8_t { Unknown, Normal, Moderate, Severe, Critical };
enum class FallbackReason : uint8_t {
  None, UserLow, UnsupportedGpu, RendererNotReady, MemoryPressure,
  ThermalPressure, BatterySaver, SustainedFrameDrops, AdaptiveFrameCap, ThermalBudget
};

// Android PowerManager thermal statuses are not this enum's ordinal values.
// Keep the mapping explicit, including CRITICAL / EMERGENCY / SHUTDOWN.
inline ThermalState thermalFromAndroid(int value) {
  if(value<0||value>6)return ThermalState::Unknown;
  if(value>=4)return ThermalState::Critical;
  if(value==3)return ThermalState::Severe;
  if(value==2)return ThermalState::Moderate;
  return ThermalState::Normal;
}
inline const char *androidThermalName(int value) {
  switch(value){
    case 0:return "None";case 1:return "Light";case 2:return "Moderate";
    case 3:return "Severe";case 4:return "Critical";case 5:return "Emergency";
    case 6:return "Shutdown";default:return "Unknown";
  }
}
struct LegacyAppearance {
  // Import the actual 0.8 preference, NOT an assumption that its Low was used.
  // These are legacy appearance controls, not three new graphics tiers.
  int vistaQuality = 1;
  bool focus = true;
  bool operator==(const LegacyAppearance &) const = default;
};
struct Settings {
  GraphicsQuality requested = GraphicsQuality::Low;
  bool automaticFallback = true;
  int frameCap = 60;
  LegacyAppearance legacy;
  bool operator==(const Settings &) const = default;
};
inline bool valid(const Settings &s) {
  return (s.requested == GraphicsQuality::Low ||
          s.requested == GraphicsQuality::Medium) &&
         (s.frameCap == 30 || s.frameCap == 60) &&
         s.legacy.vistaQuality >= 0 && s.legacy.vistaQuality <= 2;
}
inline std::string encodeSettings(const Settings &s) {
  if (!valid(s)) return {};
  return "DWGFX1 " + std::to_string(int(s.requested)) + " " +
         std::to_string(int(s.automaticFallback)) + " " +
         std::to_string(s.frameCap) + " " +
         std::to_string(s.legacy.vistaQuality) + " " +
         std::to_string(int(s.legacy.focus)) + "\n";
}
inline bool decodeSettings(const std::string &raw, Settings &out) {
  // Parse transactionally: corrupt settings never partially overwrite output.
  if (raw.size() > 256) return false;
  std::istringstream input(raw);
  std::string magic, extra;
  int quality = -1, adaptive = -1, fps = -1, legacy = -1, focus = -1;
  if (!(input >> magic >> quality >> adaptive >> fps >> legacy >> focus) ||
      magic != "DWGFX1" || quality < 0 || quality > 1 ||
      adaptive < 0 || adaptive > 1 || focus < 0 || focus > 1 ||
      (input >> extra)) return false;
  Settings candidate{GraphicsQuality(quality), bool(adaptive), fps,
                     {legacy, bool(focus)}};
  if (!valid(candidate)) return false;
  out = candidate;
  return true;
}

enum class ConfigOrigin { Primary, Backup, Legacy };
struct LoadedSettings { Settings settings; ConfigOrigin origin; };
// Read callback MUST use the existing checksum-validating readChecked().
// It receives a basename, never a world-save path. Try semantic backup recovery
// even when the primary file has a valid checksum but invalid fields.
template <class ReadChecked>
LoadedSettings loadSettings(ReadChecked read, LegacyAppearance legacy) {
  Settings result;
  for (const char *name : {"graphics.cfg", "graphics.cfg.bak"}) {
    std::string raw;
    if (read(name, raw) && decodeSettings(raw, result))
      return {result, std::string(name) == "graphics.cfg"
                          ? ConfigOrigin::Primary : ConfigOrigin::Backup};
  }
  result.legacy = legacy;
  if (!valid(result)) result.legacy = {};
  return {result, ConfigOrigin::Legacy};
}
// Write callback MUST use the existing atomicWorld() backup/checksum writer.
// A failed write is returned to the caller, never silently reported successful.
template <class AtomicWrite>
bool saveSettings(const Settings &s, AtomicWrite write) {
  const auto payload = encodeSettings(s);
  return !payload.empty() && write("graphics.cfg", payload);
}

struct DeviceProfile {
  // Zero means unknown, NOT a powerful device. RAM is recommendation only.
  uint64_t ramMiB = 0;
  int glesMajor = 0, maxTextureSize = 0;
  bool lowRam = false;
  bool shaderStartupPassed = false;
  bool mediumAssetsReady = false;
};
inline bool supportedGpu(const DeviceProfile &p) {
  return p.glesMajor >= 3 && p.maxTextureSize >= 2048;
}
inline bool mediumReady(const DeviceProfile &p) {
  return supportedGpu(p) && p.shaderStartupPassed && p.mediumAssetsReady;
}
inline GraphicsQuality recommend(const DeviceProfile &p) {
  // Allow OS-reserved RAM on nominal 6 GB devices; not a 60 FPS guarantee.
  return !p.lowRam && p.ramMiB >= 5120 && mediumReady(p)
             ? GraphicsQuality::Medium : GraphicsQuality::Low;
}

struct Decision {
  GraphicsQuality effective;
  FallbackReason reason;
  int frameCap;
  bool operator==(const Decision &) const = default;
};
// These flags gate NEW Medium-only features; they must not turn off the
// existing Low CPU renderer's own lighting, particles or legacy appearance.
struct MediumFeatures {
  bool actorAtlases = false, normalLighting = false, canopyMask = false;
  bool waterShader = false, bloom = false, newHud = false;
  int pointLights = 0, particles = 0;
};
inline MediumFeatures featuresFor(GraphicsQuality q) {
  if (q == GraphicsQuality::Medium)
    return {true, true, true, true, true, true, 4, 384};
  return {};
}

class QualityController {
 public:
  explicit QualityController(Settings settings = {})
      : settings_(valid(settings) ? settings : Settings{}) {}
  const Settings &settings() const { return settings_; }
  const DeviceProfile &device() const { return device_; }
  void setDevice(DeviceProfile p) {
    device_ = p;
    resetWindow();
    warmupMs_ = 0;
  }
  bool request(GraphicsQuality quality) {
    if (quality != GraphicsQuality::Low && quality != GraphicsQuality::Medium)
      return false;
    settings_.requested = quality;
    // Explicit user retry is allowed. Never promote automatically merely
    // because the Low renderer produces faster frames after a fallback.
    dropped_ = paced30_ = false;
    resetWindow();
    warmupMs_ = 0;
    return true;
  }
  bool setLegacyAppearance(LegacyAppearance legacy) {
    if(legacy.vistaQuality<0||legacy.vistaQuality>2)return false;
    settings_.legacy=legacy;return true;
  }
  bool setFrameCap(int fps) {
    if (fps != 30 && fps != 60) return false;
    if (settings_.frameCap != fps) {
      settings_.frameCap = fps;
      dropped_ = paced30_ = false;
      resetWindow();
      warmupMs_ = 0;
    }
    return true;
  }
  void setAutomaticFallback(bool enabled) {
    settings_.automaticFallback = enabled;
    dropped_ = paced30_ = false;
    resetWindow();
    warmupMs_ = 0;
  }
  void setSafety(bool batterySaver, ThermalState thermal, bool memoryPressure) {
    if (batterySaver_ != batterySaver || thermal_ != thermal ||
        memoryPressure_ != memoryPressure) {
      resetWindow();
      warmupMs_ = 0;
    }
    batterySaver_ = batterySaver;
    thermal_ = thermal;
    memoryPressure_ = memoryPressure;
  }
  ThermalState thermal() const { return thermal_; }
  bool reducedEffects() const {
    return thermal_==ThermalState::Severe||thermal_==ThermalState::Critical;
  }
  Decision decision() const {
    const bool critical = thermal_ == ThermalState::Critical;
    const bool warm = thermal_ == ThermalState::Moderate || reducedEffects();
    // Temperature protection is independent of optional performance fallback.
    // At SEVERE shed work and frame rate, not the detailed actor/normal renderer.
    const int fps = reducedEffects() ? 20
      : batterySaver_ || warm || (settings_.automaticFallback && paced30_) ? 30 : settings_.frameCap;
    auto low = [&](FallbackReason reason) {
      return Decision{GraphicsQuality::Low, reason, fps};
    };
    if (settings_.requested == GraphicsQuality::Low)
      return low(FallbackReason::UserLow);
    if (!supportedGpu(device_)) return low(FallbackReason::UnsupportedGpu);
    if (!mediumReady(device_)) return low(FallbackReason::RendererNotReady);
    if (memoryPressure_) return low(FallbackReason::MemoryPressure);
    if (critical) return low(FallbackReason::ThermalPressure);
    if (settings_.automaticFallback && dropped_)
      return low(FallbackReason::SustainedFrameDrops);
    return {GraphicsQuality::Medium, warm ? FallbackReason::ThermalBudget
      : batterySaver_ ? FallbackReason::BatterySaver
      : settings_.automaticFallback && paced30_ ? FallbackReason::AdaptiveFrameCap
      : FallbackReason::None, fps};
  }
  MediumFeatures features() const {
    auto f=featuresFor(decision().effective);
    if(reducedEffects()&&f.normalLighting){f.bloom=false;f.pointLights=1;}
    return f;
  }
  void pausedOrLoading() {
    resetWindow();
    warmupMs_ = 0;
  }
  // Supply completed DISPLAYED-frame intervals, not native render-only cost.
  // Android now supplies a UI-frame pacing estimate via graphicsStartFrame.
  // This is not a hardware GPU timer or presentation-timestamp measurement.
  void observeFrame(double displayedMs, bool activeGameplay = true,
                    bool loadingOrCompiling = false) {
    if (!activeGameplay || loadingOrCompiling) {
      pausedOrLoading();
      return;
    }
    if (!std::isfinite(displayedMs) || displayedMs <= 0) return;
    if (displayedMs > 1000) { pausedOrLoading(); return; }
    if (!settings_.automaticFallback ||
        decision().effective != GraphicsQuality::Medium) {
      resetWindow();
      return;
    }
    // Allow cache/shader warm-up. First lower the effective cap to 30 without
    // replacing Medium art. Only sustained failure at 30 can demote to Low.
    if (warmupMs_ < 4000) {
      warmupMs_ += displayedMs;
      return;
    }
    windowMs_ += displayedMs;
    ++samples_;
    if (displayedMs > (1000.0 / decision().frameCap) * 1.25) ++late_;
    if (windowMs_ >= 2000) {
      if (samples_ >= 10 && late_ * 5 > samples_) ++badWindows_;
      else badWindows_ = 0;
      windowMs_ = 0;
      samples_ = late_ = 0;
      const int required = decision().frameCap == 60 ? 3 : 5;
      if (badWindows_ >= required) {
        if (decision().frameCap == 60) paced30_ = true;
        else dropped_ = true;
        resetWindow();
        warmupMs_ = 0;
      }
    }
  }
 private:
  void resetWindow() {
    windowMs_ = 0;
    samples_ = late_ = badWindows_ = 0;
  }
  Settings settings_;
  DeviceProfile device_;
  ThermalState thermal_ = ThermalState::Unknown;
  bool batterySaver_ = false, memoryPressure_ = false, dropped_ = false, paced30_ = false;
  double warmupMs_ = 0, windowMs_ = 0;
  int samples_ = 0, late_ = 0, badWindows_ = 0;
};
}  // namespace av::graphics
