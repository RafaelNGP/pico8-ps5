/* pico8_app - avisa sobre imports que o rtld nao resolveu.
 * Copyright (C) 2026 RafaelNGP
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Num app, o slot do GOT de um import cujo modulo o rtld nao carregou fica
 * em NULL ou num placeholder sem nada mapeado (visto: 0x840000000), e so
 * explode no uso. Consertar o slot em runtime nao segura: o rtld o regrava
 * quando outros modulos carregam (durante o SDL_Init). A correcao eh no
 * link: definir a funcao no app (app_glue.cpp) ou puxar o objeto da libc.a
 * do SDK (build.env). Este check so aponta o que falta, no inicio do log,
 * junto com o mapa de modulos para traduzir enderecos de crash.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

void lg(const char *fmt, ...);

/* app_imports.c, gerado por deps/gen_app_imports.py */
struct p8_import { const char *name; uint8_t module; };
extern const char *const p8_import_modules[];
extern const struct p8_import p8_imports[];
extern const int p8_n_imports;
extern const int32_t p8_import_got[];

typedef struct {
    void *start;
    void *end;
    int64_t offset;
    int32_t protection;
    int32_t memory_type;
    uint8_t flags;
    char name[32];
} vq_info_t;

int sceKernelVirtualQuery(const void *, int, vq_info_t *, size_t);
int sceKernelQueryMemoryProtection(void *, void **, void **, int *);

static void dump_modules(void)
{
    const char *addr = NULL;
    char last[32] = "";

    for (int i = 0; i < 4096; i++) {
        vq_info_t vq;
        memset(&vq, 0, sizeof(vq));
        if (sceKernelVirtualQuery(addr, 1 /* FIND_NEXT */, &vq, sizeof(vq)) != 0)
            break;
        vq.name[sizeof(vq.name) - 1] = 0;
        if (vq.name[0] && strcmp(vq.name, last) != 0 && (vq.protection & 4))
            lg("  modulo %-28s text 0x%lx-0x%lx", vq.name,
               (unsigned long)(uintptr_t)vq.start, (unsigned long)(uintptr_t)vq.end);
        memcpy(last, vq.name, sizeof(last));
        if ((uintptr_t)vq.end <= (uintptr_t)addr)
            break;
        addr = vq.end;
    }
}

/* Um import valido aponta para memoria mapeada (codigo ou, para dados
 * como __stdinp, um segmento RW). */
static int slot_valid(void *addr)
{
    void *start, *end;
    int prot;

    return addr && sceKernelQueryMemoryProtection(addr, &start, &end, &prot) == 0;
}

void p8_app_check_imports(void)
{
    int bad = 0;

    dump_modules();
    for (int i = 0; i < p8_n_imports; i++) {
        void **slot = (void **)((const char *)&p8_import_got[i] + p8_import_got[i]);
        void *cur = *(void *volatile *)slot;
        if (slot_valid(cur))
            continue;
        bad++;
        lg("AVISO: import nao resolvido (%p): %s (%s)", cur, p8_imports[i].name,
           p8_import_modules[p8_imports[i].module]);
    }
    lg("imports do eboot: %d, nao resolvidos: %d", p8_n_imports, bad);
}
