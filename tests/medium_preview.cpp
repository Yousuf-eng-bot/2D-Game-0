// Staged native GPU comparison, NOT phone footage or a performance benchmark.
#define MEDIUM_GL_HOST
#include "../native/engine.cpp"
#include <iostream>
using namespace av;
int main(int argc,char**argv){
 if(argc<2)return 2;
 auto root=std::filesystem::temp_directory_path()/("medium-preview-"+hexId(entropy()));boot(root.string());
 o.draftName="NEW HORIZON Z";o.draftSeed=20261005;if(!createWorld()||!mediumGPU.create())return 3;
 g.shake=false;motionBlur=false;u.tips=false;u.language=0;
 std::vector<C>pixels(W*H),gpu(W*H);std::vector<uint8_t>rgb(W*H*6);
 std::ofstream events(argv[1]);int previousMusic=-1,previousAmbience=-1;
 const int biomes[]={0,0,0,0,2,1,3,4,0};
 const float hours[]={14,11,12,17,14,10,7,18,22};
 for(int frameNumber=0;frameNumber<1350;frameNumber++){
   int scene=frameNumber/150,at=frameNumber%150;
   if(at==0){
     g.attacking=false;clearInput();g.mx=g.my=0;j.active=false;g.attackTime=g.attackCd=0;
     int biome=biomes[scene];
     if(biome){findBiome(biome);placePlayer(o.waypointX+.5,o.waypointY+.5);}
     else if(scene==2||scene==3){auto c=campAt(scene==3?1:0,0);placePlayer(c.x-2,c.y+2);}
     else if(scene==8)placePlayer(0,0);else placePlayer(-17,-12);
     if(scene==1){
       auto props=g.props;
       for(const auto&p:props)if(p.kind==0&&!treeCut(p)){
         auto key=treeKey(p);auto t=tileAt(key.first,key.second+2);
         if(t.map==1){placePlayer(key.first+.5,key.second+2.5);break;}
       }
     }
     switchWeapon(scene==1?AXE:scene==3?BOW:SWORD);
     g.fx=scene==1?0:1;g.fy=scene==1?-1:0;
     g.camReady=false;mediumCameraZoom=1;mediumCanopyFade.clear();o.waypoint=false;
     w.clockOffset=std::fmod(double(hours[scene]-8)*60-o.seconds+14400.,1440.);
     g.toastTime=0;g.overlay=0;
   }
   // Explicit staging grants for visual coverage only; never user save data.
   g.invul=100;v.stamina=100;g.hp=maxhp();g.toastTime=0;
   g.attacking=scene==1||scene==2||scene==3;
   if(scene==0||scene>=4&&scene<=7){g.mx=at>20&&at<120?.45f:0;g.my=at>55&&at<120?-.25f:0;}
   else g.mx=g.my=0;
   if(scene==2&&at==80)beginGuard();
   if(scene==3&&at==75)skill(1);
   graphicsController.request(gfx::GraphicsQuality::Medium);
   frame(pixels.data(),1.f/30);mediumCameraZoom=1; // identical field of view for comparison
   if(!mediumGPU.draw(pixels.data(),W,H,0,0,1,false))return 4;
   glReadPixels(0,0,W,H,GL_RGBA,GL_UNSIGNED_BYTE,gpu.data());
   graphicsController.request(gfx::GraphicsQuality::Low);
   frame(pixels.data(),0); // same simulation instant; no second tick
   for(int y=0;y<H;y++)for(int x=0;x<W;x++){
     C a=pixels[y*W+x],b=gpu[(H-1-y)*W+x];size_t p=(y*W*2+x)*3,q=p+W*3;
     rgb[p]=a>>16;rgb[p+1]=a>>8;rgb[p+2]=a;
     rgb[q]=b;rgb[q+1]=b>>8;rgb[q+2]=b>>16;
   }
   std::cout.write(reinterpret_cast<const char*>(rgb.data()),rgb.size());
   float time=frameNumber/30.f;int m=av_music(),a=av_audio();
   if(m!=previousMusic){events<<time<<" music "<<m<<'\n';previousMusic=m;}
   if(a!=previousAmbience){events<<time<<" ambience "<<a<<'\n';previousAmbience=a;}
   for(int id;(id=av_sound())>0;)events<<time<<" sound "<<id<<'\n';
 }
 mediumGPU.destroy();std::filesystem::remove_all(root);
}
