/* shims_libc.c - imports glibc/libm/libdl do pico8_dyn sobre a libc do PS5
 *
 * A maioria das funcoes tem a mesma ABI nos dois lados e vai direto.
 * Aqui ficam so as que divergem: wrappers *_chk do FORTIFY_SOURCE,
 * internals do glibc (__xstat, __ctype_*_loc, __errno_location),
 * structs de layout diferente (stat, dirent), constantes diferentes
 * (flags de open, clockid, CLOCKS_PER_SEC) e a saida padrao, que vai
 * para o log.
 */

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>

#include "loader.h"

/* ------------------------------------------------------------------ */
/* Processo                                                            */

static void sh_exit(int code)
{
    lg("pico8 chamou exit(%d)", code);
    notify("pico8_loader: pico8 saiu com exit(%d)", code);
    exit(code);
}

static void sh_abort(void)
{
    lg("pico8 chamou abort()");
    notify("pico8_loader: pico8 chamou abort()");
    abort();
}

static void sh_assert_fail(const char *expr, const char *file,
                           unsigned line, const char *func)
{
    lg("assert falhou: %s (%s:%u %s)", expr, file, line, func ? func : "?");
    notify("pico8_loader: assert %s", expr);
    abort();
}

static void sh_stack_chk_fail(void)
{
    lg("__stack_chk_fail: canary do pico8 corrompido");
    notify("pico8_loader: stack smashing detectado");
    abort();
}

static int sh_libc_start_main(void)
{
    lg("__libc_start_main chamado: nao deveria acontecer");
    abort();
}

static int sh_system(const char *cmd)
{
    lg("system(\"%s\") ignorado", cmd ? cmd : "(null)");
    return -1;
}

static int *sh_errno_location(void)
{
    return &errno;
}

/* ------------------------------------------------------------------ */
/* Memoria                                                             */

/* O pico8 aloca pelo heap proprio (p8_alloc.c): o do sistema esgota. */
int sceKernelAvailableFlexibleMemorySize(size_t *);
int sceKernelAvailableDirectMemorySize(off_t, off_t, size_t, off_t *, size_t *);
off_t sceKernelGetDirectMemorySize(void);

/* ------------------------------------------------------------------ */
/* dlopen: so a libcurl existe, emulada sobre sceHttp2 (net_curl.c) */

/* Como no dlfcn real, dlerror() devolve NULL se nao houve erro desde a
 * ultima chamada; o pico8 checa isso depois de cada dlsym. */
static __thread const char *dl_err;

static void *sh_dlopen(const char *name, int flags)
{
    void *h = net_curl_dlopen(name);
    lg("dlopen(\"%s\", %d) -> %p", name ? name : "(null)", flags, h);
    if (!h)
        dl_err = "biblioteca nao disponivel no pico8_loader";
    return h;
}

static void *sh_dlsym(void *h, const char *name)
{
    void *p = net_curl_dlsym(h, name);
    if (!p) {
        lg("dlsym(%s) -> NULL", name);
        dl_err = "simbolo nao encontrado";
    }
    return p;
}

static char *sh_dlerror(void)
{
    const char *e = dl_err;
    dl_err = NULL;
    return (char *)e;
}

/* ------------------------------------------------------------------ */
/* stdio: printf/puts implicitos vao para o log                        */

static int sh_printf_chk(int flag, const char *fmt, ...)
{
    va_list ap;
    (void)flag;
    va_start(ap, fmt);
    int r = vfprintf(p8_log, fmt, ap);
    va_end(ap);
    return r;
}

static int sh_fprintf_chk(FILE *fp, int flag, const char *fmt, ...)
{
    va_list ap;
    (void)flag;
    va_start(ap, fmt);
    int r = vfprintf(fp, fmt, ap);
    va_end(ap);
    return r;
}

static int sh_sprintf_chk(char *s, int flag, size_t slen, const char *fmt, ...)
{
    va_list ap;
    (void)flag;
    va_start(ap, fmt);
    int r = (slen == (size_t)-1) ? vsprintf(s, fmt, ap)
                                 : vsnprintf(s, slen, fmt, ap);
    va_end(ap);
    return r;
}

static int sh_sprintf(char *s, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vsprintf(s, fmt, ap);
    va_end(ap);
    return r;
}

static int sh_sscanf(const char *s, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vsscanf(s, fmt, ap);
    va_end(ap);
    return r;
}

static int sh_puts(const char *s)
{
    return fprintf(p8_log, "%s\n", s);
}

static int sh_putchar(int c)
{
    return fputc(c, p8_log);
}

static size_t sh_fread_chk(void *ptr, size_t ptrlen, size_t size, size_t n,
                           FILE *fp)
{
    (void)ptrlen;
    return fread(ptr, size, n, fp);
}

