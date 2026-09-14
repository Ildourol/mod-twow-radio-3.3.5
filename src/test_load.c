#include <windows.h>
#include <stdio.h>

int main() {
    HMODULE h = LoadLibraryExA("C:\\Users\\Admin\\Documents\\Wrath client 3.3.5a\\dinput8.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!h) {
        printf("FAILED to load dinput8.dll! Error: %lu\n", GetLastError());
        return 1;
    }
    printf("SUCCESS loading dinput8.dll! Handle: %p\n", h);

    FARPROC pCreate = GetProcAddress(h, "DirectInput8Create");
    if (!pCreate) {
        printf("FAILED: DirectInput8Create not found in exports!\n");
        return 1;
    }
    printf("SUCCESS: DirectInput8Create found at %p!\n", pCreate);
    return 0;
}
