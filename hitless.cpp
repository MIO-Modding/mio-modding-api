#include <windows.h>
#include <stdio.h>
#include "./modding_api.h"

// Thread that continuously sets player health to 1
DWORD WINAPI SetHealth(LPVOID lpParam) {
    while (true) {
        Sleep(500);  // Update every 500ms
        int curr_health = GetPlayerHealth();
        if (curr_health > 0) {
            printf("Player Health: %d. Setting to 0...     \n", curr_health);
            SetPlayerHealth(0);
            fflush(stdout);
        }

    }
    return 0;
}

// This function is called when the mod is loaded
extern "C" __declspec(dllexport) void ModInit() {
    LogMessage("Hitless mod loaded!");
    
    // Start monitoring thread
    CreateThread(nullptr, 0, SetHealth, nullptr, 0, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    // Mods don't need to do anything in DllMain
    return TRUE;
}