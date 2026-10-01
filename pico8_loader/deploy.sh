#!/usr/bin/env bash
# Envia um payload ao Payload Manager (PLK) e executa.
# Uso: deploy.sh <host> <porta> <arquivo.elf>
set -euo pipefail

host=$1 port=$2 elf=$3
name=$(basename "$elf")
base=http://$host:$port

curl -fsS -X POST -H 'Content-Type: application/octet-stream' \
     --data-binary "@$elf" "$base/manage:upload?filename=$name" >/dev/null

# O Payload Manager guarda em /data/pldmgr/payloads/<nome>/<arquivo>;
# pega o caminho da listagem em vez de adivinhar.
path=$(curl -fsS "$base/list_payloads" |
       grep -oE "\"[^\"]*/$name\"" | head -1 | tr -d '"')
if [ -z "$path" ]; then
    echo "upload ok, mas $name nao apareceu em /list_payloads" >&2
    exit 1
fi

curl -fsS "$base/loadpayload:$path" >/dev/null
echo "executando $path"
