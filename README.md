# PICO-8 for PS5

Runs the **official PICO-8 for Linux** on a jailbroken PS5 as a native app
with its own icon on the Home screen. It boots straight into Splore and
works with the DualSense, with sound, and with cart downloads from the BBS.

This is a "bring your own license" project: it contains and redistributes
nothing from Lexaloffle. You need your own copy of PICO-8 (the Linux 64-bit
build, bought at <https://www.lexaloffle.com/pico-8.php>; tested with 0.2.7).

**Unofficial project, not affiliated with or endorsed by Lexaloffle.**

## Installation

No building and no commands: you copy one folder, the same way you install
ProsperoEden.

**Requirements on the PS5** (tested on a single console):

- kstuff and ShadowMountPlus running;
- elfldr listening on `127.0.0.1:9021`, which usually comes with
  kstuff/Payload Manager. The app uses elfldr to get access to `/data`
  (sandbox elevation, the same mechanism ProsperoEden uses);
- a way to copy files to the PS5, such as an FTP server.

**Steps**, with the `pico8-ps5-<version>.zip` package:

1. copy the `PPSA99808` folder from the zip, as a whole, to `/data/homebrew/`;
2. from your PICO-8 Linux zip, copy `pico8_dyn` and `pico8.dat` into
   `PPSA99808/pico8/`;
3. within ~15 s, ShadowMountPlus adds the **PICO-8** icon to the Home screen.

```
/data/homebrew/PPSA99808/        the app folder (named after its Title ID)
    eboot.bin                    the loader + SDL2 + libcurl/mbedTLS
    sandbox-elevator.elf         elevation helper (only accepts PPSA99808)
    cacert.pem                   CA certificates for Splore's HTTPS
    sce_module/libc.prx          boilerplate runtime
    sce_sys/param.json, icon0.png
    pico8/pico8_dyn, pico8.dat   your PICO-8 files
/data/pico8/                     created by the app
    .lexaloffle/pico-8/          config, favourites, downloaded carts and saves
    loader.log                   log of the last run
```

If a file is missing, or if `pico8_dyn` is a different version, the app
shows a notification. To update, close the app and copy the new
`PPSA99808` folder over the old one. Saves live in `/data/pico8`, outside
the app folder, so they are kept.

## Known limitations

- USB keyboards and the on-screen keyboard don't work: `libSceKeyboard`
  and `libSceImeDialog` don't load inside an app. Splore and games work
  with the controller.
- Tested on a single PS5. Other firmware versions may need adjustments.

If the app closes or freezes, `/data/pico8/loader.log` shows which files
were used and, after a crash, the registers, the stack and the last 256
libc/SDL calls.

## Building

Only needed for development. To just play, use the zip.

**On the PC** (Linux):

- [ps5-payload-sdk](https://github.com/ps5-payload-dev/sdk) in `/opt/ps5-payload-sdk`;
- `make`, `ninja`, `cmake`, `clang`/`lld`/`llvm-ar`, `git`, `curl`, `wget`,
  `unzip`, `zip`, and `python3` with Pillow. On Fedora, `llvm-config` comes
  from the `llvm-devel` package;
- your PICO-8 Linux copy in `~/pico-8/` (or in `PICO8_DIR`). Its
  `lexaloffle-pico8.png` becomes the icon of local builds.

```bash
cd pico8_loader
export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
export PS5_HOST=192.168.0.50   # your PS5's IP (FTP on port 2121)

make install-app   # builds the app and uploads it to /data/homebrew/PPSA99808
make upload-data   # once: uploads pico8_dyn and pico8.dat to PPSA99808/pico8/
make log           # prints /data/pico8/loader.log
make release       # builds release/pico8-ps5-<version>.zip
```

The first build downloads and compiles the PS5 port of SDL2, libcurl,
mbedTLS and the native app boilerplate, which takes a few minutes.
`install-app` needs the app to be closed on the PS5, because FTP refuses to
overwrite an `eboot.bin` that is in use.

The `make release` zip contains the `PPSA99808` folder with a generic icon
(nothing from Lexaloffle), a `README.txt` and the license notices.

## Layout

| Folder | What it is |
|---|---|
| `pico8_loader/` | the loader: loads `pico8_dyn` and binds its imports to the PS5's libc and SDL2. See its [README](pico8_loader/README.md). |
| `pico8_app/` | packages the loader as a native app (`eboot.bin`), plus the package texts in `release/`. |
| `deps/` | build scripts for SDL2, libcurl and the app, plus patches and dlmalloc. |
| `mmap_probe/`, `app_probe/` | feasibility tests run on the console (fixed addresses and code execution). |
