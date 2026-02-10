#include <windows.h>
#include <stdio.h>
#include "./modding_api.h"

// Thread that continuously monitors player position
DWORD WINAPI MonitorPlayerPosition(LPVOID lpParam) {
    while (true) {
        Sleep(500);  // Update every 500ms
        
        f32x3 loc = GetPlayerLocation();
        
        // Clear the line and print new coordinates
        printf("Player Position: X=%.2f, Y=%.2f, Z=%.2f     \n", loc.x, loc.y, loc.z);
        fflush(stdout);
    }
    return 0;
}

// This function is called when the mod is loaded
extern "C" __declspec(dllexport) void ModInit() {
    LogMessage("Position Monitor mod loaded!");
    
    // Start monitoring thread
    CreateThread(nullptr, 0, MonitorPlayerPosition, nullptr, 0, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    // Mods don't need to do anything in DllMain
    return TRUE;
}