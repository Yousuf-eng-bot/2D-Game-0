#include "../native/engine.cpp"
#include <iostream>
#include <queue>
using namespace av;
int checks=0;
#define CHECK(x) do{checks++;if(!(x)){std::cerr<<"FAIL "<<__LINE__<<": "#x"\n";return 1;}}while(0)
int main(){
 auto root=std::filesystem::temp_directory_path()/("medium-test-"+hexId(entropy()));
 boot(root.string());o.draftName="MEDIUM TEST";o.draftSeed=20261005;CHECK(createWorld());CHECK(o.generator==6);
 CHECK(!mediumEnabled());
 graphicsController.setDevice({8192,3,4096,false,true,true});
 u.tips=false;g.toastTime=0;motionBlur=false;g.shake=false;
 std::vector<C> p(W*H);
 auto map=g.map;auto ht=j.heights;auto state=encodeJourney();auto seed=rng;auto px=g.px,py=g.py;
 for(int k=0;k<12;k++){
   graphicsController.request(k%2?gfx::GraphicsQuality::Medium:gfx::GraphicsQuality::Low);
   frame(p.data(),0);
   CHECK(g.map==map&&j.heights==ht&&state==encodeJourney()&&rng==seed&&g.px==px&&g.py==py);
   CHECK(mediumWorldCaptured==bool(k%2));
 }
 CHECK(graphicsSave());graphicsController.request(gfx::GraphicsQuality::Low);graphicsBoot();CHECK(mediumEnabled());
 graphicsController.setDevice({8192,3,4096,false,false,false});CHECK(!mediumEnabled());
 CHECK(graphicsController.settings().requested==gfx::GraphicsQuality::Medium);
 graphicsController.setDevice({8192,3,4096,false,true,true});
 // Existing UI hit registry, not a test-only switch API.
 uOpen(12);u.settingsTab=3;frame(p.data(),0);
 bool found=false;
 for(auto h:u.hits)if(h.action==410&&h.arg==0){touch(0,9,h.x+h.w/2,h.y+h.h/2);touch(1,9,0,0);found=true;break;}
 CHECK(found&&!mediumEnabled());
 uAction(410,1,9);CHECK(mediumEnabled());
 g.overlay=0;clearInput();
 // All promised direction/clip frames exist, are nonempty and normal mapped.
 for(int role=0;role<8;role++)for(int dir=0;dir<8;dir++)for(int clip=0;clip<=MPRONE;clip++)for(int f=0;f<8;f++){
   const auto&s=mediumActor(role,dir,clip,f,role%4);
   CHECK(s.w>=48&&s.h>=64&&s.color.size()==s.normal.size());
   CHECK(std::count_if(s.color.begin(),s.color.end(),[](C c){return c!=0;})>200);
   CHECK(std::count_if(s.normal.begin(),s.normal.end(),[](C c){return c&&c!=0x008080ff;})>100);
 }
 CHECK(mediumActors.size()<=192);
 auto idle=mediumActor(0,2,MIDLE,0,0).color;
 CHECK(idle!=mediumActor(0,2,MWALK,2,0).color);
 CHECK(idle!=mediumActor(0,6,MIDLE,0,0).color);
 // SDF geometry unaffected by cache eviction, seed switches or negative cells.
 for(int x=-210;x<211;x+=13)for(int y=-210;y<211;y+=17){
   auto a=baseTileAt(x,y);float d=naturalPathDistance(x,y);
   auto saved=o.seed;o.seed=123;naturalPathDistance(x+700,y-900);o.seed=saved;
   auto b=baseTileAt(x,y);CHECK(a.map==b.map&&a.ground==b.ground&&a.biome==b.biome);CHECK(std::abs(d-naturalPathDistance(x,y))<.0001f);
 }
 int boundaryRoads=0,total=0,trees=0;
 for(int x=-128;x<128;x++)for(int y=-128;y<128;y++){
   auto t=baseTileAt(x,y);trees+=t.map==2;
   if(floorMod(x,32)==0){boundaryRoads+=t.ground==2||t.ground==3;total++;}
 }
 CHECK(boundaryRoads<float(total)*.35f);CHECK(trees>1000);
 // Real input-followed route, ordinary survival (no invulnerability/items).
 graphicsController.request(gfx::GraphicsQuality::Low);
 g.overlay=0;g.toastTime=0;placePlayer(0,0);g.mx=g.my=0;
 constexpr int S=97,CENTER=48;
 std::array<int,S*S> prev;prev.fill(-1);int start=CENTER*S+CENTER;prev[start]=start;
 std::queue<int> queue;queue.push(start);int goal=start;
 auto coords=[](int n){return std::pair<int,int>{n%S-CENTER,n/S-CENTER};};
 auto pass=[&](int a,int b){auto [x,y]=coords(a);auto [xx,yy]=coords(b);auto t=tileAt(xx,yy);auto p=tileAt(x,y);if(t.map!=1)return false;int dh=std::abs(landHeight(x,y)-landHeight(xx,yy));return dh==0||dh==1&&(t.ground>0||p.ground>0||floorMod(x,8)==0||floorMod(y,8)==0);};
 while(!queue.empty()){
   int a=queue.front();queue.pop();auto[x,y]=coords(a);
   if(x<0&&y<0&&x+y<coords(goal).first+coords(goal).second)goal=a;
   for(int d:{-1,-S,1,S}){int b=a+d;if(b<0||b>=S*S||std::abs(b%S-a%S)+std::abs(b/S-a/S)!=1||prev[b]>=0||!pass(a,b))continue;prev[b]=a;queue.push(b);}
 }
 std::vector<int> route;for(int n=goal;n!=start;n=prev[n])route.push_back(n);std::reverse(route.begin(),route.end());
 CHECK(route.size()>35);
 int reached=0;
 for(int n:route){auto[x,y]=coords(n);bool arrived=false;
   for(int step=0;step<180;step++){
     float dx=float((x+.5-globalX())*T),dy=float((y+.5-globalY())*T),d=std::hypot(dx,dy);
     if(d<3){arrived=true;break;}
     float strength=std::min(1.f,d/14);g.mx=dx/d*strength;g.my=dy/d*strength;tick(1.f/60);
     if(g.scene!=PLAY)break;
   }
   if(!arrived){std::cerr<<"route blocked after "<<reached<<" nodes at "<<globalX()<<","<<globalY()<<" target "<<x<<","<<y<<" HP "<<g.hp<<"\n";break;}reached++;
 }
 CHECK(reached>35&&g.scene==PLAY&&g.hp>0);
 std::cout<<"PASS natural-world ordinary-input route "<<reached<<" tiles; HP "<<g.hp<<"; no grants/invulnerability\n";
 std::cout<<"PASS "<<checks<<" assertions; 8 directions x 14 clips x 8 frames x 8 roles; CPU cache bounded; policy independent of game state\n";
 std::filesystem::remove_all(root);
}
