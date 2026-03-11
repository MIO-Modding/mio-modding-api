// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <stdio.h>
#include "mio-modding-api.h"

namespace ModAPI {
    // Constant addresses
    void* g_PlayerStaminaAddr = nullptr;
    void* g_PlayerVelocityXAddr = nullptr;
    void* g_PlayerVelocityYAddr = nullptr;

    // Base address for pointer chain
    void** g_PlayerLocationBasePtr = nullptr;
    void** g_PlayerHealthBasePtr = nullptr;
    void** g_PlayerNacreBasePtr = nullptr;
    void* g_MoveByMethodAddr = nullptr;
    void* g_PlayerObjAddr = nullptr;

    void*** g_SaveArrayPtr = nullptr;
    uint32_t* g_SaveArraySize = nullptr;

    // Offsets from Cheat Engine pointer scan
    const int PLAYER_X_OFFSET = 0x18;
    const int PLAYER_Y_OFFSET = 0x1C; // Usually Y is 4 bytes after X
    const int PLAYER_Z_OFFSET = 0x20; // Usually Z is 4 bytes after Y
    const int PLAYER_HEALTH_OFFSET = 0xD4;
    const int PLAYER_LIQUID_NACRE_OFFSET = 0x754;
}
void LoadMemoryAddresses() {
    HMODULE hModule = GetModuleHandleA("mio.exe");
    if (!hModule) {
        LogMessage("ERROR: Failed to get mio.exe module handle!");
        return;
    }

    // Get the base address of mio.exe
    uintptr_t baseAddr = (uintptr_t)hModule;

    // Add the offset from Cheat Engine
    uintptr_t playerLocationBasePtrAddr = baseAddr + 0x010EDF48;
    uintptr_t playerHealthBasePtrAddr = baseAddr + 0x010EE2E8;
    uintptr_t playerNacreBasePtrAddr = baseAddr + 0x01114AD0;
    uintptr_t playerStaminaPtrAddr = baseAddr + 0x110F9A8;

    uintptr_t saveArrayPtrAddr = baseAddr + 0x01114AD0;
    uintptr_t saveArraySizeAddr = baseAddr + 0x01114AC8;

    // Store the address
    ModAPI::g_PlayerLocationBasePtr = (void**)playerLocationBasePtrAddr;
    ModAPI::g_PlayerHealthBasePtr = (void**)playerHealthBasePtrAddr;
    ModAPI::g_PlayerNacreBasePtr = (void**)playerNacreBasePtrAddr;

    ModAPI::g_MoveByMethodAddr = (void*)(PatternScanReverse(hModule, PatternScan(hModule, "\x48\x8d\x05\x84\x42\x58\x00", "xxxxxxx"), "\x41\x57", "xx"));
    ModAPI::g_PlayerObjAddr = (void*)(baseAddr + 0x10edc50);

    ModAPI::g_PlayerStaminaAddr = (void*)playerStaminaPtrAddr;

    ModAPI::g_SaveArrayPtr = (void***)saveArrayPtrAddr;
    ModAPI::g_SaveArraySize = (uint32_t*)saveArraySizeAddr;
    ModAPI::g_PlayerVelocityXAddr = (void*)(baseAddr + 0x10EE0C8);
    ModAPI::g_PlayerVelocityYAddr = (void*)(baseAddr + 0x10EE0CC);
}
extern "C" __declspec(dllexport) void ModInit() {
    LoadMemoryAddresses();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    return TRUE;
}

