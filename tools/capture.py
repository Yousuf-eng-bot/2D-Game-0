#!/usr/bin/env python3
"""Optional C++ renderer screenshots (not Android captures). Needs Pillow.
Build libashen.so with CMake, then run this from any directory.
"""
import ctypes, pathlib, os, tempfile, shutil
from PIL import Image
root=pathlib.Path(__file__).resolve().parents[1]
lib=ctypes.CDLL(os.environ.get('ASHEN_HOST_LIBRARY',str(root/'build/libashen.so')))
lib.av_boot.argtypes=[ctypes.c_char_p]
lib.av_frame.argtypes=[ctypes.POINTER(ctypes.c_uint32),ctypes.c_float]
lib.av_touch.argtypes=[ctypes.c_int,ctypes.c_int,ctypes.c_float,ctypes.c_float]
lib.av_submit_text.argtypes=[ctypes.c_int,ctypes.c_char_p]
buf=(ctypes.c_uint32*(640*360))()
out=root/'build/captures';out.mkdir(parents=True,exist_ok=True)
def advance(seconds):
 for _ in range(int(seconds*60)):lib.av_frame(buf,1/60)
def shot(name):
 lib.av_frame(buf,0)
 Image.frombytes('RGBA',(640,360),bytes(buf),'raw','BGRA').convert('RGB').save(out/(name+'.png'))
def tap(x,y):
 lib.av_touch(0,0,x,y);lib.av_touch(1,0,x,y);advance(.6)
with tempfile.TemporaryDirectory() as tmp:
 shutil.copy(root/'android/assets/cover.bin',pathlib.Path(tmp)/'cover.bin')
 lib.av_boot(tmp.encode());advance(1.9);shot('studio');advance(2.5);shot('home')
 tap(320,235);shot('world-library');tap(320,82);shot('create-world')
 lib.av_submit_text(1,b'HOST PREVIEW');lib.av_submit_text(2,b'3032026')
 tap(320,318);advance(.8);shot('origin-camp');tap(585,36);shot('atlas')
print('Saved native-renderer captures:',out)
