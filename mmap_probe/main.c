/* mmap_probe - PICO-8 PS5 loader feasibility test
 *
 * O pico8_dyn eh um ELF nao-PIE que exige carga em endereco fixo:
 *   LOAD  0x0000000000400000  R E  (~1.6 MB)
 *   LOAD  0x0000000000795cf0  RW   (~3.9 MB em memoria com BSS)
 * Faixa total aproximada: 0x400000 .. 0xb6f000  (~7.5 MB)
 *
 * Este payload NAO carrega o PICO-8. Ele so responde a pergunta que
 * decide a arquitetura do loader: o processo do homebrew consegue
 * reservar essa faixa com MAP_FIXED? Imprime o resultado via notify
 * (toast na tela) e tambem no socket de log do SDK (klog/stdout).
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>

#include <ps5/kernel.h>   /* nao obrigatorio; presente no SDK para info extra */

/* Segmentos reais extraidos do pico8_dyn com `readelf -lW`. */
#define SEG_TEXT_ADDR   0x0000000000400000ULL
#define SEG_TEXT_SIZE   0x0000000000196000ULL   /* 0x195550 arredondado p/ pagina */

#define SEG_DATA_ADDR   0x0000000000795000ULL   /* inicio da pagina do segmento RW */
#define SEG_DATA_SIZE   0x00000000003d9000ULL   /* 0x3d8690 (memsz c/ BSS) arredondado */

#define PAGE            0x4000ULL               /* PS5 usa paginas de 16 KiB */

/* notify imprime um toast no canto da tela do PS5. Prototipo exposto
 * pelo ps5-payload-sdk (libkernel_sys). */
extern void notify(const char *fmt, ...) __attribute__((weak));

static void say(const char *fmt, const char *a, unsigned long b, long c)
{
    char buf[256];
    snprintf(buf, sizeof(buf), fmt, a, b, c);
    /* stdout vai para o log do SDK (nc na porta de klog). */
    printf("[mmap_probe] %s\n", buf);
    fflush(stdout);
    if (notify) notify("mmap_probe: %s", buf);
}

/* Tenta reservar [addr, addr+size) exatamente. Retorna 0 em sucesso. */
static int try_fixed(const char *name, uint64_t addr, uint64_t size)
{
    /* MAP_FIXED sem MAP_FIXED_NOREPLACE: queremos saber se o endereco
     * esta livre; se o kernel devolver outro endereco, falhamos de
     * proposito para nao mascarar o resultado. Usamos ANON|PRIVATE. */
    void *want = (void *)(uintptr_t)addr;
    void *got = mmap(want, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

    if (got == MAP_FAILED) {
        say("%s: FALHOU mmap (errno=%lu) size=0x%lx", name, (unsigned long)errno, (long)size);
        return -1;
    }
    if (got != want) {
        say("%s: endereco DIFERENTE (quis 0x%lx, veio outro)", name, addr, 0);
        munmap(got, size);
        return -1;
    }

    /* Prova que a pagina e realmente utilizavel: escreve e le de volta. */
    volatile uint32_t *p = (volatile uint32_t *)(uintptr_t)addr;
    p[0] = 0xC0FFEE42u;
    p[(size / 4) - 1] = 0x1337BEEFu;
    int ok = (p[0] == 0xC0FFEE42u) && (p[(size / 4) - 1] == 0x1337BEEFu);

    say("%s: OK em 0x%lx (rw verificado=%ld)", name, addr, (long)ok);
    munmap(got, size);
    return ok ? 0 : -1;
}

int main(void)
{
    printf("\n==== PICO-8 PS5 mmap_probe ====\n");
    fflush(stdout);
    if (notify) notify("mmap_probe iniciado");

    int r_text = try_fixed("TEXT 0x400000", SEG_TEXT_ADDR, SEG_TEXT_SIZE);
    int r_data = try_fixed("DATA 0x795000", SEG_DATA_ADDR, SEG_DATA_SIZE);

    /* Teste combinado: reservar a faixa inteira de uma vez, como o
     * loader real fara antes de copiar os segmentos. */
    uint64_t whole_addr = SEG_TEXT_ADDR;
    uint64_t whole_end  = SEG_DATA_ADDR + SEG_DATA_SIZE;
    uint64_t whole_size = whole_end - whole_addr;
    int r_whole = try_fixed("FAIXA INTEIRA", whole_addr, whole_size);

    if (r_text == 0 && r_data == 0 && r_whole == 0) {
        say("RESULTADO: VIAVEL - loader pode usar enderecos nativos%s", "", 0, 0);
    } else {
        say("RESULTADO: BLOQUEADO - precisamos de plano B (relink alto)%s", "", 0, 0);
    }

    printf("==== fim ====\n");
    fflush(stdout);
    return 0;
}
