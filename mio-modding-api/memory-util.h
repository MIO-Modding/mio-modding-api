#pragma once
#include "pch.h"


bool WriteMemory(void* address, const void* data, size_t size);
bool ReadMemory(void* address, void* buffer, size_t size);
bool PatchBytes(void* address, const char* bytes, size_t size);
bool NopBytes(void* address, size_t count);
void* PatternScan(HMODULE module, const char* pattern, const char* mask);
void* PatternScanReverse(HMODULE module, void* from, const char* pattern, const char* mask);

template <typename ReturnType, typename... Args>
ReturnType CallAssembly(void* address, Args... args) {
    uintptr_t addr = reinterpret_cast<uintptr_t>(address);
    typedef ReturnType(__fastcall* InternalFunc)(Args...);
    InternalFunc func = reinterpret_cast<InternalFunc>(addr);
    return func(args...);
}
template <typename T> inline T ReadMemoryTyped(void* address) {
    return *(T*)address;
}

template <typename T> inline bool WriteMemoryTyped(void* address, T value) {
    return WriteMemory(address, &value, sizeof(T));
}