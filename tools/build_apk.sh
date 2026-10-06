#!/usr/bin/env bash
# Linux x86_64 reference build, tested with JDK 11, NDK r27c, Build Tools 34.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# A fresh clone must NOT silently generate a different signing identity.
KEY="${SIGNING_KEY:-$HOME/.signing/ashen-prototype.jks}"
if [[ ! -f "$KEY" && "${ALLOW_NEW_SIGNING_KEY:-0}" != "1" ]]; then
 echo "Missing original signing key. Restore it privately and set SIGNING_KEY." >&2
 echo "For a NEW disposable test identity only, explicitly set ALLOW_NEW_SIGNING_KEY=1." >&2
 echo "A differently signed APK cannot update the existing installation. Do not uninstall to work around this." >&2
 exit 2
fi
if [[ ! -f "$KEY" ]]; then
 echo "WARNING: explicit NEW test signing identity; not compatible with the existing installed APK." >&2
fi
python3 "$ROOT/tools/restore_audio.py"
export JAVA_HOME="${JAVA_HOME:-$(dirname "$(dirname "$(readlink -f "$(command -v javac)")")")}"
export PATH="$JAVA_HOME/bin:$PATH"
SDK_ROOT="${SDK_ROOT:-$HOME/.cache/android}"
NDK="${ANDROID_NDK_HOME:-$SDK_ROOT/ndk/android-ndk-r27c}"
BT="${ANDROID_BUILD_TOOLS:-$SDK_ROOT/tools/android-14}"
JAR="${ANDROID_JAR:-$SDK_ROOT/platform/android-35/android.jar}"
CXX="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin"
BUILD="$ROOT/build/apk"
DEST="${APK_OUTPUT:-$ROOT/Death-World-0.9.2-Thermal-Fix.apk}"
rm -rf "$BUILD/classes" "$BUILD/dex" "$BUILD/gen" "$BUILD/lib"
mkdir -p "$BUILD/classes" "$BUILD/dex" "$BUILD/gen" "$(dirname "$KEY")"
for ABI in arm64-v8a armeabi-v7a x86_64; do
 case "$ABI" in
  arm64-v8a) COMPILER=aarch64-linux-android23-clang++;;
  armeabi-v7a) COMPILER=armv7a-linux-androideabi23-clang++;;
  x86_64) COMPILER=x86_64-linux-android23-clang++;;
 esac
 echo "Compiling C++20 for $ABI..."
 mkdir -p "$BUILD/lib/$ABI"
 "$CXX/$COMPILER" -std=c++20 -O2 -fPIC -shared -static-libstdc++ -Wl,-z,max-page-size=16384 -Wl,-soname,libashen.so "$ROOT/native/engine.cpp" -lz -lEGL -lGLESv3 -landroid -o "$BUILD/lib/$ABI/libashen.so"
 "$CXX/llvm-strip" "$BUILD/lib/$ABI/libashen.so"
done
"$BT/aapt" package -f -M "$ROOT/android/AndroidManifest.xml" -S "$ROOT/android/res" -A "$ROOT/android/assets" -I "$JAR" -J "$BUILD/gen" -F "$BUILD/base.apk"
javac --release 8 -encoding UTF-8 -classpath "$JAR" -d "$BUILD/classes" "$ROOT/android/java/com/ashenveil/game/MainActivity.java"
mapfile -t CLASSES < <(find "$BUILD/classes" -name '*.class')
"$BT/d8" --min-api 23 --lib "$JAR" --output "$BUILD/dex" "${CLASSES[@]}"
cp "$BUILD/base.apk" "$BUILD/unsigned.apk"
(cd "$BUILD" && zip -q -r unsigned.apk lib)
(cd "$BUILD/dex" && zip -q "$BUILD/unsigned.apk" classes.dex)
"$BT/zipalign" -f -p 4 "$BUILD/unsigned.apk" "$BUILD/aligned.apk"
if [[ ! -f "$KEY" ]]; then
 keytool -genkeypair -keystore "$KEY" -storepass android -keypass android -alias ashen-prototype -keyalg RSA -keysize 2048 -validity 10000 -dname 'CN=Ashen Veil Prototype, O=Independent Prototype, C=BD'
fi
# TESTING KEY ONLY: do not reuse this development signing configuration for production.
"$BT/apksigner" sign --ks "$KEY" --ks-pass pass:android --key-pass pass:android --ks-key-alias ashen-prototype --out "$DEST" "$BUILD/aligned.apk"
"$BT/apksigner" verify --verbose --print-certs "$DEST"
"$BT/aapt" dump badging "$DEST"
echo "APK created: $DEST"
