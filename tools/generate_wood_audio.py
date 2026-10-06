#!/usr/bin/env python3
"""Original deterministic wood chop / felling effects, 22.05kHz mono PCM."""
from pathlib import Path
import numpy as np,wave
out=Path(__file__).resolve().parents[1]/'android/assets/audio';r=22050
rng=np.random.default_rng(20261005)
for ident,duration in [(25,.28),(26,1.25)]:
 t=np.arange(int(duration*r))/r
 if ident==25:
  noise=rng.uniform(-1,1,len(t));a=.55*noise*np.exp(-t*47)+.35*np.sin(2*np.pi*(190*t-50*t*t))*np.exp(-t*26)
 else:
  noise=rng.uniform(-1,1,len(t));a=.18*noise*np.exp(-t*2)+.2*np.sin(2*np.pi*(92*t-18*t*t))*np.exp(-t*3)
  for when in [.05,.17,.32,.53,.81]:a+=.22*noise*np.exp(-np.maximum(0,t-when)*40)*(t>=when)
 a=np.clip(a,-.9,.9)*(1-np.minimum(1,t/duration)**6)
 with wave.open(str(out/f'{ident}.wav'),'wb') as f:f.setparams((1,2,r,len(t),'NONE','not compressed'));f.writeframes((a*32767).astype('<i2').tobytes())
