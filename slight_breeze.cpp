#include <windows.h>
#include <stdio.h>
#include "./modding_api.h"

// Thread that continuously moves the player 0.15 units to the right every 0.05 seconds
DWORD WINAPI MovePlayer(LPVOID lpParam) {
    while (true) {
        Sleep(10);  // Update every 50ms
        
        f32x3 loc = GetPlayerLocation();
        
        if (loc.x == -1.0f || loc.y == -1.0f) {
            continue; // wait until save is loaded
        }
        printf("Old Player Position: X=%.2f, Y=%.2f, Z=%.2f     \n", loc.x, loc.y, loc.z);

        loc.x += 0.03;
        SetPlayerLocation(loc);
        
        printf("New Player Position: X=%.2f, Y=%.2f, Z=%.2f     \n", loc.x, loc.y, loc.z);
        fflush(stdout);
    }
    return 0;
}

// This function is called when the mod is loaded
extern "C" __declspec(dllexport) void ModInit() {
    LogMessage("Slight Breeze mod loaded!");
    
    // Start monitoring thread
    CreateThread(nullptr, 0, MovePlayer, nullptr, 0, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    // Mods don't need to do anything in DllMain
    return TRUE;
}