PICO-8 for PS5 - version @VERSION@
==================================

Native app that runs the official PICO-8 for Linux on a jailbroken PS5,
with its own icon on the Home screen. It boots straight into Splore and
works with the DualSense, with sound, and with cart downloads.

This package does NOT include PICO-8. You need your own copy: the Linux
64-bit build, bought at https://www.lexaloffle.com/pico-8.php
(tested with 0.2.7).

Unofficial project, not affiliated with or endorsed by Lexaloffle.


Requirements on the PS5
-----------------------
- kstuff and ShadowMountPlus running.
- elfldr listening on 127.0.0.1:9021 (it usually comes with kstuff or
  Payload Manager). The app uses elfldr to get access to /data.
- A way to copy files to the PS5 (e.g. an FTP server).


Installation
------------
1. Copy the @TITLE_ID@ folder from this package, as a whole, to
   /data/homebrew/ on the PS5.

2. From your PICO-8 Linux zip, copy these two files into
   @TITLE_ID@/pico8/ (the folder with PUT-YOUR-FILES-HERE.txt):
       pico8_dyn
       pico8.dat

3. Within ~15 s, ShadowMountPlus adds the PICO-8 icon to the Home
   screen. Launch it from there.

When you're done, the folder on the PS5 looks like this:

   /data/homebrew/@TITLE_ID@/
       eboot.bin
       sandbox-elevator.elf
       cacert.pem
       sce_module/libc.prx
       sce_sys/param.json, icon0.png
       pico8/pico8_dyn          <- yours
       pico8/pico8.dat          <- yours

If a file is missing, the app shows a notification saying which one.


Updating
--------
Close the app on the PS5 and copy the @TITLE_ID@ folder of the new version
over the old one. Don't delete the pico8/ subfolder: that's where your
files are.


Where your data lives
---------------------
Saves, favourites, downloaded carts and settings are stored in
/data/pico8/.lexaloffle/pico-8/, outside the app folder, and stay there
when you update or reinstall the app.


Troubleshooting
---------------
/data/pico8/loader.log records every run: which files were found and, if
the app closes on its own, where it crashed (registers, stack and the last
calls). Attach this file when reporting a problem.

Limitations: USB keyboards and the on-screen keyboard don't work. Tested
on a single PS5; other firmware versions may need adjustments.


Licenses
--------
Source code: @SOURCE@ (commit @COMMIT@).
The license notices of the bundled components are in licenses/.
