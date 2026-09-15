#!/usr/bin/env bash
# Build the standalone OpenBLAS benchmark. Auto-detects cblas.h / libopenblas
# from a system (apt) install or the active conda environment.
set -euo pipefail
cd "$(dirname "$0")"

CC="${CC:-gcc}"
BUILD_DIR="$(cd .. && pwd)/build"
mkdir -p "$BUILD_DIR"
CFLAGS="-O3 -march=native"
INC=()
LIB=()

# 1) conda env (if active)
if [[ -n "${CONDA_PREFIX:-}" ]]; then
    [[ -f "$CONDA_PREFIX/include/cblas.h" ]] && INC+=("-I$CONDA_PREFIX/include")
    [[ -d "$CONDA_PREFIX/lib" ]] && LIB+=("-L$CONDA_PREFIX/lib" "-Wl,-rpath,$CONDA_PREFIX/lib")
fi

# 2) apt install locations for cblas.h (Ubuntu puts it under openblas/ or the triplet dir)
for d in /usr/include /usr/include/x86_64-linux-gnu /usr/include/openblas; do
    [[ -f "$d/cblas.h" ]] && INC+=("-I$d")
done

OUT="$BUILD_DIR/blas_bench"
echo "Compiling blas_bench -> ${OUT}"
if ! "$CC" $CFLAGS "${INC[@]:-}" blas_bench.c -o "$OUT" "${LIB[@]:-}" -lopenblas -lm 2>/tmp/blas_err; then
    echo "Build failed. OpenBLAS likely not installed. Install one of:" >&2
    echo "  sudo apt install libopenblas-dev" >&2
    echo "  conda install -c conda-forge openblas" >&2
    echo "--- compiler error ---" >&2
    cat /tmp/blas_err >&2
    exit 1
fi
echo "Build complete: ${OUT}"
