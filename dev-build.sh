#!/usr/bin/env bash
# Local host build helper for sandboxes without root. Uses a privately built
# zlib when the system package is unavailable. CI uses plain CMake instead.
set -euo pipefail
Z="${ZLIB_PREFIX:-$HOME/toolchain/zlib}"
EXTRA=()
if [ -f "$Z/include/zlib.h" ]; then
  EXTRA=(-DCMAKE_CXX_FLAGS="-I$Z/include"
         -DCMAKE_EXE_LINKER_FLAGS="-L$Z/lib"
         -DCMAKE_SHARED_LINKER_FLAGS="-L$Z/lib")
  export LD_LIBRARY_PATH="$Z/lib:${LD_LIBRARY_PATH:-}"
fi
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release "${EXTRA[@]}" >/dev/null
cmake --build build/native -j"${JOBS:-2}" "$@"
