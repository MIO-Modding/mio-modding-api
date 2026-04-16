#include "Windows.h"
#include "mio-modding-api.h"
#include <string>

using SaveEntry = ModAPI::SaveData::SaveEntry;

namespace ModAPI {
    namespace SaveData {
        extern "C" {
			MODDING_API SaveEntry* GetSaveEntry(const char* name) {
				GameString flagStr = GameString((char*)name);
				return ModAPI::Util::CallAssembly<SaveEntry*, uintptr_t, GameString>(
					ModAPI::Addresses::g_GiveFlagAddress,
					(uintptr_t)ModAPI::Addresses::g_GameAddr,
					flagStr);
			}
            MODDING_API int32_t GetSaveEntryValueCount(const char* name) {
                SaveEntry* entry = GetSaveEntry(name);
                if (!entry) {
                    return 0;
                }
                return entry->value.count;
            }

            MODDING_API bool SetSaveEntryValueCount(const char* name, int32_t count) {
                SaveEntry* entry = GetSaveEntry(name);
                if (!entry) {
                    return false;
                }
                entry->value.count = count;
                return true;
            }

            MODDING_API uint32_t GetSaveEntryValueFlags(const char* name) {
                SaveEntry* entry = GetSaveEntry(name);
                if (!entry) {
                    return 0;
                }
                return entry->value.flags;
            }

            MODDING_API bool SetSaveEntryValueFlags(const char* name, uint32_t flags) {
                SaveEntry* entry = GetSaveEntry(name);
                if (!entry) {
                    return false;
                }
                entry->value.flags = flags;
                return true;
			}
			MODDING_API char* GiveFlag(const char* flag, int32_t amount) {
				GameString flagStr = GameString((char*)flag);
				return ModAPI::Util::CallAssembly<char*, uintptr_t, GameString, int32_t>(
					ModAPI::Addresses::g_GiveFlagAddress,
					(uintptr_t)ModAPI::Addresses::g_GameAddr,
					flagStr,
					amount);
			}
        }
    }
}