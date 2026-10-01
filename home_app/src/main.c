/* PICO-8 home app - atalho na home do PS5 para o pico8_loader
 *
 * O PICO-8 precisa rodar como app em primeiro plano, e o pico8_loader eh
 * um ELF do ps5-payload-sdk, nao um app. Este app minimo so pede ao
 * websrv local (hbldr) que rode o loader: o websrv encerra este app e
 * abre o loader num app fake em primeiro plano, como o `make run`.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define WEBSRV_PORT 8080
#define LOADER_PATH "/data/homebrew/PICO8/eboot.elf"
#define LOADER_CWD  "/data/pico8"

int sceNetSocket(const char *name, int domain, int type, int protocol);
int sceNetConnect(int s, const void *addr, uint32_t len);
int sceNetSend(int s, const void *buf, size_t len, int flags);
int sceNetSocketClose(int s);
int sceKernelUsleep(unsigned int usec);

typedef struct {
    char useless1[45];
    char message[3075];
} notify_request_t;

int sceKernelSendNotificationRequest(int, notify_request_t *, size_t, int);

/* sockaddr_in do SceNet (BSD com sin_len). */
typedef struct {
    uint8_t len;
    uint8_t family;
    uint16_t port;
    uint32_t addr;
    uint8_t zero[8];
} net_sockaddr_in_t;

static void notify(const char *msg)
{
    notify_request_t req;

    memset(&req, 0, sizeof(req));
    strncpy(req.message, msg, sizeof(req.message) - 1);
    sceKernelSendNotificationRequest(0, &req, sizeof(req), 0);
}

int main(void)
{
    static const char request[] =
        "GET /hbldr?path=" LOADER_PATH "&cwd=" LOADER_CWD " HTTP/1.0\r\n"
        "Host: 127.0.0.1\r\n\r\n";
    const net_sockaddr_in_t addr = {
        sizeof(net_sockaddr_in_t), 2,
        (uint16_t)((WEBSRV_PORT << 8) | (WEBSRV_PORT >> 8)),
        0x0100007f, {0}
    };
    int s = sceNetSocket("pico8_home", 2, 1, 6);

    if (s < 0 || sceNetConnect(s, &addr, sizeof(addr)) < 0) {
        notify("PICO-8: websrv nao esta rodando (porta 8080).\n"
               "Inicie o websrv pelo Payload Manager.");
        if (s >= 0)
            sceNetSocketClose(s);
        return 1;
    }
    sceNetSend(s, request, sizeof(request) - 1, 0);

    /* O websrv encerra este app antes de abrir o loader. Se ainda
     * estivermos vivos depois de 20 s, algo deu errado. */
    for (int i = 0; i < 200; i++)
        sceKernelUsleep(100 * 1000);

    sceNetSocketClose(s);
    notify("PICO-8: o websrv nao abriu o loader.\n"
           "Confira /data/homebrew/PICO8/eboot.elf.");
    return 1;
}
