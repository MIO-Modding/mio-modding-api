#include "Windows.h"
#include "mio-modding-api.h"
#include <string>

using SaveEntry = ModAPI::SaveData::SaveEntry;

namespace ModAPI {
    namespace SaveData {
		extern "C" {
			MODDING_API SaveEntryValue* GetSaveEntry(const char* name) {
				GameString flagStr = GameString((char*)name);
				SaveEntryValue result = SaveEntryValue();
				return ModAPI::Util::CallAssembly<SaveEntryValue*, uintptr_t, GameString*, SaveEntryValue*>(
					ModAPI::Addresses::g_GetSaveEntryAddress,
					(uintptr_t)ModAPI::Addresses::g_SaveEntriesAddress,
					&flagStr,
					&result);
			}
			MODDING_API bool RemoveSaveEntry(const char* name) {
				GameString flagStr = GameString((char*)name);
				SaveEntryValue result;
				return ModAPI::Util::CallAssembly<bool, uintptr_t, GameString*, SaveEntryValue*>(
					ModAPI::Addresses::g_RemoveSaveEntryAddress,
					(uintptr_t)ModAPI::Addresses::g_SaveEntriesAddress,
					&flagStr,
					&result);
			}
            MODDING_API int32_t GetSaveEntryValueCount(const char* name) {
				SaveEntryValue* entry = GetSaveEntry(name);
                if (!entry) {
                    return 0;
                }
                return entry->count;
            }

            MODDING_API bool SetSaveEntryValueCount(const char* name, int32_t count) {
				SaveEntryValue* entry = GetSaveEntry(name);
                if (!entry) {
                    return false;
                }
                entry->count = count;
                return true;
            }

            MODDING_API uint32_t GetSaveEntryValueFlags(const char* name) {
				SaveEntryValue* entry = GetSaveEntry(name);
                if (!entry) {
                    return 0;
                }
                return entry->flags;
            }

            MODDING_API bool SetSaveEntryValueFlags(const char* name, uint32_t flags) {
				SaveEntryValue* entry = GetSaveEntry(name);
                if (!entry) {
                    return false;
                }
                entry->flags = flags;
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