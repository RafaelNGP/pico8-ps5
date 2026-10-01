#!/usr/bin/env bash
# Compila o port PS5 do SDL2 (ps5-payload-dev/SDL) como lib estatica em
# deps/sdl2, sem precisar de root nem instalar no /opt.
set -euo pipefail

: "${PS5_PAYLOAD_SDK:=/opt/ps5-payload-sdk}"
source "${PS5_PAYLOAD_SDK}/toolchain/prospero.sh"

here=$(cd "$(dirname "$0")" && pwd)
src=$here/SDL
out=$here/sdl2
rev=ee4c47d   # release-2.30.x-ps5 testado

if [ ! -d "$src" ]; then
    git clone -q -b release-2.30.x-ps5 https://github.com/ps5-payload-dev/SDL.git "$src"
fi
git -C "$src" checkout -q -f "$rev"
for p in "$here"/patches/*.patch; do
    git -C "$src" apply "$p"
done

${CMAKE} -S "$src" -B "$src/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$out" \
    -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST=OFF \
    -DSDL_OPENGL=OFF -DSDL_LIBSAMPLERATE=OFF \
    -DCMAKE_CXX_COMPILER_WORKS=ON   # SDL2 eh C; libc++ do SDK nao precisa estar instalada
${MAKE} -C "$src/build" -j"$(nproc)"
DESTDIR= ${MAKE} -C "$src/build" install
echo "SDL2 instalado em $out"
