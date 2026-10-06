// Compile from project root: g++ -std=c++20 -O2 docs/medium-thermal-fix/reproduce_stale_status.cpp -lz -o /tmp/thermal-repro
#define av previous_av
#include "quality-v091.hpp"
#undef av
#include "../../native/engine.cpp"
#include <iostream>
int main(){
 namespace old=previous_av::graphics;
 old::QualityController previous;previous.setDevice({5500,3,4096,false,true,true});
 previous.request(old::GraphicsQuality::Medium);previous.setAutomaticFallback(false);
 int oldRaw=3;
 // 0.9.1's graphicsStartFrame consumes a severe sample.
 previous.setSafety(false,old::ThermalState::Severe,false);
 // Exact old JNI behavior: update platformThermal only; setSafety is deferred.
 oldRaw=2;
 std::cout<<"0.9.1 between deviceProfile and next frame: raw="<<oldRaw
          <<", active="<<(previous.decision().effective==old::GraphicsQuality::Medium?"Medium":"Low")
          <<", hotReason="<<(previous.decision().reason==old::FallbackReason::ThermalPressure)<<"\n";
 if(previous.decision().effective!=old::GraphicsQuality::Low)return 1;
 av::graphicsController.setDevice({5500,3,4096,false,true,true});
 av::graphicsController.request(av::gfx::GraphicsQuality::Medium);
 av::graphicsController.setAutomaticFallback(false);
 av::graphicsPlatformProfile(5500,false,false,3,false);
 av::graphicsPlatformProfile(5500,false,false,2,false); // real shared JNI helper
 std::cout<<"0.9.2 immediately after deviceProfile (no frame): raw="<<av::platformThermal
          <<", active="<<(av::mediumEnabled()?"Medium":"Low")
          <<", cap="<<av_rate()<<", status="<<av::graphicsReason()<<"\n";
 return av::mediumEnabled()&&av_rate()==30?0:1;
}
