#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>

using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;

// API Version
#define MODDING_API_VERSION_MAJOR 0
#define MODDING_API_VERSION_MINOR 0
#define MODDING_API_VERSION_PATCH 5

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
typedef struct f32x2 {
    float x, y;
} f32x2;
inline f32x2 make_f32x2(float x, float y) {
    f32x2 v = {x, y};
    return v;
}

typedef struct APIVersion {
    int major, minor, patch;
} APIVersion;

inline APIVersion make_APIVersion(int major, int minor, int patch) {
    APIVersion v = {major, minor, patch};
    return v;
}

typedef struct GameString { // 16 bytes
    char* data;  // 8 bytes
    u32 size;    // 4 bytes
    u32 unused;  // 4 bytes
} GameString;

typedef struct SaveEntryValue { // 64 bytes
    u32 flags;         // 4 bytes
    i32 count;         // 4 bytes
    byte unused[56];   // 56 bytes
} SaveEntryValue;

typedef struct SaveEntry { // 88 bytes
    u32 hash;                // 4 bytes
    u32 padding;             // 4 bytes
    GameString entry_name;   // 16 bytes
    SaveEntryValue value;    // 64 bytes
} SaveEntry;

extern "C" {
    // Initialization
    MODDING_API void InitializeAddresses();
    
    // Player functions
    MODDING_API f32x3 GetPlayerLocation();
    MODDING_API bool SetPlayerLocation(f32x3 location);
    MODDING_API f32x2 GetPlayerVelocity();
    MODDING_API bool SetPlayerVelocity(f32x2 velocity);
    MODDING_API int GetPlayerHealth();
    MODDING_API bool SetPlayerHealth(int health);
    MODDING_API int GetPlayerLiquidNacre();
    MODDING_API bool SetPlayerLiquidNacre(int nacre);
    MODDING_API int GetPlayerCrystalNacre();
    MODDING_API bool SetPlayerCrystalNacre(int nacre);
    MODDING_API float GetPlayerStamina();
    MODDING_API bool SetPlayerStamina(float stamina);

    // Save entry functions - returns POINTER to actual entry in memory
    MODDING_API int GetSaveEntryCount();
    MODDING_API SaveEntry* GetSaveEntryByIndex(int index);
    MODDING_API SaveEntry* GetSaveEntry(const char* name);
    MODDING_API bool SaveEntryExists(const char* name);
    MODDING_API const char* GetSaveEntryName(int index);
    
    // Helper functions for reading/writing count
    MODDING_API bool GetSaveEntryValueCount(const char* name, i32* outCount);
    MODDING_API bool SetSaveEntryValueCount(const char* name, i32 count);
    MODDING_API bool GetSaveEntryValueFlags(const char* name, u32* outFlags);
    MODDING_API bool SetSaveEntryValueFlags(const char* name, u32 flags);
    
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