#ifdef BASELINE_ENGINE
#include BASELINE_ENGINE
#else
#include "../native/engine.cpp"
#endif
#include <iostream>
using namespace av;
int main(int argc,char**argv){
 if(argc<3)return 2;
 auto root=std::filesystem::temp_directory_path()/("low-regression-"+hexId(entropy()));boot(root.string());
 std::string raw;if(!readChecked(argv[2],raw)||!decodeFrontier(raw))return 3;
 o.name="LOW REGRESSION";u.tips=false;g.toastTime=0;g.shake=false;motionBlur=false;g.overlay=0;
 std::vector<C> p(W*H);std::ofstream out(argv[1],std::ios::binary);std::ofstream geo(std::string(argv[1])+".geo",std::ios::binary);
 for(int version=1;version<=4;version++){
   o.generator=version;
   for(int y=-131;y<=131;y+=5)for(int x=-129;x<=129;x+=5){auto t=tileAt(x,y);unsigned char a[]={t.map,t.ground,t.biome,static_cast<unsigned char>(landHeight(x,y))};geo.write(reinterpret_cast<char*>(a),4);}
   for(int scene=0;scene<6;scene++){
     if(scene==0)placePlayer(0,0);else{findBiome(scene-1);placePlayer(o.waypointX+.5,o.waypointY+.5);}
     o.waypoint=false;g.toastTime=0;g.time=13;g.walkPhase=0;w.clockOffset=scene%2?720:180;
     for(int quality=0;quality<3;quality++){
       vistaQuality=quality;vistaFocus=true;g.camReady=false;
       frame(p.data(),0);out.write(reinterpret_cast<char*>(p.data()),p.size()*4);
     }
   }
 }
 std::filesystem::remove_all(root);std::cout<<"72 Low legacy-profile frames and version 1-4 geography captured\n";
}
