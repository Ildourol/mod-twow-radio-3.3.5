#include <windows.h>
#include <stdio.h>
#include "../bass/c/bass.h"

// -------------------------------------------------------------
// DirectInput8 Proxy
// -------------------------------------------------------------
static HMODULE g_hOrigDinput8 = NULL;
typedef HRESULT (WINAPI *tDirectInput8Create)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
typedef HRESULT (WINAPI *tDllCanUnloadNow)();
typedef HRESULT (WINAPI *tDllGetClassObject)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (WINAPI *tDllRegisterServer)();
typedef HRESULT (WINAPI *tDllUnregisterServer)();
typedef void* (WINAPI *tGetdfDIJoystick)();

static tDirectInput8Create oDirectInput8Create = NULL;
static tDllCanUnloadNow oDllCanUnloadNow = NULL;
static tDllGetClassObject oDllGetClassObject = NULL;
static tDllRegisterServer oDllRegisterServer = NULL;
static tDllUnregisterServer oDllUnregisterServer = NULL;
static tGetdfDIJoystick oGetdfDIJoystick = NULL;

static void LoadOriginalDinput8()
{
    if (g_hOrigDinput8) return;
    char path[MAX_PATH];
    GetSystemDirectoryA(path, MAX_PATH);
    strcat_s(path, MAX_PATH, "\\dinput8.dll");
    g_hOrigDinput8 = LoadLibraryA(path);
    if (!g_hOrigDinput8) return;

    oDirectInput8Create = (tDirectInput8Create)GetProcAddress(g_hOrigDinput8, "DirectInput8Create");
    oDllCanUnloadNow = (tDllCanUnloadNow)GetProcAddress(g_hOrigDinput8, "DllCanUnloadNow");
    oDllGetClassObject = (tDllGetClassObject)GetProcAddress(g_hOrigDinput8, "DllGetClassObject");
    oDllRegisterServer = (tDllRegisterServer)GetProcAddress(g_hOrigDinput8, "DllRegisterServer");
    oDllUnregisterServer = (tDllUnregisterServer)GetProcAddress(g_hOrigDinput8, "DllUnregisterServer");
    oGetdfDIJoystick = (tGetdfDIJoystick)GetProcAddress(g_hOrigDinput8, "GetdfDIJoystick");
}

extern "C" {
HRESULT WINAPI Proxy_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
    LoadOriginalDinput8();
    if (oDirectInput8Create)
        return oDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    return E_FAIL;
}

HRESULT WINAPI Proxy_DllCanUnloadNow()
{
    LoadOriginalDinput8();
    return oDllCanUnloadNow ? oDllCanUnloadNow() : S_OK;
}

HRESULT WINAPI Proxy_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    LoadOriginalDinput8();
    return oDllGetClassObject ? oDllGetClassObject(rclsid, riid, ppv) : CLASS_E_CLASSNOTAVAILABLE;
}

HRESULT WINAPI Proxy_DllRegisterServer()
{
    LoadOriginalDinput8();
    return oDllRegisterServer ? oDllRegisterServer() : S_OK;
}

HRESULT WINAPI Proxy_DllUnregisterServer()
{
    LoadOriginalDinput8();
    return oDllUnregisterServer ? oDllUnregisterServer() : S_OK;
}

void* WINAPI Proxy_GetdfDIJoystick()
{
    LoadOriginalDinput8();
    return oGetdfDIJoystick ? oGetdfDIJoystick() : NULL;
}
}

// -------------------------------------------------------------
// WoW 3.3.5a (12340) Lua & Audio Engine
// -------------------------------------------------------------
typedef int (__cdecl *lua_CFunction)(void* L);
typedef void (__cdecl *tFrameScript_RegisterFunction)(const char* name, lua_CFunction func);
typedef const char* (__cdecl *tlua_tolstring)(void* L, int idx, size_t* len);
typedef double (__cdecl *tlua_tonumber)(void* L, int idx);
typedef void (__cdecl *tlua_pushboolean)(void* L, int b);
typedef void (__cdecl *tlua_pushnumber)(void* L, double n);
typedef void (__cdecl *tlua_pushstring)(void* L, const char* s);

static tFrameScript_RegisterFunction pFrameScript_RegisterFunction = (tFrameScript_RegisterFunction)0x00817F90;
static tlua_tolstring p_lua_tolstring = (tlua_tolstring)0x0084E0E0;
static tlua_tonumber p_lua_tonumber = (tlua_tonumber)0x0084E030;
static tlua_pushboolean p_lua_pushboolean = (tlua_pushboolean)0x0084E300;
static tlua_pushnumber p_lua_pushnumber = (tlua_pushnumber)0x0084E2A0;
static tlua_pushstring p_lua_pushstring = (tlua_pushstring)0x0084E350;

static HSTREAM g_RadioStream = 0;
static float g_RadioVolume = 0.5f;
static bool g_BassInitialized = false;
static char g_CurrentTitle[256] = {0};

