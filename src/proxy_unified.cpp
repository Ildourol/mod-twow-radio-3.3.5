#include <windows.h>
#include <stdio.h>
#include "../bass/c/bass.h"

// -------------------------------------------------------------
// Logging
// -------------------------------------------------------------
static void LogMsg(const char* msg)
{
    FILE* f = fopen("radio_debug.log", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

// -------------------------------------------------------------
// DirectInput8 Proxy Exports
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
// Version.dll Proxy Exports
// -------------------------------------------------------------
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
// WoW 3.3.5a (12340) Lua & Audio Engine
// -------------------------------------------------------------
typedef int (__cdecl *lua_CFunction)(void* L);
typedef void (__cdecl *tFrameScript_RegisterFunction)(const char* name, lua_CFunction func);
typedef const char* (__cdecl *tlua_tolstring)(void* L, int idx, size_t* len);
typedef double (__cdecl *tlua_tonumber)(void* L, int idx);
typedef void (__cdecl *tlua_pushboolean)(void* L, int b);
typedef void (__cdecl *tlua_pushstring)(void* L, const char* s);

static tFrameScript_RegisterFunction pFrameScript_RegisterFunction = (tFrameScript_RegisterFunction)0x00817F90;
static tlua_tolstring p_lua_tolstring = (tlua_tolstring)0x0084E0E0;
static tlua_tonumber p_lua_tonumber = (tlua_tonumber)0x0084E030;
static tlua_pushboolean p_lua_pushboolean = (tlua_pushboolean)0x0084E4D0; // Verified 0x0084E4D0 in 12340!
static tlua_pushstring p_lua_pushstring = (tlua_pushstring)0x0084E350;   // Verified 0x0084E350 in 12340!

static HSTREAM g_RadioStream = 0;
static float g_RadioVolume = 0.5f;
static bool g_BassInitialized = false;
static char g_CurrentTitle[256] = {0};
static CRITICAL_SECTION g_TitleLock;

static bool g_AutoPause = true;
static bool g_WasMinimized = false;
static bool g_PausedByMinimize = false;
static DWORD g_MinimizeTimestamp = 0;
static char g_LastStreamUrl[512] = {0};
static HWND g_hWoWWnd = NULL;

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
        sprintf_s(err, sizeof(err), "BASS_Init code: %d", BASS_ErrorGetCode());
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
                EnterCriticalSection(&g_TitleLock);
                strncpy_s(g_CurrentTitle, sizeof(g_CurrentTitle), p, len);
                g_CurrentTitle[len] = '\0';
                LeaveCriticalSection(&g_TitleLock);
            }
        }
    }
}

// Lua: PlayRadioStream("http://...")
// Returns 0 values so it never touches push functions during playback
static int Lua_PlayRadioStream(void* L)
{
    InitBass();
    size_t len = 0;
    const char* url = p_lua_tolstring(L, 1, &len);
    if (!url || len == 0) {
        return 0;
    }

    strncpy_s(g_LastStreamUrl, sizeof(g_LastStreamUrl), url, _TRUNCATE);
    g_PausedByMinimize = false;
    g_WasMinimized = false;

    if (g_RadioStream) {
        BASS_ChannelStop(g_RadioStream);
        BASS_StreamFree(g_RadioStream);
        g_RadioStream = 0;
    }

    EnterCriticalSection(&g_TitleLock);
    g_CurrentTitle[0] = '\0';
    LeaveCriticalSection(&g_TitleLock);

    g_RadioStream = BASS_StreamCreateURL(url, 0, BASS_STREAM_AUTOFREE | BASS_SAMPLE_FLOAT, NULL, NULL);
    if (g_RadioStream) {
        BASS_ChannelSetAttribute(g_RadioStream, BASS_ATTRIB_VOL, g_RadioVolume);
        BASS_ChannelSetSync(g_RadioStream, BASS_SYNC_META, 0, MetaSync, 0);
        BASS_ChannelPlay(g_RadioStream, FALSE);
    }
    return 0;
}

