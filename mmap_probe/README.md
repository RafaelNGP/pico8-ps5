# mmap_probe: feasibility test for the PICO-8 loader on PS5

This was the **first step** of the project. It doesn't load PICO-8. It
answers a single question that decided the architecture of everything
else:

> Can a homebrew process on the PS5 reserve the fixed address range that
> `pico8_dyn` requires (`0x400000`–`0xb70000`) and make that memory
> executable?

`pico8_dyn` is a **non-PIE** ELF, so it has to be loaded at those exact
addresses. If the PS5 frees the range, the loader uses the native
addresses and the remaining work is resolving imports. If it doesn't, the
fallback is to link the loader itself at a high address to free the range.

Result on 2026-10-01: **VIABLE**. The range was free and executable, so the
loader uses the native addresses.

## Requirements

- A Linux PC with **ps5-payload-sdk** installed. On Fedora/Nobara,
  `llvm-config` comes from the `llvm-devel` package; without it the SDK's
  `Makefile.inc` leaves `LLVM_CONFIG` empty and the compiler becomes `/clang`:
  ```bash
  sudo dnf install clang lld llvm llvm-devel make cmake python3-pyelftools
  cd ~/pico-8/ps5-payload-sdk
  make LLVM_CONFIG=/usr/bin/llvm-config
  sudo make LLVM_CONFIG=/usr/bin/llvm-config DESTDIR=/opt/ps5-payload-sdk install
  export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
  ```
- A **jailbroken PS5** with an ELF loader listening on port **9021**.
- The PS5 and the PC on the **same network**.

## Build

```bash
export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
cd mmap_probe
make
```

This produces `mmap_probe.elf`.

## Running it on the PS5

1. On the jailbroken PS5, keep the ELF loader listening on port 9021.
2. Pass the PS5's IP: `make test PS5_HOST=<ip>` (or export `PS5_HOST`).
3. On the PC:
   ```bash
   make test PS5_HOST=192.168.0.50
   ```
   or by hand (Fedora's `nc` is `ncat`, which doesn't accept `-q0`):
   ```bash
   ncat --send-only 192.168.0.50 9021 < mmap_probe.elf
   ```

## Reading the result

The probe reports in two ways:

- **A notification on the PS5 screen** (via `notify`). The last line is the
  one that matters:
  - `RESULTADO: VIAVEL` (viable): the range is free and executable, so the
    loader can use the native addresses.
  - `RESULTADO: PARCIAL` (partial): the range is free, but the kernel denies
    `PROT_EXEC` on anonymous memory, or the code hung (see the last `EXEC:`
    message).
  - `RESULTADO: BLOQUEADO` (blocked): the fallback (high relink) is needed.
- **The log on the PC**: if the SDK's klog/stdout output is being captured,
  you'll see `[mmap_probe] ...` lines for each segment tested, with
  `errno` on failure.

## What the probe does, in detail

1. `FAIXA 0x400000-0xb70000`: reserves TEXT+DATA+BSS at once (~7.5 MB,
   aligned to 16 KiB pages) and writes and reads at both ends.
2. `EXEC`: copies `mov eax, 42; ret` to the start of the range, calls
   `mprotect(R+X)` and calls it. Without this, having the range is useless.
3. If the whole range fails, it tests `TEXT 0x400000` and `DATA 0x794000`
   separately to show which part is taken.

The `mmap` uses `MAP_FIXED | MAP_EXCL`: on FreeBSD, `MAP_FIXED` alone
silently replaces any existing mapping, which would give a false "OK" (and
could crash the payload itself). If the kernel doesn't accept `MAP_EXCL`,
it falls back to an `mmap` with only a hint, which never overwrites.
