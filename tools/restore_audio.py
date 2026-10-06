#!/usr/bin/env python3
"""Restore missing runtime WAVs from the companion release APK, byte-checked.
Avoids persisting a redundant ~12 MiB copy beside unchanged previous releases.
Usage: python3 tools/restore_audio.py [path/to/Death-World-0.8.apk]
Present files are not overwritten, allowing deliberate developer audio edits.
"""
from pathlib import Path, PurePosixPath
import hashlib, json, sys, zipfile
ROOT=Path(__file__).resolve().parents[1]
def restore(apk=None):
    manifest=json.loads((ROOT/'tools/runtime-audio.json').read_text())
    missing=[]
    for key,spec in manifest.items():
        p=PurePosixPath(key)
        if p.is_absolute() or '..' in p.parts or len(p.parts)!=2 or p.parts[0] not in ('audio','music') or p.suffix!='.wav':
            raise ValueError('Unsafe audio manifest path')
        if not (ROOT/'android/assets'/key).is_file():missing.append((key,spec))
    if not missing:return
    candidates=[Path(apk)] if apk else [ROOT.parent/f'deliverables/death-world-0.{v}/Death-World-0.{v}.apk' for v in (8,7,6)]
    for candidate in candidates:
        if not candidate.is_file():continue
        with zipfile.ZipFile(candidate) as z:
            verified=[]
            for key,spec in missing:
                info=z.getinfo('assets/'+key)
                if info.file_size!=spec['bytes']:raise ValueError('Unexpected audio size: '+key)
                data=z.read(info)
                if hashlib.sha256(data).hexdigest()!=spec['sha256']:raise ValueError('Audio hash mismatch: '+key)
                verified.append((key,data))
            for key,data in verified:
                p=ROOT/'android/assets'/key;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
        print(f'Restored {len(missing)} verified WAV files from {candidate.name}',file=sys.stderr)
        return
    raise SystemExit('Runtime audio missing. Run: python3 tools/restore_audio.py /path/to/Death-World-0.8.apk')
if __name__=='__main__':restore(sys.argv[1] if len(sys.argv)>1 else None)