// Lua: StopRadioStream()
static int Lua_StopRadioStream(void* L)
{
    g_PausedByMinimize = false;
    g_WasMinimized = false;
    g_LastStreamUrl[0] = '\0';

    if (g_RadioStream) {
        BASS_ChannelStop(g_RadioStream);
        BASS_StreamFree(g_RadioStream);
        g_RadioStream = 0;
    }
    EnterCriticalSection(&g_TitleLock);
    g_CurrentTitle[0] = '\0';
    LeaveCriticalSection(&g_TitleLock);
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
    char titleCopy[256] = {0};
    EnterCriticalSection(&g_TitleLock);
    strcpy_s(titleCopy, sizeof(titleCopy), g_CurrentTitle);
    LeaveCriticalSection(&g_TitleLock);
    p_lua_pushstring(L, titleCopy);
    return 1;
}

// Lua: SetRadioAutoPause(enable)
static int Lua_SetRadioAutoPause(void* L)
{
    double val = p_lua_tonumber(L, 1);
    g_AutoPause = (val > 0.5);
    char buf[64];
    sprintf_s(buf, sizeof(buf), "AutoPause set to: %d", g_AutoPause ? 1 : 0);
    LogMsg(buf);
    return 0;
}

static int Lua_SetRadioStopOnMinimize(void* L)
{
    return Lua_SetRadioAutoPause(L);
}

static int Lua_SetRadioStopOnBackground(void* L)
{
    return Lua_SetRadioAutoPause(L);
}

static void DoRegisterLua()
{
    pFrameScript_RegisterFunction("PlayRadioStream", Lua_PlayRadioStream);
    pFrameScript_RegisterFunction("StopRadioStream", Lua_StopRadioStream);
    pFrameScript_RegisterFunction("SetRadioVolume", Lua_SetRadioVolume);
    pFrameScript_RegisterFunction("IsRadioPlaying", Lua_IsRadioPlaying);
    pFrameScript_RegisterFunction("GetRadioTitle", Lua_GetRadioTitle);
    pFrameScript_RegisterFunction("SetRadioAutoPause", Lua_SetRadioAutoPause);
    pFrameScript_RegisterFunction("SetRadioStopOnMinimize", Lua_SetRadioStopOnMinimize);
    pFrameScript_RegisterFunction("SetRadioStopOnBackground", Lua_SetRadioStopOnBackground);
}

// -------------------------------------------------------------
// Main-Thread IAT Hook for PeekMessageA
// Runs 100% on the WoW Main Thread inside the game render loop
// -------------------------------------------------------------
typedef BOOL (WINAPI *tPeekMessageA)(LPMSG, HWND, UINT, UINT, UINT);
static tPeekMessageA TruePeekMessageA = NULL;
static void* g_RegisteredState = NULL;

static BOOL CALLBACK EnumAllWindowsProc(HWND hwnd, LPARAM lParam)
{
    DWORD targetPid = (DWORD)lParam;
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == targetPid && !GetParent(hwnd) && (IsWindowVisible(hwnd) || IsIconic(hwnd))) {
        char cls[64] = {0};
        GetClassNameA(hwnd, cls, sizeof(cls));
        if (strncmp(cls, "GxWindow", 8) == 0 || strstr(cls, "WoW") || strstr(cls, "World")) {
            g_hWoWWnd = hwnd;
            return FALSE; // Stop enumeration
        }
    }
    return TRUE;
}

