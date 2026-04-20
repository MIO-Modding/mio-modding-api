// dllmain.cpp : Defines the entry point for the DLL application.
#include "Windows.h"
#include <stdio.h>
#include "mio-modding-api.h"
#include <string>
#include "saved-addresses.h"

namespace ModAPI {
	// Constant addresses
	namespace Addresses {
		uintptr_t g_BaseAddr;
		void* g_PlayerStaminaAddr = nullptr;
		void* g_PlayerVelocityAddr = nullptr;
		void* g_MenuStateAddr = nullptr;
		void* g_MoveByMethodAddr = nullptr;
		void* g_PlayerObjAddr = nullptr;
		void* g_GameAddr = nullptr;
		void* g_HitEnemyAddress = nullptr;
		void* g_GiveFlagAddress = nullptr;
		void* g_PlayerLocationAddr = nullptr;
		void* g_GetSaveEntryAddress = nullptr;
		void* g_SaveAddress = nullptr;
		void* g_PlayerHealthAddress = nullptr;
	}

	// Base address for pointer chain
	namespace Pointers {
		void** g_PlayerNodeBasePtr = nullptr;
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

	Internal::ReloadSavedAddresses();

	// Get the base address of mio.exe
	uintptr_t baseAddr = (uintptr_t)hModule;

	uintptr_t gameAddr = baseAddr + ModAPI::Util::GetStaticVariableOffset("game");
	uintptr_t metagameAddr = baseAddr + ModAPI::Util::GetStaticVariableOffset("metagame");
	uintptr_t plrObjAddr = gameAddr + ModAPI::Util::GetVariableOffset("Game", "mio");
	uintptr_t saveAddr = gameAddr + ModAPI::Util::GetVariableOffset("Game", "save");

	// Add the offset from Cheat Engine
	uintptr_t playerNodeBasePtrAddr = plrObjAddr + ModAPI::Util::GetVariableOffset("Mio", "node");
	uintptr_t playerLocationAddr = plrObjAddr + ModAPI::Util::GetVariableOffset("Node2", "_transform") + ModAPI::Util::GetVariableOffset("Aff", "translation");
	uintptr_t playerHealthAddr = plrObjAddr + ModAPI::Util::GetVariableOffset("Mio", "life") + ModAPI::Util::GetVariableOffset("Mio::Life", "health_points");
	uintptr_t playerNacreBasePtrAddr = baseAddr + 0x01114AD0;
	uintptr_t playerStaminaPtrAddr = plrObjAddr + ModAPI::Util::GetVariableOffset("Mio", "charge") + ModAPI::Util::GetVariableOffset("Mio::Charge", "charge");

	uintptr_t playerVelocityBaseAddr = plrObjAddr + ModAPI::Util::GetVariableOffset("Mio", "velocity");
	uintptr_t menuStateAddr = metagameAddr + ModAPI::Util::GetVariableOffset("Metagame", "state");

	uintptr_t hitEnemyFunctionAddress = baseAddr + ModAPI::Util::GetMethodOffset("public: virtual void __cdecl AI_brain::take_damage(struct Combat_hit const &)");
	uintptr_t giveFlagFunctionAddress = baseAddr + ModAPI::Util::GetMethodOffset("public: struct Save_entry * __cdecl Game::loot(struct String const &,int,enum Loot_flags)");
	uintptr_t moveByFunctionAddress = baseAddr + ModAPI::Util::GetMethodOffset("public: void __cdecl Mio::move_by_slide(struct Vec<float,3>)");
	uintptr_t getSaveEntryAddress = baseAddr + ModAPI::Util::GetMethodOffset("public: struct Save_entry * __cdecl Save::acquire(struct String const &)");
	
	// Store the address
	ModAPI::Addresses::g_BaseAddr = baseAddr;

	ModAPI::Pointers::g_PlayerNodeBasePtr = (void**)playerNodeBasePtrAddr;

	ModAPI::Addresses::g_MenuStateAddr = (void*)menuStateAddr;
	ModAPI::Addresses::g_PlayerHealthAddress = (void*)playerHealthAddr;

	ModAPI::Addresses::g_MoveByMethodAddr = (void*)(moveByFunctionAddress);

	ModAPI::Addresses::g_PlayerLocationAddr = (void*)playerLocationAddr;
	ModAPI::Addresses::g_PlayerStaminaAddr = (void*)playerStaminaPtrAddr;
	ModAPI::Addresses::g_GameAddr = (void*)(gameAddr);
	ModAPI::Addresses::g_PlayerObjAddr = (void*)(plrObjAddr);
	ModAPI::Addresses::g_SaveAddress = (void*)(saveAddr);

	ModAPI::Addresses::g_PlayerVelocityAddr = (void*)(playerVelocityBaseAddr);

	ModAPI::Addresses::g_HitEnemyAddress = (void*)hitEnemyFunctionAddress;
	ModAPI::Addresses::g_GiveFlagAddress = (void*)giveFlagFunctionAddress;
	ModAPI::Addresses::g_GetSaveEntryAddress = (void*)getSaveEntryAddress;
}
extern "C" __declspec(dllexport) void ModInit(char* id) {
	//Making the printfs actually be sent to console
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	modId = id;
	LoadMemoryAddresses();

	ModAPI::Hooks::InitializeHooks();
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return TRUE;
}

