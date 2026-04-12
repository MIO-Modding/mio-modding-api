#include "Windows.h"
#include "mio-modding-api.h"

using SaveEntry = ModAPI::SaveData::SaveEntry;

namespace ModAPI {
    namespace SaveData {
        extern "C" {
            MODDING_API int GetSaveEntryCount() {
                if (!ModAPI::SaveData::g_SaveArraySize) {
                    return 0;
                }
                return *ModAPI::SaveData::g_SaveArraySize;
            }

            // Returns a pointer to the actual entry in memory
            MODDING_API SaveEntry* GetSaveEntryByIndex(int index) {
                if (!ModAPI::Pointers::g_SaveArrayPtr || !*ModAPI::Pointers::g_SaveArrayPtr) {
                    return nullptr;
                }

                int count = GetSaveEntryCount();
                if (index < 0 || index >= count) {
                    return nullptr;
                }

                SaveEntry* entries = (SaveEntry*)*ModAPI::Pointers::g_SaveArrayPtr;
                return (SaveEntry*)((char*)entries + (index * 88));
            }

            // Returns a pointer to the actual entry in memory by name
            // We don't know the hash algorithm, so we just compare names directly
            MODDING_API SaveEntry* GetSaveEntry(const char* name) {
                if (!ModAPI::Pointers::g_SaveArrayPtr || !*ModAPI::Pointers::g_SaveArrayPtr || !name) {
                    return nullptr;
                }

                SaveEntry* entries = (SaveEntry*)*ModAPI::Pointers::g_SaveArrayPtr;
                int count = GetSaveEntryCount();

                for (int i = 0; i < count; i++) {
                    SaveEntry* entry = (SaveEntry*)((char*)entries + (i * 88));
                    // Skip empty entries
                    if (!entry->entry_name.data) {
                        continue;
                    }
                    // Compare the actual string names
                    if (strcmp(entry->entry_name.data, name) == 0) {
                        return entry;
                    }
                }

                return nullptr;
            }

            MODDING_API bool SaveEntryExists(const char* name) {
                return GetSaveEntry(name) != nullptr;
            }

            MODDING_API const char* GetSaveEntryName(int index) {
                SaveEntry* entry = GetSaveEntryByIndex(index);
                if (!entry || !entry->entry_name.data) {
                    return nullptr;
                }
                return entry->entry_name.data;
            }

            // Helper functions for reading/writing count
            MODDING_API bool GetSaveEntryValueCount(const char* name, int32_t* outCount) {
                SaveEntry* entry = GetSaveEntry(name);
                if (!entry || !outCount) {
                    return false;
                }
                *outCount = entry->value.count;
                return true;
            }

            MODDING_API bool SetSaveEntryValueCount(const char* name, int32_t count) {
                SaveEntry* entry = GetSaveEntry(name);
                if (!entry) {
                    return false;
                }
                entry->value.count = count;
                return true;
            }

            MODDING_API bool GetSaveEntryValueFlags(const char* name, uint32_t* outFlags) {
                SaveEntry* entry = GetSaveEntry(name);
                if (!entry || !outFlags) {
                    return false;
                }
                *outFlags = entry->value.flags;
                return true;
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
				uintptr_t globalPtrAddr = ModAPI::Addresses::g_BaseAddr + 0x10efbf0;
				void* actualObject = *reinterpret_cast<void**>(globalPtrAddr);
				return ModAPI::Util::CallAssembly<char*, void*, const char*, int32_t>(ModAPI::Addresses::g_GiveFlagAddress, actualObject, flag, amount);
			}
        }
    }
}