/* shims_sdl.c - imports SDL2 do pico8_dyn -> port PS5 do SDL2
 * Copyright (C) 2026 RafaelNGP
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * A ABI publica do SDL2 eh estavel desde a 2.0, entao o pico8 (feito
 * contra SDL 2.0.x do Linux) chama direto o SDL 2.30 do PS5
 * (ps5-payload-dev/SDL, compilado por deps/build_sdl2.sh). So algumas
 * funcoes passam por um wrapper que registra no log o que o pico8 pediu.
 */

#include <SDL.h>

#include "loader.h"

static int sdl_Init(Uint32 flags)
{
    int r = SDL_Init(flags);
    lg("SDL_Init(0x%x) = %d %s", flags, r, r ? SDL_GetError() : "");
    lg("  video: %s, audio: %s", SDL_GetCurrentVideoDriver(),
       SDL_GetCurrentAudioDriver());
    return r;
}

static SDL_Window *sdl_CreateWindow(const char *title, int x, int y, int w,
                                    int h, Uint32 flags)
{
    SDL_Window *win = SDL_CreateWindow(title, x, y, w, h, flags);
    lg("SDL_CreateWindow(\"%s\", %dx%d, 0x%x) = %p %s", title, w, h, flags,
       (void *)win, win ? "" : SDL_GetError());
    return win;
}

static SDL_Renderer *sdl_CreateRenderer(SDL_Window *win, int idx, Uint32 flags)
{
    SDL_Renderer *r = SDL_CreateRenderer(win, idx, flags);
    SDL_RendererInfo info;

    if (r && SDL_GetRendererInfo(r, &info) == 0)
        lg("SDL_CreateRenderer(%d, 0x%x) = %s", idx, flags, info.name);
    else
        lg("SDL_CreateRenderer(%d, 0x%x) falhou: %s", idx, flags,
           SDL_GetError());
    return r;
}

static int sdl_OpenAudio(SDL_AudioSpec *want, SDL_AudioSpec *have)
{
    int r = SDL_OpenAudio(want, have);
    lg("SDL_OpenAudio(%d Hz, fmt 0x%x, %d ch, %d samples) = %d %s",
       want->freq, want->format, want->channels, want->samples, r,
       r ? SDL_GetError() : "");
    return r;
}

static const char *sdl_GetError(void)
{
    const char *e = SDL_GetError();
    if (e && *e)
        lg("SDL_GetError: %s", e);
    return e;
}

/* Diagnostico do controle: o que o SDL detecta e o que o pico8 abre. */
static int sdl_NumJoysticks(void)
{
    static int last = -1;
    int n = SDL_NumJoysticks();

    if (n != last) {
        lg("SDL_NumJoysticks = %d %s", n, n < 0 ? SDL_GetError() : "");
        for (int i = 0; i < n; i++) {
            char guid[33];
            SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(i), guid,
                                      sizeof(guid));
            lg("  joy %d: \"%s\" guid=%s gamecontroller=%d", i,
               SDL_JoystickNameForIndex(i), guid, SDL_IsGameController(i));
        }
        last = n;
    }
    return n;
}

static SDL_GameController *sdl_GameControllerOpen(int i)
{
    SDL_GameController *gc = SDL_GameControllerOpen(i);
    lg("SDL_GameControllerOpen(%d) = %p %s", i, (void *)gc,
       gc ? "" : SDL_GetError());
    return gc;
}

static SDL_Joystick *sdl_JoystickOpen(int i)
{
    SDL_Joystick *j = SDL_JoystickOpen(i);
    lg("SDL_JoystickOpen(%d) = %p %s", i, (void *)j, j ? "" : SDL_GetError());
    return j;
}

#define D(n)    { #n, (void *)n }
#define S(n, f) { n, (void *)f }

