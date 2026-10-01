/* pico8_loader - carrega o pico8_dyn (Linux x86-64, nao-PIE) no PS5
 *
 * Etapa 1: mapeia os segmentos nos enderecos nativos, resolve os 217
 * imports (libc via shims, SDL via stubs que so registram) e chama o
 * main do PICO-8. Tudo vai para /data/pico8/loader.log, que inclui o
 * stdout/stderr do proprio PICO-8.
 */

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ucontext.h>

#include "loader.h"

#ifndef MAP_EXCL
#define MAP_EXCL 0x00004000
#endif

#define PICO8_STACK_SIZE (32u * 1024 * 1024)

FILE *p8_log;

/* ------------------------------------------------------------------ */
/* Log e notificacao                                                   */

typedef struct notify_request {
    char useless1[45];
    char message[3075];
} notify_request_t;

int sceKernelSendNotificationRequest(int, notify_request_t *, size_t, int);

void lg(const char *fmt, ...)
{
    va_list ap;
    FILE *f = p8_log ? p8_log : stdout;

    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fputc('\n', f);
    fflush(f);
}

void notify(const char *fmt, ...)
{
    notify_request_t req;
    va_list ap;

    bzero(&req, sizeof(req));
    va_start(ap, fmt);
    vsnprintf(req.message, sizeof(req.message), fmt, ap);
    va_end(ap);

    lg("[notify] %s", req.message);
    sceKernelSendNotificationRequest(0, &req, sizeof(req), 0);
}

/* ------------------------------------------------------------------ */
/* Crash handler: registra onde o PICO-8 morreu.                       */

static uint8_t fault_stack[64 * 1024];

/* Rastreio dos imports de libc: cada GOT aponta para um thunk que grava
 * o indice num buffer circular e salta para o shim. No crash, as
 * ultimas chamadas mostram em que fase o PICO-8 estava. Sem trava:
 * outra thread pode embaralhar uma entrada, o que basta para depurar. */
#define TRACE_RING 256  /* potencia de 2: o asm usa TRACE_RING - 1 */
#define MAX_TRACED 512

uint32_t p8_trace_ring[TRACE_RING];
uint32_t p8_trace_pos;
void *p8_trace_target[MAX_TRACED];
static const char *trace_names[MAX_TRACED];
static int n_traced;

/* r11 = indice (posto pelo thunk). r10/r11 sao scratch na ABI e rax eh
 * salvo, pois em funcoes variadicas %al leva o numero de args SSE. */
__asm__(
    ".text\n"
    ".p2align 4\n"
    "p8_trace_common:\n"
    "    push %rax\n"
    "    lea p8_trace_ring(%rip), %rax\n"
    "    mov p8_trace_pos(%rip), %r10d\n"
    "    incl p8_trace_pos(%rip)\n"
    "    and $255, %r10d\n"
    "    mov %r11d, (%rax,%r10,4)\n"
    "    lea p8_trace_target(%rip), %rax\n"
    "    mov (%rax,%r11,8), %r10\n"
    "    pop %rax\n"
    "    jmp *%r10\n");
void p8_trace_common(void);

static void dump_trace(void)
{
    uint32_t end = p8_trace_pos;
    uint32_t n = end < TRACE_RING ? end : TRACE_RING;

    lg("    ultimas %u chamadas de import (de %u), mais recente por ultimo:", n, end);
    for (uint32_t i = end - n; i != end; i++) {
        uint32_t idx = p8_trace_ring[i % TRACE_RING];
        lg("      %s", idx < (uint32_t)n_traced ? trace_names[idx] : "?");
    }
}

static pthread_t pico8_tid;

/* CS e SS de modo usuario: seletores de 16 bits, nao nulos, RPL 3. */
static int user_selectors(const mcontext_t *mc)
{
    return mc->mc_cs && mc->mc_cs <= 0xffff && (mc->mc_cs & 3) == 3 &&
           mc->mc_ss && mc->mc_ss <= 0xffff && (mc->mc_ss & 3) == 3;
}

