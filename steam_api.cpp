/* This is a shim Winelib/Windows library that relays libsteam_api.so exports from the
   Linux version of the Steam API so that Windows programs that depend on it
   *under Wine* can work with the current user without a full win32 Steam.

   MOCKUP:

   native linux                                  running under wine
   --------------------------------------------  ------------------------------------------------------
   {Steam Client on Linux} <=> {libsteam_api.so} <=> {steam_api.dll.so} <=> {mbw_workshop_uploader.exe}


   INSTRUCTIONS:

   1. Compile with `$ winegcc steam_api.cpp -shared -mwindows -m32 -o steam_api`

   2. Place the resulting steam_api.dll.so, plus libsteam_api.so and mbw_workshop_uploader.exe in the same folder.

   3. Call it a day. :)

   ADDITIONAL INFO:

   * https://www.winehq.org/docs/winelib-guide/bindlls

   Developed for Mount&Blade Warband's mbw_workshop_uploader.exe, which is Windows-only :( */

/* (C) 2016-03-06 Created by Swyter <swyterzone+steam_api@gmail.com>
       Licensed under MIT-like terms, this is a FOSS program. */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "windef.h"
#include "winbase.h"

#include "windows.h"
#include "wine/library.h"

#include <dlfcn.h>

static void *libsteam_api = NULL;


extern "C"
{

#define  AKE_FUNCPTR(t, f) t (* p##f)() = NULL; t WINAPI f();
#define MAKE_FUNCPTR(t, f) t (* p##f)() = NULL; t WINAPI f()              \
{                                                                         \
    printf("\n=> [i] called %s (%p), result up next -> \n\n", #f, p##f);  \
    t result = p##f();                                                    \
    printf("\n=> [-]  ended %s (%p), return %#x\n\n", #f, p##f, result);  \
    return result;                                                        \
}

#define MAKE_VOIDPTR(t, f) t (* p##f)() = NULL;                           \
t WINAPI f()                                                              \
{                                                                         \
    printf("\n=> [i] called void function %s (%p),\n\n", #f, p##f);       \
    p##f();                                                               \
}

#define CALB_FUNCPTR(t, f) t (* p##f)(int *, ulong) = NULL;               \
t WINAPI f(int *pCallback, ulong hAPICall)                                \
{                                                                         \
    printf("\n=> [i] callb function %s (%p | %x)\n\n", #f, &p##f, p##f);  \
    p##f(pCallback, hAPICall);                                            \
}

#define SAKE_FUNCPTR(t, f) t (* p##f)() = NULL;                           \
t WINAPI f()                                                                                        \
{                                                                                                                \
    printf("\n=> [i] called %s (%p | %x)\n\n", #f, &p##f, p##f);                                        \
    t result = p##f();                                                                        \
    printf("\n=> [-]  ended %s (%p), return %#x\n\n", #f, p##f, result);                                        \
    return result;                                                                                                                \
}

MAKE_FUNCPTR(bool,   SteamAPI_Init)
MAKE_VOIDPTR(void,   SteamAPI_RunCallbacks)
MAKE_VOIDPTR(void,   SteamAPI_Shutdown)

CALB_FUNCPTR(void,   SteamAPI_UnregisterCallResult)
CALB_FUNCPTR(void,   SteamAPI_RegisterCallResult)

SAKE_FUNCPTR(int, SteamUGC)
SAKE_FUNCPTR(int, SteamUser)
SAKE_FUNCPTR(int, SteamRemoteStorage)
SAKE_FUNCPTR(int, SteamUtils)
SAKE_FUNCPTR(int, SteamClient);


#undef MAKE_FUNCPTR

    BOOL __attribute__ ((visibility ("default"))) WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
    {
        switch (reason)
        {
            case DLL_PROCESS_ATTACH:
            {
                DisableThreadLibraryCalls(instance);

                libsteam_api = wine_dlopen("./libsteam_api.so", RTLD_NOW, NULL, 0);

                if (libsteam_api)
                {

                    printf("=> [!] loading steam_api shim/relay (libsteam_api.so base_addr: %p) (%p, %d, %p)\n", libsteam_api, instance, reason, reserved);

                    #define LOAD_FUNCPTR(f)                                                                                 \
                        if (p##f = wine_dlsym(libsteam_api, #f, NULL, 0))                                                   \
                        {                                                                                                   \
                            printf("=> redirected symbolic export (%s <= %p)\n", #f, p##f);                                 \
                        }                                                                                                   \
                        else                                                                                                \
                        {                                                                                                   \
                            MessageBoxA(NULL, "Some of the function pointers couldn't be resolved from "                    \
                                              "the steam_api.dll library, probably a new version.", #f, MB_OK|MB_ICONSTOP); \
                            return FALSE;                                                                                   \
                        }

                    LOAD_FUNCPTR(SteamAPI_Init);
                    LOAD_FUNCPTR(SteamUGC);
                    LOAD_FUNCPTR(SteamAPI_RunCallbacks);
                    LOAD_FUNCPTR(SteamAPI_Shutdown);
                    LOAD_FUNCPTR(SteamUser);
                    LOAD_FUNCPTR(SteamRemoteStorage);
                    LOAD_FUNCPTR(SteamUtils);
                    LOAD_FUNCPTR(SteamAPI_UnregisterCallResult);
                    LOAD_FUNCPTR(SteamClient);
                    LOAD_FUNCPTR(SteamAPI_RegisterCallResult);

                    #undef LOAD_FUNCPTR

                    return TRUE;

                }
                else
                    MessageBoxA(NULL, "Ensure that libsteam_api.so is in the same folder "
                                      "than steam_api.dll so that the relay works.", "Um...", MB_OK|MB_ICONSTOP);

                break;
            }

            case DLL_PROCESS_DETACH:
            {
                printf("=> unloading steam_api shim/relay (%p, %d, %p)\n", instance, reason, reserved);
                wine_dlclose(libsteam_api, NULL, 0);
                break;
            }
        }

        return TRUE;
    }

}