const shim_t shims_sdl[] = {
    S("SDL_Init", sdl_Init),
    D(SDL_SetHint),   /* chamado todo frame; sem log */
    S("SDL_CreateWindow", sdl_CreateWindow),
    S("SDL_CreateRenderer", sdl_CreateRenderer),
    S("SDL_OpenAudio", sdl_OpenAudio),
    S("SDL_GetError", sdl_GetError),
    S("SDL_NumJoysticks", sdl_NumJoysticks),
    S("SDL_GameControllerOpen", sdl_GameControllerOpen),
    S("SDL_JoystickOpen", sdl_JoystickOpen),
    D(SDL_PollEvent),
    D(SDL_ClearError),
    D(SDL_CreateRGBSurfaceFrom),
    D(SDL_CreateTexture),
    D(SDL_CreateThread),
    D(SDL_Delay),
    D(SDL_DestroyRenderer),
    D(SDL_DestroyTexture),
    D(SDL_DestroyWindow),
    D(SDL_DetachThread),
    D(SDL_free),
    D(SDL_FreeSurface),
    D(SDL_GameControllerAddMapping),
    D(SDL_GameControllerClose),
    D(SDL_GameControllerGetAttached),
    D(SDL_GameControllerGetAxis),
    D(SDL_GameControllerGetButton),
    D(SDL_GameControllerGetJoystick),
    D(SDL_GameControllerMapping),
    D(SDL_GameControllerNameForIndex),
    D(SDL_GetAudioDriver),
    D(SDL_GetClipboardText),
    D(SDL_GetCurrentAudioDriver),
    D(SDL_GetCurrentDisplayMode),
    D(SDL_GetCurrentVideoDriver),
    D(SDL_GetDesktopDisplayMode),
    D(SDL_GetDisplayBounds),
    D(SDL_GetDisplayMode),
    D(SDL_GetDisplayName),
    D(SDL_GetKeyboardFocus),
    D(SDL_GetKeyboardState),
    D(SDL_GetKeyFromScancode),
    D(SDL_GetModState),
    D(SDL_GetMouseState),
    D(SDL_GetNumAudioDrivers),
    D(SDL_GetNumDisplayModes),
    D(SDL_GetNumRenderDrivers),
    D(SDL_GetNumVideoDisplays),
    D(SDL_GetNumVideoDrivers),
    D(SDL_GetRenderDriverInfo),
    D(SDL_GetRendererInfo),
    D(SDL_GetScancodeName),
    D(SDL_GetTicks),
    D(SDL_GetVersion),
    D(SDL_GetVideoDriver),
    D(SDL_GetWindowDisplayIndex),
    D(SDL_GetWindowID),
    D(SDL_GetWindowPosition),
    D(SDL_GetWindowSize),
    D(SDL_GetWindowSurface),
    D(SDL_GL_CreateContext),
    D(SDL_GL_DeleteContext),
    D(SDL_GL_SetAttribute),
    D(SDL_HasClipboardText),
    D(SDL_InitSubSystem),
    D(SDL_IsGameController),
    D(SDL_JoystickClose),
    D(SDL_JoystickGetAxis),
    D(SDL_JoystickGetButton),
    D(SDL_JoystickInstanceID),
    D(SDL_JoystickNameForIndex),
    D(SDL_JoystickNumAxes),
    D(SDL_JoystickNumBalls),
    D(SDL_JoystickNumButtons),
    D(SDL_LockAudio),
    D(SDL_LockSurface),
    D(SDL_PauseAudio),
    D(SDL_RaiseWindow),
    D(SDL_RenderClear),
    D(SDL_RenderCopy),
    D(SDL_RenderPresent),
    D(SDL_SetClipboardText),
    D(SDL_SetRenderDrawColor),
    D(SDL_SetWindowGrab),
    D(SDL_SetWindowIcon),
    D(SDL_SetWindowTitle),
    D(SDL_ShowCursor),
    D(SDL_UnlockAudio),
    D(SDL_UnlockSurface),
    D(SDL_UpdateTexture),
    D(SDL_UpdateWindowSurface),
    D(SDL_WarpMouseInWindow),
    { NULL, NULL }
};

/* O SDL aloca pelo heap proprio (p8_alloc.c). Precisa vir antes de
 * qualquer alocacao do SDL, ou seja, antes do pico8 rodar. */
void shims_sdl_init(void)
{
    int r = SDL_SetMemoryFunctions(p8_malloc, p8_calloc, p8_realloc, p8_free);
    lg("SDL_SetMemoryFunctions = %d", r);
}
