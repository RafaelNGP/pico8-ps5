#!/usr/bin/env bash
# Copyright (C) 2026 RafaelNGP
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Empacota um app nativo do PS5 (eboot.bin FSELF + sce_sys) a partir de uma
# pasta nossa, usando o ps5-native-app-boilerplate como ferramenta de build.
#
# Uso: build_app.sh <pasta-do-app>   (ex.: pico8_app, app_probe)
#
# A pasta precisa de src/ (C/C++) e sce_sys/param.json. Opcionais:
#   TEXT_BASE  vaddr da imagem (ex.: 0x1000000), para liberar 0x400000;
#   build.env  APP_DEFS, APP_INCS, APP_LIBS (caminhos relativos a pasta)
#              e SDK_LIBC_OBJS (objetos avulsos da libc.a do SDK);
#   ELEVATE    inclui o helper de elevacao do boilerplate (via elfldr
#              local): elevation.cpp entra no build e o
#              sandbox-elevator.elf, travado no titleId do app, vai
#              para /app0.
# Saida em <pasta>/build/<TITLE_ID>/.
set -euo pipefail

here=$(cd "$(dirname "$0")" && pwd)
app=$(cd "$here/../${1:?pasta do app}" && pwd)
bp=$here/boilerplate
rev=dd44bbd   # ps5-native-app-boilerplate testado
title=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["titleId"])' \
        "$app/sce_sys/param.json")

if [ ! -d "$bp" ]; then
    git clone -q https://github.com/blackbearreloaded/ps5-native-app-boilerplate.git "$bp"
fi
git -C "$bp" checkout -q -f "$rev"
# TEXT com vaddr != 0 e nenhum simbolo exportado (o conversor nao
# publica exports; sem isso, dlopen & cia. definidos no app vazam).
git -C "$bp" apply "$here/patches/boilerplate-native.patch"

# Apps carregam em 0x400000 + vaddr, bem onde o pico8_dyn (nao-PIE)
# precisa ficar. Um arquivo TEXT_BASE na pasta do app desloca a imagem.
if [ -f "$app/TEXT_BASE" ]; then
    base=$(tr -d '[:space:]' < "$app/TEXT_BASE")
    sed -i "s/^    \.text 0 :/    .text $base :/" "$bp/tooling/native/ps5-pie.ld"
    grep -q "\.text $base :" "$bp/tooling/native/ps5-pie.ld"
fi

# Os caminhos APP_* sao relativos a raiz do boilerplate.
link=apps/$(basename "$app")
mkdir -p "$bp/apps"
ln -sfn "$app" "$bp/$link"

stage=$app/build
rm -rf "$stage/src" "$stage/sce_sys" "$stage/assets"
mkdir -p "$stage/src" "$stage/sce_sys" "$stage/assets"
cp -rL "$app/src/." "$stage/src/"
cp "$app/sce_sys/param.json" "$stage/sce_sys/"
# APP_ICON_LOGO vazio (make release) gera o icone generico: o logo da
# Lexaloffle so entra no build local, nunca num pacote distribuido.
python3 "$here/../pico8_app/make_icon.py" "$stage/sce_sys/icon0.png" \
    "${APP_ICON_LOGO-${PICO8_DIR:-$HOME/pico-8}/lexaloffle-pico8.png}"

APP_DEFS= APP_INCS= APP_LIBS= SDK_LIBC_OBJS=
[ ! -f "$app/build.env" ] || . "$app/build.env"
# O Title ID do param.json chega ao codigo como APP_TITLE_ID.
APP_DEFS+=" APP_TITLE_ID=$title"
incs="$link/build/src/elevation"
for i in $APP_INCS; do incs+=" $link/$i"; done
libs=
for l in $APP_LIBS; do libs+=" $link/$l"; done
if [ -n "$SDK_LIBC_OBJS" ]; then
    tmp=$(mktemp -d)
    (cd "$tmp" && ar x "$bp/.deps/native/ps5-payload-sdk/target/lib/libc.a" $SDK_LIBC_OBJS)
    rm -f "$stage/sdk_libc.a"
    ar rcs "$stage/sdk_libc.a" "$tmp"/*.o
    rm -rf "$tmp"
    libs+=" $link/build/sdk_libc.a"
fi

root_files=
if [ -f "$app/ELEVATE" ]; then
    ex=$bp/examples/sandbox-elevation
    mkdir -p "$stage/src/elevation"
    cp "$ex/elevation.hpp" "$ex/protocol.hpp" "$stage/src/elevation/"
    sed 's|"\.\./elevation.hpp"|"elevation.hpp"|; s|"\.\./protocol.hpp"|"protocol.hpp"|' \
        "$ex/src/elevation.cpp" > "$stage/src/elevation/elevation.cpp"
    # O helper so aceita o titleId para o qual foi compilado.
    sed -i "s/PPSA99790/$title/" "$ex/payload/main.cpp"
    make -C "$bp" sandbox-elevation-helper
    root_files=build/sandbox-elevation/sandbox-elevator.elf
fi

# clang-18 eh o padrao do boilerplate; o clang do sistema tambem serve.
log=$stage/build.log
make -C "$bp" app USE_CCACHE=0 PS5_CLANG="${PS5_CLANG:-clang}" \
    APP_SOURCE_DIR="$link/build/src" \
    APP_DEFINITIONS="$APP_DEFS" \
    APP_INCLUDE_PATHS="$incs" \
    APP_STATIC_ARCHIVES="${libs# }" \
    APP_SCE_SYS="$link/build/sce_sys" \
    APP_PARAM="$link/build/sce_sys/param.json" \
    APP_ASSETS="$link/build/assets" \
    APP_ROOT_FILES="$root_files" 2>&1 | tee "$log"
[ "${PIPESTATUS[0]}" -eq 0 ] || exit 1

# O boilerplate compila com -Wall -Wextra, mas sem -Werror. Warning no
# codigo do app (nao no dos deps) quebra o build, como -Werror faria.
if grep -E "apps/$(basename "$app")/build/src/.*warning:" "$log"; then
    echo "warnings no codigo do app (acima)" >&2
    exit 1
fi

rm -rf "$stage/$title"
cp -r "$bp/dist/$title" "$stage/$title"
echo "app em $stage/$title"
