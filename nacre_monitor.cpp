#include <windows.h>
#include <stdio.h>
#include "./modding_api.h"

// Thread that continuously monitors player nacre
DWORD WINAPI MonitorPlayerNacre(LPVOID lpParam) {
    while (true) {
        Sleep(500);  // Update every 500ms
        
        int nacre = GetPlayerNacre();
        
        // Clear the line and print new nacre
        printf("Player Nacre: %d     \n", nacre);
        fflush(stdout);
    }
    return 0;
}

// This function is called when the mod is loaded
extern "C" __declspec(dllexport) void ModInit() {
    LogMessage("Nacre Monitor mod loaded!");
    
    // Start monitoring thread
    CreateThread(nullptr, 0, MonitorPlayerNacre, nullptr, 0, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    // Mods don't need to do anything in DllMain
    return TRUE;
}