# mmap_probe — teste de viabilidade do loader PICO-8 no PS5

Este é o **primeiro passo** do projeto. Ele não carrega o PICO-8.
Responde uma única pergunta que decide a arquitetura de todo o resto:

> O processo de homebrew no PS5 consegue reservar a faixa de endereços
> fixos que o `pico8_dyn` exige (`0x400000`–`0xb70000`) e tornar essa
> memória executável?

O `pico8_dyn` é um ELF **não-PIE**, então precisa ser carregado nesses
endereços exatos. Se o PS5 liberar a faixa, o loader usa os endereços
nativos e o trabalho é só resolver imports. Se não liberar, o plano B é
linkar o próprio loader num endereço alto para desocupar a faixa.

## Pré-requisitos

- Um PC Linux (seu Nobara serve) com o **ps5-payload-sdk** instalado.
  No Fedora/Nobara o `llvm-config` vem no pacote `llvm-devel`; sem ele o
  `Makefile.inc` do SDK deixa `LLVM_CONFIG` vazio e o compilador vira `/clang`:
  ```bash
  sudo dnf install clang lld llvm llvm-devel make cmake python3-pyelftools
  cd ~/pico-8/ps5-payload-sdk
  make LLVM_CONFIG=/usr/bin/llvm-config
  sudo make LLVM_CONFIG=/usr/bin/llvm-config DESTDIR=/opt/ps5-payload-sdk install
  export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
  ```
- Um **PS5 já jailbroken** com um ELF loader ativo na porta **9021**
  (Relapse + etaHEN/elfldr, firmware ≤ 13.60).
- PS5 e PC na **mesma rede**.

## Build

```bash
export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
cd mmap_probe
make
```

Isso gera `mmap_probe.elf`.

## Rodar no PS5

1. No PS5 já jailbroken, deixe o ELF loader escutando na porta 9021
   (é o estado normal depois de rodar o Relapse + elfldr).
2. O IP do PS5 (<PS5_IP>) já é o padrão no Makefile; para outro, use `make test PS5_HOST=<ip>`.
3. No PC:
   ```bash
   make test
   ```
   ou manualmente (o `nc` do Fedora é o `ncat`, que não aceita `-q0`):
   ```bash
   ncat --send-only <PS5_IP> 9021 < mmap_probe.elf
   ```

## Lendo o resultado

O probe reporta de duas formas:

- **Toast na tela do PS5** (via `notify`): aparece uma notificação no
  canto. A última linha é a que importa:
  - `RESULTADO: VIAVEL` → faixa livre e executável. Seguimos com endereços nativos.
  - `RESULTADO: PARCIAL` → faixa livre, mas o kernel nega `PROT_EXEC` em
    memória anônima (ou o código travou — veja a última mensagem `EXEC:`).
  - `RESULTADO: BLOQUEADO` → precisamos do plano B (relink alto).
- **Log no PC**: se você tiver o `klog`/stdout do SDK capturando a saída
  (ex. `nc -l` na porta de log, conforme seu setup do etaHEN), verá as
  linhas `[mmap_probe] ...` com cada segmento testado e o `errno` em
  caso de falha.

Me mande o texto do resultado (foto do toast ou o log) e eu decido o
próximo passo:
- **VIAVEL** → escrevo o esqueleto do loader de ELF + a tabela de shims
  dos 217 imports.
- **BLOQUEADO** → ajusto a estratégia de carga antes de qualquer loader.

## O que o probe faz, em detalhe

1. `FAIXA 0x400000-0xb70000` — reserva TEXT+DATA+BSS de uma vez (~7,5 MB,
   alinhado a páginas de 16 KiB), escreve e lê nas pontas.
2. `EXEC` — copia `mov eax, 42; ret` para o início da faixa, faz
   `mprotect(R+X)` e chama. Sem isso não adianta ter a faixa.
3. Se a faixa inteira falhar, testa `TEXT 0x400000` e `DATA 0x794000`
   separados para mostrar qual pedaço está ocupado.

O `mmap` usa `MAP_FIXED | MAP_EXCL`: no FreeBSD, `MAP_FIXED` sozinho
substitui em silêncio qualquer mapeamento existente, o que daria "OK"
falso (e poderia derrubar o próprio payload). Se o kernel não aceitar
`MAP_EXCL`, cai para um mmap só com hint, que nunca sobrescreve.
