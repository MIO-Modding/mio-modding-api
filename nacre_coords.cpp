#include <windows.h>
#include <stdio.h>
#include <cmath>
#include <cstdlib>
#include "./modding_api.h"

// Thread that continuously sets player nacre to x pos every 500ms
DWORD WINAPI SetNacreToX(LPVOID lpParam) {
    while (true) {
        Sleep(500);  // Update every 500ms
        f32x3 loc = GetPlayerLocation();
        int curr_nacre = GetPlayerNacre();

        if (!(loc.x == -1.0f || curr_nacre == 32758)) {
            int rounded_x = static_cast<int>(std::round(std::abs(loc.x)));
            SetPlayerNacre(curr_nacre);
        }
    }
    return 0;
}

// This function is called when the mod is loaded
extern "C" __declspec(dllexport) void ModInit() {
    LogMessage("NacreCoords mod loaded!");
    
    // Start monitoring thread
    CreateThread(nullptr, 0, SetNacreToX, nullptr, 0, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    // Mods don't need to do anything in DllMain
    return TRUE;
}