static HWND GetWoWWindow()
{
    if (g_hWoWWnd && IsWindow(g_hWoWWnd))
        return g_hWoWWnd;

    DWORD myPid = GetCurrentProcessId();

    // Strategy 1: FindWindowEx across known WoW Class names
    const char* classNames[] = {
        "GxWindowClassD3d",
        "GxWindowClassD3d9Ex",
        "GxWindowClassOpenGl",
        "GxWindowClass",
        NULL
    };

    for (int i = 0; classNames[i]; i++) {
        HWND h = NULL;
        while ((h = FindWindowExA(NULL, h, classNames[i], NULL)) != NULL) {
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            if (pid == myPid && (IsWindowVisible(h) || IsIconic(h))) {
                g_hWoWWnd = h;
                char msg[128];
                sprintf_s(msg, sizeof(msg), "SUCCESS: Captured WoW HWND 0x%p (Class: %s)", g_hWoWWnd, classNames[i]);
                LogMsg(msg);
                return g_hWoWWnd;
            }
        }
    }

    // Strategy 2: Title search
    HWND hTitle = NULL;
    while ((hTitle = FindWindowExA(NULL, hTitle, NULL, "World of Warcraft")) != NULL) {
        DWORD pid = 0;
        GetWindowThreadProcessId(hTitle, &pid);
        if (pid == myPid && (IsWindowVisible(hTitle) || IsIconic(hTitle))) {
            g_hWoWWnd = hTitle;
            char msg[128];
            sprintf_s(msg, sizeof(msg), "SUCCESS: Captured WoW HWND 0x%p via title", g_hWoWWnd);
            LogMsg(msg);
            return g_hWoWWnd;
        }
    }

    // Strategy 3: EnumWindows across our process
    EnumWindows(EnumAllWindowsProc, (LPARAM)myPid);
    if (g_hWoWWnd && IsWindow(g_hWoWWnd)) {
        char msg[128];
        sprintf_s(msg, sizeof(msg), "SUCCESS: Captured WoW HWND 0x%p via EnumWindows", g_hWoWWnd);
        LogMsg(msg);
        return g_hWoWWnd;
    }

    return NULL;
}

static DWORD WINAPI MinimizeMonitorThread(LPVOID lpParam)
{
    LogMsg("MinimizeMonitorThread active: monitoring WoW window state (minimize & background)...");
    bool s_lastShouldPause = false;
    int s_unfocusedTicks = 0;

    while (true) {
        Sleep(100);

        HWND hWoW = GetWoWWindow();
        if (!hWoW) {
            continue;
        }

        // Check 1: Is WoW minimized to taskbar / iconic?
        bool isMin = (IsIconic(hWoW) != 0);

        // Check 2: In windowed mode, is WoW currently the active foreground window?
        HWND hFg = GetForegroundWindow();
        DWORD fgPid = 0;
        if (hFg) {
            GetWindowThreadProcessId(hFg, &fgPid);
        }
        bool isWoWFocused = (hFg != NULL && fgPid == GetCurrentProcessId());

        // Unified pause condition: minimized OR unfocused (leaving window)
        bool rawShouldPause = g_AutoPause && (isMin || !isWoWFocused);

        if (rawShouldPause) {
            s_unfocusedTicks++;
        } else {
            s_unfocusedTicks = 0;
        }

        // Immediate pause if minimized; 2-tick (~200ms) debounce if unfocused/background
        bool shouldPause = g_AutoPause && (isMin || (s_unfocusedTicks >= 2));

        if (shouldPause != s_lastShouldPause) {
            s_lastShouldPause = shouldPause;
            char stateMsg[192];
            sprintf_s(stateMsg, sizeof(stateMsg), "Window state: Iconic=%d, WoWFocused=%d (FgPid=%u, MyPid=%u) -> Pause=%d, Stream=0x%p",
                isMin ? 1 : 0, isWoWFocused ? 1 : 0, fgPid, GetCurrentProcessId(), shouldPause ? 1 : 0, (void*)g_RadioStream);
            LogMsg(stateMsg);
        }

        if (shouldPause && !g_WasMinimized) {
            g_WasMinimized = true;
            if (g_RadioStream) {
                DWORD act = BASS_ChannelIsActive(g_RadioStream);
                if (act == BASS_ACTIVE_PLAYING || act == BASS_ACTIVE_STALLED) {
                    BASS_ChannelPause(g_RadioStream);
                    g_PausedByMinimize = true;
                    g_MinimizeTimestamp = GetTickCount();
                    LogMsg("WoW window minimized/background -> Radio auto-paused.");
                }
            }
        } else if (!shouldPause && g_WasMinimized) {
            g_WasMinimized = false;
            if (g_PausedByMinimize) {
                g_PausedByMinimize = false;
                DWORD duration = GetTickCount() - g_MinimizeTimestamp;
                BOOL resumed = FALSE;
                if (duration < 8000 && g_RadioStream) {
                    resumed = BASS_ChannelPlay(g_RadioStream, FALSE);
                    DWORD act = BASS_ChannelIsActive(g_RadioStream);
                    if (act != BASS_ACTIVE_PLAYING && act != BASS_ACTIVE_STALLED) {
                        resumed = FALSE;
                    }
                }
                if (!resumed && g_LastStreamUrl[0]) {
                    if (g_RadioStream) {
                        BASS_ChannelStop(g_RadioStream);
                        BASS_StreamFree(g_RadioStream);
                        g_RadioStream = 0;
                    }
                    g_RadioStream = BASS_StreamCreateURL(g_LastStreamUrl, 0, BASS_STREAM_AUTOFREE | BASS_SAMPLE_FLOAT, NULL, NULL);
                    if (g_RadioStream) {
                        BASS_ChannelSetAttribute(g_RadioStream, BASS_ATTRIB_VOL, g_RadioVolume);
                        BASS_ChannelSetSync(g_RadioStream, BASS_SYNC_META, 0, MetaSync, 0);
                        BASS_ChannelPlay(g_RadioStream, FALSE);
                    }
                }
                LogMsg("WoW window restored/focused -> Radio auto-resumed.");
            }
        }
    }
    return 0;
}