static void on_fault(int sig, siginfo_t *si, void *uc_)
{
    /* O SDK poe uc_mcontext em +16 (FreeBSD), mas no app o PS5 entrega
     * 48 bytes a mais antes dele. No layout errado, CS/SS caem em cima de
     * outros campos (visto: 0xffffffff e 0). */
    mcontext_t *mc = &((ucontext_t *)uc_)->uc_mcontext;
    mcontext_t *alt = (mcontext_t *)((char *)uc_ + 64);
    if (!user_selectors(mc) && user_selectors(alt))
        mc = alt;

    lg("*** thread %s, si_code=%d si_errno=%d si_addr=%p",
       pthread_equal(pthread_self(), pico8_tid) ? "do pico8" : "OUTRA",
       si->si_code, si->si_errno, si->si_addr);
    lg("    trapno=0x%lx err=0x%lx addr=0x%lx rflags=0x%lx cs=0x%lx",
       (unsigned long)mc->mc_trapno, (unsigned long)mc->mc_err,
       (unsigned long)mc->mc_addr, (unsigned long)mc->mc_rflags,
       (unsigned long)mc->mc_cs);
    lg("    r8=0x%lx r9=0x%lx r10=0x%lx r11=0x%lx",
       (unsigned long)mc->mc_r8, (unsigned long)mc->mc_r9,
       (unsigned long)mc->mc_r10, (unsigned long)mc->mc_r11);
    lg("    r12=0x%lx r13=0x%lx r14=0x%lx r15=0x%lx",
       (unsigned long)mc->mc_r12, (unsigned long)mc->mc_r13,
       (unsigned long)mc->mc_r14, (unsigned long)mc->mc_r15);

    lg("*** CRASH sinal %d addr=%p rip=0x%lx rsp=0x%lx", sig, si->si_addr,
       (unsigned long)mc->mc_rip, (unsigned long)mc->mc_rsp);
    lg("    rax=0x%lx rbx=0x%lx rcx=0x%lx rdx=0x%lx",
       (unsigned long)mc->mc_rax, (unsigned long)mc->mc_rbx,
       (unsigned long)mc->mc_rcx, (unsigned long)mc->mc_rdx);
    lg("    rdi=0x%lx rsi=0x%lx rbp=0x%lx",
       (unsigned long)mc->mc_rdi, (unsigned long)mc->mc_rsi,
       (unsigned long)mc->mc_rbp);

    /* Os primeiros enderecos de retorno na pilha ajudam a achar quem
     * chamou (o pico8_dyn nao usa frame pointer em tudo). */
    /* Enderecos de retorno: pico8 (fixo) ou loader (o log mostra a base
     * do loader para converter em offset do ELF). */
    dump_trace();
    uint64_t *sp = (uint64_t *)mc->mc_rsp;
    lg("    loader: on_fault=%p", (void *)on_fault);
    for (int i = 0; (uint64_t)(uintptr_t)sp >= 0x10000 && i < 24; i++)
        lg("    pilha[%2d] = 0x%lx%s", i, (unsigned long)sp[i],
           sp[i] >= P8_TEXT_LO && sp[i] < P8_TEXT_HI ? " (pico8 text)" : "");
    notify("pico8_loader: CRASH sinal %d em rip=0x%lx", sig,
           (unsigned long)mc->mc_rip);
    _exit(128 + sig);
}

static void install_fault_handler(void)
{
    stack_t ss = { .ss_sp = fault_stack, .ss_size = sizeof(fault_stack) };
    struct sigaction sa;

    sigaltstack(&ss, NULL);
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = on_fault;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
}

/* ------------------------------------------------------------------ */
/* Imports nao implementados: thunk que registra o nome e devolve 0.   */

