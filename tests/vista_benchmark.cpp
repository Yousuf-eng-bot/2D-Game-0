#define main staged_capture_entry
#include "vista_capture.cpp"
#undef main
#include <chrono>
int main() {
  auto root = std::filesystem::temp_directory_path() /
              ("vista-bench-" + hexId(entropy()));
  std::filesystem::create_directories(root);
  boot(root.string());
  o.draftSeed = 20261004;
  o.draftName = "BENCH";
  createWorld();
  u.tips = false;
  g.shake = false;
  motionBlur = false;
  std::cout
      << "HOST CPU RENDER ONLY; dt=0; 640x360; no Android display, gameplay, "
         "audio, GPU or thermal measurement. Warm cache, 120 frames/scene.\n";
#ifdef BASELINE_ENGINE
  for (int q = 1; q <= 1; q++) {
#else
  for (int q = 0; q < 3; q++) {
    vistaQuality = q;
#endif
    for (int b = 0; b < 6; b++) {
      stage(b % 5, b == 5 ? 22 : 11);
      std::vector<double> ms;
      for (int n = 0; n < 132; n++) {
        g.time = n / 30.f;
        auto t = std::chrono::steady_clock::now();
        frame(pixels.data(), 0);
        double dt = std::chrono::duration<double, std::milli>(
                        std::chrono::steady_clock::now() - t)
                        .count();
        if (n >= 12)
          ms.push_back(dt);
      }
      std::sort(ms.begin(), ms.end());
      double sum = 0;
      for (auto x : ms)
        sum += x;
      std::cout << "quality=" << q << " scene=" << b
                << " mean_ms=" << sum / ms.size() << " median_ms=" << ms[60]
                << " p95_ms=" << ms[114] << '\n';
    }
  }
  std::filesystem::remove_all(root);
}
