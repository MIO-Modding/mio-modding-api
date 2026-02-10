#include <windows.h>
#include <stdio.h>
#include "./modding_api.h"

// Thread that continuously monitors player health
DWORD WINAPI MonitorPlayerHealth(LPVOID lpParam) {
    while (true) {
        Sleep(500);  // Update every 500ms
        
        int health = GetPlayerHealth();
        
        // Clear the line and print new health
        printf("Player Health: %d     \n", health);
        fflush(stdout);
    }
    return 0;
}

// This function is called when the mod is loaded
extern "C" __declspec(dllexport) void ModInit() {
    LogMessage("Health Monitor mod loaded!");
    
    // Start monitoring thread
    CreateThread(nullptr, 0, MonitorPlayerHealth, nullptr, 0, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    // Mods don't need to do anything in DllMain
    return TRUE;
}