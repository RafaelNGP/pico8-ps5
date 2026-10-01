# Security policy

## Reporting a vulnerability

Please **don't open a public issue** for security problems. Report them
privately through GitHub instead:
<https://github.com/RafaelNGP/pico8-ps5/security/advisories/new>.

Include what you found, the release (`v0.1.1`, …) or commit, and how to
reproduce it. I'll reply as soon as I can and credit you in the fix unless you'd
rather stay anonymous.

## Scope

In scope: this repository's code (the loader, the app glue, the build and
packaging scripts) and the release zips published here.

Out of scope:

- PICO-8 itself, which belongs to Lexaloffle Games;
- the PS5 jailbreak, kstuff, elfldr, ShadowMountPlus and other system payloads;
- third-party components such as SDL2, curl, mbedTLS and
  ps5-native-app-boilerplate. Please report those upstream; if one affects this
  app, an issue here to bump it is welcome.

## Supported versions

Only the latest release gets fixes.