static int sh_feof(FILE *fp)        { return feof(fp); }
static int sh_ferror(FILE *fp)      { return ferror(fp); }
static int sh_fgetc(FILE *fp)       { return fgetc(fp); }
static int sh_fputc(int c, FILE *f) { return fputc(c, f); }

/* ------------------------------------------------------------------ */
/* FORTIFY_SOURCE: os *_chk so repassam (o tamanho ja foi checado pelo  */
/* compilador quando conhecido).                                       */

static void *sh_memcpy_chk(void *d, const void *s, size_t n, size_t dl)
{
    (void)dl;
    return memcpy(d, s, n);
}

static void *sh_memset_chk(void *d, int c, size_t n, size_t dl)
{
    (void)dl;
    return memset(d, c, n);
}

static char *sh_strcpy_chk(char *d, const char *s, size_t dl)
{
    (void)dl;
    return strcpy(d, s);
}

static char *sh_stpcpy_chk(char *d, const char *s, size_t dl)
{
    (void)dl;
    return stpcpy(d, s);
}

static char *sh_strcat_chk(char *d, const char *s, size_t dl)
{
    (void)dl;
    return strcat(d, s);
}

static char *sh_strncpy_chk(char *d, const char *s, size_t n, size_t dl)
{
    (void)dl;
    return strncpy(d, s, n);
}

/* setjmp/longjmp: o jmp_buf do glibc (200 bytes) cabe o do PS5 (96).
 * _setjmp precisa ser a funcao real, nunca um wrapper em C. */
static void sh_longjmp_chk(jmp_buf env, int val)
{
    _longjmp(env, val);
}

/* ------------------------------------------------------------------ */
/* Arquivos                                                            */

/* Flags do open no Linux x86-64 vs FreeBSD. */
#define L_O_CREAT     0x40
#define L_O_EXCL      0x80
#define L_O_TRUNC     0x200
#define L_O_APPEND    0x400
#define L_O_NONBLOCK  0x800
#define L_O_DIRECTORY 0x10000
#define L_O_CLOEXEC   0x80000

static int sh_open(const char *path, int lflags, ...)
{
    int flags = lflags & 3;   /* O_RDONLY/O_WRONLY/O_RDWR iguais */
    mode_t mode = 0;

    if (lflags & L_O_CREAT)     flags |= O_CREAT;
    if (lflags & L_O_EXCL)      flags |= O_EXCL;
    if (lflags & L_O_TRUNC)     flags |= O_TRUNC;
    if (lflags & L_O_APPEND)    flags |= O_APPEND;
    if (lflags & L_O_NONBLOCK)  flags |= O_NONBLOCK;
    if (lflags & L_O_DIRECTORY) flags |= O_DIRECTORY;
    if (lflags & L_O_CLOEXEC)   flags |= O_CLOEXEC;

    if (lflags & L_O_CREAT) {
        va_list ap;
        va_start(ap, lflags);
        mode = (mode_t)va_arg(ap, int);
        va_end(ap);
    }
    return open(path, flags, mode);
}

/* struct stat do glibc x86-64 (144 bytes). */
struct linux_stat {
    uint64_t st_dev;
    uint64_t st_ino;
    uint64_t st_nlink;
    uint32_t st_mode;
    uint32_t st_uid;
    uint32_t st_gid;
    uint32_t pad0;
    uint64_t st_rdev;
    int64_t  st_size;
    int64_t  st_blksize;
    int64_t  st_blocks;
    int64_t  st_atime_sec, st_atime_nsec;
    int64_t  st_mtime_sec, st_mtime_nsec;
    int64_t  st_ctime_sec, st_ctime_nsec;
    int64_t  reserved[3];
};
_Static_assert(sizeof(struct linux_stat) == 144, "linux_stat");

static int sh_xstat(int ver, const char *path, struct linux_stat *ls)
{
    struct stat st;
    (void)ver;

    if (stat(path, &st) != 0)
        return -1;

    memset(ls, 0, sizeof(*ls));
    ls->st_dev        = st.st_dev;
    ls->st_ino        = st.st_ino;
    ls->st_nlink      = st.st_nlink;
    ls->st_mode       = st.st_mode;   /* bits S_IF* sao iguais */
    ls->st_uid        = st.st_uid;
    ls->st_gid        = st.st_gid;
    ls->st_rdev       = st.st_rdev;
    ls->st_size       = st.st_size;
    ls->st_blksize    = st.st_blksize;
    ls->st_blocks     = st.st_blocks;
    ls->st_atime_sec  = st.st_atim.tv_sec;
    ls->st_atime_nsec = st.st_atim.tv_nsec;
    ls->st_mtime_sec  = st.st_mtim.tv_sec;
    ls->st_mtime_nsec = st.st_mtim.tv_nsec;
    ls->st_ctime_sec  = st.st_ctim.tv_sec;
    ls->st_ctime_nsec = st.st_ctim.tv_nsec;
    return 0;
}

