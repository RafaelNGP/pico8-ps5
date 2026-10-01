#!/usr/bin/env bash
# Empacota home_app/ como app nativo do PS5 (eboot.bin FSELF + sce_sys)
# usando o ps5-native-app-boilerplate como ferramenta de build. Saida em
# home_app/build/PPSA99808/, pronta para /data/homebrew/PICO8/.
set -euo pipefail

here=$(cd "$(dirname "$0")" && pwd)
app=$(cd "$here/../home_app" && pwd)
bp=$here/boilerplate
rev=dd44bbd   # ps5-native-app-boilerplate testado
title=PPSA99808

if [ ! -d "$bp" ]; then
    git clone -q https://github.com/blackbearreloaded/ps5-native-app-boilerplate.git "$bp"
fi
git -C "$bp" checkout -q "$rev"

# Os caminhos APP_* sao relativos a raiz do boilerplate.
ln -sfn "$app" "$bp/pico8"

stage=$app/build
rm -rf "$stage/sce_sys" "$stage/assets"
mkdir -p "$stage/sce_sys" "$stage/assets"
cp "$app/sce_sys/param.json" "$stage/sce_sys/"
python3 "$app/make_icon.py" "$stage/sce_sys/icon0.png" \
    "${PICO8_DIR:-$HOME/pico-8}/lexaloffle-pico8.png"

# clang-18 eh o padrao do boilerplate; o clang do sistema tambem serve.
make -C "$bp" USE_CCACHE=0 PS5_CLANG="${PS5_CLANG:-clang}" \
    APP_SOURCE_DIR=pico8/src \
    APP_SCE_SYS=pico8/build/sce_sys \
    APP_PARAM=pico8/build/sce_sys/param.json \
    APP_ASSETS=pico8/build/assets

rm -rf "$stage/$title"
cp -r "$bp/dist/$title" "$stage/$title"
echo "app em $stage/$title"
