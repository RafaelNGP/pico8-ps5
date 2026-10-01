#!/usr/bin/env python3
# Copyright (C) 2026 RafaelNGP
# SPDX-License-Identifier: GPL-3.0-or-later

"""Gera pico8_app/src/app_imports.c a partir do eboot linkado.

Num app, o rtld do PS5 deixa em NULL (ou num placeholder) o slot do GOT
de um import cujo modulo nao carregou (visto: sceKeyboardInit). Em payload
o elfldr resolvia tudo. O arquivo gerado lista cada import com o modulo
que o fornece (o primeiro stub .so do SDK que o define, na ordem dos
NEEDED) e o slot do GOT, para o app_check.c apontar no log o que faltou.

Uso: gen_app_imports.py <llvm-pie.elf> <dir dos stubs> <saida.c>
O `make app` do pico8_loader roda isto entre dois builds.
"""

import re
import subprocess
import sys


def run(*cmd):
    return subprocess.run(cmd, check=True, capture_output=True, text=True).stdout


def main():
    elf, stubs, out = sys.argv[1:4]

    needed = re.findall(r'\[(\S+)\.sprx\]', run('llvm-readelf', '-d', elf))
    names = sorted({l.split()[-1] for l in run('nm', '-D', '--undefined-only', elf).splitlines()
                    if l.strip()})

    provides = {}
    for mod in needed:
        for line in run('nm', '-D', '--defined-only', f'{stubs}/{mod}.so').splitlines():
            provides.setdefault(line.split()[-1], mod)

    mods = list(dict.fromkeys(provides.get(n, '?') for n in names))
    o = []
    o.append('/* Gerado por deps/gen_app_imports.py - nao editar.')
    o.append(' * Copyright (C) 2026 RafaelNGP')
    o.append(' * SPDX-License-Identifier: GPL-3.0-or-later')
    o.append(' */')
    o.append('')
    o.append('#include <stdint.h>')
    o.append('')
    o.append('const char *const p8_import_modules[] = {')
    o += [f'    "{m}",' for m in mods]
    o.append('};')
    o.append('')
    o.append('const struct p8_import { const char *name; uint8_t module; } p8_imports[] = {')
    o += [f'    {{ "{n}", {mods.index(provides.get(n, "?"))} }},' for n in names]
    o.append('};')
    o.append('const int p8_n_imports = sizeof(p8_imports) / sizeof(p8_imports[0]);')
    o.append('')
    o.append('/* Offset PC-relativo de cada campo ate o slot do GOT do import:')
    o.append(' * slot = (char *)&p8_import_got[i] + p8_import_got[i]. Resolvido no')
    o.append(' * link, entao o compilador nao tem como presumir nada sobre o valor. */')
    o.append('__asm__(')
    o.append('    ".section .rodata\\n"')
    o.append('    ".p2align 2\\n"')
    o.append('    ".globl p8_import_got\\n"')
    o.append('    "p8_import_got:\\n"')
    o += [f'    "    .long {n}@GOTPCREL\\n"' for n in names]
    o.append('    ".text\\n");')
    o.append('')
    open(out, 'w').write('\n'.join(o))
    print(f'{len(names)} imports, {len(mods)} modulos -> {out}')


if __name__ == '__main__':
    main()
