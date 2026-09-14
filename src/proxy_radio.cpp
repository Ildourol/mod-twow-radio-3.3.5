#include <windows.h>
#include <stdio.h>
#include "../bass/c/bass.h"

// Original version.dll handle and function pointers
static HMODULE g_hOrigVersion = NULL;

typedef BOOL (WINAPI *tGetFileVersionInfoA)(LPCSTR, DWORD, DWORD, LPVOID);
typedef BOOL (WINAPI *tGetFileVersionInfoByHandle)(int, HANDLE, LPVOID, DWORD);
typedef BOOL (WINAPI *tGetFileVersionInfoExA)(DWORD, LPCSTR, DWORD, DWORD, LPVOID);
typedef BOOL (WINAPI *tGetFileVersionInfoExW)(DWORD, LPCWSTR, DWORD, DWORD, LPVOID);
typedef DWORD (WINAPI *tGetFileVersionInfoSizeA)(LPCSTR, LPDWORD);
typedef DWORD (WINAPI *tGetFileVersionInfoSizeByHandle)(int, HANDLE, LPDWORD);
typedef DWORD (WINAPI *tGetFileVersionInfoSizeExA)(DWORD, LPCSTR, LPDWORD);
typedef DWORD (WINAPI *tGetFileVersionInfoSizeExW)(DWORD, LPCWSTR, LPDWORD);
typedef DWORD (WINAPI *tGetFileVersionInfoSizeW)(LPCWSTR, LPDWORD);
typedef BOOL (WINAPI *tGetFileVersionInfoW)(LPCWSTR, DWORD, DWORD, LPVOID);
typedef DWORD (WINAPI *tVerFindFileA)(DWORD, LPCSTR, LPCSTR, LPCSTR, LPSTR, PUINT, LPSTR, PUINT);
typedef DWORD (WINAPI *tVerFindFileW)(DWORD, LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, PUINT, LPWSTR, PUINT);
typedef DWORD (WINAPI *tVerInstallFileA)(DWORD, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPSTR, PUINT);
typedef DWORD (WINAPI *tVerInstallFileW)(DWORD, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, PUINT);
typedef DWORD (WINAPI *tVerLanguageNameA)(DWORD, LPSTR, DWORD);
typedef DWORD (WINAPI *tVerLanguageNameW)(DWORD, LPWSTR, DWORD);
typedef BOOL (WINAPI *tVerQueryValueA)(LPCVOID, LPCSTR, LPVOID*, PUINT);
typedef BOOL (WINAPI *tVerQueryValueW)(LPCVOID, LPCWSTR, LPVOID*, PUINT);

static tGetFileVersionInfoA oGetFileVersionInfoA = NULL;
static tGetFileVersionInfoByHandle oGetFileVersionInfoByHandle = NULL;
static tGetFileVersionInfoExA oGetFileVersionInfoExA = NULL;
static tGetFileVersionInfoExW oGetFileVersionInfoExW = NULL;
static tGetFileVersionInfoSizeA oGetFileVersionInfoSizeA = NULL;
static tGetFileVersionInfoSizeByHandle oGetFileVersionInfoSizeByHandle = NULL;
static tGetFileVersionInfoSizeExA oGetFileVersionInfoSizeExA = NULL;
static tGetFileVersionInfoSizeExW oGetFileVersionInfoSizeExW = NULL;
static tGetFileVersionInfoSizeW oGetFileVersionInfoSizeW = NULL;
static tGetFileVersionInfoW oGetFileVersionInfoW = NULL;
static tVerFindFileA oVerFindFileA = NULL;
static tVerFindFileW oVerFindFileW = NULL;
static tVerInstallFileA oVerInstallFileA = NULL;
static tVerInstallFileW oVerInstallFileW = NULL;
static tVerLanguageNameA oVerLanguageNameA = NULL;
static tVerLanguageNameW oVerLanguageNameW = NULL;
static tVerQueryValueA oVerQueryValueA = NULL;
static tVerQueryValueW oVerQueryValueW = NULL;

