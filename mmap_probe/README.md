# mmap_probe — teste de viabilidade do loader PICO-8 no PS5

Este é o **primeiro passo** do projeto. Ele não carrega o PICO-8.
Responde uma única pergunta que decide a arquitetura de todo o resto:

> O processo de homebrew no PS5 consegue reservar a faixa de endereços
> fixos que o `pico8_dyn` exige (`0x400000`–`0xb6f000`)?

O `pico8_dyn` é um ELF **não-PIE**, então precisa ser carregado nesses
endereços exatos. Se o PS5 liberar a faixa, o loader usa os endereços
nativos e o trabalho é só resolver imports. Se não liberar, o plano B é
linkar o próprio loader num endereço alto para desocupar a faixa.

## Pré-requisitos

- Um PC Linux (seu Nobara serve) com o **ps5-payload-sdk** instalado:
  ```bash
  git clone https://github.com/ps5-payload-dev/sdk ps5-payload-sdk
  cd ps5-payload-sdk
  sudo apt-get install build-essential cmake clang clang-15 lld lld-15   # ou os equivalentes do Fedora/Nobara
  make
  sudo make DESTDIR=/opt/ps5-payload-sdk install
  export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
  ```
- Um **PS5 já jailbroken** com um ELF loader ativo na porta **9021**
  (Relapse + etaHEN/elfldr, firmware ≤ 13.60).
- `netcat` (`nc`) no PC.
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
2. Descubra o IP do PS5 (Ajustes → Rede → Status da conexão).
3. No PC:
   ```bash
   make test PS5_HOST=192.168.x.x
   ```
   ou manualmente:
   ```bash
   nc -q0 192.168.x.x 9021 < mmap_probe.elf
   ```

## Lendo o resultado

O probe reporta de duas formas:

- **Toast na tela do PS5** (via `notify`): aparece uma notificação no
  canto. A última linha é a que importa:
  - `RESULTADO: VIAVEL` → a faixa está livre. Seguimos com endereços nativos.
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

1. `TEXT 0x400000` — tenta reservar só o segmento de código (~1,6 MB),
   escreve e lê de volta para provar que a página é utilizável.
2. `DATA 0x795000` — o mesmo para o segmento de dados+BSS (~3,9 MB).
3. `FAIXA INTEIRA` — reserva os dois de uma vez (~7,5 MB), que é o que o
   loader real fará antes de copiar os segmentos do `pico8_dyn`.

Cada `mmap` usa `MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE`. Se o kernel
devolver um endereço diferente do pedido, o probe trata como falha de
propósito, para não mascarar o resultado.
