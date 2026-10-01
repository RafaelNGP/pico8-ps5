/* app_probe - o pico8_loader pode rodar dentro de um app nativo?
 *
 * Para o PICO-8 ser um app autonomo (so kstuff + ShadowMountPlus), o
 * loader tem que rodar no proprio processo do eboot.bin. Este probe
 * responde, no console, o que isso exige:
 *   1. elevacao de sandbox pelo elfldr local (mecanismo do ProsperoEden);
 *   2. leitura de /data/pico8/pico8_dyn;
 *   3. faixa fixa 0x400000/0x794000 livre no processo do app;
 *   4. TEXT RW -> RX e execucao de codigo ali; DATA continua gravavel.
 * Cada passo vai para /data/pico8/app_probe.log (se /data abrir) e o
 * resumo vira notificacao.
 */

#include "elevation.hpp"

#include <cerrno>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef MAP_EXCL
#define MAP_EXCL 0x00004000
#endif

extern "C"
{
    int sceKernelSendNotificationRequest(std::uint32_t, void *, std::size_t, int);
    int sceKernelUsleep(std::uint32_t);

    struct SceKernelVirtualQueryInfo
    {
        void *start;
        void *end;
        std::int64_t offset;
        std::int32_t protection;
        std::int32_t memoryType;
        std::uint8_t flags; /* flexible, direct, stack, pooled, committed */
        char name[32];
    };
    int sceKernelVirtualQuery(const void *, int, SceKernelVirtualQueryInfo *, std::size_t);
}

namespace
{
struct NotificationRequest
{
    std::uint8_t reserved[45];
    char message[3075];
};

int log_fd = -1;
char summary[512];

void notify(const char *msg) noexcept
{
    NotificationRequest n{};
    std::snprintf(n.message, sizeof(n.message), "%s", msg);
    sceKernelSendNotificationRequest(0, &n, sizeof(n), 0);
}

/* Grava e sincroniza a cada linha: se o probe morrer, o log mostra onde. */
void lg(const char *fmt, ...) noexcept
{
    char line[512];
    va_list ap;
    va_start(ap, fmt);
    int n = std::vsnprintf(line, sizeof(line) - 1, fmt, ap);
    va_end(ap);
    if (n < 0)
        return;
    if (n > (int)sizeof(line) - 2)
        n = sizeof(line) - 2;
    line[n++] = '\n';
    if (log_fd >= 0)
    {
        (void)write(log_fd, line, n);
        (void)fsync(log_fd);
    }
}

void add(const char *item) noexcept
{
    std::strncat(summary, item, sizeof(summary) - std::strlen(summary) - 1);
}

/* Lista as regioes mapeadas abaixo de 4 GiB sem alterar nada. Devolve
 * true se alguma cruza [lo, hi). */
bool dump_and_check(std::uint64_t lo, std::uint64_t hi) noexcept
{
    bool busy = false;
    const char *addr = nullptr;
    for (int i = 0; i < 256; i++)
    {
        SceKernelVirtualQueryInfo vq{};
        int r = sceKernelVirtualQuery(addr, 1 /* FIND_NEXT */, &vq, sizeof(vq));
        if (r != 0)
        {
            lg("vq fim r=0x%x", r);
            break;
        }
        auto s = reinterpret_cast<std::uint64_t>(vq.start);
        auto e = reinterpret_cast<std::uint64_t>(vq.end);
        if (s >= 0x100000000ull)
            break;
        vq.name[31] = 0;
        lg("vq 0x%09lx-0x%09lx prot=%x type=%d fl=%x '%s'", (unsigned long)s, (unsigned long)e,
           vq.protection, vq.memoryType, vq.flags, vq.name);
        if (s < hi && e > lo)
            busy = true;
        if (e <= s)
            break;
        addr = reinterpret_cast<const char *>(e);
    }
    return busy;
}

void *map_fixed(std::uint64_t lo, std::uint64_t hi) noexcept
{
    void *want = reinterpret_cast<void *>(lo);
    void *got = mmap(want, hi - lo, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANON | MAP_FIXED | MAP_EXCL, -1, 0);
    lg("mmap 0x%lx-0x%lx -> %p errno=%d", (unsigned long)lo, (unsigned long)hi, got,
       got == MAP_FAILED ? errno : 0);
    return got == want ? got : nullptr;
}
} // namespace

int main()
{
    std::strcpy(summary, "PICO-8 probe:");

    const auto st = elevation::request(elevation::Capability::filesystem);
    char buf[64];
    std::snprintf(buf, sizeof(buf), " elev=%u", static_cast<unsigned>(st));
    add(buf);

    log_fd = open("/data/pico8/app_probe.log", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    add(log_fd >= 0 ? " log=ok" : " log=FALHA");
    lg("==== app_probe pid=%d ====", getpid());
    lg("elevation status=%u (0=ok)", static_cast<unsigned>(st));

    int fd = open("/data/pico8/pico8_dyn", O_RDONLY);
    unsigned char hdr[4] = {};
    bool can_read = fd >= 0 && read(fd, hdr, 4) == 4 && std::memcmp(hdr, "\x7f" "ELF", 4) == 0;
    lg("ler pico8_dyn: fd=%d elf=%d errno=%d", fd, can_read, can_read ? 0 : errno);
    if (fd >= 0)
        close(fd);
    add(can_read ? " ler=ok" : " ler=FALHA");

    lg("main=%p pilha=%p", reinterpret_cast<void *>(&main), static_cast<void *>(buf));
    const bool busy = dump_and_check(0x400000, 0xb70000);
    lg("faixa 0x400000-0xb70000 ocupada: %d", busy);

    /* Sem MAP_FIXED: so mostra onde o kernel poria um hint 0x400000. */
    void *hint = mmap(reinterpret_cast<void *>(0x400000), 0x1000, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANON, -1, 0);
    lg("mmap hint 0x400000 -> %p", hint);
    if (hint != MAP_FAILED)
        munmap(hint, 0x1000);

    void *text = nullptr;
    void *data = nullptr;
    if (busy)
    {
        add(" faixa=OCUPADA");
    }
    else
    {
        text = map_fixed(0x400000, 0x598000);
        data = map_fixed(0x794000, 0xb70000);
        add(text && data ? " faixa=ok" : " faixa=FALHA");
    }

    if (text && data)
    {
        static const unsigned char code[] = {0xB8, 0x2A, 0x00, 0x00, 0x00, 0xC3};
        std::memcpy(text, code, sizeof(code));
        int r = mprotect(text, 0x598000 - 0x400000, PROT_READ | PROT_EXEC);
        lg("mprotect TEXT R+X = %d errno=%d", r, r ? errno : 0);
        if (r == 0)
        {
            lg("chamando codigo em 0x400000...");
            notify("PICO-8 probe: testando execucao (se travar, eh aqui)");
            int v = reinterpret_cast<int (*)()>(text)();
            lg("retornou %d (esperado 42)", v);
            add(v == 42 ? " exec=ok" : " exec=FALHA");
        }
        else
        {
            add(" exec=FALHA(mprotect)");
        }
        /* Mapeamentos separados: o DATA deve continuar gravavel. */
        volatile std::uint32_t *d = static_cast<volatile std::uint32_t *>(data);
        d[0] = 0xC0FFEE42u;
        lg("DATA gravavel apos mprotect do TEXT: %d", d[0] == 0xC0FFEE42u);
    }

    lg("resumo: %s", summary);
    notify(summary);
    if (log_fd >= 0)
        close(log_fd);

    /* Fica vivo ate o usuario fechar pelo botao PS. */
    for (;;)
        sceKernelUsleep(1000 * 1000);
}
