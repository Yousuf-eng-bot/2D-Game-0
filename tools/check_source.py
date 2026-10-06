#!/usr/bin/env python3
"""Verify a freshly extracted source snapshot. Deliberate edits change hashes.
This verifies file integrity, not Android runtime performance or signing access.
"""
from pathlib import Path, PurePosixPath
import hashlib
import json
import sys

ROOT = Path(__file__).resolve().parents[1]

def main():
    path = ROOT / 'SOURCE-MANIFEST.json'
    if not path.is_file():
        raise SystemExit('No SOURCE-MANIFEST.json. Run this in the extracted source ZIP/repository snapshot.')
    manifest = json.loads(path.read_text())
    if manifest.get('format') != 1 or manifest['file_count'] != len(manifest['files']):
        raise SystemExit('Invalid source manifest')
    seen = set()
    errors = []
    for spec in manifest['files']:
        rel = PurePosixPath(spec['path'])
        if rel.is_absolute() or '..' in rel.parts or str(rel) in seen:
            raise SystemExit('Unsafe or duplicate manifest path')
        seen.add(str(rel))
        p = ROOT.joinpath(*rel.parts)
        if not p.is_file() or p.is_symlink():
            errors.append(str(rel) + ': missing/nonregular file')
            continue
        data = p.read_bytes()
        if len(data) != spec['bytes'] or hashlib.sha256(data).hexdigest() != spec['sha256']:
            errors.append(str(rel) + ': changed hash/size')
    wavs = [name for name in seen if name.startswith('android/assets/') and name.endswith('.wav')]
    if len(wavs) != manifest['runtime_wav_count']:
        errors.append('WAV count mismatch')
    if errors:
        print('\n'.join(errors), file=sys.stderr)
        raise SystemExit('Snapshot differs. If edits were intentional, export a fresh manifest; otherwise restore the missing/changed files.')
    print(f'PASS {len(seen)} exported file hashes; {len(wavs)} runtime WAVs present; version {manifest["version"]}.')

if __name__ == '__main__':
    main()
