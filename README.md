# PICO-8 no PS5

Roda o **PICO-8 oficial para Linux** num PS5 desbloqueado, como um app
nativo com ícone próprio na Home. Abre direto no Splore e funciona com o
DualSense, com áudio e com downloads de carts pela internet.

O modelo é "traga sua própria licença": o projeto não contém nem
redistribui nada da Lexaloffle. Você precisa da sua cópia do PICO-8
(versão Linux 64-bit, comprada em <https://www.lexaloffle.com/pico-8.php>;
testado com a 0.2.7).

## Instalação

Não precisa compilar nem rodar comandos. Basta copiar uma pasta, como no
ProsperoEden.

**Requisitos no PS5** (testado num único console):

- kstuff e ShadowMountPlus rodando;
- o elfldr ouvindo em `127.0.0.1:9021`, que costuma vir junto do
  kstuff/Payload Manager. O app usa o elfldr para liberar o acesso a
  `/data` (elevação de sandbox, o mesmo mecanismo do ProsperoEden);
- um jeito de copiar arquivos para o PS5, como um servidor FTP.

**Passos**, com o zip `pico8-ps5-<versão>.zip`:

1. copie a pasta `PPSA99808` do zip, inteira, para `/data/homebrew/`;
2. do zip Linux do seu PICO-8, copie o `pico8_dyn` e o `pico8.dat` para
   dentro de `PPSA99808/pico8/`;
3. em até ~15 s, o ShadowMountPlus registra o ícone **PICO-8** na Home.

```
/data/homebrew/PPSA99808/        a pasta do app (o nome é o Title ID)
    eboot.bin                    o loader + SDL2 + libcurl/mbedTLS
    sandbox-elevator.elf         helper da elevação (só aceita o PPSA99808)
    cacert.pem                   certificados para o HTTPS do Splore
    sce_module/libc.prx          runtime do boilerplate
    sce_sys/param.json, icon0.png
    pico8/pico8_dyn, pico8.dat   seus arquivos do PICO-8
/data/pico8/                     criada pelo app
    .lexaloffle/pico-8/          config, favoritos, carts baixados e saves
    loader.log                   log da última execução
```

Se faltar algum arquivo, ou se o `pico8_dyn` for de outra versão, o app
avisa com uma notificação. Para atualizar, feche o app e copie a pasta
`PPSA99808` nova por cima da antiga. Os saves ficam em `/data/pico8`, fora da
pasta do app, e não se perdem.

## Limitações conhecidas

- Teclado USB e teclado na tela não funcionam: a `libSceKeyboard` e a
  `libSceImeDialog` não carregam num app. O Splore e os jogos funcionam
  com o controle.
- Testado num único PS5. Outro firmware pode precisar de ajustes.

Se o app fechar sozinho ou travar, o `/data/pico8/loader.log` mostra
quais arquivos foram usados e, num crash, os registradores, a pilha e as
últimas 256 chamadas de libc/SDL.

## Compilando

Só para desenvolver. Quem só quer jogar usa o zip.

**No PC** (Linux):

- [ps5-payload-sdk](https://github.com/ps5-payload-dev/sdk) em `/opt/ps5-payload-sdk`;
- `make`, `ninja`, `cmake`, `clang`/`lld`/`llvm-ar`, `git`, `curl`, `wget`,
  `unzip`, `zip`, `python3` com Pillow. No Fedora, o `llvm-config` vem do
  pacote `llvm-devel`;
- seu PICO-8 Linux em `~/pico-8/` (ou em `PICO8_DIR`). O
  `lexaloffle-pico8.png` dele vira o ícone do build local.

```bash
cd pico8_loader
export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
export PS5_HOST=192.168.0.50   # IP do seu PS5 (FTP na porta 2121)

make install-app   # compila e envia o app para /data/homebrew/PPSA99808
make upload-data   # 1x: envia pico8_dyn e pico8.dat para PPSA99808/pico8/
make log           # mostra o /data/pico8/loader.log
make release       # gera release/pico8-ps5-<versão>.zip
```

Na primeira vez, o build baixa e compila o SDL2 do PS5, a libcurl, o
mbedTLS e o boilerplate de apps nativos, e leva alguns minutos. O
`install-app` precisa do app fechado no PS5, porque o FTP recusa
sobrescrever o `eboot.bin` em uso.

O zip do `make release` leva a pasta `PPSA99808` com um ícone genérico (sem
nada da Lexaloffle), um `LEIA-ME.txt` e os avisos de licença.

## Estrutura

| Pasta | O que é |
|---|---|
| `pico8_loader/` | o loader: carrega o `pico8_dyn` e liga os imports dele à libc e ao SDL2 do PS5. Ver o [README](pico8_loader/README.md). |
| `pico8_app/` | empacota o loader como app nativo (`eboot.bin`), mais os textos do pacote em `release/`. |
| `deps/` | scripts de build do SDL2, da libcurl e do app, além de patches e do dlmalloc. |
| `mmap_probe/`, `app_probe/` | testes de viabilidade feitos no console (endereços fixos e execução). |
