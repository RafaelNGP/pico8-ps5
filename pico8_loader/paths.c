/* paths - onde ficam os arquivos do PICO-8 e o cacert.pem
 *
 * Como app nativo, tudo que o usuario instala fica dentro da pasta do app,
 * como no ProsperoEden: o pacote traz <titleId>/cacert.pem e o usuario poe
 * o pico8_dyn e o pico8.dat em <titleId>/pico8/. O pico8 acha o pico8.dat ao lado
 * do executavel (codo_prefix_with_program_path sobre /proc/self/exe), entao
 * basta apontar p8_bin para a pasta certa.
 *
 * A pasta do app eh /app0 no sandbox, mas depois da elevacao o processo
 * enxerga a raiz real, onde ela aparece em /mnt/sandbox/<titleId>_000/app0.
 * O /data/homebrew/<titleId> cobre o local padrao do ShadowMountPlus, e o
 * /data/pico8 eh o layout antigo. Saves, log e
 * HOME ficam sempre em P8_DIR, fora da pasta do app, para uma atualizacao
 * nao apagar nada.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "loader.h"

char p8_bin[256];
char p8_cacert[256];

static const char *const app_roots[] = {
    "/app0",
    "/mnt/sandbox/" P8_TITLE_ID "_000/app0",
    "/data/homebrew/" P8_TITLE_ID,
    NULL,
};

/* Versao do pico8_dyn com que o loader foi testado. Outras podem mudar a
 * lista de imports ou o _start e nao abrir. */
#define P8_TESTED_VERSION "0.2.7"

/* Acha a string " pico-8 <versao>" no executavel. */
static void check_version(void)
{
    static const char tag[] = " pico-8 ";
    char buf[64 * 1024 + 32], ver[16] = "";
    size_t keep = 0, n;
    FILE *f = fopen(p8_bin, "rb");

    if (!f)
        return;
    while (!ver[0] && (n = fread(buf + keep, 1, sizeof(buf) - 32 - keep, f)) > 0) {
        n += keep;
        buf[n] = 0;
        for (size_t i = 0; i + sizeof(tag) - 1 < n; i++) {
            if (memcmp(buf + i, tag, sizeof(tag) - 1) == 0 &&
                buf[i + sizeof(tag) - 1] >= '0' && buf[i + sizeof(tag) - 1] <= '9') {
                snprintf(ver, sizeof(ver), "%s", buf + i + sizeof(tag) - 1);
                break;
            }
        }
        keep = n < 32 ? n : 32;
        memmove(buf, buf + n - keep, keep);
    }
    fclose(f);

    lg("versao do pico8_dyn: %s", ver[0] ? ver : "?");
    if (strcmp(ver, P8_TESTED_VERSION) != 0)
        notify("PICO-8: this pico8_dyn is version %s; tested with %s.\n"
               "If PICO-8 does not start, use version " P8_TESTED_VERSION ".",
               ver[0] ? ver : "unknown", P8_TESTED_VERSION);
}

static int exists(const char *path)
{
    return access(path, R_OK) == 0;
}

/* Procura <raiz>/<sub> nas pastas do app e depois em P8_DIR/<antigo>.
 * Os caminhos tentados so vao para o log se nenhum existir. */
static int find(char *out, size_t len, const char *sub, const char *legacy)
{
    int i;

    for (i = 0; app_roots[i]; i++) {
        snprintf(out, len, "%s/%s", app_roots[i], sub);
        if (exists(out))
            return 0;
    }
    snprintf(out, len, "%s/%s", P8_DIR, legacy);
    if (exists(out))
        return 0;

    lg("nao achei %s; procurei em:", sub);
    for (i = 0; app_roots[i]; i++)
        lg("  %s/%s", app_roots[i], sub);
    lg("  %s/%s", P8_DIR, legacy);
    return -1;
}

int p8_find_files(void)
{
    char dat[256];

    if (find(p8_bin, sizeof(p8_bin), "pico8/pico8_dyn", "pico8_dyn") != 0) {
        notify("PICO-8: game files not found.\n"
               "Copy pico8_dyn and pico8.dat from your PICO-8 Linux zip "
               "into the pico8 folder inside the app folder "
               "(" P8_TITLE_ID "/pico8/).");
        return -1;
    }
    lg("pico8_dyn: %s", p8_bin);
    check_version();

    snprintf(dat, sizeof(dat), "%.*s/pico8.dat",
             (int)(strrchr(p8_bin, '/') - p8_bin), p8_bin);
    if (!exists(dat)) {
        notify("PICO-8: pico8.dat not found.\n"
               "Copy pico8.dat next to pico8_dyn (" P8_TITLE_ID "/pico8/).");
        lg("  nao achei %s", dat);
        return -1;
    }

    if (find(p8_cacert, sizeof(p8_cacert), "cacert.pem", "cacert.pem") != 0) {
        /* Sem CAs o Splore abre, mas nenhum download HTTPS funciona. */
        lg("AVISO: cacert.pem nao encontrado, downloads HTTPS vao falhar");
        p8_cacert[0] = 0;
    } else {
        lg("cacert.pem: %s", p8_cacert);
    }
    return 0;
}
