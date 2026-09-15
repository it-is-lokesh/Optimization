#!/usr/bin/env bash
# Build one standalone C or C++ source file into build/<source-name>.
set -euo pipefail

cd "$(dirname "$0")"

# CC="${CC:-gcc}"
CC="${CC:-/opt/homebrew/opt/llvm/bin/clang}"
CXX="${CXX:-/opt/homebrew/opt/llvm/bin/clang++}"
BUILD_DIR="$(cd .. && pwd)/build"
mkdir -p "$BUILD_DIR"

# Change this line to select a different standalone program.
SOURCE="${SOURCE:-no_volatile.c}"
SOURCE_NAME="${SOURCE##*/}"
OUT="$BUILD_DIR/${SOURCE_NAME%.*}"

case "$SOURCE" in
    *.c)
        COMPILER="$CC"
        FLAGS="${CFLAGS:--O3 -march=native -Wall -Wextra -std=c11}"
        ;;
    *.cpp)
        COMPILER="$CXX"
        FLAGS="${CXXFLAGS:--O3 -march=native -Wall -Wextra -std=c++17}"
        ;;
    *)
        echo "Unsupported source type: $SOURCE (expected .c or .cpp)" >&2
        exit 1
        ;;
esac

if [[ ! -f "$SOURCE" ]]; then
    echo "Source file not found: $SOURCE" >&2
    exit 1
fi

echo "Compiling $SOURCE -> $OUT"
"$COMPILER" $FLAGS "$SOURCE" -o "$OUT"

echo "Build complete: $OUT"
