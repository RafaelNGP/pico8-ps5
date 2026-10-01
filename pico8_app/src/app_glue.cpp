/* pico8_app - ganchos do pico8_loader quando ele roda como eboot.bin
 *
 * Como app nativo, o loader nasce no sandbox e nao enxerga /data. O
 * helper de elevacao do boilerplate (via elfldr local, como no
 * ProsperoEden) libera o filesystem antes de abrir pico8_dyn.
 */

#include "elevation.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>

extern "C" int p8_app_elevate(void)
{
    return static_cast<int>(elevation::request(elevation::Capability::filesystem));
}

/* A libc.a do SDK implementa dlopen/dlsym/dlclose como trampolins para
 * __dlopen & cia., que o elfldr preenche em payloads; num app eles nao
 * existem e o conversor recusa o import. So o SDL_dynapi chama dlopen
 * (para trocar a SDL embutida via SDL_DYNAMIC_API), entao basta falhar. */
extern "C" void *dlopen(const char *, int)
{
    return nullptr;
}

extern "C" void *dlsym(void *, const char *)
{
    return nullptr;
}

extern "C" int dlclose(void *)
{
    return -1;
}

/* O if_nametoindex da libc.a depende de syscall(). O curl so o usa para
 * zone ids IPv6 ("fe80::1%eth0"), que o PICO-8 nao precisa. */
extern "C" unsigned int if_nametoindex(const char *)
{
    return 0;
}

/* O readlink dos stubs vem da libkernel_sys, que um app nao carrega. O
 * shim do loader ja responde /proc/self/exe; o resto nao existe aqui. */
extern "C" long readlink(const char *, char *, unsigned long)
{
    errno = EINVAL;
    return -1;
}

/* libSceKeyboard e libSceImeDialog nao carregam num app (load ->
 * 0x80020063) e o rtld deixa os slots com um endereco sem nada mapeado
 * (0x840000000). Consertar o GOT em runtime nao segura: o rtld regrava o
 * slot quando o SDL_Init carrega outros modulos. Definidas aqui, o linker
 * liga o SDL direto nelas e o import some. O SDL trata o erro: sem teclado
 * USB e sem teclado na tela, o controle continua funcionando. */
namespace
{
constexpr int sce_error = static_cast<int>(0x80020001u); /* SCE_KERNEL_ERROR_UNKNOWN */
}

extern "C" int sceKeyboardInit(void)
{
    return sce_error;
}

extern "C" int sceKeyboardOpen(int, int, int, void *)
{
    return sce_error;
}

extern "C" int sceKeyboardReadState(int, void *)
{
    return sce_error;
}

extern "C" int sceKeyboardClose(int)
{
    return sce_error;
}

extern "C" int sceImeDialogInit(const void *, void *)
{
    return sce_error;
}

extern "C" int sceImeDialogGetStatus(void)
{
    return 0; /* SCE_IME_DIALOG_STATUS_NONE */
}

extern "C" int sceImeDialogGetResult(void *)
{
    return sce_error;
}

extern "C" int sceImeDialogTerm(void)
{
    return sce_error;
}

/* O getcwd da libSceLibcInternal chama por dentro um modulo que o app nao
 * carrega e salta para um placeholder (crash visto logo apos o SDL_Init).
 * O loader faz chdir(P8_DIR) antes do pico8, que nao importa chdir: o
 * diretorio atual eh sempre esse. */
extern "C" char *getcwd(char *buf, unsigned long size)
{
    static const char cwd[] = "/data/pico8";

    if (!buf)
    {
        size = size ? size : sizeof(cwd);
        buf = static_cast<char *>(std::malloc(size));
        if (!buf)
            return nullptr;
    }
    if (size < sizeof(cwd))
    {
        errno = ERANGE;
        return nullptr;
    }
    std::memcpy(buf, cwd, sizeof(cwd));
    return buf;
}
