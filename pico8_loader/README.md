# pico8_loader

Runs the **official PICO-8 for Linux** (`pico8_dyn`, x86-64) on a
jailbroken PS5, "bring your own license" style: you provide your own
`pico8_dyn` and `pico8.dat`, which are not redistributed.

The loader follows the idea of the Android→PS Vita ports (`so_loader`): it
loads the ELF at its original addresses, resolves its imports with native
functions and never modifies the binary.

## How it works

| Part | What it does |
|---|---|
| `main.c` | Reserves the low address window, maps the (non-PIE) `pico8_dyn` at the addresses from its own program headers, applies the relocations and calls `init` and `main` (with `-splore`) on a thread with a 32 MiB stack. A crash handler writes registers, stack and the last calls to the log. |
| `shims_libc.c` | The 125 glibc/libm/libdl imports, on top of the PS5 libc. Handles what differs: `*_chk`, `__xstat`, `__ctype_*_loc`, `struct dirent`, `open` flags, `clock`/`clock_gettime`, `dlerror`. PICO-8's stdout/stderr goes to the log. |
| `shims_sdl.c` | The 91 `SDL_*` imports go straight to the PS5 port of SDL2 (the SDL2 ABI is stable). A few go through wrappers that log. |
| `net_curl.c` | Answers Splore's `dlopen("libcurl.so")` with a real libcurl (8.18 + mbedTLS). |
| `paths.c` | Finds `pico8_dyn`, `pico8.dat` and `cacert.pem` inside the app folder, checks the version and tells the user (in English) what is missing. |
| `p8_alloc.c` | Private heap (dlmalloc mspace, growing with `mmap`) for PICO-8, SDL and curl. |

The loader runs as the `eboot.bin` of a native app (`../pico8_app`) with its
own Home icon.

PS5 quirks found along the way:

- Making part of an anonymous `mmap` `PROT_EXEC` removes write access from
  the whole mapping. That's why TEXT, thunks and DATA are three separate
  `mmap`s.
- The kernel hands out low addresses first, so the loader reserves
  `0x400000`–`0x2400000` (`PROT_NONE`, which costs no memory) before any
  allocation. The real ranges then come from `pico8_dyn`'s `PT_LOAD`
  headers, so a larger future version fits without code changes.
- Only the foreground app is shown on the TV, so a background payload is
  not an option: the loader has to run inside the app's own `eboot.bin`.
- SDL asks for 64 MiB of video memory; `deps/patches/sdl2-dmem-fallback.patch`
  retries with 32 MiB (enough for 1080p) when a process can't get 64.
- Native TLS (`sceHttp2`) failed with `0x8095f00c` on HTTPS downloads,
  hence libcurl with mbedTLS.

Quirks of running inside a native app (where an ELF payload would have had
elfldr resolve everything):

- The kernel loads the eboot at `0x400000 + vaddr`, right where
  `pico8_dyn` has to go. `pico8_app/TEXT_BASE` (`0x4000000`) and
  `deps/patches/boilerplate-native.patch` move the eboot to `0x4400000`,
  above the reserved window.
- The app starts sandboxed, without access to `/data`. The boilerplate's
  elevation (through the local elfldr) unlocks the filesystem. After it,
  `/app0` is no longer visible; the app folder shows up at
  `/mnt/sandbox/<titleId>_000/app0`.
- The rtld leaves imports from modules it doesn't load as NULL, or as a
  placeholder with nothing mapped (`0x840000000`): `libSceKeyboard`,
  `libSceImeDialog`, `libScePosixForWebKit` and `libkernel_sys`. Patching
  the GOT at runtime doesn't stick, because the rtld rewrites the slot.
  The fix is at link time: define the function in the app
  (`pico8_app/src/app_glue.cpp`) or pull the object from the SDK's `libc.a`
  (`pico8_app/build.env`). `app_check.c` logs whatever is left unresolved.
- Functions that do resolve may jump to a placeholder internally:
  `libSceLibcInternal`'s `getcwd` does, which is why the app has its own.
- The system libc heap has a small fixed capacity: an 8 MB `malloc` fails,
  and from then on even `malloc(12)` fails, with hundreds of MB still
  free. That's why `p8_alloc.c` exists.
- In the signal handler, `mcontext` sits at `+64` in the ucontext, not at
  `+16` as the SDK headers say.

## Usage

Building and installing are described in the [project README](../README.md):
`make` builds the app, `make install-app` and `make upload-data` copy it and
your PICO-8 files to the PS5 over FTP, and `make log` prints the log.

The targets that talk to the PS5 need its IP: pass `PS5_HOST=<ip>`, export
it, or put `PS5_HOST := <ip>` in `pico8_loader/local.mk` (ignored by git).

## Where files live on the PS5

- `/data/homebrew/PPSA99808/`: the app (`make install-app`), `cacert.pem`
  and, in `pico8/`, the user's `pico8_dyn` and `pico8.dat`
  (`make upload-data`). `paths.c` searches, in order: `/app0`,
  `/mnt/sandbox/PPSA99808_000/app0` (the same folder as seen after
  elevation), `/data/homebrew/PPSA99808` and, last, the old layout in
  `/data/pico8`.
- `/data/pico8/`: what PICO-8 writes (`.lexaloffle/pico-8/`, with config,
  favourites, carts and saves) and `loader.log`.

The `loader.log` messages themselves are still in Portuguese.