static void LoadOriginalVersionDll()
{
    if (g_hOrigVersion) return;
    char path[MAX_PATH];
    GetSystemDirectoryA(path, MAX_PATH);
    strcat_s(path, MAX_PATH, "\\version.dll");
    g_hOrigVersion = LoadLibraryA(path);
    if (!g_hOrigVersion) return;

    oGetFileVersionInfoA = (tGetFileVersionInfoA)GetProcAddress(g_hOrigVersion, "GetFileVersionInfoA");
    oGetFileVersionInfoByHandle = (tGetFileVersionInfoByHandle)GetProcAddress(g_hOrigVersion, "GetFileVersionInfoByHandle");
    oGetFileVersionInfoExA = (tGetFileVersionInfoExA)GetProcAddress(g_hOrigVersion, "GetFileVersionInfoExA");
    oGetFileVersionInfoExW = (tGetFileVersionInfoExW)GetProcAddress(g_hOrigVersion, "GetFileVersionInfoExW");
    oGetFileVersionInfoSizeA = (tGetFileVersionInfoSizeA)GetProcAddress(g_hOrigVersion, "GetFileVersionInfoSizeA");
    oGetFileVersionInfoSizeByHandle = (tGetFileVersionInfoSizeByHandle)GetProcAddress(g_hOrigVersion, "GetFileVersionInfoSizeByHandle");
    oGetFileVersionInfoSizeExA = (tGetFileVersionInfoSizeExA)GetProcAddress(g_hOrigVersion, "GetFileVersionInfoSizeExA");
    oGetFileVersionInfoSizeExW = (tGetFileVersionInfoSizeExW)GetProcAddress(g_hOrigVersion, "GetFileVersionInfoSizeExW");
    oGetFileVersionInfoSizeW = (tGetFileVersionInfoSizeW)GetProcAddress(g_hOrigVersion, "GetFileVersionInfoSizeW");
    oGetFileVersionInfoW = (tGetFileVersionInfoW)GetProcAddress(g_hOrigVersion, "GetFileVersionInfoW");
    oVerFindFileA = (tVerFindFileA)GetProcAddress(g_hOrigVersion, "VerFindFileA");
    oVerFindFileW = (tVerFindFileW)GetProcAddress(g_hOrigVersion, "VerFindFileW");
    oVerInstallFileA = (tVerInstallFileA)GetProcAddress(g_hOrigVersion, "VerInstallFileA");
    oVerInstallFileW = (tVerInstallFileW)GetProcAddress(g_hOrigVersion, "VerInstallFileW");
    oVerLanguageNameA = (tVerLanguageNameA)GetProcAddress(g_hOrigVersion, "VerLanguageNameA");
    oVerLanguageNameW = (tVerLanguageNameW)GetProcAddress(g_hOrigVersion, "VerLanguageNameW");
    oVerQueryValueA = (tVerQueryValueA)GetProcAddress(g_hOrigVersion, "VerQueryValueA");
    oVerQueryValueW = (tVerQueryValueW)GetProcAddress(g_hOrigVersion, "VerQueryValueW");
}

extern "C" {
BOOL WINAPI Proxy_GetFileVersionInfoA(LPCSTR a, DWORD b, DWORD c, LPVOID d) { LoadOriginalVersionDll(); return oGetFileVersionInfoA ? oGetFileVersionInfoA(a, b, c, d) : FALSE; }
BOOL WINAPI Proxy_GetFileVersionInfoByHandle(int a, HANDLE b, LPVOID c, DWORD d) { LoadOriginalVersionDll(); return oGetFileVersionInfoByHandle ? oGetFileVersionInfoByHandle(a, b, c, d) : FALSE; }
BOOL WINAPI Proxy_GetFileVersionInfoExA(DWORD a, LPCSTR b, DWORD c, DWORD d, LPVOID e) { LoadOriginalVersionDll(); return oGetFileVersionInfoExA ? oGetFileVersionInfoExA(a, b, c, d, e) : FALSE; }
BOOL WINAPI Proxy_GetFileVersionInfoExW(DWORD a, LPCWSTR b, DWORD c, DWORD d, LPVOID e) { LoadOriginalVersionDll(); return oGetFileVersionInfoExW ? oGetFileVersionInfoExW(a, b, c, d, e) : FALSE; }
DWORD WINAPI Proxy_GetFileVersionInfoSizeA(LPCSTR a, LPDWORD b) { LoadOriginalVersionDll(); return oGetFileVersionInfoSizeA ? oGetFileVersionInfoSizeA(a, b) : 0; }
DWORD WINAPI Proxy_GetFileVersionInfoSizeByHandle(int a, HANDLE b, LPDWORD c) { LoadOriginalVersionDll(); return oGetFileVersionInfoSizeByHandle ? oGetFileVersionInfoSizeByHandle(a, b, c) : 0; }
DWORD WINAPI Proxy_GetFileVersionInfoSizeExA(DWORD a, LPCSTR b, LPDWORD c) { LoadOriginalVersionDll(); return oGetFileVersionInfoSizeExA ? oGetFileVersionInfoSizeExA(a, b, c) : 0; }
DWORD WINAPI Proxy_GetFileVersionInfoSizeExW(DWORD a, LPCWSTR b, LPDWORD c) { LoadOriginalVersionDll(); return oGetFileVersionInfoSizeExW ? oGetFileVersionInfoSizeExW(a, b, c) : 0; }
DWORD WINAPI Proxy_GetFileVersionInfoSizeW(LPCWSTR a, LPDWORD b) { LoadOriginalVersionDll(); return oGetFileVersionInfoSizeW ? oGetFileVersionInfoSizeW(a, b) : 0; }
BOOL WINAPI Proxy_GetFileVersionInfoW(LPCWSTR a, DWORD b, DWORD c, LPVOID d) { LoadOriginalVersionDll(); return oGetFileVersionInfoW ? oGetFileVersionInfoW(a, b, c, d) : FALSE; }
DWORD WINAPI Proxy_VerFindFileA(DWORD a, LPCSTR b, LPCSTR c, LPCSTR d, LPSTR e, PUINT f, LPSTR g, PUINT h) { LoadOriginalVersionDll(); return oVerFindFileA ? oVerFindFileA(a, b, c, d, e, f, g, h) : 0; }
DWORD WINAPI Proxy_VerFindFileW(DWORD a, LPCWSTR b, LPCWSTR c, LPCWSTR d, LPWSTR e, PUINT f, LPWSTR g, PUINT h) { LoadOriginalVersionDll(); return oVerFindFileW ? oVerFindFileW(a, b, c, d, e, f, g, h) : 0; }
DWORD WINAPI Proxy_VerInstallFileA(DWORD a, LPCSTR b, LPCSTR c, LPCSTR d, LPCSTR e, LPCSTR f, LPSTR g, PUINT h) { LoadOriginalVersionDll(); return oVerInstallFileA ? oVerInstallFileA(a, b, c, d, e, f, g, h) : 0; }
DWORD WINAPI Proxy_VerInstallFileW(DWORD a, LPCWSTR b, LPCWSTR c, LPCWSTR d, LPCWSTR e, LPCWSTR f, LPWSTR g, PUINT h) { LoadOriginalVersionDll(); return oVerInstallFileW ? oVerInstallFileW(a, b, c, d, e, f, g, h) : 0; }
DWORD WINAPI Proxy_VerLanguageNameA(DWORD a, LPSTR b, DWORD c) { LoadOriginalVersionDll(); return oVerLanguageNameA ? oVerLanguageNameA(a, b, c) : 0; }
DWORD WINAPI Proxy_VerLanguageNameW(DWORD a, LPWSTR b, DWORD c) { LoadOriginalVersionDll(); return oVerLanguageNameW ? oVerLanguageNameW(a, b, c) : 0; }
BOOL WINAPI Proxy_VerQueryValueA(LPCVOID a, LPCSTR b, LPVOID* c, PUINT d) { LoadOriginalVersionDll(); return oVerQueryValueA ? oVerQueryValueA(a, b, c, d) : FALSE; }
BOOL WINAPI Proxy_VerQueryValueW(LPCVOID a, LPCWSTR b, LPVOID* c, PUINT d) { LoadOriginalVersionDll(); return oVerQueryValueW ? oVerQueryValueW(a, b, c, d) : FALSE; }
}

