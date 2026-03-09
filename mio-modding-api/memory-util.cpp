#include "pch.h"
#include <psapi.h>

bool WriteMemory(void* address, const void* data, size_t size) {
    DWORD oldProtect;
    if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }
    memcpy(address, data, size);
    VirtualProtect(address, size, oldProtect, &oldProtect);
    return true;
}

bool ReadMemory(void* address, void* buffer, size_t size) {
    DWORD oldProtect;
    if (!VirtualProtect(address, size, PAGE_READONLY, &oldProtect)) {
        return false;
    }
    memcpy(buffer, address, size);
    VirtualProtect(address, size, oldProtect, &oldProtect);
    return true;
}

bool PatchBytes(void* address, const char* bytes, size_t size) {
    DWORD oldProtect;
    if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }
    memcpy(address, bytes, size);
    VirtualProtect(address, size, oldProtect, &oldProtect);
    return true;
}

bool NopBytes(void* address, size_t count) {
    DWORD oldProtect;
    if (!VirtualProtect(address, count, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }
    memset(address, 0x90, count);
    VirtualProtect(address, count, oldProtect, &oldProtect);
    return true;
}

void* PatternScan(HMODULE module, const char* pattern, const char* mask) {
    MODULEINFO moduleInfo;
    GetModuleInformation(GetCurrentProcess(), module, &moduleInfo,
        sizeof(MODULEINFO));

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
        if (found)
            return &base[i];
    }
    return nullptr;
}
void* PatternScanReverse(HMODULE module, void* from, const char* pattern, const char* mask) {
    char* base = (char*)module;
    size_t patternLength = strlen(mask);

    for (size_t i = (uintptr_t)from - (uintptr_t)base; i > 0; i--) {
        bool found = true;
        for (size_t j = 0; j < patternLength; j++) {
            if (mask[j] != '?' && pattern[j] != base[i + j]) {
                found = false;
                break;
            }
        }
        if (found)
            return &base[i];
    }
    return nullptr;
}