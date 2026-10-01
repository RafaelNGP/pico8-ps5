# app_probe: feasibility test for running the loader inside a native app

A minimal native app (`eboot.bin`) that answered whether `pico8_loader` can
run inside an app's own process instead of as a payload. It checks:

1. sandbox elevation through the local elfldr;
2. reading `/data/pico8/pico8_dyn`;
3. whether the fixed range `0x400000`–`0xb70000` is free in the app process
   (it lists the address space with `sceKernelVirtualQuery` first);
4. `RW → RX` and executing code there, with DATA staying writable.

Results go to `/data/pico8/app_probe.log` and a notification.

Result on 2026-10-01: with the default image base the eboot itself sits at
`0x400000`. With `TEXT_BASE` = `0x1000000` the range is free and code runs.
That led to `pico8_app/TEXT_BASE` (now `0x4000000`).

**It uses the same Title ID as the real app (`PPSA99808`).** Installing it
replaces PICO-8 on the Home screen until you reinstall the app.

Build with `deps/build_app.sh app_probe`.
