#pragma once
// Original pixel sprites authored as small deterministic drawing programs.
// Frames are cached, with albedo + height-derived tangent-space normal maps.
// The current backend CPU-composes these into a G-buffer; GPU lighting is real,
// but this is intentionally NOT described as a fully GPU-batched sprite engine.
namespace av {
struct MediumSprite {
  int w=72,h=88;
  std::vector<C> color, normal;
  std::vector<uint8_t> height;
  explicit MediumSprite(int width=72,int height_=88):w(width),h(height_),color(w*h),normal(w*h),height(w*h){}
  void dot(int x,int y,C c,int z=12) {
    if(x>=0&&y>=0&&x<w&&y<h){color[y*w+x]=c;height[y*w+x]=uint8_t(std::clamp(z,0,127));}
  }
  void bar(int x,int y,int ww,int hh,C c,int z=12) {
    for(int yy=y;yy<y+hh;yy++)for(int xx=x;xx<x+ww;xx++)dot(xx,yy,c,z);
  }
  void stroke(int x,int y,int xx,int yy,C c,int width=1,int z=12) {
    int dx=std::abs(xx-x),sx=x<xx?1:-1,dy=-std::abs(yy-y),sy=y<yy?1:-1,e=dx+dy;
    for(;;){bar(x-width/2,y-width/2,width,width,c,z);if(x==xx&&y==yy)break;int e2=2*e;if(e2>=dy){e+=dy;x+=sx;}if(e2<=dx){e+=dx;y+=sy;}}
  }
  void oval(int x,int y,int rx,int ry,C c,int z=18) {
    for(int yy=-ry;yy<=ry;yy++)for(int xx=-rx;xx<=rx;xx++){
      float d=float(xx*xx)/std::max(1,rx*rx)+float(yy*yy)/std::max(1,ry*ry);
      if(d<=1)dot(x+xx,y+yy,c,z+int(std::sqrt(1-d)*12));
    }
  }
  void bake() {
    for(int y=0;y<h;y++)for(int x=0;x<w;x++)if(color[y*w+x]){
      auto z=[&](int xx,int yy){xx=std::clamp(xx,0,w-1);yy=std::clamp(yy,0,h-1);return color[yy*w+xx]?height[yy*w+xx]:height[y*w+x];};
      float nx=(z(x-1,y)-z(x+1,y))*.09f,ny=(z(x,y-1)-z(x,y+1))*.09f,nz=1;
      float d=std::sqrt(nx*nx+ny*ny+1);
      normal[y*w+x]=(uint32_t(std::clamp(h-8-y,0,120))<<24)|(uint32_t(int(128+nx/d*126))<<16)|(uint32_t(int(128+ny/d*126))<<8)|uint32_t(int(128+nz/d*126));
    }
  }
};
enum MediumClip { MIDLE, MWALK, MRUN, MSTRIKE1, MSTRIKE2, MSTRIKE3, MCHOP, MBOW, MGUARD, MDODGE, MHURT, MDEATH, MCROUCH, MPRONE };
struct MediumCached { MediumSprite sprite; uint64_t age=0; };
std::map<uint64_t,MediumCached> mediumActors;
uint64_t mediumAge=0;
MediumSprite mediumMakeActor(int role,int direction,int clip,int frame,int gear) {
  MediumSprite s;
  float a=direction*PI/4,fx=std::cos(a),fy=std::sin(a),rx=-fy,ry=fx;
  float t=frame/8.f,cycle=t*2*PI;
  bool moving=clip==MWALK||clip==MRUN;
  int bob=moving?int(std::abs(std::sin(cycle))*(clip==MRUN?3:2)):int(std::sin(cycle)*1.2f);
  int lean=clip==MHURT?-6:(clip>=MSTRIKE1&&clip<=MCHOP)?int(std::sin(t*PI)*(clip==MSTRIKE2?-3:clip==MSTRIKE3?9:6)):0;
  int crouch=clip==MCROUCH?9:clip==MDODGE?12:0;
  const C outline=0xff18252a,skin=role?0xffa2aa91:0xffc79572,skinHi=role?0xffc6cab0:0xffe4ba8e,boot=0xff3b3031,leather=0xff70523c;
  C coat=role==0?0xff9d4341:role==1?0xff716773:role==2?0xff41786d:role==3?0xff687789:role==4?0xffb19655:role==5?0xff695979:0xff853c47;
  C coatHi=mix(coat,0xffdacaa1,70),pants=role==0?0xff404d56:0xff41424d;
  int cx=36+int(fx*lean),hip=59-bob-crouch,shoulder=38-bob-crouch,hy=24-bob-crouch;
  auto limb=[&](int x,int y,int xx,int yy,C c,int width){s.stroke(x,y,xx,yy,outline,width+2,18);s.stroke(x,y,xx,yy,c,width,23);s.stroke(x-1,y,xx-1,yy,mix(c,0xffd7c8a1,55),1,27);};
  if(clip==MDEATH||clip==MPRONE){
    int spread=clip==MPRONE?7:std::min(frame,7);
    s.oval(36,76-spread,17+spread,7,outline);
    s.oval(36,75-spread,15+spread,5,coat);
    s.oval(17,74-spread,7,6,outline);s.oval(16,73-spread,5,4,skin);
    s.stroke(43,76-spread,59,81,boot,6);s.stroke(27,76-spread,21,82,leather,4);
    s.bake();return s;
  }
  // Eight displacement-keyed foot poses, not a time-only treadmill.
  for(int order=0;order<2;order++){
    int side=(fy<0?1:-1)*(order?1:-1);
    float stride=moving?std::sin(cycle+(side<0?PI:0))*(clip==MRUN?12:8):0;
    int lift=moving?int(std::max(0.f,std::cos(cycle+(side<0?PI:0)))*5):0;
    int lx=cx+int(rx*side*6),ly=hip+int(ry*side*3);
    int bx=cx+int(rx*side*7+side*2+fx*stride),by=79+int(fy*stride*.45f+ry*side*2)-lift-bob;
    limb(lx,ly,(lx+bx)/2,(ly+by)/2,pants,6);
    limb((lx+bx)/2,(ly+by)/2,bx,by,leather,5);
    s.oval(bx+int(fx*2),by,5,3,outline);s.bar(bx-3,by-2,6,3,boot,20);
    s.stroke(bx-3,by-3,bx+2,by-3,0xffb69469,1,25);
    s.stroke(lx-2,ly+4,lx-2,ly+9,0xff8b8b78,1,25);
  }
  // Tail/cape, coat panels and stitched hem.
  s.oval(cx,hip-4,13,15,outline);
  s.oval(cx,hip-6,11,14,coat);
  s.bar(cx-11,hip,22,6,coat);
  for(int x=-9;x<=9;x+=3)s.dot(cx+x,hip+4,coatHi,22);
  s.oval(cx,shoulder+7,13,14,outline);
  s.oval(cx,shoulder+7,11,12,coat);
  s.bar(cx-8,shoulder,4,17,coatHi,30);s.bar(cx+7,shoulder+2,3,15,mix(coat,outline,80),20);
  s.stroke(cx,shoulder-1,cx,hip-6,0xffcfb27a,1,29);
  for(int y=shoulder+4;y<hip-5;y+=5)s.dot(cx+2,y,0xffe7cc93,30);
  s.bar(cx-11,hip-3,23,5,outline,16);s.bar(cx-10,hip-2,21,3,leather,20);
  s.bar(cx-2,hip-3,5,5,0xffd3ad62,29);s.bar(cx-1,hip-2,3,3,0xff463e31,23);
  s.bar(cx+8,hip-1,5,8,outline);s.bar(cx+9,hip,3,6,0xff93734d,24);
  s.dot(cx+10,hip+1,0xffdac89b,30);
  // Distinct armour/equipment tiers, kept separate from collision/weapon reach.
  if(gear>0||role==3||role>=6){
    C metal=gear>=2?0xffa6b4b6:0xff65777e;
    s.oval(cx-10,shoulder,5,4,outline);s.oval(cx-10,shoulder-1,4,3,metal);
    s.oval(cx+10,shoulder,5,4,outline);s.oval(cx+10,shoulder-1,4,3,metal);
    s.bar(cx-5,shoulder+4,10,11,mix(metal,coat,85),32);
    s.stroke(cx-4,shoulder+5,cx+4,shoulder+5,0xffdce0c6,1,35);
  }
  // Pack is particularly legible when facing north; straps remain visible south.
  if(fy<-.25f){
    s.oval(cx,shoulder+10,9,12,outline);s.oval(cx,shoulder+9,7,10,0xff927553);
    s.bar(cx-6,shoulder+2,13,5,0xffb59d74,34);
    s.bar(cx-5,shoulder+9,10,7,0xff695442,28);s.bar(cx-1,shoulder+8,3,4,0xffc9b477,36);
    s.stroke(cx-8,shoulder+4,cx-8,shoulder+17,0xffccb78c,2,32);
  }else{s.stroke(cx-8,shoulder-2,cx+6,hip-5,outline,4);s.stroke(cx-8,shoulder-2,cx+6,hip-5,0xffb19366,2,34);}
  // Hands naturally lowered while idle; attack anticipation/recovery is explicit.
  float reach=(clip>=MSTRIKE1&&clip<=MCHOP)?std::sin(t*PI)*13:clip==MGUARD?11:clip==MBOW?(t<.7f?4+t*17:8-(t-.7f)*15):0;
  for(int side:{-1,1}){
    int ax=cx+side*13,ay=shoulder+3;
    int hx=ax+int(fx*reach)+side,handY=hip-5-int(reach*.7f);
    if(moving)handY+=int(std::sin(cycle+side*PI/2)*4);
    limb(ax,ay,hx,handY,coat,5);s.oval(hx,handY+1,4,4,outline);s.oval(hx,handY,3,3,leather);
    s.bar(hx-2,handY+1,4,2,skin,28);s.dot(hx+1,handY+1,skinHi,30);
  }
  s.bar(cx-3,hy+8,7,7,outline);s.bar(cx-2,hy+8,5,5,skin,27);
  int hx=cx+int(fx*2);
  s.oval(hx,hy,10,12,outline);s.oval(hx,hy-1,8,10,role==2||role==5?coat:0xff44383a);
  if(fy>-.35f){
    s.oval(hx+int(fx*2),hy+3,7,7,skin);s.bar(hx-4,hy,9,3,skinHi,32);
    s.bar(hx-7,hy-4,14,5,0xff3a3033,29);s.bar(hx-6,hy-5,10,2,0xff75604b,31);
    if(fx<.8f)s.bar(hx-4,hy+3,2,2,outline,29);
    if(fx>-.8f)s.bar(hx+3,hy+3,2,2,outline,29);
    if(clip==MIDLE&&frame==6){s.bar(hx-4,hy+4,2,1,skin,30);s.bar(hx+3,hy+4,2,1,skin,30);}
    s.dot(hx+int(fx*4),hy+5,0xffefc79a,35);s.stroke(hx-2,hy+8,hx+2,hy+8,0xff6f4540,1,28);
  }else{s.stroke(hx-5,hy-6,hx+4,hy-6,0xff8d775e,1,28);s.bar(hx-6,hy+6,12,4,coat,25);}
  // Red scarf is the player signature, even with a changed coat/weapon.
  if(role==0){s.stroke(cx-7,hy+12,cx+6,hy+12,0xff682e32,5,29);s.stroke(cx-6,hy+11,cx+5,hy+11,0xffd0745b,2,34);s.stroke(cx-6,hy+13,cx-11+int(std::sin(cycle)*2),hy+24,0xffa94842,4,26);}
  if(role==3){s.bar(cx-24,shoulder+2,14,23,outline,26);s.bar(cx-23,shoulder+3,12,21,0xff6b7d84,30);s.stroke(cx-18,shoulder+4,cx-18,shoulder+22,0xffd2b47c,2,36);}
  if(role>=6){s.stroke(hx-8,hy-9,hx+8,hy-9,0xffdcbb72,3,34);for(int x:{-6,0,6})s.bar(hx+x-1,hy-14,3,5,0xffead496,35);}
  s.bake();return s;
}
const MediumSprite &mediumActor(int role,int dir,int clip,int frame,int gear) {
  uint64_t key=uint64_t(role)|uint64_t(dir)<<4|uint64_t(clip)<<8|uint64_t(frame)<<13|uint64_t(gear)<<17;
  auto it=mediumActors.find(key);
  if(it!=mediumActors.end()){it->second.age=++mediumAge;return it->second.sprite;}
  if(mediumActors.size()>=192){auto victim=std::min_element(mediumActors.begin(),mediumActors.end(),[](auto&a,auto&b){return a.second.age<b.second.age;});mediumActors.erase(victim);}
  return mediumActors.emplace(key,MediumCached{mediumMakeActor(role,dir,clip,frame,gear),++mediumAge}).first->second.sprite;
}
void mediumBlit(const MediumSprite &s,float x,float y,int alpha=255,float angle=0,float scale=1,bool flash=false) {
  float co=std::cos(angle),si=std::sin(angle),minX=1e9f,minY=1e9f,maxX=-1e9f,maxY=-1e9f;
  for(int a:{0,s.w})for(int b:{0,s.h}){
    float dx=(a-s.w*.5f)*scale,dy=(b-(s.h-8))*scale;
    float xx=x+dx*co-dy*si,yy=y+dx*si+dy*co;
    minX=std::min(minX,xx);maxX=std::max(maxX,xx);minY=std::min(minY,yy);maxY=std::max(maxY,yy);
  }
  for(int py=std::max(0,int(std::floor(minY)));py<=std::min(H-1,int(std::ceil(maxY)));py++)
  for(int px=std::max(0,int(std::floor(minX)));px<=std::min(W-1,int(std::ceil(maxX)));px++){
    float dx=(px-x)/scale,dy=(py-y)/scale;
    int xx=int(std::round(dx*co+dy*si+s.w*.5f)),yy=int(std::round(-dx*si+dy*co+s.h-8));
    if(xx<0||xx>=s.w||yy<0||yy>=s.h)continue;
    int q=yy*s.w+xx;C c=s.color[q];if(!c)continue;
    if(flash)c=mix(c,WHITE,170);
    pix[py*W+px]=alpha>=255?c:mix(pix[py*W+px],c,alpha);
    if(surfaceNormals){
      C n=s.normal[q];
      if(angle!=0){float nx=float((n>>16)&255)-128,ny=float((n>>8)&255)-128;n=(n&0xff0000ff)|(uint32_t(std::clamp(int(128+nx*co-ny*si),0,255))<<16)|(uint32_t(std::clamp(int(128+nx*si+ny*co),0,255))<<8);}
      surfaceNormals[py*W+px]=alpha>=255?n:(mix(surfaceNormals[py*W+px],n,alpha)&0xffffff);
    }
  }
}
int mediumDirection(float angle){return (int(std::round(angle/(PI/4)))+16)%8;}
float mediumPlayerDeath=0;
void drawMediumPlayer(int x,int y,float phase,bool moving) {
  float facing=j.active?j.aim:std::atan2(g.fy,g.fx);
  if(j.toolAnim>0)facing=j.toolAim;
  // Medium and Low now share one baked character, so the player looks and
  // animates identically on both quality tiers. Medium adds its own extras:
  // real normal lighting, the dash echo trail, swing arcs and landing dust.
  if(spriteCharactersReady()){
    if(g.hp<=0)mediumPlayerDeath+=renderDt;else mediumPlayerDeath=0;
    for(int r=0;r<3;r++)ellipse(x,y+2,15-r*2,4-r,0xff2c3733);
    int flash=g.hurtTime>0?int(std::min(1.f,g.hurtTime/.22f)*190):0;
    if(g.hurtTime>0&&int(g.time*28)%2==0)flash=std::min(255,flash+60);
    int alpha=g.hp<=0?std::max(90,255-int(mediumPlayerDeath*65)):255;
    CharOutfit kit=playerOutfit();
    int fy=y-int(j.z);
    if(g.dash>0)for(int k=3;k>=1;k--)
      drawCharActor(kit,playerAnim,facing,x-int(g.fx*k*7),fy-int(g.fy*k*5),
                    PAL_PLAYER,0,35+k*12,nullptr);
    int kx=x,ky=fy;charKnockback(g.hurtTime,g.hurtx,g.hurty,kx,ky);charLunge(facing,kx,ky);
    drawCharActor(kit,playerAnim,facing,kx,ky,PAL_PLAYER,flash,alpha,
                  surfaceNormals);
    if(g.hp<=0)return;
    int hx=x+(std::cos(facing)>=0?13:-13),hy=fy-26;
    if(j.counter>0)arc(x,fy-31,27,-PI*.9f,PI*.1f,0xffe6d39b,2);
    if(j.active && j.elapsed>=j.windup && j.elapsed<j.windup+j.activeTime){
      float weaponAngle=strikeAngle(j.elapsed);
      int rr=g.weapon==AXE?42:37;
      arc(hx,hy,rr,weaponAngle-.75f,weaponAngle,0xfff0cf92,2);
      arc(hx,hy,rr-3,weaponAngle-.5f,weaponAngle,0xffa4bdb0,1);
      if(j.heavy)ring(g.px,g.py,int(20+50*clamp((j.elapsed-j.windup)/std::max(.01f,j.activeTime),0,1)),0xffcfb87c);
    }
    drawActorDust(x,y,facing);
    return;
  }
  int clip= moving ? (len(g.vx,g.vy)>95 ? MRUN:MWALK):MIDLE;
  int frame=moving?(int(phase/10.88f*8)%8+8)%8:int(g.time*6)%8;
  if(j.stance==1)clip=MCROUCH;
  if(j.stance==2)clip=MPRONE;
  if(j.active){clip=g.weapon==BOW?MBOW:MSTRIKE1+j.combo%3;frame=std::clamp(int(j.elapsed/std::max(.01f,j.windup+j.activeTime+j.recovery)*8),0,7);}
  if(j.toolAnim>0){clip=MCHOP;frame=std::clamp(int((1-j.toolAnim/.4f)*8),0,7);facing=j.toolAim;}
  if(j.guard>0)clip=MGUARD;
  if(g.dash>0){clip=MDODGE;frame=std::clamp(int((.22f-g.dash)/.22f*8),0,7);}
  if(g.hurtTime>.17f)clip=MHURT;
  if(g.hp<=0){mediumPlayerDeath+=renderDt;clip=MDEATH;frame=std::clamp(int(mediumPlayerDeath*10),0,7);}else mediumPlayerDeath=0;
  const auto &sprite=mediumActor(0,mediumDirection(facing),clip,frame,std::clamp(equippedValue(1)/3,0,3));
  // Small contact shadow always anchors feet; sun shadow is in the world pass.
  for(int r=0;r<3;r++)ellipse(x,y+2,15-r*2,4-r,0xff2c3733);
  if(g.dash>0)for(int k=3;k>=1;k--)mediumBlit(sprite,x-int(g.fx*k*7),y-int(g.fy*k*5)-j.z,35+k*12);
  mediumBlit(sprite,float(x),float(y)-j.z,g.hp<=0?std::max(90,255-int(mediumPlayerDeath*65)):255,0,1,g.hurtTime>.18f);
  if(g.hp<=0||j.stance==2)return;
  float weaponAngle=facing+1.1f;
  if(!j.active&&j.toolAnim<=0&&j.guard<=0)weaponAngle=PI/2+.2f*std::cos(facing); // lowered idle
  if(j.active)weaponAngle=strikeAngle(j.elapsed);
  if(j.toolAnim>0){float t=1-j.toolAnim/.4f;weaponAngle=facing-1.5f+2.8f*easeJourney(t);}
  if(j.guard>0)weaponAngle=facing-PI/2;
  if(j.counter>0)arc(x,y-31,27,-PI*.9f,PI*.1f,0xffe6d39b,2);
  int hx=x+(std::cos(facing)>=0?15:-15),hy=y-26-int(j.z);
  weaponArt(hx,hy,j.toolAnim>0?AXE:g.weapon,weaponAngle,j.active&&g.weapon==BOW?.75f+.4f*clamp(j.elapsed/std::max(.01f,j.windup),0,1):j.active?1.08f:1);
  if(j.active && j.elapsed>=j.windup && j.elapsed<j.windup+j.activeTime){
    int rr=g.weapon==AXE?42:37;
    arc(hx,hy,rr,weaponAngle-.75f,weaponAngle,0xfff0cf92,2);
    arc(hx,hy,rr-3,weaponAngle-.5f,weaponAngle,0xffa4bdb0,1);
    if(j.heavy)ring(g.px,g.py,int(20+50*clamp((j.elapsed-j.windup)/std::max(.01f,j.activeTime),0,1)),0xffcfb87c);
  }
  if(j.landing>0)ellipse(x,y+3,18+int((.2f-j.landing)*45),4,0xffa4a480);
}
void drawMediumEnemy(const Enemy &e) {
  int x=sx(e.x),y=syAt(e.x,e.y);
  if(x<-95||x>W+95||y<-10||y>H+110)return;
  if(!e.alive && e.death<=0)return;
  if(spriteCharactersReady()){
    for(int r=0;r<3;r++)ellipse(x,y+2,(e.boss()?21:15)-r*2,4-r,0xff2b3633);
    drawSpriteEnemy(e,x,y,surfaceNormals);
    if(e.kind==4){circle(x+20,y-57,3,0xffc99472,true);if(surfaceNormals)for(int dy=-2;dy<=2;dy++)for(int dx=-2;dx<=2;dx++){int xx=x+20+dx,yy=y-57+dy;if(xx>=0&&xx<W&&yy>=0&&yy<H)surfaceNormals[yy*W+xx]=0xb08080ff;}}
    drawEnemyBanner(e,x,y);
    return;
  }
  int role=e.boss()?7:e.elite?6:e.kind==3?3:e.kind==1?4:e.kind==4?5:(e.entityId%3==1?2:1);
  float angle=e.alertTime>0?std::atan2(g.py-e.y,g.px-e.x):std::atan2(e.y-e.previousY,e.x-e.previousX);
  bool moving=std::hypot(e.x-e.previousX,e.y-e.previousY)>.03f;
  int clip=moving?MWALK:MIDLE,frame=int((moving?e.step*1.7f:g.time*5))%8;
  if(e.alertTime<=0&&(e.routine==WORKING||e.routine==COOKING||e.routine==GATHERING))clip=MCHOP;
  if(e.wind>0){clip=e.kind==1?MBOW:MSTRIKE1;frame=std::clamp(int((1-e.wind/std::max(.01f,e.windMax))*5),0,4);}
  if(e.stateTime>0){clip=MSTRIKE1;frame=5+std::clamp(int((.18f-e.stateTime)*16),0,2);}
  if(e.stun>0)clip=MHURT;
  if(!e.alive){clip=MDEATH;frame=std::clamp(int((1.5f-e.death)*8),0,7);}
  ellipse(x,y+2,e.boss()?21:15,4,0xff2b3633);
  const auto&s=mediumActor(role,mediumDirection(angle),clip,(frame+8)%8,role==3||role>=6?2:0);
  int alpha=e.alive?255:int(clamp(e.death/.7f,0,1)*255);
  mediumBlit(s,x,y,alpha,0,e.boss()?1.28f:role==3?1.08f:1.f,e.flash>0);
  if(e.alive){
    int weapon=e.kind==1?BOW:e.kind==3?AXE:SWORD;
    weaponArt(x+14,y-23,weapon,e.wind>0?angle-1+float(frame)*.25f:PI/2+.2f,.8f);
    if(e.kind==4){circle(x+20,y-57,3,0xffc99472,true);if(surfaceNormals)for(int dy=-2;dy<=2;dy++)for(int dx=-2;dx<=2;dx++){int xx=x+20+dx,yy=y-57+dy;if(xx>=0&&xx<W&&yy>=0&&yy<H)surfaceNormals[yy*W+xx]=0xb08080ff;}}
    box(x-20,y-79,40,5,INK,0xff68716d);rect(x-19,y-78,int(38*e.hp/e.maxhp),3,e.elite?GOLD:0xffb66b60);
    if(e.elite||e.boss())center(x,y-89,e.boss()?"GUARDIAN":"ELITE",GOLD);
    if(e.wind>0)center(x,y-100,"!",0xffffd78e,2);
  }
}
std::map<int,MediumCached> mediumTrees;
std::map<std::pair<int64_t,int64_t>,float> mediumCanopyFade;
const MediumSprite &mediumTree(int species,int biome,int variant) {
  int key=species*256+biome*32+variant;
  auto it=mediumTrees.find(key);
  if(it!=mediumTrees.end()){it->second.age=++mediumAge;return it->second.sprite;}
  if(mediumTrees.size()>=64){auto v=std::min_element(mediumTrees.begin(),mediumTrees.end(),[](auto&a,auto&b){return a.second.age<b.second.age;});mediumTrees.erase(v);}
  C*savedNormals=surfaceNormals;surfaceNormals=nullptr;
  auto&base=treeSprite(species,biome,variant);
  surfaceNormals=savedNormals;
  MediumSprite s(128,144);
  for(int y=0;y<144;y++)for(int x=0;x<128;x++){
    C c=base.pixels[y*128+x];if(!c)continue;
    int rr=c>>16&255,gg=c>>8&255,bb=c&255;
    bool leaf=gg>rr*1.1f || biome==2&&y<106;
    if(leaf){
      C tint=biome==0?(variant%4==0?0xffafa074:variant%4==1?0xff748761:0xff7e9d6c):biome==3?0xff6f8d70:0xff89957c;
      c=mix(c,tint,70);s.dot(x,y,c,15+int(22*std::max(0.f,1-std::hypot((x-64)/65.f,(y-55)/72.f))));
      if(y<95&&(hash(key*787+x*91+y*313)%487==0)){s.oval(x,y,2,2,biome==0?0xffbf8160:0xffd5c39a,34);}
    } else s.dot(x,y,mix(c,0xff8a795e,30),22+int(std::max(0,5-std::abs(x-64))));
  }
  // Bark scars and moss are material detail rather than white-noise speckles.
  for(int y=95;y<135;y+=7){s.stroke(62,y,62,y+4,0xffb29872,1,29);s.stroke(67,y+2,67,y+5,0xff414c3a,1,18);}
  s.bake();
  return mediumTrees.emplace(key,MediumCached{std::move(s),++mediumAge}).first->second.sprite;
}
void drawMediumTree(const Prop&p) {
  int x=sx(p.x),y=syAt(p.x,p.y);
  if(x<-150||x>W+150||y<-15||y>H+150)return;
  if(treeCut(p)){drawNewTree(p);return;}
  auto key=treeKey(p);
  if(mediumCanopyFade.size()>384)mediumCanopyFade.clear();
  float target=g.py<p.y+8 && g.py>p.y-127 && std::abs(g.px-p.x)<62?.28f:1.f;
  auto [it,inserted]=mediumCanopyFade.emplace(key,1.f);
  float &fade=it->second;
  fade+=(target-fade)*(1-std::exp(-12*renderDt));
  auto&s=mediumTree(treeSpecies(p),p.biome,int(coordinateHash(key.first,key.second,9187)%8));
  float sway=std::sin(g.time*1.15f+float(coordinateHash(key.first,key.second)%99))*.014f;
  if(j.toolAnim>0&&len(p.x-g.px,p.y-g.py)<85)sway+=std::sin(g.time*75)*.025f*j.toolAnim/.4f;
  mediumBlit(s,x,y,int(255*fade),sway);
  // Visibility fades the canopy smoothly; redraw the trunk at full alpha.
  if(fade<.99f){thickLine(x,y-3,x,y-27,0xff6f5c45,4);line(x-2,y-24,x-2,y-5,0xffaf9771);}
}
void mediumGroundDecor() {
  if(!mediumEnabled()||!g.openWorld)return;
  int ix=int(g.camx/T)-1,iy=int(g.camy/T)-1;
  for(int ty=std::max(0,iy);ty<std::min(MH,iy+H/T+3);ty++)for(int tx=std::max(0,ix);tx<std::min(MW,ix+W/T+3);tx++){
    int idx=ty*MW+tx;auto wx=o.originX+tx,wy=o.originY+ty;auto h=coordinateHash(wx,wy,771823);
    int x=sx(tx*T),y=syAt(tx*T,ty*T),b=g.biomes[idx];
    if(g.map[idx]==4){
      for(int yy=0;yy<T;yy++)for(int xx=0;xx<T;xx++){
        int px=x+xx,py=y+yy;if(px<0||py<0||px>=W||py>=H)continue;
        float wave=std::sin((wx*T+xx)*.10f+g.time*1.8f)+std::sin((wy*T+yy)*.13f-g.time*1.3f);
        if(surfaceNormals)surfaceNormals[py*W+px]=(uint32_t(128+int(wave*18))<<16)|(uint32_t(128+int(std::cos(wave)*17))<<8)|248;
        if(int(wave*8)==10 && ((xx+int(h))%13<5))blendPixel(px,py,0xffb9d2bd,80);
      }
      continue;
    }
    if(g.ground[idx]>=2) {
      if(h%3==0)for(int n=0;n<3;n++)line(x+5+n*3,y+int(h%15),x+7+n*3,y+int(h%15)+2,0xff79785a);
      continue;
    }
    if(b==1||b==4)continue;
    for(int k=0;k<4;k++){
      int px=x+int((h>>(k*7))%24),py=y+int((h>>(k*7+4))%22);
      float d=len(tx*T+12-g.px,ty*T+12-g.py);
      int bend=int(std::sin(g.time*1.7f+float(h%71))*2)+(d<40?int((tx*T-g.px)*.08f):0);
      C grass=b==2?0xffb0bfb3:b==3?0xff6e875f:0xff839967;
      line(px,py,px+bend,py-4-int(h%4),grass);
      line(px+2,py,px+3+bend,py-3,mix(grass,0xffc0b483,60));
      if((h+k)%19==0){circle(px+bend,py-6,2,(h%2)?0xffc6b688:0xffb08d9b,true);point(px+bend,py-6,0xffe9d8a9);}
      if((h+k)%47==0){rect(px,py-2,1,4,0xffc0aa7c);ellipse(px,py-3,3,2,0xffa45b45);point(px-1,py-4,0xffdbcda7);}
    }
  }
}
void mediumAtmosphere() {
  if(!mediumEnabled())return;
  // Fixed-size cosmetic ambient set. World RNG and saves are never touched.
  float hour=worldHour();bool night=hour<6||hour>19;
  for(int i=0;i<28;i++){
    uint32_t h=hash(i*1739+uint32_t(o.seed));
    int x=int(h%W+g.time*(3+i%4))%W,y=int((h>>12)%H+std::sin(g.time*.35f+i)*11);
    if(y<0)y+=H;
    if(night){if(i>12)continue;C c=0xffbed389;blendPixel(x,y,c,int(80+80*std::sin(g.time*2+i)));if(surfaceNormals&&x>=0&&x<W&&y>=0&&y<H)surfaceNormals[y*W+x]=0xbc8080ff;}
    else if(i%4==0){int f=int(std::sin(g.time*7+i)*2);line(x-2,y+f,x,y,0xffb6aa7c);line(x,y,x+2,y+f,0xffd1c6a4);}
    else blendPixel(x,y,0xffd4c29c,75);
  }
}
void drawMediumFallingTree(const FallingTree &f) {
  float t=clamp(f.time/.95f,0,1),angle=f.dir*t*t*1.53f;
  int alpha=int(clamp((1.35f-f.time)/.3f,0,1)*255);
  int variant=int(coordinateHash(o.originX+int64_t(std::floor(f.x/T)),o.originY+int64_t(std::floor(f.y/T)),9187)%8);
  mediumBlit(mediumTree(f.species,f.biome,variant),sx(f.x),syAt(f.x,f.y),alpha,angle);
}
void mediumReleaseCaches() {
  mediumActors.clear();mediumTrees.clear();mediumCanopyFade.clear();
}
} // namespace av