#define THUNK_SIZE 32
#define MAX_THUNKS ((P8_THUNK_HI - P8_THUNK_LO) / THUNK_SIZE)

static const char *thunk_names[MAX_THUNKS];
static unsigned thunk_calls[MAX_THUNKS];
static int n_thunks;

static uint64_t unimpl_called(uint64_t idx)
{
    unsigned n = ++thunk_calls[idx];
    /* Funcoes chamadas todo frame inundariam o log. */
    if (n <= 3 || (n & (n - 1)) == 0)
        lg("[unimpl] %s (chamada %u)", thunk_names[idx], n);
    return 0;
}

static void *make_thunk(const char *name)
{
    if (n_thunks == MAX_THUNKS)
        return NULL;

    int idx = n_thunks++;
    uint8_t *t = (uint8_t *)(uintptr_t)(P8_THUNK_LO + (uint64_t)idx * THUNK_SIZE);
    uint64_t target = (uint64_t)(uintptr_t)unimpl_called;

    thunk_names[idx] = name;
    /* mov rdi, idx ; mov rax, unimpl_called ; jmp rax */
    t[0] = 0x48; t[1] = 0xBF;
    memcpy(t + 2, &(uint64_t){ (uint64_t)idx }, 8);
    t[10] = 0x48; t[11] = 0xB8;
    memcpy(t + 12, &target, 8);
    t[20] = 0xFF; t[21] = 0xE0;
    return t;
}

static void *make_trace_thunk(const char *name, void *target)
{
    if (n_thunks == MAX_THUNKS || n_traced == MAX_TRACED)
        return target;

    int idx = n_traced++;
    uint8_t *t = (uint8_t *)(uintptr_t)(P8_THUNK_LO + (uint64_t)n_thunks++ * THUNK_SIZE);
    uint64_t common = (uint64_t)(uintptr_t)p8_trace_common;

    trace_names[idx] = name;
    p8_trace_target[idx] = target;
    /* mov r11d, idx ; mov r10, p8_trace_common ; jmp r10 */
    t[0] = 0x41; t[1] = 0xBB;
    memcpy(t + 2, &(uint32_t){ (uint32_t)idx }, 4);
    t[6] = 0x49; t[7] = 0xBA;
    memcpy(t + 8, &common, 8);
    t[16] = 0x41; t[17] = 0xFF; t[18] = 0xE2;
    return t;
}

/* ------------------------------------------------------------------ */
/* Carga do ELF                                                        */

/* Devolve a entrada da tabela (ou NULL). A entrada pode existir com
 * addr NULL: os stubs da libc do SDK sao weak, entao uma funcao que
 * nao existe no PS5 resolve para 0 em runtime. */
static const shim_t *lookup(const shim_t *tab, const char *name)
{
    for (; tab->name; tab++)
        if (strcmp(tab->name, name) == 0)
            return tab;
    return NULL;
}

static int load_segments(int fd, Elf64_Ehdr *eh)
{
    Elf64_Phdr ph[16];

    if (eh->e_phnum > 16 ||
        pread(fd, ph, eh->e_phnum * sizeof(Elf64_Phdr), eh->e_phoff) !=
            (ssize_t)(eh->e_phnum * sizeof(Elf64_Phdr))) {
        lg("phdrs invalidos");
        return -1;
    }

    for (int i = 0; i < eh->e_phnum; i++) {
        if (ph[i].p_type == PT_TLS || ph[i].p_type == PT_INTERP) {
            if (ph[i].p_type == PT_TLS) {
                lg("pico8_dyn tem PT_TLS: nao suportado");
                return -1;
            }
            continue;
        }
        if (ph[i].p_type != PT_LOAD)
            continue;
        uint64_t lo = ph[i].p_vaddr, hi = lo + ph[i].p_memsz;
        if (!((lo >= P8_TEXT_LO && hi <= P8_TEXT_HI) ||
              (lo >= P8_DATA_LO && hi <= P8_DATA_HI))) {
            lg("segmento fora da faixa: 0x%lx+0x%lx",
               (unsigned long)ph[i].p_vaddr, (unsigned long)ph[i].p_memsz);
            return -1;
        }
        void *dst = (void *)(uintptr_t)ph[i].p_vaddr;
        if (pread(fd, dst, ph[i].p_filesz, ph[i].p_offset) !=
            (ssize_t)ph[i].p_filesz) {
            lg("pread do segmento %d falhou errno=%d", i, errno);
            return -1;
        }
        lg("LOAD 0x%lx filesz=0x%lx memsz=0x%lx flags=%x",
           (unsigned long)ph[i].p_vaddr, (unsigned long)ph[i].p_filesz,
           (unsigned long)ph[i].p_memsz, ph[i].p_flags);
    }
    return 0;
}

