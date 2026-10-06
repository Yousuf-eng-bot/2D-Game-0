#!/usr/bin/env python3
"""Side-by-side native GPU renderer demo, not an Android recording. Needs numpy/imageio-ffmpeg.
Usage: python3 tools/make_preview.py HOST_CAPTURE_BINARY OUTPUT.mp4
Its audio is mixed from the same bundled assets at the native event timestamps;
it does not establish Android output latency, device FPS or audio quality.
"""
from pathlib import Path
import sys, subprocess, tempfile, wave, numpy as np
import os
from restore_audio import restore
restore()
root=Path(__file__).resolve().parents[1]
capture=str(Path(sys.argv[1]).resolve());output=str(Path(sys.argv[2]).resolve())
ffmpeg="ffmpeg"
rate=22050;duration=int(sys.argv[3]) if len(sys.argv)>3 else 45;N=int(rate*duration)
with tempfile.TemporaryDirectory() as temp:
    temp=Path(temp);events=temp/'events.txt';silent=temp/'silent.mp4';audio=temp/'mix.wav'
    render=subprocess.Popen([capture,str(events)],cwd=root,stdout=subprocess.PIPE,env={**os.environ,"EGL_PLATFORM":"surfaceless","LIBGL_ALWAYS_SOFTWARE":"1","LP_NUM_THREADS":"2"})
    encode=subprocess.run([ffmpeg,'-v','error','-y','-f','rawvideo','-pixel_format','rgb24','-video_size','1280x360','-framerate','30','-i','pipe:0','-vf',"pad=1280:432:0:38:color=0x101c22,drawtext=fontfile=assets/fonts/NotoSans.ttf:text='LOW - preserved appearance':x=18:y=7:fontsize=19:fontcolor=0xd9bd83,drawtext=fontfile=assets/fonts/NotoSans.ttf:text='MEDIUM - normal-map GPU lighting':x=658:y=7:fontsize=19:fontcolor=0xd9bd83,drawtext=fontfile=assets/fonts/NotoSans.ttf:text='STAGED NATIVE GL COMPARISON - NOT PHONE FOOTAGE OR AN FPS MEASUREMENT':x=18:y=404:fontsize=15:fontcolor=0xb1c3be",'-an','-c:v','libx264','-threads','1','-preset','veryfast','-crf','21','-pix_fmt','yuv420p',str(silent)],stdin=render.stdout,check=True)
    render.stdout.close()
    if render.wait()!=0:raise RuntimeError('Native render failed')
    timeline=[(float(t),kind,int(ident)) for t,kind,ident in (line.split() for line in events.read_text().splitlines())]
    mix=np.zeros(N,dtype=np.float64)
    def load(p):
        with wave.open(str(p),'rb') as w:
            assert w.getframerate()==rate and w.getnchannels()==1
            return np.frombuffer(w.readframes(w.getnframes()),dtype='<i2').astype(float)/32768
    cache={}
    def pcm(p):
        if p not in cache:cache[p]=load(p)
        return cache[p]
    for kind in ['music','ambience']:
        changes=[(t,i) for t,k,i in timeline if k==kind]+[(duration,0)]
        for j,(t,ident) in enumerate(changes[:-1]):
            if not ident:continue
            end=min(N,int(changes[j+1][0]*rate));start=int(t*rate)
            path=root/'android/assets'/('music' if kind=='music' else 'audio')/f'{ident if kind=="music" else [0,13,14,22,23,24][ident]}.wav'
            data=pcm(path);n=end-start
            if n>0:mix[start:end]+=np.resize(data,n)*(.32 if kind=='music' else .24)
    for t,kind,ident in timeline:
        if kind!='sound':continue
        data=pcm(root/'android/assets/audio'/f'{ident}.wav');start=int(t*rate);n=min(len(data),N-start)
        if n>0:mix[start:start+n]+=data[:n]*(.3 if 17<=ident<=19 else .62)
    if np.max(np.abs(mix))>.97:mix*=.97/np.max(np.abs(mix))
    with wave.open(str(audio),'wb') as w:
        w.setnchannels(1);w.setsampwidth(2);w.setframerate(rate);w.writeframes((mix*32767).astype('<i2').tobytes())
    subprocess.run([ffmpeg,'-v','error','-y','-i',str(silent),'-i',str(audio),'-c:v','copy','-c:a','aac','-b:a','96k','-shortest','-movflags','+faststart',output],check=True)
print(f'Created {duration}s native renderer preview:',output)
