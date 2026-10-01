#!/usr/bin/env bash
# Copyright (C) 2026 RafaelNGP
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Compila mbedTLS + libcurl estaticos para PS5 em deps/curl. O pico8 faz
# dlopen("libcurl.so"); o loader responde com estas funcoes. O TLS nativo
# do PS5 (sceHttp2/sceSsl) falhou com 0x8095f00c nos carts em HTTPS.
set -euo pipefail

: "${PS5_PAYLOAD_SDK:=/opt/ps5-payload-sdk}"
source "${PS5_PAYLOAD_SDK}/toolchain/prospero.sh"

here=$(cd "$(dirname "$0")" && pwd)
out=$here/curl
mbedtls_ver=3.6.4
curl_ver=8.18.0
# Padrao embutido na libcurl (layout antigo). Em runtime, o loader aponta
# o CURLOPT_CAINFO para o cacert.pem da pasta do app (pico8_loader/paths.c).
ca_bundle=/data/pico8/cacert.pem

cd "$here"
[ -f mbedtls-$mbedtls_ver.tar.bz2 ] || curl -fsSLO \
    https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-$mbedtls_ver/mbedtls-$mbedtls_ver.tar.bz2
[ -f curl-$curl_ver.tar.xz ] || curl -fsSLO https://curl.se/download/curl-$curl_ver.tar.xz
[ -f cacert.pem ] || curl -fsSLo cacert.pem https://curl.se/ca/cacert.pem

rm -rf mbedtls-$mbedtls_ver curl-$curl_ver
tar xf mbedtls-$mbedtls_ver.tar.bz2
tar xf curl-$curl_ver.tar.xz

${CMAKE} -S mbedtls-$mbedtls_ver -B mbedtls-$mbedtls_ver/build \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$out" \
    -DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF \
    -DUSE_SHARED_MBEDTLS_LIBRARY=OFF -DUSE_STATIC_MBEDTLS_LIBRARY=ON \
    -DCMAKE_CXX_COMPILER_WORKS=ON
${MAKE} -C mbedtls-$mbedtls_ver/build -j"$(nproc)"
DESTDIR= ${MAKE} -C mbedtls-$mbedtls_ver/build install

cd curl-$curl_ver
./configure --host=x86_64-pc-freebsd --prefix="$out" \
    --enable-static --disable-shared \
    --with-mbedtls="$out" --without-zlib --without-libpsl \
    --without-brotli --without-zstd --without-nghttp2 --without-libidn2 \
    --disable-ldap --disable-rtsp --disable-dict --disable-telnet \
    --disable-tftp --disable-pop3 --disable-imap --disable-smtp \
    --disable-gopher --disable-mqtt --disable-smb --disable-ftp \
    --disable-file --disable-docs --disable-manual --disable-unix-sockets \
    --with-ca-bundle=$ca_bundle --without-ca-path \
    CPPFLAGS="-I$out/include" LDFLAGS="-L$out/lib"
${MAKE} -j"$(nproc)" -C lib
DESTDIR= ${MAKE} -C lib install
DESTDIR= ${MAKE} -C include install
echo "libcurl instalada em $out"
