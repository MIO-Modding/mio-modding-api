// dllmain.cpp : Defines the entry point for the DLL application.
#include "Windows.h"
#include <stdio.h>
#include "mio-modding-api.h"
#include <string>
#include "mio-modding-api-internal.h"
#include "flamby-handling.h"

namespace ModAPI {
	// Constant addresses
	namespace Addresses {
		void* g_PlayerStaminaAddr = nullptr;
		void* g_PlayerVelocityXAddr = nullptr;
		void* g_PlayerVelocityYAddr = nullptr;
		void* g_MoveByMethodAddr = nullptr;
		void* g_PlayerObjAddr = nullptr;
		void* g_HitEnemyAddress = nullptr;
	}

	// Base address for pointer chain
	namespace Pointers {
		void** g_PlayerLocationBasePtr = nullptr;
		void** g_PlayerHealthBasePtr = nullptr;
		void** g_PlayerNacreBasePtr = nullptr;
		void*** g_SaveArrayPtr = nullptr;
	}

	namespace SaveData {
		uint32_t* g_SaveArraySize = nullptr;
	}

	// Offsets from Cheat Engine pointer scan
	namespace Offsets {
		const int PLAYER_X_OFFSET = 0x18;
		const int PLAYER_Y_OFFSET = 0x1C; // Usually Y is 4 bytes after X
		const int PLAYER_Z_OFFSET = 0x20; // Usually Z is 4 bytes after Y
		const int PLAYER_HEALTH_OFFSET = 0xD4;
		const int PLAYER_LIQUID_NACRE_OFFSET = 0x754;
	}
	namespace Util {
		MODDING_API void LogMessage(char* modId, const char* message) {
			printf("[%s] %s\n", modId, message);
			fflush(stdout);
		}
	}
}
char* modId;
void LogMessage(const char* message) {
	ModAPI::Util::LogMessage(modId, message);
}
void LoadMemoryAddresses() {
	HMODULE hModule = GetModuleHandleA("mio.exe");
	if (!hModule) {
		ModAPI::Util::LogMessage(modId, "ERROR: Failed to get mio.exe module handle!");
		return;
	}

	// Get the base address of mio.exe
	uintptr_t baseAddr = (uintptr_t)hModule;

	uintptr_t plrObjAddr = (baseAddr + 0x10efc50);

	// Add the offset from Cheat Engine
	uintptr_t playerLocationBasePtrAddr = plrObjAddr + 0x2F8;
	uintptr_t playerHealthBasePtrAddr = plrObjAddr + 0x698;
	uintptr_t playerNacreBasePtrAddr = baseAddr + 0x01114AD0;
	uintptr_t playerStaminaPtrAddr = baseAddr + 0x11119a8;

	uintptr_t saveArrayPtrAddr = baseAddr + 0x1116bf8;
	uintptr_t saveArraySizeAddr = baseAddr + 0x1116bf0;

	uintptr_t hitEnemyFunctionAddress = baseAddr + 0x75ed70;

	// Store the address
	ModAPI::Pointers::g_PlayerLocationBasePtr = (void**)playerLocationBasePtrAddr;
	ModAPI::Pointers::g_PlayerHealthBasePtr = (void**)playerHealthBasePtrAddr;
	ModAPI::Pointers::g_PlayerNacreBasePtr = (void**)playerNacreBasePtrAddr;

	ModAPI::Addresses::g_MoveByMethodAddr = (void*)(baseAddr+0x9d6270);

	ModAPI::Addresses::g_PlayerStaminaAddr = (void*)playerStaminaPtrAddr;
	ModAPI::Addresses::g_PlayerObjAddr = (void*)(baseAddr + 0x10efc50);

	ModAPI::Pointers::g_SaveArrayPtr = (void***)saveArrayPtrAddr;
	ModAPI::SaveData::g_SaveArraySize = (uint32_t*)saveArraySizeAddr;
	ModAPI::Addresses::g_PlayerVelocityXAddr = (void*)(plrObjAddr + 0x478);
	ModAPI::Addresses::g_PlayerVelocityYAddr = (void*)(plrObjAddr + 0x47C);

	ModAPI::Addresses::g_HitEnemyAddress = (void*)hitEnemyFunctionAddress;
}

extern "C" __declspec(dllexport) void ModInit(char* id) {
	//Making the printfs actually be sent to console
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	modId = id;
	LoadMemoryAddresses();

	ModAPI::Hooks::InitializeHooks();

	LogMessage("Loading Flamby Data");
	LoadFlambyData(fs::path("flamby"));
	LogMessage("Loaded Flamby Data");

	//Warning: This file exists on my pc not in the repo
	ModifyGin("flamby/assets.gin", "texture_42", "decomp/156597_texture_42.dds");

	PatchChecksum();

	LogMessage("Applying Flamby Data");
	ApplyFlambyData();
	LogMessage("Applying Flamby Data");
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return TRUE;
}

