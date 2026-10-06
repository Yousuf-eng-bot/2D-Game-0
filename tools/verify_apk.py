#!/usr/bin/env python3
"""Validate a packaged artifact; does NOT install it or assert phone gameplay."""
from pathlib import Path
from restore_audio import restore
restore()
import hashlib, io, re, subprocess, sys, tempfile, wave, zipfile
root = Path(__file__).resolve().parents[1]
apk = Path(sys.argv[1]).resolve()
sdk = Path.home()/'.cache/android'
bt = sdk/'tools/android-14'
readelf = sdk/'ndk/android-ndk-r27c/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-readelf'
expected_cert = '6860d6fd6332594af5f30916fb67d9717088f63ebecb1c8d34476d27ae8f710b'
def run(*args):
    return subprocess.check_output([str(x) for x in args], text=True, stderr=subprocess.STDOUT)
sig = run(bt/'apksigner', 'verify', '--verbose', '--print-certs', apk)
assert expected_cert in sig
assert 'Verified using v2 scheme (APK Signature Scheme v2): true' in sig
assert 'Verified using v3 scheme (APK Signature Scheme v3): true' in sig
print('PASS original certificate, v1/v2/v3 verification:', expected_cert)
badging=run(bt/'aapt','dump','badging',apk)
assert "name='com.ashenveil.game' versionCode='11' versionName='0.9.2'" in badging
for x in ["sdkVersion:'23'", "targetSdkVersion:'35'", "application-label:'Death World'"]:assert x in badging
print('PASS package, label, code 11 / 0.9.2, minimum API23 / target API35')
permissions=run(bt/'aapt','dump','permissions',apk)
assert 'uses-permission' not in permissions
print('PASS no requested Android permissions')
run(bt/'zipalign','-c','-p','4',apk)
print('PASS ZIP alignment')
methods=re.findall(r'static native\s+\S+\s+(\w+)\(', (root/'android/java/com/ashenveil/game/MainActivity.java').read_text())
assert len(methods)==18
with zipfile.ZipFile(apk) as z, tempfile.TemporaryDirectory() as tmp:
    assert z.testzip() is None
    assets=list((root/'android/assets').rglob('*'))
    assets=[p for p in assets if p.is_file()]
    for p in assets:
        name='assets/'+p.relative_to(root/'android/assets').as_posix()
        assert z.read(name)==p.read_bytes(),name
    print('PASS ZIP integrity and exact bytes of all',len(assets),'bundled assets/licenses')
    wavs=[n for n in z.namelist() if n.endswith('.wav')]
    assert len(wavs)==32
    for n in wavs:
        assert z.getinfo(n).compress_type==zipfile.ZIP_STORED,n
        with wave.open(io.BytesIO(z.read(n))) as w:
            assert w.getnchannels()==1 and w.getsampwidth()==2 and w.getframerate()==22050
            size=w.getnframes()*w.getnchannels()*w.getsampwidth()
            if '/audio/' in n:assert size<1024*1024,n
            else:assert abs(w.getnframes()/w.getframerate()-38.4)<.01,n
    print('PASS 26 short PCM effects/beds below SoundPool decoded-size bound; 6 stored 38.4s PCM music loops')
    for abi,machine in [('arm64-v8a','AArch64'),('armeabi-v7a','ARM'),('x86_64','Advanced Micro Devices X86-64')]:
        lib=Path(tmp)/(abi+'.so');lib.write_bytes(z.read('lib/'+abi+'/libashen.so'))
        header=run(readelf,'-h',lib);assert machine in header
        loads=[ln.split() for ln in run(readelf,'-lW',lib).splitlines() if ln.strip().startswith('LOAD')]
        assert loads and all(int(fields[-1],16)>=16384 for fields in loads)
        dyn=run(readelf,'--dyn-syms','--wide',lib)
        for m in methods:assert 'Java_com_ashenveil_game_MainActivity_'+m in dyn,(abi,m)
        print('PASS native machine, 16KB ELF LOAD alignment and all 18 JNI entry points:',abi)
print('APK bytes:',apk.stat().st_size)
print('APK SHA256:',hashlib.sha256(apk.read_bytes()).hexdigest())
print('NOTE: Artifact/native validation only; not a device install, gameplay or audio/FPS measurement.')
