#!/usr/bin/env bash
# Build script: compiles all C sources in this folder into a single binary.
set -euo pipefail

cd "$(dirname "$0")"

# CC="${CC:-gcc}"
CC="${CC:-/opt/homebrew/opt/llvm/bin/clang}"
BUILD_DIR="$(cd .. && pwd)/build"
mkdir -p "$BUILD_DIR"
# -O3 -march=native lets the compiler auto-vectorize (SSE/AVX) the clean
# ikj inner loop; combined with `restrict` in the kernels this is a big win.
# -fopenmp enables the multithreaded kernel.
CFLAGS="${CFLAGS:--O3 -march=native -Wall -Wextra -std=c11 -fopenmp}"
OUT="${OUT:-$BUILD_DIR/optimization}"

# Collect all C sources and headers.
# Exclude standalone benchmarks that have their own main()/external deps
# (e.g. blas_bench.c needs OpenBLAS and is built separately via build_blas.sh).
SOURCES=()
for f in *.c; do
    [[ "$f" == "blas_bench.c" ]] && continue
    SOURCES+=("$f")
done
HEADERS=(*.h)

echo "Sources: ${SOURCES[*]}"
echo "Headers: ${HEADERS[*]}"

echo "Compiling -> ${OUT}"
"$CC" $CFLAGS "${SOURCES[@]}" -o "$OUT"

echo "Build complete: ${OUT}"
