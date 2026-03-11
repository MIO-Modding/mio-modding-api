#pragma once
#include "pch.h"
#include <cstddef>
#include <cstdint>

#ifdef MODDING_API_EXPORTS
#define MODDING_API __declspec(dllexport)
#else
#define MODDING_API __declspec(dllimport)
#endif


// VECTORS

typedef struct Vector3 {
	float x, y, z;
	Vector3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};
typedef struct Vector2 {
	float x, y;
	Vector2(float x, float y) {
		this->x = x;
		this->y = y;
	}
	Vector2(int64_t xyint) {
		uint32_t b1 = static_cast<uint32_t>(static_cast<uint64_t>(xyint) >> 32);
		uint32_t b2 = static_cast<uint32_t>(xyint & 0xFFFFFFFF);

		float y;
		float x;
		memcpy(&y, &b1, sizeof(float));
		memcpy(&x, &b2, sizeof(float));
		this->x = x;
		this->y = y;
	}
	int64_t CreateXYInt64() {
		uint32_t u1;
		uint32_t u2;
		memcpy(&u1, &y, sizeof(y));
		memcpy(&u2, &x, sizeof(x));

		uint64_t combined_u64 = (static_cast<uint64_t>(u1) << 32) | u2;
		int64_t combined_s64 = static_cast<int64_t>(combined_u64);

		return combined_s64;
	}
};


// VARIABLES

namespace ModAPI {
	// Base address for pointer chain
	namespace Pointers {
		extern void** g_PlayerLocationBasePtr;
		extern void** g_PlayerHealthBasePtr;
		extern void** g_PlayerNacreBasePtr;
		extern void*** g_SaveArrayPtr;
	}
	// Constant addresses
	namespace Addresses {
		extern void* g_PlayerStaminaAddr;
		extern void* g_PlayerVelocityXAddr;
		extern void* g_PlayerVelocityYAddr;
		extern void* g_MoveByMethodAddr;
		extern void* g_PlayerObjAddr;
	}
	// Offsets from Cheat Engine pointer scan
	namespace Offsets {
		extern const int PLAYER_X_OFFSET;
		extern const int PLAYER_Y_OFFSET; // Usually Y is 4 bytes after X
		extern const int PLAYER_Z_OFFSET; // Usually Z is 4 bytes after Y
		extern const int PLAYER_HEALTH_OFFSET;
		extern const int PLAYER_LIQUID_NACRE_OFFSET;
	}
	namespace Player {
		extern "C" {
			MODDING_API Vector3 GetPlayerLocation();
			MODDING_API bool SetPlayerLocation(Vector3 location);
			MODDING_API Vector2 GetPlayerVelocity();
			MODDING_API bool SetPlayerVelocity(Vector2 velocity);
			MODDING_API int GetPlayerHealth();
			MODDING_API bool SetPlayerHealth(int health);
			MODDING_API int GetPlayerLiquidNacre();
			MODDING_API bool SetPlayerLiquidNacre(int nacre);
			MODDING_API int GetPlayerCrystalNacre();
			MODDING_API bool SetPlayerCrystalNacre(int nacre);
			MODDING_API float GetPlayerStamina();
			MODDING_API bool SetPlayerStamina(float stamina);
			MODDING_API int64_t MovePlayer(Vector2 vector);
		}
	}
	namespace SaveData {
		extern uint32_t* g_SaveArraySize;
		typedef struct GameString { // 16 bytes
			char* data;               // 8 bytes
			uint32_t size;                 // 4 bytes
			uint32_t unused;               // 4 bytes
		} GameString;

		typedef struct SaveEntryValue { // 64 bytes
			uint32_t flags;                    // 4 bytes
			int32_t count;                    // 4 bytes
			BYTE unused[56];              // 56 bytes
		} SaveEntryValue;

		typedef struct SaveEntry { // 88 bytes
			uint32_t hash;                // 4 bytes
			uint32_t padding;             // 4 bytes
			GameString entry_name;   // 16 bytes
			SaveEntryValue value;    // 64 bytes
		} SaveEntry;

		extern "C" {
			MODDING_API int GetSaveEntryCount();
			MODDING_API SaveEntry* GetSaveEntryByIndex(int index);
			MODDING_API SaveEntry* GetSaveEntry(const char* name);
			MODDING_API bool SaveEntryExists(const char* name);
			MODDING_API const char* GetSaveEntryName(int index);

			MODDING_API bool GetSaveEntryValueCount(const char* name, int32_t* outCount);
			MODDING_API bool SetSaveEntryValueCount(const char* name, int32_t count);
			MODDING_API bool GetSaveEntryValueFlags(const char* name, uint32_t* outFlags);
			MODDING_API bool SetSaveEntryValueFlags(const char* name, uint32_t flags);
		}
	}
	namespace Util {
		extern "C" {
			MODDING_API bool WriteMemory(void* address, const void* data, size_t size);
			MODDING_API bool ReadMemory(void* address, void* buffer, size_t size);
			MODDING_API bool PatchBytes(void* address, const char* bytes, size_t size);
			MODDING_API bool NopBytes(void* address, size_t count);
			MODDING_API void* PatternScan(HMODULE module, const char* pattern, const char* mask);
			MODDING_API void* PatternScanReverse(HMODULE module, void* from, const char* pattern, const char* mask);
			MODDING_API void* FollowPointer(void* basePtr, int offset);
		}
		template <typename ReturnType, typename... Args>
		ReturnType CallAssembly(void* address, Args... args) {
			uintptr_t addr = reinterpret_cast<uintptr_t>(address);
			typedef ReturnType(__fastcall* InternalFunc)(Args...);
			InternalFunc func = reinterpret_cast<InternalFunc>(addr);
			return func(args...);
		}
		template <typename T>
		T ReadMemoryTyped(void* address) {
			return *(T*)address;
		}

		template <typename T>
		bool WriteMemoryTyped(void* address, T value) {
			return WriteMemory(address, &value, sizeof(T));
		}
	}
}