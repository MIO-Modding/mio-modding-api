#include "modding_api.h"
#include <stdio.h>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

namespace ModAPI {
    // Constant addresses
    void* g_PlayerStaminaAddr = nullptr;

    // Base address for pointer chain
    void** g_PlayerLocationBasePtr = nullptr;
    void** g_PlayerHealthBasePtr = nullptr;
    void** g_PlayerNacreBasePtr = nullptr;
    
    // Offsets from Cheat Engine pointer scan
    const int PLAYER_X_OFFSET = 0x18;
    const int PLAYER_Y_OFFSET = 0x1C;  // Usually Y is 4 bytes after X
    const int PLAYER_Z_OFFSET = 0x20;  // Usually Z is 4 bytes after Y
    const int PLAYER_HEALTH_OFFSET = 0xD4;
    const int PLAYER_NACRE_OFFSET = 0x754;
}

// Helper function to follow pointer chain
void* FollowPointer(void* basePtr, int offset) {
    if (!basePtr) return nullptr;
    
    void* ptr = *(void**)basePtr;
    if (!ptr) return nullptr;
    
    return (void*)((uintptr_t)ptr + offset);
}

// ========================================
// Memory Functions
// ========================================

extern "C" {
    MODDING_API bool WriteMemory(void* address, const void* data, size_t size) {
        DWORD oldProtect;
        if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            return false;
        }
        memcpy(address, data, size);
        VirtualProtect(address, size, oldProtect, &oldProtect);
        return true;
    }
    
    MODDING_API bool ReadMemory(void* address, void* buffer, size_t size) {
        DWORD oldProtect;
        if (!VirtualProtect(address, size, PAGE_READONLY, &oldProtect)) {
            return false;
        }
        memcpy(buffer, address, size);
        VirtualProtect(address, size, oldProtect, &oldProtect);
        return true;
    }
    
    MODDING_API bool PatchBytes(void* address, const char* bytes, size_t size) {
        DWORD oldProtect;
        if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            return false;
        }
        memcpy(address, bytes, size);
        VirtualProtect(address, size, oldProtect, &oldProtect);
        return true;
    }
    
    MODDING_API bool NopBytes(void* address, size_t count) {
        DWORD oldProtect;
        if (!VirtualProtect(address, count, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            return false;
        }
        memset(address, 0x90, count);
        VirtualProtect(address, count, oldProtect, &oldProtect);
        return true;
    }
    
    MODDING_API void* PatternScan(const char* pattern, const char* mask) {
        MODULEINFO moduleInfo;
        GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &moduleInfo, sizeof(MODULEINFO));
        
        char* base = (char*)moduleInfo.lpBaseOfDll;
        size_t size = moduleInfo.SizeOfImage;
        size_t patternLength = strlen(mask);
        
        for (size_t i = 0; i < size - patternLength; i++) {
            bool found = true;
            for (size_t j = 0; j < patternLength; j++) {
                if (mask[j] != '?' && pattern[j] != base[i + j]) {
                    found = false;
                    break;
                }
            }
            if (found) return &base[i];
        }
        return nullptr;
    }
    
    // ========================================
    // Player Functions
    // ========================================
    
    MODDING_API f32x3 GetPlayerLocation() {
        f32x3 loc = {-1.0f, -1.0f, -1.0f};
        
        if (!ModAPI::g_PlayerLocationBasePtr) {
            return loc;
        }
        
        // Follow the pointer chain
        void* playerXAddr = FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_X_OFFSET);
        void* playerYAddr = FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_Y_OFFSET);
        void* playerZAddr = FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_Z_OFFSET);
        
        if (playerXAddr) {
            loc.x = *(float*)playerXAddr;
        }
        if (playerYAddr) {
            loc.y = *(float*)playerYAddr;
        }
        if (playerZAddr) {
            loc.z = *(float*)playerZAddr;
        }
        
        return loc;
    }
    
    MODDING_API bool SetPlayerLocation(f32x3 location) {
        if (!ModAPI::g_PlayerLocationBasePtr) {
            return false;
        }
        
        void* playerXAddr = FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_X_OFFSET);
        void* playerYAddr = FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_Y_OFFSET);
        void* playerZAddr = FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_Z_OFFSET);
        
        bool success = true;
        if (playerXAddr) {
            success &= WriteMemoryTyped(playerXAddr, location.x);
        }
        if (playerYAddr) {
            success &= WriteMemoryTyped(playerYAddr, location.y);
        }
        if (playerZAddr) {
            success &= WriteMemoryTyped(playerZAddr, location.z);
        }
        
        return success;
    }

    MODDING_API int GetPlayerHealth() {
        if (!ModAPI::g_PlayerHealthBasePtr) {
            return -1;
        }

        void* playerHealthAddr = FollowPointer(ModAPI::g_PlayerHealthBasePtr, ModAPI::PLAYER_HEALTH_OFFSET);

        if (playerHealthAddr) {
            return *(int*)playerHealthAddr;
        } else {
            return -1;
        }
    }

    MODDING_API bool SetPlayerHealth(int health) {
        if (!ModAPI::g_PlayerHealthBasePtr) {
            return false;
        }

        void* playerHealthAddr = FollowPointer(ModAPI::g_PlayerHealthBasePtr, ModAPI::PLAYER_HEALTH_OFFSET);

        bool success = true;
        if (playerHealthAddr) {
            success &= WriteMemoryTyped(playerHealthAddr, health);
        }

        return success;
    }

    MODDING_API int GetPlayerNacre() {
        if (!ModAPI::g_PlayerNacreBasePtr) {
            return -1;
        }

        void* playerNacreAddr = FollowPointer(ModAPI::g_PlayerNacreBasePtr, ModAPI::PLAYER_NACRE_OFFSET);

        if (playerNacreAddr) {
            return *(int*)playerNacreAddr;
        } else {
            return -1;
        }
    }

    MODDING_API bool SetPlayerNacre(int nacre) {
        if (!ModAPI::g_PlayerNacreBasePtr) {
            return false;
        }

        void* playerNacreAddr = FollowPointer(ModAPI::g_PlayerNacreBasePtr, ModAPI::PLAYER_NACRE_OFFSET);

        bool success = true;
        if (playerNacreAddr) {
            success &= WriteMemoryTyped(playerNacreAddr, nacre);
        }

        return success;
    }

    MODDING_API int GetPlayerStamina() {
        if (!ModAPI::g_PlayerStaminaAddr) {
            return -1;
        }

        if (ModAPI::g_PlayerStaminaAddr) {
            return *(float*)ModAPI::g_PlayerStaminaAddr;
        } else {
            return -1;
        }
    }

    MODDING_API bool SetPlayerStamina(float stamina) {
        if (!ModAPI::g_PlayerStaminaAddr) {
            return false;
        }

        bool success = true;
        if (ModAPI::g_PlayerStaminaAddr) {
            success &= WriteMemoryTyped(ModAPI::g_PlayerStaminaAddr, stamina);
        }

        return success;
    }
    
    // ========================================
    // Initialization
    // ========================================
    
    MODDING_API void InitializeAddresses() {        
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
        
        // Store the address
        ModAPI::g_PlayerLocationBasePtr = (void**)playerLocationBasePtrAddr;
        ModAPI::g_PlayerHealthBasePtr = (void**)playerHealthBasePtrAddr;
        ModAPI::g_PlayerNacreBasePtr = (void**)playerNacreBasePtrAddr;

        ModAPI::g_PlayerStaminaAddr = (void*)(baseAddr + 0x110F9A8);
    }
    
    // ========================================
    // Utility Functions
    // ========================================
    
    MODDING_API void LogMessage(const char* message) {
        printf("[MOD API] %s\n", message);
    }
    
    MODDING_API APIVersion GetAPIVersion() {
        APIVersion v = {
            MODDING_API_VERSION_MAJOR,
            MODDING_API_VERSION_MINOR,
            MODDING_API_VERSION_PATCH
        };
        return v;
    }
}