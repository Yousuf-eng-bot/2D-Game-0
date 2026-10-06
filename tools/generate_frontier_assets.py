#!/usr/bin/env python3
"""Original synthesized music, UI/character Foley and nature beds. No recordings.
The prepared outputs ship in the APK/source; Python+Pillow+NumPy only needed to regenerate.
"""
import pathlib, wave, numpy as np
from PIL import Image, ImageOps
R=pathlib.Path(__file__).resolve().parents[1]
A=R/'android/assets'; (A/'music').mkdir(exist_ok=True)
rate=22050
rng=np.random.default_rng(303)
def wav(path,samples):
    a=np.asarray(samples,dtype=np.float64)
    a=np.nan_to_num(a)
    peak=np.max(np.abs(a))
    if peak>.94: a*=.94/peak
    with wave.open(str(path),'wb') as f:
        f.setnchannels(1);f.setsampwidth(2);f.setframerate(rate);f.writeframes((np.clip(a,-1,1)*32767).astype('<i2').tobytes())
def tone(freq,dur,amp=.1,kind='bell'):
    t=np.arange(int(rate*dur))/rate
    if kind=='pad':
        a=(np.sin(2*np.pi*freq*t)+.22*np.sin(2*np.pi*freq*2*t)+.1*np.sin(2*np.pi*freq*3*t))
        env=np.minimum(t/.4,1)*np.minimum((dur-t)/.7,1)
    else:
        a=np.sin(2*np.pi*freq*t)+.23*np.sin(2*np.pi*freq*2*t)*np.exp(-t*4)+.08*np.sin(2*np.pi*freq*3*t)
        env=(1-np.exp(-t*130))*np.exp(-t*(2.2 if kind=='bell' else 4.5))
    return a*env*amp
def hz(n):return 440*2**((n-69)/12)
# 16 bars, 4 beats each at 100 BPM = 38.4 seconds. Circular rendering makes
# reverb/note tails cross the loop boundary rather than click or go silent.
N=int(38.4*rate)
for biome in range(6):
    a=np.zeros(N)
    def add(start,s):
        idx=(int(start*rate)+np.arange(len(s)))%N
        np.add.at(a,idx,s)
    roots=[50,55,58,53] if biome in (0,1,3) else [45,48,50,43]
    shift=[0,2,-2,7,-5,-7][biome]
    for bar in range(16):
        root=roots[(bar//2)%4]+shift
        for interval in [0,7,14]:add(bar*2.4,tone(hz(root+interval),3.4,.034,'pad'))
        add(bar*2.4,tone(hz(root-12),1.5,.062,'pluck'))
        for step in range(4):
            note=root+[12,19,24,14,19,26,17,24][(bar+step)%8]
            if biome==4 and step%2:continue
            add(bar*2.4+step*.6,tone(hz(note),1.8,.065 if biome==0 else .045,'bell'))
        # Soft pulse, never competing with the attack sounds.
        if biome in (2,5):
            t=np.arange(int(.15*rate))/rate
            beat=np.sin(2*np.pi*(65*t-55*t*t))*np.exp(-t*30)*.08
            add(bar*2.4,beat);add(bar*2.4+1.2,beat*.65)
    a+=np.roll(a,int(.27*rate))*.14+np.roll(a,int(.51*rate))*.07
    wav(A/'music'/f'{biome+1}.wav',a)
# Transition whoosh, crystalline studio ident, three walking surfaces, hurt grunt, guardian roar.
t=np.arange(int(.38*rate))/rate
noise=rng.normal(0,1,len(t));noise=np.convolve(noise,np.ones(7)/7,'same')
wav(A/'audio/15.wav',noise*np.sin(np.pi*t/.38)**2*.45+tone(660,.38,.12))
s=np.zeros(int(1.8*rate))
for delay,note in [(0,50),(.17,57),(.35,62),(.52,69)]:
    q=tone(hz(note),1.15,.17);i=int(delay*rate);s[i:i+len(q)]+=q
wav(A/'audio/16.wav',s)
for i in [17,18,19]:
    dur=.15;t=np.arange(int(dur*rate))/rate;n=rng.normal(0,1,len(t));smooth=np.convolve(n,np.ones(6 if i==17 else 2)/ (6 if i==17 else 2),'same')
    a=smooth*np.exp(-t*(36 if i!=19 else 24))*.27+np.sin(2*np.pi*(90 if i==17 else 140)*t)*np.exp(-t*43)*.1
    wav(A/'audio'/f'{i}.wav',a)
t=np.arange(int(.28*rate))/rate
phase=2*np.pi*(125*t-55*t*t)
voice=(np.sin(phase)+.5*np.sin(phase*2)+.2*np.sin(phase*5))*(1-np.exp(-t*70))*np.exp(-t*11)
wav(A/'audio/20.wav',voice*.3)
t=np.arange(int(.85*rate))/rate
phase=2*np.pi*(58*t+11*np.sin(t*5))
roar=(np.sin(phase)+.32*np.sin(phase*3)+rng.normal(0,.12,len(t)))*np.sin(np.pi*t/.85)**1.4
wav(A/'audio/21.wav',roar*.32)
for i in [22,23,24]:
    duration=8;t=np.arange(int(duration*rate))/rate;n=rng.normal(0,1,len(t));
    base=np.convolve(n,np.ones(90)/90,'same')*.8
    if i==22:base+=np.sin(2*np.pi*118*t)*(.017+.009*np.sin(2*np.pi*t/8))
    if i==23:
        for k in range(6):
            start=k*1.25+.2;q=np.arange(int(.3*rate))/rate;f=320+40*(k%3);s=np.sin(2*np.pi*f*q)*np.exp(-q*15)*.12
            j=int(start*rate);base[j:j+len(q)]+=s
    if i==24:base+=np.sin(2*np.pi*43*t)*.08+np.sin(2*np.pi*71*t)*.04
    # Short edge fade avoids a hard discontinuity on loops.
    edge=int(.04*rate);base[:edge]*=np.linspace(0,1,edge);base[-edge:]*=np.linspace(1,0,edge)
    wav(A/'audio'/f'{i}.wav',base)
im=ImageOps.fit(Image.open((R/'assets/wild-earth-menu.png')).convert('RGB'),(640,360),method=Image.Resampling.LANCZOS)
p=np.array(im).astype(np.uint16)
rgb=((p[:,:,0]>>3)<<11)|((p[:,:,1]>>2)<<5)|(p[:,:,2]>>3)
(A/'cover.bin').write_bytes(rgb.astype('<u2').tobytes())
im.save(R/'assets/menu-preview.png')
print('Prepared panorama, six original 38.4-second music loops, character/UI sounds and five biome ambience beds.')