/* struct dirent do glibc. O DIR* do PS5 eh opaco para o pico8; so a
 * entrada devolvida precisa ser convertida. */
struct linux_dirent {
    uint64_t d_ino;
    int64_t  d_off;
    uint16_t d_reclen;
    uint8_t  d_type;          /* DT_* iguais */
    char     d_name[256];
};

static struct linux_dirent ldent;

static struct linux_dirent *sh_readdir(DIR *d)
{
    struct dirent *e = readdir(d);
    if (!e)
        return NULL;

    memset(&ldent, 0, sizeof(ldent));
    ldent.d_ino = e->d_fileno;
    ldent.d_reclen = sizeof(ldent);
    ldent.d_type = e->d_type;
    memcpy(ldent.d_name, e->d_name, e->d_namlen);
    ldent.d_name[e->d_namlen] = 0;
    return &ldent;
}

static ssize_t sh_readlink(const char *path, char *buf, size_t len)
{
    if (strcmp(path, "/proc/self/exe") == 0) {
        size_t n = strlen(p8_bin);
        if (n > len)
            n = len;
        memcpy(buf, p8_bin, n);
        return (ssize_t)n;
    }
    return readlink(path, buf, len);
}

/* ------------------------------------------------------------------ */
/* Tempo                                                               */

static int sh_clock_gettime(int lclk, struct timespec *ts)
{
    clockid_t clk;

    switch (lclk) {
    case 0:  clk = CLOCK_REALTIME; break;
    case 1:                              /* CLOCK_MONOTONIC */
    case 4:                              /* CLOCK_MONOTONIC_RAW */
    case 7:  clk = CLOCK_MONOTONIC; break;  /* CLOCK_BOOTTIME */
    case 2:  clk = CLOCK_PROCESS_CPUTIME_ID; break;
    case 3:  clk = CLOCK_THREAD_CPUTIME_ID; break;
    default: clk = CLOCK_MONOTONIC; break;
    }
    return clock_gettime(clk, ts);
}

/* glibc: CLOCKS_PER_SEC = 1000000; FreeBSD/PS5: 128. */
static long sh_clock(void)
{
    clock_t c = clock();
    if (c == (clock_t)-1)
        return -1;
    return (long)((int64_t)c * 1000000 / CLOCKS_PER_SEC);
}

/* ------------------------------------------------------------------ */
/* ctype no formato glibc: tabelas indexaveis de -128 a 255.           */

#define G_ISupper  0x0100
#define G_ISlower  0x0200
#define G_ISalpha  0x0400
#define G_ISdigit  0x0800
#define G_ISxdigit 0x1000
#define G_ISspace  0x2000
#define G_ISprint  0x4000
#define G_ISgraph  0x8000
#define G_ISblank  0x0001
#define G_IScntrl  0x0002
#define G_ISpunct  0x0004
#define G_ISalnum  0x0008

static unsigned short ctype_b[384];
static int32_t ctype_lower[384];
static int32_t ctype_upper[384];

static const unsigned short *ctype_b_ptr = ctype_b + 128;
static const int32_t *ctype_lower_ptr = ctype_lower + 128;
static const int32_t *ctype_upper_ptr = ctype_upper + 128;

void shims_libc_init(void)
{
    size_t dmem_free = 0;
    off_t dmem_start = 0;
    sceKernelAvailableDirectMemorySize(0, sceKernelGetDirectMemorySize(), 0,
                                       &dmem_start, &dmem_free);
    size_t flex = 0;
    sceKernelAvailableFlexibleMemorySize(&flex);
    lg("[mem] direta: total %zu, maior bloco livre %zu; flexivel livre %zu",
       (size_t)sceKernelGetDirectMemorySize(), dmem_free, flex);
    p8_alloc_init();

    for (int i = 0; i < 384; i++) {
        int c = i - 128;
        unsigned short m = 0;

        if (c >= 0 && c < 128) {
            if (isupper(c))  m |= G_ISupper;
            if (islower(c))  m |= G_ISlower;
            if (isalpha(c))  m |= G_ISalpha;
            if (isdigit(c))  m |= G_ISdigit;
            if (isxdigit(c)) m |= G_ISxdigit;
            if (isspace(c))  m |= G_ISspace;
            if (isprint(c))  m |= G_ISprint;
            if (isgraph(c))  m |= G_ISgraph;
            if (isblank(c))  m |= G_ISblank;
            if (iscntrl(c))  m |= G_IScntrl;
            if (ispunct(c))  m |= G_ISpunct;
            if (isalnum(c))  m |= G_ISalnum;
        }
        ctype_b[i] = m;
        /* Locale "C": so ASCII muda de caixa. */
        ctype_lower[i] = (c >= 'A' && c <= 'Z') ? c + 32 : c;
        ctype_upper[i] = (c >= 'a' && c <= 'z') ? c - 32 : c;
    }
}

