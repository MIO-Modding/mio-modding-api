#pragma once
#include <windows.h>

// API Version
#define MODDING_API_VERSION_MAJOR 0
#define MODDING_API_VERSION_MINOR 0
#define MODDING_API_VERSION_PATCH 4

// Export macro for API functions
#ifdef MODDING_API_EXPORTS
#define MODDING_API __declspec(dllexport)
#else
#define MODDING_API __declspec(dllimport)
#endif

typedef struct f32x3 {
    float x, y, z;
} f32x3;

inline f32x3 make_f32x3(float x, float y, float z) {
    f32x3 v = {x, y, z};
    return v;
}

typedef struct APIVersion {
    int major, minor, patch;
} APIVersion;

inline APIVersion make_APIVersion(int major, int minor, int patch) {
    APIVersion v = {major, minor, patch};
    return v;
}

extern "C" {
    // Initialization
    MODDING_API void InitializeAddresses();
    
    // Player functions
    MODDING_API f32x3 GetPlayerLocation();
    MODDING_API bool SetPlayerLocation(f32x3 location);
    MODDING_API int GetPlayerHealth();
    MODDING_API bool SetPlayerHealth(int health);
    MODDING_API int GetPlayerNacre();
    MODDING_API bool SetPlayerNacre(int nacre);
    MODDING_API int GetPlayerStamina();
    MODDING_API bool SetPlayerStamina(float stamina);
    
    // Memory functions
    MODDING_API bool WriteMemory(void* address, const void* data, size_t size);
    MODDING_API bool ReadMemory(void* address, void* buffer, size_t size);
    MODDING_API bool PatchBytes(void* address, const char* bytes, size_t size);
    MODDING_API bool NopBytes(void* address, size_t count);
    MODDING_API void* PatternScan(const char* pattern, const char* mask);

    // Utility
    MODDING_API void LogMessage(const char* message);
    MODDING_API APIVersion GetAPIVersion();
}

// Templated helpers for convenience (C++ only, outside extern "C")
template<typename T>
inline T ReadMemoryTyped(void* address) {
    return *(T*)address;
}

template<typename T>
inline bool WriteMemoryTyped(void* address, T value) {
    return WriteMemory(address, &value, sizeof(T));
}