static int relocate(Elf64_Ehdr *eh)
{
    Elf64_Phdr *ph = (Elf64_Phdr *)(uintptr_t)(P8_TEXT_LO + eh->e_phoff);
    Elf64_Dyn *dyn = NULL;

    for (int i = 0; i < eh->e_phnum; i++)
        if (ph[i].p_type == PT_DYNAMIC)
            dyn = (Elf64_Dyn *)(uintptr_t)ph[i].p_vaddr;
    if (!dyn) {
        lg("sem PT_DYNAMIC");
        return -1;
    }

    Elf64_Sym *symtab = NULL;
    const char *strtab = NULL;
    Elf64_Rela *rel[2] = { NULL, NULL };
    size_t relsz[2] = { 0, 0 };

    for (; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
        case DT_SYMTAB:   symtab = (void *)(uintptr_t)dyn->d_un.d_ptr; break;
        case DT_STRTAB:   strtab = (void *)(uintptr_t)dyn->d_un.d_ptr; break;
        case DT_RELA:     rel[0] = (void *)(uintptr_t)dyn->d_un.d_ptr; break;
        case DT_RELASZ:   relsz[0] = dyn->d_un.d_val; break;
        case DT_JMPREL:   rel[1] = (void *)(uintptr_t)dyn->d_un.d_ptr; break;
        case DT_PLTRELSZ: relsz[1] = dyn->d_un.d_val; break;
        }
    }

    int n_libc = 0, n_sdl = 0, n_unimpl = 0;

    for (int t = 0; t < 2; t++) {
        for (size_t k = 0; k < relsz[t] / sizeof(Elf64_Rela); k++) {
            Elf64_Rela *r = &rel[t][k];
            uint32_t type = ELF64_R_TYPE(r->r_info);
            const char *name = strtab + symtab[ELF64_R_SYM(r->r_info)].st_name;
            uint64_t *where = (uint64_t *)(uintptr_t)r->r_offset;

            if (type == R_X86_64_COPY) {
                /* stdin/stdout/stderr: o pico8 le o FILE* direto da
                 * variavel. stdout/stderr vao para o log. */
                if (strcmp(name, "stdin") == 0)
                    *where = (uint64_t)(uintptr_t)stdin;
                else if (strcmp(name, "stdout") == 0 ||
                         strcmp(name, "stderr") == 0)
                    *where = (uint64_t)(uintptr_t)p8_log;
                else {
                    lg("R_X86_64_COPY desconhecido: %s", name);
                    return -1;
                }
                continue;
            }
            if (type != R_X86_64_JMP_SLOT && type != R_X86_64_GLOB_DAT) {
                lg("relocacao tipo %u nao suportada (%s)", type, name);
                return -1;
            }
            if (strcmp(name, "__gmon_start__") == 0) {
                *where = 0;   /* weak, sem profiler */
                continue;
            }

            const shim_t *sh = lookup(shims_libc, name);
            void *addr = NULL;
            if (sh) {
                n_libc++;
            } else if ((sh = lookup(shims_sdl, name))) {
                n_sdl++;
            }
            if (sh && sh->addr) {
                /* GLOB_DAT pode ser ponteiro de dado: so as chamadas via
                 * PLT (libc e SDL) passam pelo rastreio. */
                addr = type == R_X86_64_JMP_SLOT
                           ? make_trace_thunk(name, sh->addr) : sh->addr;
            } else {
                if (sh)
                    lg("AVISO: %s nao existe na libc do PS5", name);
                addr = make_thunk(name);
                if (!addr) {
                    lg("thunks esgotados em %s", name);
                    return -1;
                }
                n_unimpl++;
                if (!sh)
                    lg("import sem shim: %s", name);
            }
            *where = (uint64_t)(uintptr_t)addr + r->r_addend;
        }
    }

    lg("imports: %d libc, %d sdl, %d sem implementacao",
       n_libc, n_sdl, n_unimpl);
    return 0;
}

