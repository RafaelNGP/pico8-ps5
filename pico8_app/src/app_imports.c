/* Gerado por deps/gen_app_imports.py - nao editar. */

#include <stdint.h>

const char *const p8_import_modules[] = {
    "libSceLibcInternal",
    "libkernel",
    "libSceAudioOut",
    "libSceNet",
    "libScePad",
    "libSceSystemService",
    "libSceUserService",
    "libSceVideoOut",
};

const struct p8_import { const char *name; uint8_t module; } p8_imports[] = {
    { "_Exit", 0 },
    { "__error", 1 },
    { "__inet_aton", 0 },
    { "__inet_ntop", 1 },
    { "__inet_pton", 1 },
    { "__isthreaded", 0 },
    { "__stderrp", 0 },
    { "__stdinp", 0 },
    { "__stdoutp", 0 },
    { "__udivti3", 0 },
    { "_close", 1 },
    { "_exit", 1 },
    { "_init_env", 0 },
    { "_open", 1 },
    { "_read", 1 },
    { "abort", 0 },
    { "accept", 1 },
    { "acos", 0 },
    { "acosf", 0 },
    { "asin", 0 },
    { "asinf", 0 },
    { "atan", 0 },
    { "atan2", 0 },
    { "atan2f", 0 },
    { "atanf", 0 },
    { "atexit", 0 },
    { "atof", 0 },
    { "atoi", 0 },
    { "basename", 0 },
    { "bind", 1 },
    { "bsearch", 0 },
    { "bzero", 0 },
    { "calloc", 0 },
    { "chdir", 1 },
    { "chmod", 1 },
    { "clock", 0 },
    { "clock_gettime", 1 },
    { "close", 1 },
    { "closedir", 0 },
    { "connect", 1 },
    { "cos", 0 },
    { "cosf", 0 },
    { "cosh", 0 },
    { "difftime", 0 },
    { "environ", 1 },
    { "exit", 0 },
    { "exp", 0 },
    { "expf", 0 },
    { "fclose", 0 },
    { "fcntl", 1 },
    { "fdopen", 0 },
    { "feof", 0 },
    { "ferror", 0 },
    { "fflush", 0 },
    { "fgetc", 0 },
    { "fgets", 0 },
    { "fileno", 0 },
    { "fmod", 0 },
    { "fmodf", 0 },
    { "fopen", 0 },
    { "fprintf", 0 },
    { "fputc", 0 },
    { "fputs", 0 },
    { "fread", 0 },
    { "free", 0 },
    { "freeifaddrs", 0 },
    { "freopen", 0 },
    { "frexp", 0 },
    { "fseek", 0 },
    { "fseeko", 0 },
    { "fstat", 1 },
    { "ftell", 0 },
    { "ftello", 0 },
    { "fwrite", 0 },
    { "getargv", 1 },
    { "getenv", 0 },
    { "geteuid", 1 },
    { "getifaddrs", 0 },
    { "getpeername", 1 },
    { "getpid", 1 },
    { "getsockname", 1 },
    { "getsockopt", 1 },
    { "gettimeofday", 1 },
    { "ioctl", 1 },
    { "isalnum", 0 },
    { "isalpha", 0 },
    { "isblank", 0 },
    { "iscntrl", 0 },
    { "isgraph", 0 },
    { "islower", 0 },
    { "isprint", 0 },
    { "ispunct", 0 },
    { "isspace", 0 },
    { "isupper", 0 },
    { "isxdigit", 0 },
    { "ldexp", 0 },
    { "localeconv", 0 },
    { "localtime", 0 },
    { "log", 0 },
    { "log10", 0 },
    { "log10f", 0 },
    { "logf", 0 },
    { "lround", 0 },
    { "lroundf", 0 },
    { "lstat", 1 },
    { "malloc", 0 },
    { "memchr", 0 },
    { "memcmp", 0 },
    { "memcpy", 0 },
    { "memmove", 0 },
    { "memrchr", 0 },
    { "memset", 0 },
    { "mkdir", 1 },
    { "mktime", 0 },
    { "mmap", 1 },
    { "mprotect", 1 },
    { "munmap", 1 },
    { "nanosleep", 1 },
    { "open", 1 },
    { "opendir", 0 },
    { "pipe", 1 },
    { "poll", 1 },
    { "posix_memalign", 0 },
    { "pow", 0 },
    { "powf", 0 },
    { "pread", 1 },
    { "printf", 0 },
    { "pthread_attr_init", 1 },
    { "pthread_attr_setdetachstate", 1 },
    { "pthread_attr_setstacksize", 1 },
    { "pthread_cond_broadcast", 1 },
    { "pthread_cond_destroy", 1 },
    { "pthread_cond_init", 1 },
    { "pthread_cond_signal", 1 },
    { "pthread_cond_timedwait", 1 },
    { "pthread_cond_wait", 1 },
    { "pthread_create", 1 },
    { "pthread_detach", 1 },
    { "pthread_equal", 1 },
    { "pthread_getschedparam", 1 },
    { "pthread_getspecific", 1 },
    { "pthread_join", 1 },
    { "pthread_key_create", 1 },
    { "pthread_key_delete", 1 },
    { "pthread_mutex_destroy", 1 },
    { "pthread_mutex_init", 1 },
    { "pthread_mutex_lock", 1 },
    { "pthread_mutex_trylock", 1 },
    { "pthread_mutex_unlock", 1 },
    { "pthread_mutexattr_init", 1 },
    { "pthread_mutexattr_settype", 1 },
    { "pthread_once", 1 },
    { "pthread_self", 1 },
    { "pthread_set_name_np", 1 },
    { "pthread_setcanceltype", 1 },
    { "pthread_setschedparam", 1 },
    { "pthread_setspecific", 1 },
    { "pthread_sigmask", 1 },
    { "putchar", 0 },
    { "puts", 0 },
    { "qsort", 0 },
    { "rand", 0 },
    { "read", 1 },
    { "readdir", 0 },
    { "realloc", 0 },
    { "realpath", 0 },
    { "recv", 1 },
    { "remove", 0 },
    { "rename", 1 },
    { "rewind", 0 },
    { "scalbn", 0 },
    { "scalbnf", 0 },
    { "sceAudioOutClose", 2 },
    { "sceAudioOutInit", 2 },
    { "sceAudioOutOpen", 2 },
    { "sceAudioOutOutput", 2 },
    { "sceKernelAllocateMainDirectMemory", 1 },
    { "sceKernelAvailableDirectMemorySize", 1 },
    { "sceKernelAvailableFlexibleMemorySize", 1 },
    { "sceKernelClose", 1 },
    { "sceKernelCreateEqueue", 1 },
    { "sceKernelDeleteEqueue", 1 },
    { "sceKernelGetDirectMemorySize", 1 },
    { "sceKernelMapDirectMemory", 1 },
    { "sceKernelOpen", 1 },
    { "sceKernelQueryMemoryProtection", 1 },
    { "sceKernelRead", 1 },
    { "sceKernelReleaseDirectMemory", 1 },
    { "sceKernelSendNotificationRequest", 1 },
    { "sceKernelVirtualQuery", 1 },
    { "sceKernelWaitEqueue", 1 },
    { "sceNetConnect", 3 },
    { "sceNetErrnoLoc", 3 },
    { "sceNetPoolCreate", 3 },
    { "sceNetPoolDestroy", 3 },
    { "sceNetRecv", 3 },
    { "sceNetResolverCreate", 3 },
    { "sceNetResolverDestroy", 3 },
    { "sceNetResolverStartAton", 3 },
    { "sceNetResolverStartNtoa", 3 },
    { "sceNetSend", 3 },
    { "sceNetSetsockopt", 3 },
    { "sceNetSocket", 3 },
    { "sceNetSocketClose", 3 },
    { "scePadClose", 4 },
    { "scePadGetHandle", 4 },
    { "scePadInit", 4 },
    { "scePadOpen", 4 },
    { "scePadReadState", 4 },
    { "scePadSetLightBar", 4 },
    { "scePadSetVibration", 4 },
    { "scePadSetVibrationMode", 4 },
    { "sceSystemServiceHideSplashScreen", 5 },
    { "sceSystemServiceLaunchWebBrowser", 5 },
    { "sceUserServiceGetForegroundUser", 6 },
    { "sceUserServiceGetLoginUserIdList", 6 },
    { "sceUserServiceGetUserName", 6 },
    { "sceUserServiceInitialize", 6 },
    { "sceVideoOutAddFlipEvent", 7 },
    { "sceVideoOutClose", 7 },
    { "sceVideoOutDeleteFlipEvent", 7 },
    { "sceVideoOutOpen", 7 },
    { "sceVideoOutRegisterBuffers2", 7 },
    { "sceVideoOutSetBufferAttribute2", 7 },
    { "sceVideoOutSetFlipRate", 7 },
    { "sceVideoOutSubmitFlip", 7 },
    { "sched_get_priority_max", 1 },
    { "sched_get_priority_min", 1 },
    { "sched_yield", 1 },
    { "select", 1 },
    { "sem_destroy", 1 },
    { "sem_getvalue", 1 },
    { "sem_init", 1 },
    { "sem_post", 1 },
    { "sem_timedwait", 1 },
    { "sem_trywait", 1 },
    { "sem_wait", 1 },
    { "send", 1 },
    { "setbuf", 0 },
    { "setenv", 0 },
    { "seteuid", 1 },
    { "setsockopt", 1 },
    { "setvbuf", 0 },
    { "sigaction", 1 },
    { "sigaddset", 1 },
    { "sigaltstack", 1 },
    { "sigemptyset", 1 },
    { "signal", 1 },
    { "sin", 0 },
    { "sincos", 0 },
    { "sinf", 0 },
    { "sinh", 0 },
    { "snprintf", 0 },
    { "socket", 1 },
    { "sprintf", 0 },
    { "sqrt", 0 },
    { "sqrtf", 0 },
    { "srand", 0 },
    { "stat", 1 },
    { "stpcpy", 0 },
    { "strcasecmp", 0 },
    { "strcat", 0 },
    { "strchr", 0 },
    { "strcmp", 0 },
    { "strcoll", 0 },
    { "strcpy", 0 },
    { "strcspn", 0 },
    { "strdup", 0 },
    { "strerror", 0 },
    { "strerror_r", 0 },
    { "strftime", 0 },
    { "strlcat", 0 },
    { "strlcpy", 0 },
    { "strlen", 0 },
    { "strncasecmp", 0 },
    { "strncmp", 0 },
    { "strncpy", 0 },
    { "strpbrk", 0 },
    { "strrchr", 0 },
    { "strspn", 0 },
    { "strstr", 0 },
    { "strtod", 0 },
    { "strtok", 0 },
    { "strtok_r", 0 },
    { "strtol", 0 },
    { "strtoll", 0 },
    { "strtoul", 0 },
    { "strtoull", 0 },
    { "sysconf", 1 },
    { "sysctl", 1 },
    { "sysctlbyname", 1 },
    { "tan", 0 },
    { "tanf", 0 },
    { "tanh", 0 },
    { "time", 0 },
    { "tolower", 0 },
    { "toupper", 0 },
    { "unlink", 1 },
    { "utime", 0 },
    { "vfprintf", 0 },
    { "vsnprintf", 0 },
    { "vsprintf", 0 },
    { "vsscanf", 0 },
    { "wcscmp", 0 },
    { "wcslen", 0 },
    { "wcsncmp", 0 },
    { "wcsstr", 0 },
    { "wcstombs", 0 },
    { "write", 1 },
};
const int p8_n_imports = sizeof(p8_imports) / sizeof(p8_imports[0]);

