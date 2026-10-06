#!/usr/bin/env python3
"""Reproducible preparation of original title art and synthesized, non-vocal game sounds.
Needs Pillow and NumPy. The prepared assets are included; this script need not run to build.
"""
from pathlib import Path
from PIL import Image,ImageOps
import numpy as np,wave
root=Path(__file__).resolve().parents[1]
out=root/'android/assets';(out/'audio').mkdir(parents=True,exist_ok=True)
im=ImageOps.fit(Image.open(root/'assets/wild-earth-menu.png').convert('RGB'),(640,360),method=Image.Resampling.NEAREST)
a=np.asarray(im).astype(np.uint16)
p=((a[:,:,0]>>3)<<11)|((a[:,:,1]>>2)<<5)|(a[:,:,2]>>3)
(out/'cover.bin').write_bytes(p.astype('<u2').tobytes())
im.save(root/'assets/cover-preview.png')
rate=22050;rng=np.random.default_rng(71683)
for k in range(1,15):
 dur={1:.19,2:.15,3:.22,4:.5,5:.7,6:.5,7:.22,8:.32,9:.28,10:.65,11:.19,12:.09,13:8,14:8}[k]
 t=np.arange(int(rate*dur))/rate;env=np.maximum(0,1-t/dur)**2;noise=rng.uniform(-1,1,len(t))
 low=np.convolve(noise,np.ones(7)/7,mode='same')
 if k in (1,11):s=(noise-low)*np.sin(np.pi*t/dur)**1.6*.28
 elif k in (2,8):s=(np.sin(2*np.pi*(130*t-125*t*t))*.5+noise*.5)*np.exp(-t*(25 if k==2 else 15))*.7
 elif k==3:s=(np.sin(2*np.pi*920*t)+.3*np.sin(2*np.pi*1400*t))*env*.3
 elif k==4:s=(np.sin(2*np.pi*(110*t+250*t*t))*.45+low*.8)*env
 elif k==5:s=(np.sin(2*np.pi*440*t)+.5*np.sin(2*np.pi*660*t)+.25*np.sin(2*np.pi*880*t))*env*.2
 elif k==6:s=np.sin(2*np.pi*(500*t+500*t*t))*env*.25
 elif k==7:s=(np.sin(2*np.pi*220*t)+.5*np.sin(2*np.pi*410*t))*np.exp(-t*20)*.35
 elif k==9:s=(np.sin(2*np.pi*(100*t-95*t*t))*.35+low*.7)*env
 elif k==10:s=(np.sin(2*np.pi*55*t)+np.sin(2*np.pi*73*t)*.4+low*.5)*np.sin(np.pi*t/dur)**2*.25
 elif k==12:s=np.sin(2*np.pi*690*t)*env*.15
 else:
  s=low*.025+np.sin(2*np.pi*.22*t)*low*.04
  if k==13:
   for when in [1.2,1.5,3.8,5.9,6.15]:
    u=t-when;mask=(u>0)&(u<.15);s+=mask*np.sin(2*np.pi*(1900*u+2400*u*u))*np.sin(np.clip(u/.15,0,1)*np.pi)**2*.065
  else:s+=np.sin(2*np.pi*42*t)*.008
  s*=np.minimum(1,t/.2)*np.minimum(1,(dur-t)/.2)
 with wave.open(str(out/'audio'/f'{k}.wav'),'wb') as f:
  f.setnchannels(1);f.setsampwidth(2);f.setframerate(rate);f.writeframes((np.clip(s,-1,1)*32767).astype('<i2').tobytes())
print('Prepared title image and 14 original sound assets.')
