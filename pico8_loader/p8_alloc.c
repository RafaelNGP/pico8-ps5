/* p8_alloc - heap proprio para pico8, SDL e curl (dlmalloc em mspace)
 * Copyright (C) 2026 RafaelNGP
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Como app nativo, o heap da libc do PS5 tem capacidade fixa pequena: o
 * primeiro buffer de ~8 MB do pico8 falha e dai em diante ate malloc(12)
 * devolve NULL, com 400+ MB de memoria flexivel livre. Este heap cresce
 * por mmap anonimo (memoria flexivel). Cada consumidor libera pelo mesmo
 * alocador que alocou: o pico8 pelos shims, o SDL por
 * SDL_SetMemoryFunctions e o curl por curl_global_init_mem.
 */

#define ONLY_MSPACES 1
#define USE_LOCKS 1
#define HAVE_MORECORE 0
/* Paginas do PS5 sao de 16 KiB; segmentos de 16 MiB evitam mmaps miudos. */
#define DEFAULT_GRANULARITY ((size_t)16U * 1024U * 1024U)
#define MALLOC_FAILURE_ACTION
/* Codigo de terceiros: nao herda o -Werror/-Wextra deste projeto. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#pragma clang diagnostic ignored "-Wgnu-null-pointer-arithmetic"
#pragma clang diagnostic ignored "-Wnull-pointer-arithmetic"
#include "dlmalloc/malloc.c"   /* -I deps */
#pragma clang diagnostic pop

#include "loader.h"

static mspace heap;

void p8_alloc_init(void)
{
    if (!heap)
        heap = create_mspace(0, 1);
    lg("[mem] heap proprio (dlmalloc) %s", heap ? "ok" : "FALHOU");
}

void *p8_malloc(size_t size)
{
    void *p = mspace_malloc(heap, size);
    if (!p && size)
        lg("[mem] p8_malloc(%zu) falhou", size);
    return p;
}

void *p8_calloc(size_t n, size_t size)
{
    void *p = mspace_calloc(heap, n, size);
    if (!p && n && size)
        lg("[mem] p8_calloc(%zu, %zu) falhou", n, size);
    return p;
}

void *p8_realloc(void *ptr, size_t size)
{
    void *p = mspace_realloc(heap, ptr, size);
    if (!p && size)
        lg("[mem] p8_realloc(%zu) falhou", size);
    return p;
}

void p8_free(void *ptr)
{
    mspace_free(heap, ptr);
}

char *p8_strdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = p8_malloc(n);
    return p ? memcpy(p, s, n) : NULL;
}
