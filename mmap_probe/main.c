/* mmap_probe - PICO-8 PS5 loader feasibility test
 * Copyright (C) 2026 RafaelNGP
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * O pico8_dyn eh um ELF nao-PIE que exige carga em endereco fixo:
 *   LOAD  0x0000000000400000  R E  (~1.6 MB)
 *   LOAD  0x0000000000795cf0  RW   (~3.9 MB em memoria com BSS)
 * Faixa total aproximada: 0x400000 .. 0xb6f000  (~7.5 MB)
 *
 * Este payload NAO carrega o PICO-8. Ele responde as duas perguntas que
 * decidem a arquitetura do loader:
 *   1. O processo consegue reservar essa faixa no endereco exato?
 *   2. Memoria anonima nessa faixa pode virar executavel (RW -> RX)?
 * Imprime o resultado via notify (toast na tela) e tambem no stdout
 * (klog do elfldr).
 */

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <sys/mman.h>

/* Segmentos reais extraidos do pico8_dyn com `readelf -lW`. */
#define SEG_TEXT_ADDR   0x0000000000400000ULL
#define SEG_TEXT_SIZE   0x0000000000198000ULL   /* 0x195550 arredondado p/ 16 KiB */

#define SEG_DATA_ADDR   0x0000000000794000ULL   /* 0x795cf0 alinhado p/ baixo em 16 KiB */
#define SEG_DATA_END    0x0000000000b70000ULL   /* 0x795cf0+0x3d8690 alinhado p/ cima */

#define PAGE            0x4000ULL               /* PS5 usa paginas de 16 KiB */

#ifndef MAP_EXCL
#define MAP_EXCL        0x00004000              /* FreeBSD: com MAP_FIXED, falha se ocupado */
#endif

typedef struct notify_request {
    char useless1[45];
    char message[3075];
} notify_request_t;

int sceKernelSendNotificationRequest(int, notify_request_t *, size_t, int);

static void say(const char *fmt, ...)
{
    notify_request_t req;
    va_list ap;

    bzero(&req, sizeof(req));
    va_start(ap, fmt);
    vsnprintf(req.message, sizeof(req.message), fmt, ap);
    va_end(ap);

    printf("[mmap_probe] %s\n", req.message);
    fflush(stdout);
    sceKernelSendNotificationRequest(0, &req, sizeof(req), 0);
}

/* Reserva [addr, addr+size) exatamente, sem sobrescrever nada que ja
 * esteja mapeado. MAP_FIXED sozinho no FreeBSD substitui mapeamentos
 * existentes em silencio -- daria "OK" mesmo com a faixa ocupada (e
 * poderia derrubar o proprio payload). Por isso MAP_FIXED|MAP_EXCL. */
static void *reserve(const char *name, uint64_t addr, uint64_t size)
{
    void *want = (void *)(uintptr_t)addr;
    void *got = mmap(want, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANON | MAP_FIXED | MAP_EXCL, -1, 0);

    if (got == MAP_FAILED && errno == EINVAL) {
        /* Kernel sem MAP_EXCL: cai para hint sem MAP_FIXED, que nunca
         * sobrescreve e so devolve `want` se a faixa estiver livre. */
        got = mmap(want, size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANON, -1, 0);
    }
    if (got == MAP_FAILED) {
        say("%s: FALHOU mmap errno=%d size=0x%lx", name, errno,
            (unsigned long)size);
        return NULL;
    }
    if (got != want) {
        say("%s: endereco DIFERENTE (quis 0x%lx, veio %p)", name,
            (unsigned long)addr, got);
        munmap(got, size);
        return NULL;
    }

    /* Prova que a memoria eh utilizavel: escreve e le nas pontas. */
    volatile uint32_t *p = (volatile uint32_t *)got;
    p[0] = 0xC0FFEE42u;
    p[(size / 4) - 1] = 0x1337BEEFu;
    if (p[0] != 0xC0FFEE42u || p[(size / 4) - 1] != 0x1337BEEFu) {
        say("%s: mapeou mas leitura/escrita nao confere", name);
        munmap(got, size);
        return NULL;
    }

    say("%s: OK em 0x%lx", name, (unsigned long)addr);
    return got;
}

/* Copia `mov eax, 42; ret` para a pagina, troca para R+X e executa.
 * Se o kernel negar PROT_EXEC em memoria anonima, o loader precisa de
 * outro caminho (ex.: shm JIT do sistema ou mapear via arquivo). */
static int try_exec(void *page)
{
    static const uint8_t code[] = { 0xB8, 0x2A, 0x00, 0x00, 0x00, 0xC3 };

    memcpy(page, code, sizeof(code));
    if (mprotect(page, PAGE, PROT_READ | PROT_EXEC) != 0) {
        say("EXEC: mprotect(R+X) FALHOU errno=%d", errno);
        return -1;
    }

    /* Se travar aqui, a ultima mensagem no log sera esta. */
    say("EXEC: mprotect OK, chamando codigo em %p...", page);
    int r = ((int (*)(void))page)();
    say("EXEC: retornou %d (esperado 42)", r);
    return r == 42 ? 0 : -1;
}

int main(void)
{
    printf("\n==== PICO-8 PS5 mmap_probe ====\n");
    say("mmap_probe iniciado");

    /* Faixa inteira de uma vez, como o loader real fara antes de copiar
     * os segmentos. Se falhar, testa cada segmento para saber qual
     * pedaco esta ocupado. */
    uint64_t whole_size = SEG_DATA_END - SEG_TEXT_ADDR;
    void *whole = reserve("FAIXA 0x400000-0xb70000", SEG_TEXT_ADDR, whole_size);
    int r_map = whole ? 0 : -1;
    int r_exec = -1;

    if (whole) {
        r_exec = try_exec(whole);
        munmap(whole, whole_size);
    } else {
        void *t = reserve("TEXT 0x400000", SEG_TEXT_ADDR, SEG_TEXT_SIZE);
        void *d = reserve("DATA 0x794000", SEG_DATA_ADDR,
                          SEG_DATA_END - SEG_DATA_ADDR);
        if (t) munmap(t, SEG_TEXT_SIZE);
        if (d) munmap(d, SEG_DATA_END - SEG_DATA_ADDR);
    }

    if (r_map == 0 && r_exec == 0)
        say("RESULTADO: VIAVEL - enderecos nativos + exec OK");
    else if (r_map == 0)
        say("RESULTADO: PARCIAL - faixa livre, mas sem exec em mem anonima");
    else
        say("RESULTADO: BLOQUEADO - faixa ocupada, precisa plano B (relink)");

    printf("==== fim ====\n");
    fflush(stdout);
    return 0;
}
