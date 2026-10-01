# pico8_loader

Roda o **PICO-8 oficial para Linux** (`pico8_dyn`, x86-64) num PS5
desbloqueado, no modelo "traga sua própria licença": você fornece o seu
`pico8_dyn` e o `pico8.dat`, e nada da Lexaloffle é redistribuído.

O loader segue a ideia dos ports Android→PS Vita (`so_loader`): carrega o
ELF nos endereços originais, resolve os imports com funções nativas e não
altera o binário.

## Como funciona

| Peça | O que faz |
|---|---|
| `main.c` | Mapeia o `pico8_dyn` (não-PIE) em `0x400000`/`0x794000`, aplica as relocações, chama `init` e `main` (com `-splore`) numa thread com pilha de 32 MiB. Tem um handler de crash que grava no log. |
| `shims_libc.c` | Os 125 imports de glibc/libm/libdl sobre a libc do PS5. Trata o que difere: `*_chk`, `__xstat`, `__ctype_*_loc`, `struct dirent`, flags do `open`, `clock`/`clock_gettime`, `dlerror`. O stdout/stderr do PICO-8 vai para o log. |
| `shims_sdl.c` | Os 91 imports `SDL_*` vão direto para o port PS5 do SDL2 (a ABI do SDL2 é estável). Alguns passam por wrappers que registram no log. |
| `net_curl.c` | Responde ao `dlopen("libcurl.so")` do Splore com uma libcurl real (8.18 + mbedTLS). |

Particularidades do PS5 descobertas no caminho:

- Dar `PROT_EXEC` a parte de um `mmap` anônimo tira a escrita do
  mapeamento inteiro. Por isso TEXT, thunks e DATA são três `mmap`s.
- Só o app em primeiro plano aparece na TV. Um payload comum roda em
  segundo plano, então o loader é lançado pelo hbldr do
  [websrv](https://github.com/ps5-payload-dev/websrv), que abre um app
  fake e roda o ELF dentro dele.
- Um payload em segundo plano só consegue 32 MiB de memória de vídeo; o
  SDL pede 64 MiB (`deps/patches/sdl2-dmem-fallback.patch`).
- O TLS nativo (`sceHttp2`) falhou com `0x8095f00c` nos downloads HTTPS;
  por isso a libcurl com mbedTLS.

## Pré-requisitos

- PS5 desbloqueado com Payload Manager (porta 8084) e servidor FTP (porta 2121).
- `ps5-payload-sdk` instalado em `/opt/ps5-payload-sdk` (ver `../mmap_probe/README.md`).
- Seu `pico8_dyn` e `pico8.dat` em `~/pico-8/` (versão Linux, 0.2.7).

## Uso

```bash
export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
cd pico8_loader
make              # compila SDL2, libcurl e o loader (a 1ª vez demora)
make upload-data  # envia pico8_dyn, pico8.dat e cacert.pem para /data/pico8
make websrv       # inicia o websrv (repetir após reiniciar o PS5)
make run          # abre o PICO-8 na TV, direto no Splore
make log          # mostra /data/pico8/loader.log
```

O IP do PS5 é `<PS5_IP>` por padrão; para outro: `make run PS5_HOST=<ip>`.

## Onde ficam os dados no PS5

- `/data/pico8/` — `pico8_dyn`, `pico8.dat`, `cacert.pem`, `loader.log`
- `/data/pico8/.lexaloffle/pico-8/` — config, favoritos, carts baixados e saves (`cdata/`)
- `/data/homebrew/PICO8/eboot.elf` — cópia do loader usada pelo websrv
