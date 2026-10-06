#pragma once
// Versioned, coordinate-pure natural generation. Never used for old worlds.
namespace av {
struct NaturalSegment { double ax,ay,bx,by; };
float naturalPathDistance(int64_t x, int64_t y) {
  // A sparse seeded graph of jittered anchors and quadratic curves. Not a
  // modulus road grid (nor just parallel sinusoidal roads). Cache is bounded,
  // keyed in global coordinates, and has no dependency on render quality.
  constexpr int spacing=96;
  using Key=std::pair<int64_t,int64_t>;
  static uint64_t cachedSeed=0;
  static std::map<Key,std::vector<NaturalSegment>> cache;
  if(cachedSeed!=o.seed){cache.clear();cachedSeed=o.seed;}
  Key cell{floorDiv(x,spacing),floorDiv(y,spacing)};
  auto it=cache.find(cell);
  if(it==cache.end()){
    if(cache.size()>=64)cache.clear();
    std::vector<NaturalSegment> lines;
    auto node=[](int64_t cx,int64_t cy){auto h=coordinateHash(cx,cy,912371);return std::pair<double,double>{double(cx*spacing+16+int(h%64)),double(cy*spacing+16+int((h>>12)%64))};};
    auto curve=[&](double ax,double ay,double bx,double by,uint64_t h){
      double dx=bx-ax,dy=by-ay,l=std::max(1.,std::hypot(dx,dy));
      double bend=int(h%35)-17,xx=(ax+bx)*.5-dy/l*bend,yy=(ay+by)*.5+dx/l*bend;
      double px=ax,py=ay;
      for(int k=1;k<=4;k++){double t=k*.25,u=1-t,nx=u*u*ax+2*u*t*xx+t*t*bx,ny=u*u*ay+2*u*t*yy+t*t*by;lines.push_back({px,py,nx,ny});px=nx;py=ny;}
    };
    for(int cy=-1;cy<=1;cy++)for(int cx=-1;cx<=1;cx++){
      int64_t gx=cell.first+cx,gy=cell.second+cy;auto a=node(gx,gy);auto h=coordinateHash(gx,gy,581399);
      // One guaranteed outward edge, occasional secondary links and loops.
      for(int n=0;n<2;n++)if(n==int(h%2)||((h>>7)%4==0)){
        auto b=node(gx+(n==0),gy+(n==1));curve(a.first,a.second,b.first,b.second,h+n*77);
      }
    }
    if(std::abs(cell.first)<=1&&std::abs(cell.second)<=1){
      auto a=node(0,0);curve(0,0,a.first,a.second,912); // safe, non-grid departure from camp
      for(int cx=0;cx<=1;cx++){auto c=campAt(cx,0);curve(c.x,c.y,a.first,a.second,c.hash);}
    }
    it=cache.emplace(cell,std::move(lines)).first;
  }
  double best=1e12;
  for(const auto&s:it->second){double dx=s.bx-s.ax,dy=s.by-s.ay,l=dx*dx+dy*dy,t=std::clamp(((x-s.ax)*dx+(y-s.ay)*dy)/std::max(1.,l),0.,1.);double px=x-(s.ax+dx*t),py=y-(s.ay+dy*t);best=std::min(best,px*px+py*py);}
  return float(std::sqrt(best));
}
Tile naturalTileAt(int64_t x, int64_t y) {
  Tile t;
  t.biome = uint8_t(biomeAt(x,y));
  auto c = campAt(floorDiv(x,CELL),floorDiv(y,CELL));
  double dx = double(x-c.x), dy = double(y-c.y);
  float edge = noiseAt(x,y,9,51831);
  bool clearing = c.exists && dx*dx+dy*dy < (c.boss ? 74 : 34) + edge*18;
  bool spawn = std::abs(x)<7 && std::abs(y)<7;
  float p = naturalPathDistance(x,y);
  bool trail = p < .75f + noiseAt(x,y,6,7415)*.65f;
  float wet = noiseAt(x,y,29,2345)*.7f + noiseAt(x,y,8,441)*.3f;
  if (clearing || spawn || trail) {
    t.ground = clearing ? 4 : trail ? 2 : 0;
    if (trail && wet < .23f && !clearing && !spawn) t.ground = 3;
  } else if (wet < (t.biome==3 ? .35f : .20f)) t.map = 4;
  else {
    auto h=coordinateHash(x,y,987);
    float grove=noiseAt(x,y,21,57311)*.62f + noiseAt(x,y,7,87211)*.38f;
    float density = t.biome==0 ? 3 + std::max(0.f,grove-.30f)*29
                  : t.biome==3 ? 4+grove*16 : t.biome==1 ? 1+grove*4 : 2+grove*11;
    // Irregular glades and scattered edge saplings, not uniformly random rows.
    if (p<3) density*=.42f;
    if (float(h%1000)<density*10) t.map=2;
    else if ((h>>12)%100<3) t.map=3;
  }
  if (t.map==2 && treeCut(x,y)) t.map=1;
  return t;
}
} // namespace av
