# PICO-8 no PS5

Roda o **PICO-8 oficial para Linux** num PS5 desbloqueado, como um app
nativo com ícone próprio na Home. Abre direto no Splore e funciona com o
DualSense, com áudio e com downloads de carts pela internet.

O modelo é "traga sua própria licença": o projeto não contém nem
redistribui nada da Lexaloffle. Você precisa da sua cópia do PICO-8
(versão Linux 64-bit, comprada em <https://www.lexaloffle.com/pico-8.php>).

## O que você precisa

**No PS5** (testado num único console):

- kstuff e ShadowMountPlus rodando;
- o elfldr ouvindo em `127.0.0.1:9021`, que costuma vir junto do
  kstuff/Payload Manager. O app usa o elfldr para liberar o acesso a
  `/data` (elevação de sandbox, o mesmo mecanismo do ProsperoEden);
- um servidor FTP, só para a instalação (o padrão aqui é a porta 2121).

**Arquivos do seu PICO-8** (do zip Linux, em `~/pico-8/` ou em `PICO8_DIR`):

| Arquivo | Para quê |
|---|---|
| `pico8_dyn` | o executável que o loader carrega |
| `pico8.dat` | os dados do PICO-8 |
| `lexaloffle-pico8.png` | vira o ícone da Home (sem ele, sai um ícone genérico) |

**No PC** (Linux, só para compilar e instalar):

- [ps5-payload-sdk](https://github.com/ps5-payload-dev/sdk) em `/opt/ps5-payload-sdk`;
- `make`, `ninja`, `cmake`, `clang`/`lld`/`llvm-ar`, `git`, `curl`, `wget`,
  `unzip`, `python3` com Pillow. No Fedora, o `llvm-config` vem do pacote
  `llvm-devel`.

## Instalação

```bash
git clone <este repositório> pico8-ps5 && cd pico8-ps5/pico8_loader
export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
export PS5_HOST=192.168.0.50                      # IP do seu PS5

make upload-data   # 1x: pico8_dyn, pico8.dat, cacert.pem -> /data/pico8
make install-app   # compila e envia o app -> /data/homebrew/PICO8
```

Na primeira vez, o build baixa e compila o SDL2 do PS5, a libcurl, o
mbedTLS e o boilerplate de apps nativos, e leva alguns minutos. Depois do
`install-app`, o ShadowMountPlus registra o ícone **PICO-8** na Home em até
~15 s. Para atualizar o app, basta rodar `make install-app` de novo, com o
app fechado no PS5 (o FTP recusa sobrescrever o `eboot.bin` em uso).

## O que vai para o PS5

```
/data/homebrew/PICO8/            app nativo (PPSA99808), gerado pelo build
    eboot.bin                    o loader + SDL2 + libcurl/mbedTLS
    sandbox-elevator.elf         helper da elevação (só aceita o PPSA99808)
    sce_module/libc.prx          runtime do boilerplate
    sce_sys/param.json, icon0.png
/data/pico8/                     seus arquivos
    pico8_dyn, pico8.dat, cacert.pem
    loader.log                   log da última execução (make log)
    .lexaloffle/pico-8/          config, favoritos, carts baixados e saves
```

## Limitações conhecidas

- Teclado USB e teclado na tela não funcionam: a `libSceKeyboard` e a
  `libSceImeDialog` não carregam num app. O Splore e os jogos funcionam
  com o controle.
- Testado num único PS5. Outro firmware pode precisar de ajustes.

## Estrutura

| Pasta | O que é |
|---|---|
| `pico8_loader/` | o loader: carrega o `pico8_dyn` e liga os imports dele à libc e ao SDL2 do PS5. Ver o [README](pico8_loader/README.md). |
| `pico8_app/` | empacota o loader como app nativo (`eboot.bin`). |
| `deps/` | scripts de build do SDL2, da libcurl e do app, além de patches e do dlmalloc. |
| `mmap_probe/`, `app_probe/` | testes de viabilidade feitos no console (endereços fixos e execução). |

Se o app fechar sozinho ou travar, `make log` mostra o
`/data/pico8/loader.log`: o crash vem com registradores, pilha e as
últimas 256 chamadas de libc/SDL.
