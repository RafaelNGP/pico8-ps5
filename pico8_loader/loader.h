/* pico8_loader - definicoes compartilhadas */
#pragma once

#include <stdio.h>
#include <stdint.h>

/* Onde os arquivos do usuario ficam no PS5 (enviados por FTP). */
#define P8_DIR       "/data/pico8"
#define P8_BIN       P8_DIR "/pico8_dyn"
#define P8_LOG       P8_DIR "/loader.log"

/* Enderecos fixos do pico8_dyn (nao-PIE), alinhados a paginas de
 * 16 KiB. Faixa validada pelo mmap_probe em 2026-10-01.
 *
 * Tres mapeamentos separados. No PS5, dar PROT_EXEC a parte de um
 * mmap anonimo tira a escrita do mapeamento inteiro (o BSS ficava
 * read-only), entao codigo e dados nao podem dividir o mesmo mmap. */
#define P8_TEXT_LO   0x0000000000400000ULL
#define P8_TEXT_HI   0x0000000000598000ULL   /* 0x595550 alinhado */
#define P8_THUNK_LO  0x0000000000598000ULL   /* thunks dos imports sem shim */
#define P8_THUNK_HI  0x000000000059c000ULL
#define P8_DATA_LO   0x0000000000794000ULL   /* 0x795cf0 alinhado p/ baixo */
#define P8_DATA_HI   0x0000000000b70000ULL   /* 0xb6e380 alinhado p/ cima */

typedef struct {
    const char *name;
    void *addr;
} shim_t;

/* Tabelas de shims (NULL-terminadas). */
extern const shim_t shims_libc[];
extern const shim_t shims_sdl[];

/* FILE* do log; o stdout/stderr do pico8 tambem apontam pra ca. */
extern FILE *p8_log;

void lg(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void notify(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

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
