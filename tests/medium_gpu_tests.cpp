#define MEDIUM_GL_HOST
#include "../native/engine.cpp"
#include <iostream>
using namespace av;
int checks=0;
#define CHECK(x) do{checks++;if(!(x)){std::cerr<<"FAIL "<<__LINE__<<": "#x" "<<mediumGPU.error<<"\n";return 1;}}while(0)
std::vector<C> readGPU(){
 std::vector<C> raw(W*H),result(W*H);glReadPixels(0,0,W,H,GL_RGBA,GL_UNSIGNED_BYTE,raw.data());
 for(int y=0;y<H;y++)for(int x=0;x<W;x++){C c=raw[(H-1-y)*W+x];result[y*W+x]=0xff000000|((c&255)<<16)|(c&0xff00)|((c>>16)&255);}
 return result;
}
void ppm(const std::filesystem::path&p,const std::vector<C>&pixels){std::ofstream f(p,std::ios::binary);f<<"P6\n640 360\n255\n";for(C c:pixels){char b[]={char(c>>16),char(c>>8),char(c)};f.write(b,3);}}
int main(int argc,char**argv){
 auto root=std::filesystem::temp_directory_path()/("medium-gpu-"+hexId(entropy()));boot(root.string());
 o.draftName="NEW HORIZON Z";o.draftSeed=20261005;CHECK(createWorld());
 u.tips=false;g.toastTime=0;g.shake=false;motionBlur=false;
 std::vector<C>p(W*H);
 CHECK(mediumGPU.create());std::cout<<"GL driver: "<<mediumGPU.driver<<"\n";
 graphicsController.request(gfx::GraphicsQuality::Low);frame(p.data(),0);
 CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));
 auto low=readGPU();CHECK(low==p); // GPU presentation preserves every Low logical pixel.
 auto map=g.map;auto state=encodeJourney();auto seed=rng;
 graphicsController.request(gfx::GraphicsQuality::Medium);mediumCameraZoom=1;
 frame(p.data(),0);CHECK(mediumWorldCaptured);
 CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));auto lit=readGPU();
 CHECK(lit!=low&&lit!=p);CHECK(g.map==map&&encodeJourney()==state&&rng==seed);
 auto normals=mediumNormals;for(auto &n:mediumNormals)n=(n&0xff000000)|0x008080ff;
 CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));auto flat=readGPU();CHECK(flat!=lit);
 mediumNormals=normals;
 int changed=0;for(size_t i=0;i<lit.size();i++)changed+=lit[i]!=flat[i];CHECK(changed>1000);
 // Isolate actual point-light height occlusion from normal-vector lighting.
 for(auto &n:mediumNormals)n &= 0x80ffffff;
 CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));auto unoccluded=readGPU();
 int shadowChanges=0;for(size_t i=0;i<lit.size();i++)shadowChanges+=lit[i]!=unoccluded[i];
 CHECK(shadowChanges>20);mediumNormals=normals;
 std::cout<<"PASS height-field point-light shadow changes "<<shadowChanges<<" pixels\n";
 // UI pixels bypass lighting and remain exact (no bloom/text blur).
 for(int y=12;y<56;y++)for(int x=220;x<510;x++)if(mediumOverlay[y*W+x])CHECK(lit[y*W+x]==mediumOverlay[y*W+x]);
 for(int n=0;n<24;n++){
   graphicsController.request(n%2?gfx::GraphicsQuality::Medium:gfx::GraphicsQuality::Low);
   frame(p.data(),0);CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));
   CHECK(g.map==map&&state==encodeJourney()&&rng==seed);
 }
 for(int n=0;n<3;n++){
   graphicsController.request(gfx::GraphicsQuality::Medium);
   mediumGPU.destroy();CHECK(!mediumEnabled());CHECK(graphicsController.settings().requested==gfx::GraphicsQuality::Medium);
   CHECK(mediumGPU.create());CHECK(mediumEnabled());frame(p.data(),0);CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));
 }
 std::cout<<"PASS Low exact-pixel GPU present; actual normal shader changes "<<changed<<" pixels; UI preserved; 24 switches; 3 EGL context recreations\n";
 // Verify logical lighting + nearest present at a larger physical surface.
 // Exact 2x replication would fail with the old physical-pixel lighting path.
 graphicsController.request(gfx::GraphicsQuality::Medium);mediumCameraZoom=1;
 frame(p.data(),0);CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));
 auto logical=readGPU();
 CHECK(mediumGPU.create(nullptr,W*2,H*2));
 CHECK(mediumGPU.draw(p.data(),W*2,H*2,0,0,2,false));
 std::vector<C> doubled(W*H*4);glReadPixels(0,0,W*2,H*2,GL_RGBA,GL_UNSIGNED_BYTE,doubled.data());
 bool exactDouble=true;
 for(int y=0;y<H*2;y++)for(int x=0;x<W*2;x++){
   C c=doubled[(H*2-1-y)*W*2+x];
   C argb=0xff000000|((c&255)<<16)|(c&0xff00)|((c>>16)&255);
   if(argb!=logical[(y/2)*W+x/2])exactDouble=false;
 }
 CHECK(exactDouble);
 CHECK(mediumGPU.create());
 // Regression for the phone report: Medium remains actual rendered Medium
 // with legacy in-game battery saver enabled, not just a selected button.
 g.lowPower=true;graphicsController.request(gfx::GraphicsQuality::Medium);
 frame(p.data(),0);CHECK(mediumEnabled()&&mediumWorldCaptured);
 CHECK(graphicsController.decision().frameCap==30);
 CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));CHECK(readGPU()!=p);
 CHECK(std::string(av_report()).find("Active graphics: Medium")!=std::string::npos);
 g.lowPower=false;
 platformBatterySaver=true;frame(p.data(),0);
 CHECK(mediumEnabled()&&graphicsController.decision().frameCap==30);
 CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));
 platformBatterySaver=false;
 // Actual rendering, not just a changed status label, at the user's raw 2.
 graphicsController.setAutomaticFallback(false);
 auto stableMap=g.map;auto stableState=encodeJourney();auto stableRng=rng;
 for(int thermal:{2,3,4,2,5,2,6,2,0}) {
   graphicsPlatformProfile(5500,false,false,thermal,false);
   CHECK(mediumEnabled()==(thermal<4)); // immediate, before frame()
   frame(p.data(),0);
   CHECK(mediumWorldCaptured==(thermal<4));
   CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));
   if(thermal<4) {
     CHECK(readGPU()!=p);
     CHECK(graphicsController.features().actorAtlases && graphicsController.features().normalLighting);
     GLint budget=-1;glGetUniformiv(mediumGPU.program,mediumGPU.uniform("heavyEffects"),&budget);
     CHECK(budget==(thermal==3?0:1));
     if(thermal==3){
       CHECK(av_rate()==20);
       GLint lights=-1;glGetUniformiv(mediumGPU.program,mediumGPU.uniform("lightCount"),&lights);CHECK(lights<=1);
       // Cooling still uses genuine normals; do not merely relabel Low.
       auto shaded=readGPU();auto retainedNormals=mediumNormals;
       for(auto &n:mediumNormals)n=(n&0xff000000)|0x008080ff;
       CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));CHECK(readGPU()!=shaded);
       mediumNormals=retainedNormals;
     }
   } else {
     CHECK(graphicsController.decision().reason==gfx::FallbackReason::ThermalPressure);
     CHECK(av_rate()==20);
     CHECK(readGPU()==p); // genuine Low, rather than a false active-Medium label
   }
   CHECK(g.map==stableMap&&encodeJourney()==stableState&&rng==stableRng);
 }
 // English screenshots of the exact reported Graphics page conditions.
 if(argc>1){
   std::filesystem::path output=argv[1];std::filesystem::create_directories(output);
   int oldLanguage=u.language;u.language=1;
   for(int thermal:{2,3,4}){
     graphicsPlatformProfile(5500,false,false,thermal,false);
     g.overlay=0;frame(p.data(),0);CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));
     ppm(output/("thermal-"+num(thermal)+"-world.ppm"),readGPU());
     uOpen(12);u.settingsTab=3;frame(p.data(),0);CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));
     ppm(output/("thermal-"+num(thermal)+"-settings.ppm"),readGPU());
   }
   u.language=oldLanguage;g.overlay=0;clearInput();
 }
 graphicsPlatformProfile(5500,false,false,0,false);
 graphicsController.setAutomaticFallback(true);
 std::cout<<"PASS exact nearest 2x upscale, actual Medium at thermal 2/3 with auto fallback off, cooling normal lighting, critical 4-6 Low, immediate recovery and unchanged game state\n";
 if(argc>1){
  std::filesystem::path output=argv[1];std::filesystem::create_directories(output);
  std::ofstream meta(output/"scenes.tsv");meta<<"scene\tseed\tgenerator\tx\ty\thour\n";
  struct Shot{const char*name;int biome;float hour;int dx,dy;};
  for(auto shot:std::vector<Shot>{{"camp-day",0,11,0,0},{"camp-night",0,22,0,0},{"forest-walk",0,14,-17,-12},{"forest-road",0,9,22,19},{"desert",1,11,0,0},{"snow",2,15,0,0},{"swamp",3,7,0,0},{"volcanic",4,18,0,0}}){
   if(shot.biome==0)placePlayer(shot.dx,shot.dy);else{findBiome(shot.biome);placePlayer(o.waypointX+.5,o.waypointY+.5);}
   o.waypoint=false;g.toastTime=0;g.camReady=false;g.overlay=0;mediumCameraZoom=1;mediumCanopyFade.clear();
   w.clockOffset=std::fmod(double(shot.hour-8)*60-o.seconds+14400,1440.);
   meta<<shot.name<<'\t'<<o.seed<<'\t'<<o.generator<<'\t'<<globalX()<<'\t'<<globalY()<<'\t'<<shot.hour<<'\n';
   for(int q=0;q<2;q++){
     graphicsController.request(q?gfx::GraphicsQuality::Medium:gfx::GraphicsQuality::Low);
     frame(p.data(),0);CHECK(mediumGPU.draw(p.data(),W,H,0,0,1,false));ppm(output/(std::string(shot.name)+(q?"-medium.ppm":"-low.ppm")),readGPU());
   }
  }
  for(int generation:{4,5}){
    o.generator=generation;
    std::ofstream mapImage(output/(std::string("generation-")+std::to_string(generation)+".ppm"),std::ios::binary);
    mapImage<<"P6\n256 256\n255\n";
    for(int y=-128;y<128;y++)for(int x=-128;x<128;x++){
      auto t=tileAt(x,y);C c=t.map==4?0xff244753:t.ground==4?0xffc1aa74:t.ground==2||t.ground==3?0xffa6976b:t.map==2?0xff263f30:t.map==3?0xff8b8c78:0xff677a50;
      char rgb[]={char(c>>16),char(c>>8),char(c)};mapImage.write(rgb,3);
    }
  }
  o.generator=5;
  // Separate art sheet is a debugging asset, not a fake in-game screenshot.
  std::ofstream raw(output/"player-rgba.bin",std::ios::binary);auto s=mediumMakeActor(0,2,MIDLE,0,0);raw.write(reinterpret_cast<const char*>(s.color.data()),s.color.size()*4);
 }
 std::cout<<"PASS "<<checks<<" GPU assertions. Host Mesa/llvmpipe is not a physical phone FPS measurement.\n";
 mediumGPU.destroy();std::filesystem::remove_all(root);
}