static void LogMsg(const char* msg)
{
    FILE* f = fopen("radio_debug.log", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

static void InitBass()
{
    if (g_BassInitialized) return;
    if (BASS_Init(-1, 44100, 0, NULL, NULL)) {
        BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 1);
        BASS_SetConfig(BASS_CONFIG_NET_PREBUF, 0);
        g_BassInitialized = true;
        LogMsg("BASS initialized successfully.");
    } else {
        char err[64];
        sprintf_s(err, sizeof(err), "BASS_Init failed, code: %d", BASS_ErrorGetCode());
        LogMsg(err);
    }
}

static void CALLBACK MetaSync(HSYNC handle, DWORD channel, DWORD data, void* user)
{
    const char* meta = BASS_ChannelGetTags(channel, BASS_TAG_META);
    if (meta) {
        const char* p = strstr(meta, "StreamTitle='");
        if (p) {
            p += 13;
            const char* end = strstr(p, "';");
            if (end) {
                int len = (int)(end - p);
                if (len > 255) len = 255;
                strncpy_s(g_CurrentTitle, sizeof(g_CurrentTitle), p, len);
                g_CurrentTitle[len] = '\0';
            }
        }
    }
}

// Lua: PlayRadioStream("http://...")
static int Lua_PlayRadioStream(void* L)
{
    InitBass();
    size_t len = 0;
    const char* url = p_lua_tolstring(L, 1, &len);
    if (!url || len == 0) {
        p_lua_pushboolean(L, 0);
        return 1;
    }

    if (g_RadioStream) {
        BASS_ChannelStop(g_RadioStream);
        BASS_StreamFree(g_RadioStream);
        g_RadioStream = 0;
    }

    g_CurrentTitle[0] = '\0';
    g_RadioStream = BASS_StreamCreateURL(url, 0, BASS_STREAM_AUTOFREE | BASS_STREAM_BLOCK | BASS_SAMPLE_FLOAT, NULL, NULL);
    if (g_RadioStream) {
        BASS_ChannelSetAttribute(g_RadioStream, BASS_ATTRIB_VOL, g_RadioVolume);
        BASS_ChannelSetSync(g_RadioStream, BASS_SYNC_META, 0, MetaSync, 0);
        BASS_ChannelPlay(g_RadioStream, FALSE);
        p_lua_pushboolean(L, 1);
    } else {
        p_lua_pushboolean(L, 0);
    }
    return 1;
}

// Lua: StopRadioStream()
static int Lua_StopRadioStream(void* L)
{
    if (g_RadioStream) {
        BASS_ChannelStop(g_RadioStream);
        BASS_StreamFree(g_RadioStream);
        g_RadioStream = 0;
    }
    g_CurrentTitle[0] = '\0';
    return 0;
}

// Lua: SetRadioVolume(0.0 - 1.0)
static int Lua_SetRadioVolume(void* L)
{
    double vol = p_lua_tonumber(L, 1);
    if (vol < 0.0) vol = 0.0;
    if (vol > 1.0) vol = 1.0;
    g_RadioVolume = (float)vol;
    if (g_RadioStream) {
        BASS_ChannelSetAttribute(g_RadioStream, BASS_ATTRIB_VOL, g_RadioVolume);
    }
    return 0;
}

// Lua: GetRadioVolume()
static int Lua_GetRadioVolume(void* L)
{
    p_lua_pushnumber(L, g_RadioVolume);
    return 1;
}

// Lua: IsRadioPlaying()
static int Lua_IsRadioPlaying(void* L)
{
    int playing = 0;
    if (g_RadioStream) {
        DWORD act = BASS_ChannelIsActive(g_RadioStream);
        playing = (act == BASS_ACTIVE_PLAYING || act == BASS_ACTIVE_STALLED) ? 1 : 0;
    }
    p_lua_pushboolean(L, playing);
    return 1;
}

// Lua: GetRadioTitle()
static int Lua_GetRadioTitle(void* L)
{
    p_lua_pushstring(L, g_CurrentTitle);
    return 1;
}

// Background thread waiting for WoW UI/Lua state to initialize
static DWORD WINAPI LuaRegisterThread(LPVOID)
{
    LogMsg("Radio Thread started, waiting for WoW Lua state (0x00D3F78C)...");

    void** pLuaState = (void**)0x00D3F78C;
    int tries = 0;
    while (!*pLuaState && tries < 600) { // wait up to 30 seconds
        Sleep(50);
        tries++;
    }

    if (!*pLuaState) {
        LogMsg("Timeout waiting for Lua state!");
        return 0;
    }

    LogMsg("Lua state initialized! Registering functions...");
    // Allow an extra 300ms for default UI frames to settle
    Sleep(300);

    pFrameScript_RegisterFunction("PlayRadioStream", Lua_PlayRadioStream);
    pFrameScript_RegisterFunction("StopRadioStream", Lua_StopRadioStream);
    pFrameScript_RegisterFunction("SetRadioVolume", Lua_SetRadioVolume);
    pFrameScript_RegisterFunction("GetRadioVolume", Lua_GetRadioVolume);
    pFrameScript_RegisterFunction("IsRadioPlaying", Lua_IsRadioPlaying);
    pFrameScript_RegisterFunction("GetRadioTitle", Lua_GetRadioTitle);

    LogMsg("SUCCESS: All radio Lua functions registered into WoW engine!");
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        LogMsg("=== Radio Proxy DLL Attached to Process ===");
        LoadOriginalDinput8();
        CreateThread(NULL, 0, LuaRegisterThread, NULL, 0, NULL);
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        if (g_RadioStream) {
            BASS_ChannelStop(g_RadioStream);
            BASS_StreamFree(g_RadioStream);
            g_RadioStream = 0;
        }
        if (g_BassInitialized) {
            BASS_Free();
            g_BassInitialized = false;
        }
    }
    return TRUE;
}
