/* net_curl.c - responde ao dlopen("libcurl.so") do pico8
 *
 * O Splore faz dlopen("libcurl.so") e pega por dlsym curl_easy_init,
 * _setopt, _perform, _cleanup e _strerror. Linkamos a libcurl real
 * (8.18 + mbedTLS, deps/build_curl.sh) e entregamos essas funcoes. A ABI
 * de curl_easy_* eh a mesma do Linux. O TLS nativo do PS5 (sceHttp2)
 * falhou com 0x8095f00c nos downloads HTTPS, por isso nao eh usado.
 *
 * O pacote de CAs fica em /data/pico8/cacert.pem (`make upload-data`).
 */

#include <pthread.h>
#include <string.h>

#include <curl/curl.h>

#include "loader.h"

static pthread_once_t curl_once = PTHREAD_ONCE_INIT;

static void curl_init_once(void)
{
    CURLcode r = curl_global_init(CURL_GLOBAL_DEFAULT);
    lg("curl: %s, global_init = %d", curl_version(), r);
}

/* Registra cada download; o pico8 so ve o resultado. */
static CURLcode p8_curl_easy_perform(CURL *c)
{
    CURLcode r = curl_easy_perform(c);
    long status = 0;
    char *url = NULL;

    curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_getinfo(c, CURLINFO_EFFECTIVE_URL, &url);
    if (r != CURLE_OK)
        lg("curl: erro %d (%s) %s", r, curl_easy_strerror(r), url ? url : "");
    else
        lg("curl: %ld %s", status, url ? url : "");
    return r;
}

/* Handle devolvido pelo dlopen("libcurl.so*"). */
static int curl_handle;

void *net_curl_dlopen(const char *name)
{
    if (!name || strncmp(name, "libcurl.so", 10) != 0)
        return NULL;
    pthread_once(&curl_once, curl_init_once);
    return &curl_handle;
}

void *net_curl_dlsym(void *h, const char *name)
{
    static const shim_t fns[] = {
        { "curl_easy_init",     (void *)curl_easy_init },
        { "curl_easy_setopt",   (void *)curl_easy_setopt },
        { "curl_easy_perform",  (void *)p8_curl_easy_perform },
        { "curl_easy_cleanup",  (void *)curl_easy_cleanup },
        { "curl_easy_strerror", (void *)curl_easy_strerror },
    };

    if (h != &curl_handle)
        return NULL;
    for (size_t i = 0; i < sizeof(fns) / sizeof(fns[0]); i++)
        if (strcmp(fns[i].name, name) == 0)
            return fns[i].addr;
    return NULL;
}
