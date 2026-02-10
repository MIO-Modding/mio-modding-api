#include <windows.h>
#include <stdio.h>
#include "./modding_api.h"

// Thread that continuously sets player stamina to 100 every 20ms
DWORD WINAPI SetStamina(LPVOID lpParam) {
    while (true) {
        Sleep(20);  // Update every 20ms
        SetPlayerStamina(100.0f);
    }
    return 0;
}

// This function is called when the mod is loaded
extern "C" __declspec(dllexport) void ModInit() {
    LogMessage("Infinite Stamina mod loaded!");
    
    // Start monitoring thread
    CreateThread(nullptr, 0, SetStamina, nullptr, 0, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    // Mods don't need to do anything in DllMain
    return TRUE;
}