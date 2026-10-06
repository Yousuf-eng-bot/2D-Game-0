#!/usr/bin/env python3
"""Download pinned official Linux Android tools after you review the SDK/NDK licenses.
Needs Python 3.9+. No pip dependencies. Around 0.8 GB download / 3 GB unpacked.
Tools remain in ~/.cache/android unless SDK_ROOT overrides it.
"""
import hashlib, os, pathlib, stat, urllib.request, zipfile
ROOT=pathlib.Path(os.environ.get('SDK_ROOT',pathlib.Path.home()/'.cache/android'))
PACKAGES={
 'ndk':('android-ndk-r27c-linux.zip','59c2f6dc96743b5daf5d1626684640b20a6bd2b1d85b13156b90333741bad5cc'),
 'tools':('build-tools_r34-linux.zip','e858c4b60069d0431051b225d384413b1643e1289b00a4825aed347f25bd510f'),
 'platform':('platform-35_r02.zip','0988cacad01b38a18a47bac14a0695f246bc76c1b06c0eeb8eb0dc825ab0c8e0'),
}
ROOT.mkdir(parents=True,exist_ok=True)
for name,(filename,expected) in PACKAGES.items():
 archive=ROOT/(name+'.zip')
 if not archive.exists():
  print('Downloading',filename,flush=True)
  urllib.request.urlretrieve('https://dl.google.com/android/repository/'+filename,archive)
 with archive.open('rb') as f:
  h=hashlib.sha256()
  for chunk in iter(lambda:f.read(1024*1024),b''):h.update(chunk)
 if h.hexdigest()!=expected:raise RuntimeError('Checksum mismatch: '+str(archive))
 dest=ROOT/name
 if not dest.exists():
  print('Extracting',name,flush=True)
  with zipfile.ZipFile(archive) as z:
   z.extractall(dest)
   for info in z.infolist():
    path=dest/info.filename;mode=info.external_attr>>16
    if stat.S_ISLNK(mode):
     path.unlink();path.symlink_to(z.read(info).decode())
    elif mode and not info.is_dir():path.chmod(mode)
 print('Ready:',dest)
print('Now run: bash tools/build_apk.sh')