static BOOL WINAPI Hooked_PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
    // Executed on WoW Main Thread!
    void** pLuaState = (void**)0x00D3F78C;
    if (pLuaState && *pLuaState && *pLuaState != g_RegisteredState) {
        g_RegisteredState = *pLuaState;
        DoRegisterLua();
        char msg[128];
        sprintf_s(msg, sizeof(msg), "SUCCESS: Registered Radio Lua functions ON MAIN THREAD for state %p", g_RegisteredState);
        LogMsg(msg);
    }

    return TruePeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

static bool HookIAT()
{
    HMODULE hModule = GetModuleHandleA(NULL);
    if (!hModule) return false;

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;

    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return false;

    IMAGE_DATA_DIRECTORY importDesc = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!importDesc.VirtualAddress) return false;

    PIMAGE_IMPORT_DESCRIPTOR pImport = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)hModule + importDesc.VirtualAddress);

    while (pImport->Name) {
        const char* modName = (const char*)((BYTE*)hModule + pImport->Name);
        if (_stricmp(modName, "user32.dll") == 0) {
            PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((BYTE*)hModule + pImport->FirstThunk);
            PIMAGE_THUNK_DATA pOrigThunk = (PIMAGE_THUNK_DATA)((BYTE*)hModule + pImport->OriginalFirstThunk);

            while (pOrigThunk->u1.AddressOfData) {
                if (!(pOrigThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
                    PIMAGE_IMPORT_BY_NAME pImportByName = (PIMAGE_IMPORT_BY_NAME)((BYTE*)hModule + pOrigThunk->u1.AddressOfData);
                    if (strcmp((const char*)pImportByName->Name, "PeekMessageA") == 0) {
                        DWORD oldProtect;
                        VirtualProtect(&pThunk->u1.Function, sizeof(void*), PAGE_READWRITE, &oldProtect);
                        TruePeekMessageA = (tPeekMessageA)pThunk->u1.Function;
                        pThunk->u1.Function = (DWORD)Hooked_PeekMessageA;
                        VirtualProtect(&pThunk->u1.Function, sizeof(void*), oldProtect, &oldProtect);
                        LogMsg("SUCCESS: Hooked PeekMessageA in Wow.exe IAT!");
                        return true;
                    }
                }
                pThunk++;
                pOrigThunk++;
            }
        }
        pImport++;
    }
    LogMsg("ERROR: PeekMessageA not found in IAT!");
    return false;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        InitializeCriticalSection(&g_TitleLock);
        LogMsg("=== Radio Proxy DLL Attached to Process ===");
        LoadOriginalDinput8();
        LoadOriginalVersionDll();
        HookIAT();
        CreateThread(NULL, 0, MinimizeMonitorThread, NULL, 0, NULL);
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
        DeleteCriticalSection(&g_TitleLock);
    }
    return TRUE;
}
