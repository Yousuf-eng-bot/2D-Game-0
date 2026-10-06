#!/usr/bin/env python3
"""Convert the supplied AI-generated v0.5 title artwork to the native RGB565 asset."""
from pathlib import Path
from PIL import Image, ImageOps
import numpy as np
root=Path(__file__).resolve().parents[1]
im=ImageOps.fit(Image.open(root/'assets/wild-earth-menu.png').convert('RGB'),(640,360),method=Image.Resampling.LANCZOS)
p=np.array(im).astype(np.uint16)
rgb=((p[:,:,0]>>3)<<11)|((p[:,:,1]>>2)<<5)|(p[:,:,2]>>3)
(root/'android/assets/cover.bin').write_bytes(rgb.astype('<u2').tobytes())
im.save(root/'assets/menu-preview.png')
print('Updated natural woodland title panorama.')