static const unsigned short **sh_ctype_b_loc(void) { return &ctype_b_ptr; }
static const int32_t **sh_ctype_tolower_loc(void)  { return &ctype_lower_ptr; }
static const int32_t **sh_ctype_toupper_loc(void)  { return &ctype_upper_ptr; }

static char *sh_setlocale(int cat, const char *loc)
{
    (void)cat;
    (void)loc;
    return "C";
}

/* ------------------------------------------------------------------ */
/* libm                                                                */

static void sh_sincos(double x, double *s, double *c)
{
    *s = sin(x);
    *c = cos(x);
}

/* ------------------------------------------------------------------ */

#define D(n)      { #n, (void *)n }
#define S(n, f)   { n, (void *)f }

const shim_t shims_libc[] = {
    /* processo */
    S("exit", sh_exit),
    S("abort", sh_abort),
    S("__assert_fail", sh_assert_fail),
    S("__stack_chk_fail", sh_stack_chk_fail),
    S("__libc_start_main", sh_libc_start_main),
    S("__errno_location", sh_errno_location),
    S("system", sh_system),
    D(getenv),

    /* dl */
    S("dlopen", sh_dlopen),
    S("dlsym", sh_dlsym),
    S("dlerror", sh_dlerror),

    /* memoria */
    S("malloc", p8_malloc), S("calloc", p8_calloc),
    S("realloc", p8_realloc), S("free", p8_free),
    D(memchr), D(memcmp), D(memcpy), D(memmove), D(memset),
    S("__memcpy_chk", sh_memcpy_chk),
    S("__memset_chk", sh_memset_chk),

    /* strings */
    D(strlen), D(strcmp), D(strncmp), D(strcasecmp), D(strcoll),
    D(strchr), D(strstr), D(strpbrk), D(strspn), D(strtok),
    D(strcpy), D(strncpy), D(strcat), D(stpcpy), D(strtod), D(strerror),
    S("__strcpy_chk", sh_strcpy_chk),
    S("__stpcpy_chk", sh_stpcpy_chk),
    S("__strcat_chk", sh_strcat_chk),
    S("__strncpy_chk", sh_strncpy_chk),
    D(qsort), D(rand), D(srand),

    /* ctype / locale */
    S("__ctype_b_loc", sh_ctype_b_loc),
    S("__ctype_tolower_loc", sh_ctype_tolower_loc),
    S("__ctype_toupper_loc", sh_ctype_toupper_loc),
    S("setlocale", sh_setlocale),
    D(localeconv),

    /* setjmp */
    D(_setjmp),
    S("__longjmp_chk", sh_longjmp_chk),

    /* stdio */
    D(fopen), S("fopen64", fopen),
    D(freopen), S("freopen64", freopen),
    D(fdopen), D(fclose), D(fflush),
    D(fread), S("__fread_chk", sh_fread_chk), D(fwrite),
    S("fgetc", sh_fgetc), S("_IO_getc", sh_fgetc), D(fgets),
    S("fputc", sh_fputc), D(fputs),
    D(fseek), D(ftell), D(rewind),
    S("fseeko64", fseeko), S("ftello64", ftello),
    S("feof", sh_feof), S("ferror", sh_ferror),
    S("__printf_chk", sh_printf_chk),
    S("__fprintf_chk", sh_fprintf_chk),
    S("__sprintf_chk", sh_sprintf_chk),
    S("sprintf", sh_sprintf),
    S("sscanf", sh_sscanf),
    S("__isoc99_sscanf", sh_sscanf),
    S("puts", sh_puts),
    S("putchar", sh_putchar),

    /* arquivos */
    S("open", sh_open), D(close),
    S("__xstat", sh_xstat), S("__xstat64", sh_xstat),
    D(opendir), S("readdir", sh_readdir), D(closedir),
    D(mkdir), D(chmod), D(remove), D(rename), D(mkstemp), D(utime),
    D(getcwd), S("readlink", sh_readlink),

    /* tempo */
    D(time), D(difftime), D(gmtime), D(localtime), D(mktime), D(strftime),
    S("clock", sh_clock),
    S("clock_gettime", sh_clock_gettime),

    /* libm */
    D(acos), D(asin), D(atan), D(atan2), D(cos), D(cosh), D(sin), D(sinh),
    D(tan), D(tanh), D(exp), D(log), D(log10), D(pow), D(fmod), D(frexp),
    D(ldexp), D(sqrt), D(sqrtf),
    S("sincos", sh_sincos),

    { NULL, NULL }
};
