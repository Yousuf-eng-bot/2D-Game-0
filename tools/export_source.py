#!/usr/bin/env python3
"""Export a portable current-source ZIP. No keys, user uploads or build tools.

Existing runtime WAVs are preferred; the original space-constrained workspace
can supply missing originals from its retained release APK. Audio is streamed
straight into the source ZIP, without persisting redundant WAV copies.
"""
from pathlib import Path, PurePosixPath
import argparse
import hashlib
import json
import stat
import xml.etree.ElementTree as ET
import zipfile

ROOT = Path(__file__).resolve().parents[1]
TOP = 'death-world'
SKIP_DIRS = {'.git', '.signing', '.android', '.cache', '.venv', '__pycache__',
             'build', 'dist', 'out', 'node_modules', 'uploads', 'deliverables'}
SKIP_SUFFIXES = {'.apk', '.aab', '.idsig', '.jks', '.keystore', '.p12', '.pfx',
                 '.pem', '.key', '.zip', '.mp4', '.pyc', '.o', '.so', '.class', '.dex'}
ROOT_FILES = ['README.md', 'AGENTS.md', 'HANDOFF_BN.md', 'SIGNING.md',
              'CMakeLists.txt', 'LICENSE', 'THIRD-PARTY-NOTICES.md',
              '.gitignore', '.gitattributes', '.clang-format']

def allowed(path):
    rel = path.relative_to(ROOT)
    if path.is_symlink() or not path.is_file():
        return False
    if any(part in SKIP_DIRS for part in rel.parts):
        return False
    if path.suffix.lower() in SKIP_SUFFIXES:
        return False
    if path.name.startswith('.env') or path.name in {'.netrc', '.git-credentials', 'credentials.json', 'local.properties'}:
        return False
    # Old generated screenshot galleries are not runtime source assets.
    if rel.parts[:2] == ('docs', 'images'):
        return False
    # Backup archives are intentionally separate from the active source tree.
    if rel.parts[:2] == ('docs', 'history'):
        return False
    return True

def main():
    android = ET.parse(ROOT / 'android/AndroidManifest.xml').getroot()
    version = android.attrib['{http://schemas.android.com/apk/res/android}versionName']
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT.parent / f'Death-World-{version}-Source.zip')
    parser.add_argument('--audio-apk', type=Path)
    args = parser.parse_args()
    audio = json.loads((ROOT / 'tools/runtime-audio.json').read_text())
    audio_paths = set()
    for key in audio:
        p = PurePosixPath(key)
        if p.is_absolute() or '..' in p.parts or len(p.parts) != 2 or p.parts[0] not in ('audio', 'music') or p.suffix != '.wav':
            raise ValueError('Unsafe runtime audio manifest path')
        audio_paths.add('android/assets/' + key)
    files = {}
    for name in ROOT_FILES:
        p = ROOT / name
        if not p.is_file():
            raise FileNotFoundError(p)
        files[name] = p.read_bytes()
    for folder in ['native', 'android', 'assets', 'tools', 'tests', 'docs']:
        for p in sorted((ROOT / folder).rglob('*')):
            if allowed(p):
                files[p.relative_to(ROOT).as_posix()] = p.read_bytes()
    missing = sorted(audio_paths - files.keys())
    if missing:
        candidates = [args.audio_apk] if args.audio_apk else [
            ROOT.parent / 'deliverables/death-world-0.8/Death-World-0.8.apk']
        source = next((p for p in candidates if p and p.is_file()), None)
        if source is None:
            raise SystemExit('Missing runtime WAVs: restore them or pass --audio-apk ORIGINAL_RELEASE.apk')
        with zipfile.ZipFile(source) as apk:
            for rel in missing:
                key = rel.removeprefix('android/assets/')
                data = apk.read('assets/' + key)
                spec = audio[key]
                if len(data) != spec['bytes'] or hashlib.sha256(data).hexdigest() != spec['sha256']:
                    raise ValueError('Original audio hash/size mismatch: ' + key)
                files[rel] = data
    records = [{'path': rel, 'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
               for rel, data in sorted(files.items())]
    manifest = {
        'format': 1, 'project': 'Death World', 'version': version,
        'file_count': len(records), 'runtime_wav_count': len(audio_paths),
        'scope': 'Active source and required runtime assets; no private keys, user uploads, APKs, SDK/build caches, historical source ZIPs or old generated screenshot galleries.',
        'files': records,
    }
    files['SOURCE-MANIFEST.json'] = (json.dumps(manifest, ensure_ascii=False, indent=2) + '\n').encode()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(args.output, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for rel, data in sorted(files.items()):
            info = zipfile.ZipInfo(TOP + '/' + rel)
            info.create_system = 3
            info.external_attr = (stat.S_IFREG | (0o755 if rel.endswith(('.sh', '.py')) else 0o644)) << 16
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, data, compress_type=zipfile.ZIP_DEFLATED, compresslevel=9)
    with zipfile.ZipFile(args.output) as archive:
        if archive.testzip() is not None:
            raise ValueError('ZIP CRC check failed')
    print(f'Created {args.output}: {len(records)} source/asset files + manifest; {len(audio_paths)} WAVs; {args.output.stat().st_size} bytes')
    print('SHA256:', hashlib.sha256(args.output.read_bytes()).hexdigest())

if __name__ == '__main__':
    main()