// -------------------------------------------------------------
// WoW 3.3.5a (12340) Lua & Audio Engine Hook
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

static void InitBass()
{
    if (g_BassInitialized) return;
    if (BASS_Init(-1, 44100, 0, NULL, NULL)) {
        BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 1);
        BASS_SetConfig(BASS_CONFIG_NET_PREBUF, 0);
        g_BassInitialized = true;
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
    // BASS_StreamCreateURL(url, offset, flags, proc, user)
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

// -------------------------------------------------------------
// Hook FrameScript_RegisterFunction
// -------------------------------------------------------------
static BYTE g_OrigBytes[5] = {0};
static bool g_Hooked = false;
static bool g_Registered = false;

static void RegisterCustomFunctions()
{
    if (g_Registered) return;
    g_Registered = true;
    pFrameScript_RegisterFunction("PlayRadioStream", Lua_PlayRadioStream);
    pFrameScript_RegisterFunction("StopRadioStream", Lua_StopRadioStream);
    pFrameScript_RegisterFunction("SetRadioVolume", Lua_SetRadioVolume);
    pFrameScript_RegisterFunction("GetRadioVolume", Lua_GetRadioVolume);
    pFrameScript_RegisterFunction("IsRadioPlaying", Lua_IsRadioPlaying);
    pFrameScript_RegisterFunction("GetRadioTitle", Lua_GetRadioTitle);
}

static void __cdecl Hooked_FrameScript_RegisterFunction(const char* name, lua_CFunction func)
{
    DWORD oldProtect;
    VirtualProtect((LPVOID)0x00817F90, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((LPVOID)0x00817F90, g_OrigBytes, 5);

    pFrameScript_RegisterFunction(name, func);

    RegisterCustomFunctions();

    BYTE jump[5] = { 0xE9, 0, 0, 0, 0 };
    DWORD offset = (DWORD)Hooked_FrameScript_RegisterFunction - ((DWORD)0x00817F90 + 5);
    memcpy(&jump[1], &offset, 4);
    memcpy((LPVOID)0x00817F90, jump, 5);
    VirtualProtect((LPVOID)0x00817F90, 5, oldProtect, &oldProtect);
}

static void InstallHook()
{
    if (g_Hooked) return;
    DWORD oldProtect;
    if (VirtualProtect((LPVOID)0x00817F90, 5, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(g_OrigBytes, (LPVOID)0x00817F90, 5);
        BYTE jump[5] = { 0xE9, 0, 0, 0, 0 };
        DWORD offset = (DWORD)Hooked_FrameScript_RegisterFunction - ((DWORD)0x00817F90 + 5);
        memcpy(&jump[1], &offset, 4);
        memcpy((LPVOID)0x00817F90, jump, 5);
        VirtualProtect((LPVOID)0x00817F90, 5, oldProtect, &oldProtect);
        g_Hooked = true;
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        LoadOriginalVersionDll();
        InstallHook();
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
