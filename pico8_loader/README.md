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
| `paths.c` | Acha o `pico8_dyn`, o `pico8.dat` e o `cacert.pem` dentro da pasta do app, confere a versão e avisa em português se faltar algo. |
| `p8_alloc.c` | Heap próprio (dlmalloc em mspace, crescendo por `mmap`) para o PICO-8, o SDL e o curl. |

O mesmo código roda de dois jeitos: como **app nativo**
(`../pico8_app`, `eboot.bin` com ícone na Home, o modo normal) ou como
**payload** do ps5-payload-sdk lançado pelo websrv (desenvolvimento).

Particularidades do PS5 descobertas no caminho:

- Dar `PROT_EXEC` a parte de um `mmap` anônimo tira a escrita do
  mapeamento inteiro. Por isso TEXT, thunks e DATA são três `mmap`s.
- Só o app em primeiro plano aparece na TV. Por isso o loader roda
  dentro do próprio `eboot.bin` do app. No modo payload, quem faz esse
  papel é o hbldr do [websrv](https://github.com/ps5-payload-dev/websrv),
  que abre um app fake e roda o ELF dentro dele.
- Um payload em segundo plano só consegue 32 MiB de memória de vídeo; o
  SDL pede 64 MiB (`deps/patches/sdl2-dmem-fallback.patch`).
- O TLS nativo (`sceHttp2`) falhou com `0x8095f00c` nos downloads HTTPS;
  por isso a libcurl com mbedTLS.

Particularidades de rodar como app nativo, em vez de payload:

- O kernel carrega o eboot em `0x400000 + vaddr`, bem onde o `pico8_dyn`
  precisa ficar. O `pico8_app/TEXT_BASE` (`0x1000000`) e o
  `deps/patches/boilerplate-native.patch` movem o eboot para `0x1400000`.
- O app nasce no sandbox, sem acesso a `/data`. A elevação do boilerplate
  (via elfldr local) libera o filesystem.
- O rtld deixa em NULL, ou num placeholder sem nada mapeado
  (`0x840000000`), os imports de módulos que ele não carrega:
  `libSceKeyboard`, `libSceImeDialog`, `libScePosixForWebKit` e
  `libkernel_sys`. Corrigir o GOT em runtime não segura, porque o rtld
  regrava o slot. A correção é no link: definir a função no app
  (`pico8_app/src/app_glue.cpp`) ou puxar o objeto da `libc.a` do SDK
  (`pico8_app/build.env`). O `app_check.c` lista no log o que ficou sem
  resolver.
- Funções que resolvem podem saltar para um placeholder por dentro: o
  `getcwd` da `libSceLibcInternal` faz isso, e por isso o app tem o seu
  próprio.
- O heap da libc do sistema tem capacidade fixa pequena: um `malloc` de
  8 MB falha e, a partir daí, até `malloc(12)` falha, mesmo com centenas
  de MB livres. Por isso existe o `p8_alloc.c`.
- No handler de sinal, o `mcontext` fica em `+64` no ucontext, e não em
  `+16` como dizem os headers do SDK.

## Uso

A instalação normal está no [README do projeto](../README.md):
`make upload-data` e depois `make install-app`.

Para desenvolver o loader como payload, sem reempacotar o app:

```bash
export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
make              # compila SDL2, libcurl e o loader (a 1ª vez demora)
make websrv       # inicia o websrv (repetir após reiniciar o PS5)
make run          # envia para /data/pico8/pico8_loader.elf e abre via hbldr
make log          # mostra /data/pico8/loader.log
```

O IP do PS5 é `<PS5_IP>` por padrão; para outro, use `PS5_HOST=<ip>`.

## Onde ficam os arquivos no PS5

- `/data/homebrew/PPSA99808/`: o app (`make install-app`), o `cacert.pem` e,
  em `pico8/`, o `pico8_dyn` e o `pico8.dat` do usuário (`make upload-data`).
  O `paths.c` procura nesta ordem: `/app0`,
  `/mnt/sandbox/PPSA99808_000/app0` (o mesmo lugar visto depois da
  elevação), `/data/homebrew/PPSA99808` e, por fim, o layout antigo em
  `/data/pico8`.
- `/data/pico8/`: o que o PICO-8 escreve (`.lexaloffle/pico-8/`, com
  config, favoritos, carts e saves) e o `loader.log`.
- `/data/pico8/pico8_loader.elf`: o loader como payload (`make run`).