/* Testa escrita sem arriscar SIGSEGV: read() de um pipe devolve
 * EFAULT se a pagina nao aceitar escrita. Reescreve o mesmo byte. */
static int check_writable(uint64_t addr)
{
    int p[2];
    uint8_t *b = (uint8_t *)(uintptr_t)addr;
    int ok;

    if (pipe(p) != 0)
        return -1;
    write(p[1], b, 1);
    ok = read(p[0], b, 1) == 1;
    if (!ok)
        lg("endereco 0x%lx nao aceita escrita (errno=%d)",
           (unsigned long)addr, errno);
    close(p[0]);
    close(p[1]);
    return ok ? 0 : -1;
}

/* Extrai main/init do _start padrao do glibc:
 *   mov $fini,%r8 ; mov $init,%rcx ; mov $main,%rdi ; call *libc_start_main */
static int parse_start(uint64_t entry, uint64_t *main_addr, uint64_t *init_addr)
{
    const uint8_t *p = (const uint8_t *)(uintptr_t)entry;

    for (int i = 0; i < 32; i++) {
        if (p[i] == 0x48 && p[i + 1] == 0xC7 && p[i + 2] == 0xC1)
            *init_addr = *(const uint32_t *)(p + i + 3);
        if (p[i] == 0x48 && p[i + 1] == 0xC7 && p[i + 2] == 0xC7)
            *main_addr = *(const uint32_t *)(p + i + 3);
    }
    return (*main_addr && *init_addr) ? 0 : -1;
}

/* ------------------------------------------------------------------ */

typedef int (*main_fn)(int, char **, char **);
typedef void (*init_fn)(int, char **, char **);

static uint64_t g_main, g_init;
/* -splore: abre direto no navegador de carts (BBS) em vez do console. */
static char *g_argv[] = { p8_bin, "-splore", NULL };

extern char **environ;

#ifdef P8_APP
int p8_app_elevate(void);          /* pico8_app/src/app_glue.cpp */
void p8_app_check_imports(void);   /* pico8_app/src/app_check.c */
#endif

static void *pico8_thread(void *arg)
{
    (void)arg;
    pico8_tid = pthread_self();
    install_fault_handler();

    lg("chamando init (0x%lx)...", (unsigned long)g_init);
    ((init_fn)(uintptr_t)g_init)(2, g_argv, environ);

    lg("chamando main (0x%lx)...", (unsigned long)g_main);
    int r = ((main_fn)(uintptr_t)g_main)(2, g_argv, environ);

    lg("main do pico8 retornou %d", r);
    notify("pico8_loader: main retornou %d", r);
    return NULL;
}

