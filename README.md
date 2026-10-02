# PICO-8 for PS5

<p align="center">
  <a href="https://github.com/RafaelNGP/pico8-ps5/releases/latest"><img alt="release" src="https://img.shields.io/github/v/release/RafaelNGP/pico8-ps5?color=brightgreen"></a>
  <a href="#installation"><img alt="platform: PS5 homebrew" src="https://img.shields.io/badge/platform-PS5%20homebrew-003791"></a>
  <a href="https://www.lexaloffle.com/pico-8.php"><img alt="requires: PICO-8 0.2.7 (Linux)" src="https://img.shields.io/badge/requires-PICO--8%200.2.7%20(Linux)-83769c"></a>
  <a href="LICENSE"><img alt="license" src="https://img.shields.io/github/license/RafaelNGP/pico8-ps5"></a>
  <a href="https://github.com/RafaelNGP/pico8-ps5/issues/new?template=bug_report.yml"><img alt="issues: report a problem" src="https://img.shields.io/badge/issues-report%20a%20problem-ff0000"></a>
</p>

Runs the **official PICO-8 for Linux** on a jailbroken PS5 as a native app
with its own icon on the Home screen. It boots straight into Splore and
works with the DualSense, with sound, and with cart downloads from the BBS.

This is a "bring your own license" project: PICO-8 itself is not included.
You need your own copy of PICO-8 (the Linux 64-bit
build, bought at <https://www.lexaloffle.com/pico-8.php>; tested with 0.2.7).

**Unofficial project, not affiliated with or endorsed by Lexaloffle.**

## Installation

No building and no commands: you copy one folder to the PS5.

**Requirements on the PS5** (tested on a single console):

- kstuff and ShadowMountPlus running;
- elfldr listening on `127.0.0.1:9021`, which usually comes with
  kstuff/Payload Manager. The app uses elfldr to get access to `/data`
  (sandbox elevation);
- a way to copy files to the PS5, such as an FTP server.

**Steps**, with the `PPSA99808.zip` from the [latest release](https://github.com/RafaelNGP/pico8-ps5/releases/latest):

1. extract the zip and copy the `PPSA99808` folder, as a whole, to `/data/homebrew/`;
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
    README.txt, LICENSE, licenses/
/data/pico8/                     created by the app
    .lexaloffle/pico-8/          config, favourites, downloaded carts and saves
    loader.log                   log of the last run
```

If a file is missing, or if `pico8_dyn` is a different version, the app
shows a notification. To update, close the app and copy the new
`PPSA99808` folder over the old one. If the Home screen keeps the old icon,
delete the PICO-8 icon from the Home screen (only the icon, not the folder):
ShadowMountPlus adds it again within ~15 s. Saves live in `/data/pico8`,
outside the app folder, so they are kept.

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
  `unzip`, `zip` and `python3`. On Fedora, `llvm-config` comes
  from the `llvm-devel` package;
- your PICO-8 Linux copy in `~/pico-8/` (or in `PICO8_DIR`), used by
  `make upload-data`.

```bash
cd pico8_loader
export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
export PS5_HOST=192.168.0.50   # your PS5's IP (FTP on port 2121); or put
                               # "PS5_HOST := <ip>" in pico8_loader/local.mk

make               # builds the app into pico8_app/build/PPSA99808
make install-app   # builds the app and uploads it to /data/homebrew/PPSA99808
make upload-data   # once: uploads pico8_dyn and pico8.dat to PPSA99808/pico8/
make log           # prints /data/pico8/loader.log
make release       # builds release/<version>/PPSA99808.zip
```

The first build downloads and compiles the PS5 port of SDL2, libcurl,
mbedTLS and the native app boilerplate, which takes a few minutes.
`install-app` needs the app to be closed on the PS5, because FTP refuses to
overwrite an `eboot.bin` that is in use.

The `make release` zip has the `PPSA99808` folder at its root, as the
[PS5 homebrew catalog](https://github.com/blackbearreloaded/ps5-homebrew-catalog/blob/main/docs/artifact-formats.md)
expects, with a `README.txt` and the license notices inside it. Raise
`contentVersion` in `pico8_app/sce_sys/param.json` for every release: consoles
use it to detect updates.

## Layout

| Folder | What it is |
|---|---|
| `pico8_loader/` | the loader: loads `pico8_dyn` and binds its imports to the PS5's libc and SDL2. See its [README](pico8_loader/README.md). |
| `pico8_app/` | packages the loader as a native app (`eboot.bin`), plus the package texts in `release/`. |
| `deps/` | build scripts for SDL2, libcurl and the app, plus patches and dlmalloc. |
| `mmap_probe/`, `app_probe/` | feasibility tests run on the console (fixed addresses and code execution). |

## License

PICO-8 for PS5 is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 3 or (at your option) any
later version. See [LICENSE](LICENSE).

The app bundles code under other licenses, all compatible with the GPL-3.0:
[ps5-native-app-boilerplate](https://github.com/blackbearreloaded/ps5-native-app-boilerplate)
(GPL-3.0-or-later), SDL2 (zlib), curl (curl license), mbedTLS (Apache-2.0) and
dlmalloc (MIT-0). Their notices ship in the release zip under `licenses/`.

PICO-8 is (c) Lexaloffle Games and is not included: you need your own copy.
The app icon (`pico8_app/sce_sys/icon0.png`) is the PICO-8 logo, (c) Lexaloffle
Games; it is not covered by the GPL.
