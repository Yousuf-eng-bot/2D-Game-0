#include POLICY_HEADER
#include <iostream>
using namespace av::graphics;
int main(){
 QualityController c;c.setDevice({5500,3,4096,false,true,true});c.request(GraphicsQuality::Medium);
 for(int i=0;i<1800;i++)c.observeFrame(1000./30);
 std::cout << "60-second 30-FPS pacing, requested Medium/60: active=" << (c.decision().effective==GraphicsQuality::Medium?"Medium":"Low") << ", effective cap=" << c.decision().frameCap << '\n';
 c.request(GraphicsQuality::Medium);c.setSafety(true,ThermalState::Normal,false);
 std::cout << "Battery saver: active=" << (c.decision().effective==GraphicsQuality::Medium?"Medium":"Low") << ", effective cap=" << c.decision().frameCap << '\n';
}