/* Offset PC-relativo de cada campo ate o slot do GOT do import:
 * slot = (char *)&p8_import_got[i] + p8_import_got[i]. Resolvido no
 * link, entao o compilador nao tem como presumir nada sobre o valor. */
__asm__(
    ".section .rodata\n"
    ".p2align 2\n"
    ".globl p8_import_got\n"
    "p8_import_got:\n"
    "    .long _Exit@GOTPCREL\n"
    "    .long __error@GOTPCREL\n"
    "    .long __inet_aton@GOTPCREL\n"
    "    .long __inet_ntop@GOTPCREL\n"
    "    .long __inet_pton@GOTPCREL\n"
    "    .long __isthreaded@GOTPCREL\n"
    "    .long __stderrp@GOTPCREL\n"
    "    .long __stdinp@GOTPCREL\n"
    "    .long __stdoutp@GOTPCREL\n"
    "    .long __udivti3@GOTPCREL\n"
    "    .long _close@GOTPCREL\n"
    "    .long _exit@GOTPCREL\n"
    "    .long _init_env@GOTPCREL\n"
    "    .long _open@GOTPCREL\n"
    "    .long _read@GOTPCREL\n"
    "    .long abort@GOTPCREL\n"
    "    .long accept@GOTPCREL\n"
    "    .long acos@GOTPCREL\n"
    "    .long acosf@GOTPCREL\n"
    "    .long asin@GOTPCREL\n"
    "    .long asinf@GOTPCREL\n"
    "    .long atan@GOTPCREL\n"
    "    .long atan2@GOTPCREL\n"
    "    .long atan2f@GOTPCREL\n"
    "    .long atanf@GOTPCREL\n"
    "    .long atexit@GOTPCREL\n"
    "    .long atof@GOTPCREL\n"
    "    .long atoi@GOTPCREL\n"
    "    .long basename@GOTPCREL\n"
    "    .long bind@GOTPCREL\n"
    "    .long bsearch@GOTPCREL\n"
    "    .long bzero@GOTPCREL\n"
    "    .long calloc@GOTPCREL\n"
    "    .long chdir@GOTPCREL\n"
    "    .long chmod@GOTPCREL\n"
    "    .long clock@GOTPCREL\n"
    "    .long clock_gettime@GOTPCREL\n"
    "    .long close@GOTPCREL\n"
    "    .long closedir@GOTPCREL\n"
    "    .long connect@GOTPCREL\n"
    "    .long cos@GOTPCREL\n"
    "    .long cosf@GOTPCREL\n"
    "    .long cosh@GOTPCREL\n"
    "    .long difftime@GOTPCREL\n"
    "    .long environ@GOTPCREL\n"
    "    .long exit@GOTPCREL\n"
    "    .long exp@GOTPCREL\n"
    "    .long expf@GOTPCREL\n"
    "    .long fclose@GOTPCREL\n"
    "    .long fcntl@GOTPCREL\n"
    "    .long fdopen@GOTPCREL\n"
    "    .long feof@GOTPCREL\n"
    "    .long ferror@GOTPCREL\n"
    "    .long fflush@GOTPCREL\n"
    "    .long fgetc@GOTPCREL\n"
    "    .long fgets@GOTPCREL\n"
    "    .long fileno@GOTPCREL\n"
    "    .long fmod@GOTPCREL\n"
    "    .long fmodf@GOTPCREL\n"
    "    .long fopen@GOTPCREL\n"
    "    .long fprintf@GOTPCREL\n"
    "    .long fputc@GOTPCREL\n"
    "    .long fputs@GOTPCREL\n"
    "    .long fread@GOTPCREL\n"
    "    .long free@GOTPCREL\n"
    "    .long freeifaddrs@GOTPCREL\n"
    "    .long freopen@GOTPCREL\n"
    "    .long frexp@GOTPCREL\n"
    "    .long fseek@GOTPCREL\n"
    "    .long fseeko@GOTPCREL\n"
    "    .long fstat@GOTPCREL\n"
    "    .long ftell@GOTPCREL\n"
    "    .long ftello@GOTPCREL\n"
    "    .long fwrite@GOTPCREL\n"
    "    .long getargv@GOTPCREL\n"
    "    .long getenv@GOTPCREL\n"
    "    .long geteuid@GOTPCREL\n"
    "    .long getifaddrs@GOTPCREL\n"
    "    .long getpeername@GOTPCREL\n"
    "    .long getpid@GOTPCREL\n"
    "    .long getsockname@GOTPCREL\n"
    "    .long getsockopt@GOTPCREL\n"
    "    .long gettimeofday@GOTPCREL\n"
    "    .long ioctl@GOTPCREL\n"
    "    .long isalnum@GOTPCREL\n"
    "    .long isalpha@GOTPCREL\n"
    "    .long isblank@GOTPCREL\n"
    "    .long iscntrl@GOTPCREL\n"
    "    .long isgraph@GOTPCREL\n"
    "    .long islower@GOTPCREL\n"
    "    .long isprint@GOTPCREL\n"
    "    .long ispunct@GOTPCREL\n"
    "    .long isspace@GOTPCREL\n"
    "    .long isupper@GOTPCREL\n"
    "    .long isxdigit@GOTPCREL\n"
    "    .long ldexp@GOTPCREL\n"
    "    .long localeconv@GOTPCREL\n"
    "    .long localtime@GOTPCREL\n"
    "    .long log@GOTPCREL\n"
    "    .long log10@GOTPCREL\n"
    "    .long log10f@GOTPCREL\n"
    "    .long logf@GOTPCREL\n"
    "    .long lround@GOTPCREL\n"
    "    .long lroundf@GOTPCREL\n"
    "    .long lstat@GOTPCREL\n"
    "    .long malloc@GOTPCREL\n"
    "    .long memchr@GOTPCREL\n"
    "    .long memcmp@GOTPCREL\n"
    "    .long memcpy@GOTPCREL\n"
    "    .long memmove@GOTPCREL\n"
    "    .long memrchr@GOTPCREL\n"
    "    .long memset@GOTPCREL\n"
    "    .long mkdir@GOTPCREL\n"
    "    .long mktime@GOTPCREL\n"
    "    .long mmap@GOTPCREL\n"
    "    .long mprotect@GOTPCREL\n"
    "    .long munmap@GOTPCREL\n"
    "    .long nanosleep@GOTPCREL\n"
    "    .long open@GOTPCREL\n"
    "    .long opendir@GOTPCREL\n"
    "    .long pipe@GOTPCREL\n"
    "    .long poll@GOTPCREL\n"
    "    .long posix_memalign@GOTPCREL\n"
    "    .long pow@GOTPCREL\n"
    "    .long powf@GOTPCREL\n"
    "    .long pread@GOTPCREL\n"
    "    .long printf@GOTPCREL\n"
    "    .long pthread_attr_init@GOTPCREL\n"
    "    .long pthread_attr_setdetachstate@GOTPCREL\n"
    "    .long pthread_attr_setstacksize@GOTPCREL\n"
    "    .long pthread_cond_broadcast@GOTPCREL\n"
    "    .long pthread_cond_destroy@GOTPCREL\n"
    "    .long pthread_cond_init@GOTPCREL\n"
    "    .long pthread_cond_signal@GOTPCREL\n"
    "    .long pthread_cond_timedwait@GOTPCREL\n"
    "    .long pthread_cond_wait@GOTPCREL\n"
    "    .long pthread_create@GOTPCREL\n"
    "    .long pthread_detach@GOTPCREL\n"
    "    .long pthread_equal@GOTPCREL\n"
    "    .long pthread_getschedparam@GOTPCREL\n"
    "    .long pthread_getspecific@GOTPCREL\n"
    "    .long pthread_join@GOTPCREL\n"
    "    .long pthread_key_create@GOTPCREL\n"
    "    .long pthread_key_delete@GOTPCREL\n"
    "    .long pthread_mutex_destroy@GOTPCREL\n"
    "    .long pthread_mutex_init@GOTPCREL\n"
    "    .long pthread_mutex_lock@GOTPCREL\n"
    "    .long pthread_mutex_trylock@GOTPCREL\n"
    "    .long pthread_mutex_unlock@GOTPCREL\n"
    "    .long pthread_mutexattr_init@GOTPCREL\n"
    "    .long pthread_mutexattr_settype@GOTPCREL\n"
    "    .long pthread_once@GOTPCREL\n"
    "    .long pthread_self@GOTPCREL\n"
    "    .long pthread_set_name_np@GOTPCREL\n"
    "    .long pthread_setcanceltype@GOTPCREL\n"
    "    .long pthread_setschedparam@GOTPCREL\n"
    "    .long pthread_setspecific@GOTPCREL\n"
    "    .long pthread_sigmask@GOTPCREL\n"
    "    .long putchar@GOTPCREL\n"
    "    .long puts@GOTPCREL\n"
    "    .long qsort@GOTPCREL\n"
    "    .long rand@GOTPCREL\n"
    "    .long read@GOTPCREL\n"
    "    .long readdir@GOTPCREL\n"
    "    .long realloc@GOTPCREL\n"
    "    .long realpath@GOTPCREL\n"
    "    .long recv@GOTPCREL\n"
    "    .long remove@GOTPCREL\n"
    "    .long rename@GOTPCREL\n"
    "    .long rewind@GOTPCREL\n"
    "    .long scalbn@GOTPCREL\n"
    "    .long scalbnf@GOTPCREL\n"
    "    .long sceAudioOutClose@GOTPCREL\n"
    "    .long sceAudioOutInit@GOTPCREL\n"
    "    .long sceAudioOutOpen@GOTPCREL\n"
    "    .long sceAudioOutOutput@GOTPCREL\n"
    "    .long sceKernelAllocateMainDirectMemory@GOTPCREL\n"
    "    .long sceKernelAvailableDirectMemorySize@GOTPCREL\n"
    "    .long sceKernelAvailableFlexibleMemorySize@GOTPCREL\n"
    "    .long sceKernelClose@GOTPCREL\n"
    "    .long sceKernelCreateEqueue@GOTPCREL\n"
    "    .long sceKernelDeleteEqueue@GOTPCREL\n"
    "    .long sceKernelGetDirectMemorySize@GOTPCREL\n"
    "    .long sceKernelMapDirectMemory@GOTPCREL\n"
    "    .long sceKernelOpen@GOTPCREL\n"
    "    .long sceKernelQueryMemoryProtection@GOTPCREL\n"
    "    .long sceKernelRead@GOTPCREL\n"
    "    .long sceKernelReleaseDirectMemory@GOTPCREL\n"
    "    .long sceKernelSendNotificationRequest@GOTPCREL\n"
    "    .long sceKernelVirtualQuery@GOTPCREL\n"
    "    .long sceKernelWaitEqueue@GOTPCREL\n"
    "    .long sceNetConnect@GOTPCREL\n"
    "    .long sceNetErrnoLoc@GOTPCREL\n"
    "    .long sceNetPoolCreate@GOTPCREL\n"
    "    .long sceNetPoolDestroy@GOTPCREL\n"
    "    .long sceNetRecv@GOTPCREL\n"
    "    .long sceNetResolverCreate@GOTPCREL\n"
    "    .long sceNetResolverDestroy@GOTPCREL\n"
    "    .long sceNetResolverStartAton@GOTPCREL\n"
    "    .long sceNetResolverStartNtoa@GOTPCREL\n"
    "    .long sceNetSend@GOTPCREL\n"
    "    .long sceNetSetsockopt@GOTPCREL\n"
    "    .long sceNetSocket@GOTPCREL\n"
    "    .long sceNetSocketClose@GOTPCREL\n"
    "    .long scePadClose@GOTPCREL\n"
    "    .long scePadGetHandle@GOTPCREL\n"
    "    .long scePadInit@GOTPCREL\n"
    "    .long scePadOpen@GOTPCREL\n"
    "    .long scePadReadState@GOTPCREL\n"
    "    .long scePadSetLightBar@GOTPCREL\n"
    "    .long scePadSetVibration@GOTPCREL\n"
    "    .long scePadSetVibrationMode@GOTPCREL\n"
    "    .long sceSystemServiceHideSplashScreen@GOTPCREL\n"
    "    .long sceSystemServiceLaunchWebBrowser@GOTPCREL\n"
    "    .long sceUserServiceGetForegroundUser@GOTPCREL\n"
    "    .long sceUserServiceGetLoginUserIdList@GOTPCREL\n"
    "    .long sceUserServiceGetUserName@GOTPCREL\n"
    "    .long sceUserServiceInitialize@GOTPCREL\n"
    "    .long sceVideoOutAddFlipEvent@GOTPCREL\n"
    "    .long sceVideoOutClose@GOTPCREL\n"
    "    .long sceVideoOutDeleteFlipEvent@GOTPCREL\n"
    "    .long sceVideoOutOpen@GOTPCREL\n"
    "    .long sceVideoOutRegisterBuffers2@GOTPCREL\n"
    "    .long sceVideoOutSetBufferAttribute2@GOTPCREL\n"
    "    .long sceVideoOutSetFlipRate@GOTPCREL\n"
    "    .long sceVideoOutSubmitFlip@GOTPCREL\n"
    "    .long sched_get_priority_max@GOTPCREL\n"
    "    .long sched_get_priority_min@GOTPCREL\n"
    "    .long sched_yield@GOTPCREL\n"
    "    .long select@GOTPCREL\n"
    "    .long sem_destroy@GOTPCREL\n"
    "    .long sem_getvalue@GOTPCREL\n"
    "    .long sem_init@GOTPCREL\n"
    "    .long sem_post@GOTPCREL\n"
    "    .long sem_timedwait@GOTPCREL\n"
    "    .long sem_trywait@GOTPCREL\n"
    "    .long sem_wait@GOTPCREL\n"
    "    .long send@GOTPCREL\n"
    "    .long setbuf@GOTPCREL\n"
    "    .long setenv@GOTPCREL\n"
    "    .long seteuid@GOTPCREL\n"
    "    .long setsockopt@GOTPCREL\n"
    "    .long setvbuf@GOTPCREL\n"
    "    .long sigaction@GOTPCREL\n"
    "    .long sigaddset@GOTPCREL\n"
    "    .long sigaltstack@GOTPCREL\n"
    "    .long sigemptyset@GOTPCREL\n"
    "    .long signal@GOTPCREL\n"
    "    .long sin@GOTPCREL\n"
    "    .long sincos@GOTPCREL\n"
    "    .long sinf@GOTPCREL\n"
    "    .long sinh@GOTPCREL\n"
    "    .long snprintf@GOTPCREL\n"
    "    .long socket@GOTPCREL\n"
    "    .long sprintf@GOTPCREL\n"
    "    .long sqrt@GOTPCREL\n"
    "    .long sqrtf@GOTPCREL\n"
    "    .long srand@GOTPCREL\n"
    "    .long stat@GOTPCREL\n"
    "    .long stpcpy@GOTPCREL\n"
    "    .long strcasecmp@GOTPCREL\n"
    "    .long strcat@GOTPCREL\n"
    "    .long strchr@GOTPCREL\n"
    "    .long strcmp@GOTPCREL\n"
    "    .long strcoll@GOTPCREL\n"
    "    .long strcpy@GOTPCREL\n"
    "    .long strcspn@GOTPCREL\n"
    "    .long strdup@GOTPCREL\n"
    "    .long strerror@GOTPCREL\n"
    "    .long strerror_r@GOTPCREL\n"
    "    .long strftime@GOTPCREL\n"
    "    .long strlcat@GOTPCREL\n"
    "    .long strlcpy@GOTPCREL\n"
    "    .long strlen@GOTPCREL\n"
    "    .long strncasecmp@GOTPCREL\n"
    "    .long strncmp@GOTPCREL\n"
    "    .long strncpy@GOTPCREL\n"
    "    .long strpbrk@GOTPCREL\n"
    "    .long strrchr@GOTPCREL\n"
    "    .long strspn@GOTPCREL\n"
    "    .long strstr@GOTPCREL\n"
    "    .long strtod@GOTPCREL\n"
    "    .long strtok@GOTPCREL\n"
    "    .long strtok_r@GOTPCREL\n"
    "    .long strtol@GOTPCREL\n"
    "    .long strtoll@GOTPCREL\n"
    "    .long strtoul@GOTPCREL\n"
    "    .long strtoull@GOTPCREL\n"
    "    .long sysconf@GOTPCREL\n"
    "    .long sysctl@GOTPCREL\n"
    "    .long sysctlbyname@GOTPCREL\n"
    "    .long tan@GOTPCREL\n"
    "    .long tanf@GOTPCREL\n"
    "    .long tanh@GOTPCREL\n"
    "    .long time@GOTPCREL\n"
    "    .long tolower@GOTPCREL\n"
    "    .long toupper@GOTPCREL\n"
    "    .long unlink@GOTPCREL\n"
    "    .long utime@GOTPCREL\n"
    "    .long vfprintf@GOTPCREL\n"
    "    .long vsnprintf@GOTPCREL\n"
    "    .long vsprintf@GOTPCREL\n"
    "    .long vsscanf@GOTPCREL\n"
    "    .long wcscmp@GOTPCREL\n"
    "    .long wcslen@GOTPCREL\n"
    "    .long wcsncmp@GOTPCREL\n"
    "    .long wcsstr@GOTPCREL\n"
    "    .long wcstombs@GOTPCREL\n"
    "    .long write@GOTPCREL\n"
    ".text\n");