int main(void)
{
    /* Reserva as faixas antes de qualquer malloc grande, para nada do
     * proprio loader cair dentro delas. */
    static const struct { uint64_t lo, hi; } regions[] = {
        { P8_TEXT_LO,  P8_TEXT_HI },
        { P8_THUNK_LO, P8_THUNK_HI },
        { P8_DATA_LO,  P8_DATA_HI },
    };
    int map_errno = 0;
    uint64_t map_fail = 0;
    for (size_t i = 0; i < sizeof(regions) / sizeof(regions[0]); i++) {
        void *want = (void *)(uintptr_t)regions[i].lo;
        void *got = mmap(want, regions[i].hi - regions[i].lo,
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANON | MAP_FIXED | MAP_EXCL, -1, 0);
        if (got != want && !map_fail) {
            map_errno = errno;
            map_fail = regions[i].lo;
        }
    }

#ifdef P8_APP
    /* Como eboot.bin, o sandbox esconde /data ate a elevacao. Vem depois
     * das faixas fixas porque o helper aloca memoria. */
    int elev = p8_app_elevate();
#endif

    mkdir(P8_DIR, 0777);
    p8_log = fopen(P8_LOG, "w");
    /* Sem buffer: o stdout/stderr do pico8 tambem vem para ca, e uma
     * linha sem '\n' se perderia num crash. */
    if (p8_log)
        setvbuf(p8_log, NULL, _IONBF, 0);

    lg("==== pico8_loader ====");
#ifdef P8_APP
    lg("app nativo, elevacao status=%d (0=ok)", elev);
    p8_app_check_imports();
#endif
    notify("pico8_loader iniciado");

    if (map_fail) {
        notify("pico8_loader: faixa 0x%lx indisponivel (errno=%d)",
               (unsigned long)map_fail, map_errno);
        return 1;
    }

    if (p8_find_files() != 0)
        return 1;
    int fd = open(p8_bin, O_RDONLY);
    if (fd < 0) {
        notify("PICO-8: nao consegui abrir %s (errno=%d)", p8_bin, errno);
        return 1;
    }

    Elf64_Ehdr eh;
    if (pread(fd, &eh, sizeof(eh), 0) != sizeof(eh) ||
        memcmp(eh.e_ident, ELFMAG, SELFMAG) != 0 ||
        eh.e_ident[EI_CLASS] != ELFCLASS64 || eh.e_machine != EM_X86_64 ||
        eh.e_type != ET_EXEC) {
        notify("PICO-8: %s nao eh o pico8_dyn da versao Linux 64-bit", p8_bin);
        return 1;
    }

    if (load_segments(fd, &eh) != 0) {
        notify("pico8_loader: falha ao carregar segmentos (ver log)");
        return 1;
    }
    close(fd);

    shims_libc_init();
    shims_sdl_init();
    if (relocate(&eh) != 0) {
        notify("pico8_loader: falha nas relocacoes (ver log)");
        return 1;
    }

    if (parse_start(eh.e_entry, &g_main, &g_init) != 0) {
        notify("pico8_loader: _start com formato inesperado");
        return 1;
    }

    if (mprotect((void *)(uintptr_t)P8_TEXT_LO, P8_TEXT_HI - P8_TEXT_LO,
                 PROT_READ | PROT_EXEC) != 0 ||
        mprotect((void *)(uintptr_t)P8_THUNK_LO, P8_THUNK_HI - P8_THUNK_LO,
                 PROT_READ | PROT_EXEC) != 0) {
        notify("pico8_loader: mprotect R+X falhou errno=%d", errno);
        return 1;
    }
    if (check_writable(P8_DATA_LO) != 0 ||
        check_writable(P8_DATA_HI - 1) != 0) {
        notify("pico8_loader: DATA perdeu escrita apos mprotect");
        return 1;
    }
    lg("TEXT/thunks R+X, DATA RW verificado");

    chdir(P8_DIR);
    setenv("HOME", P8_DIR, 1);

    pthread_attr_t attr;
    pthread_t th;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PICO8_STACK_SIZE);
    if (pthread_create(&th, &attr, pico8_thread, NULL) != 0) {
        notify("pico8_loader: pthread_create falhou");
        return 1;
    }
    pthread_join(th, NULL);

    lg("==== fim ====");
    return 0;
}
