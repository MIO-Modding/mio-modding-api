#include "mio-modding-api.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace ModAPI {
	namespace Util {
		MODDING_API Enums::MenuState GetGameMenuState() {
			if (!Addresses::g_MenuStateAddr) {
				return Enums::MenuState::Start;
			}

			volatile int* ptr = reinterpret_cast<volatile int*>(Addresses::g_MenuStateAddr);

			return static_cast<Enums::MenuState>(*ptr);
		}

		MODDING_API bool SetGameMenuState(Enums::MenuState state) {
			if (!Addresses::g_MenuStateAddr) {
				return false;
			}

			int* ptr = reinterpret_cast<int*>(Addresses::g_MenuStateAddr);
			int new_value = state;

			return Util::WriteMemoryTyped(ptr, new_value);
		}

		MODDING_API bool CheckIfSaveLoaded() {
			if (!Addresses::g_MenuStateAddr) {
				return false;
			}

			return (Enums::MenuState::Game == GetGameMenuState());
		}

		MODDING_API void WaitForSaveLoaded() {
			while (!Addresses::g_MenuStateAddr) {
				Sleep(100); // Prevent CPU burn
			}

			while (true) {
				if (Util::CheckIfSaveLoaded()) {
					break;
				}

				Sleep(100); // Prevent CPU burn
			}

			// small extra delay to let subsystems finish initializing
			Sleep(500);
		}
		MODDING_API fs::path GetFolderPathForMod(const char* mod) {
			return fs::path("mods") / fs::path(mod);
		}
	}
}