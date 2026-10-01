/* pico8_loader - definicoes compartilhadas
 * Copyright (C) 2026 RafaelNGP
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <stdio.h>
#include <stdint.h>

/* Dados que o PICO-8 escreve (saves, carts baixados, config) e o log.
 * Fica fora da pasta do app, para uma atualizacao nao apagar nada. */
#define P8_DIR       "/data/pico8"
#define P8_LOG       P8_DIR "/loader.log"
/* O Title ID vem do pico8_app/sce_sys/param.json: o Makefile e o
 * deps/build_app.sh passam -DAPP_TITLE_ID=<id>. */
#ifndef APP_TITLE_ID
#error "APP_TITLE_ID nao definido (vem do pico8_app/sce_sys/param.json)"
#endif
#define P8_STR_(x)   #x
#define P8_STR(x)    P8_STR_(x)
#define P8_TITLE_ID  P8_STR(APP_TITLE_ID)

/* Onde estao o pico8_dyn e o cacert.pem (paths.c): dentro da pasta do
 * app ou, no layout antigo, em P8_DIR. */
extern char p8_bin[256];
extern char p8_cacert[256];
int p8_find_files(void);

/* O pico8_dyn nao eh PIE: os segmentos vao nos enderecos do proprio ELF.
 * O main reserva esta janela antes de qualquer alocacao (o kernel do PS5
 * entrega primeiro os enderecos baixos) e depois mapeia nela as faixas
 * lidas dos cabecalhos. A 0.2.7 usa 0x400000-0xb70000. O eboot do app
 * fica acima da janela (pico8_app/TEXT_BASE). */
#define P8_WINDOW_LO 0x0000000000400000ULL
#define P8_WINDOW_HI 0x0000000002400000ULL   /* 32 MiB */
#define P8_PAGE      0x4000ULL               /* paginas de 16 KiB */

int sceKernelAvailableFlexibleMemorySize(size_t *);

typedef struct {
    const char *name;
    void *addr;
} shim_t;

/* Tabelas de shims (NULL-terminadas). */
extern const shim_t shims_libc[];
extern const shim_t shims_sdl[];

/* FILE* do log (NULL se nao abriu) e a saida do stdout/stderr do pico8:
 * o log ou, sem ele, o stdout do sistema. p8_out nunca eh NULL. */
extern FILE *p8_log;
extern FILE *p8_out;

void lg(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void notify(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
/* Falha interna: detalhes no log, mensagem generica na tela. */
void p8_fail(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/* libcurl emulada (net_curl.c). */
void *net_curl_dlopen(const char *name);
void *net_curl_dlsym(void *h, const char *name);

/* Prepara tabelas de ctype no formato glibc. */
void shims_libc_init(void);

void shims_sdl_init(void);

/* Heap proprio (p8_alloc.c). */
void p8_alloc_init(void);
void *p8_malloc(size_t size);
void *p8_calloc(size_t n, size_t size);
void *p8_realloc(void *ptr, size_t size);
void p8_free(void *ptr);
char *p8_strdup(const char *s);
