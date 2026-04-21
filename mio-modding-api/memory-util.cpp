#include "Windows.h"
#include <psapi.h>
#include "mio-modding-api.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "saved-addresses.h"

namespace fs = std::filesystem;


namespace ModAPI {
    namespace Util {
		extern "C" {
			MODDING_API bool WriteMemory(void* address, const void* data, size_t size) {
				DWORD oldProtect;
				if(!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
					return false;
				}
				memcpy(address, data, size);
				VirtualProtect(address, size, oldProtect, &oldProtect);
				return true;
			}

            MODDING_API bool ReadMemory(void* address, void* buffer, size_t size) {
                DWORD oldProtect;
                if (!VirtualProtect(address, size, PAGE_READONLY, &oldProtect)) {
                    return false;
                }
                memcpy(buffer, address, size);
                VirtualProtect(address, size, oldProtect, &oldProtect);
                return true;
            }

            MODDING_API bool PatchBytes(void* address, const char* bytes, size_t size) {
                DWORD oldProtect;
                if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    return false;
                }
                memcpy(address, bytes, size);
                VirtualProtect(address, size, oldProtect, &oldProtect);
                return true;
            }

            MODDING_API bool NopBytes(void* address, size_t count) {
                DWORD oldProtect;
                if (!VirtualProtect(address, count, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    return false;
                }
                memset(address, 0x90, count);
                VirtualProtect(address, count, oldProtect, &oldProtect);
                return true;
            }

            MODDING_API void* PatternScan(HMODULE module, const char* pattern, const char* mask) {
                MODULEINFO moduleInfo;
                GetModuleInformation(GetCurrentProcess(), module, &moduleInfo,
                    sizeof(MODULEINFO));

                char* base = (char*)moduleInfo.lpBaseOfDll;
                size_t size = moduleInfo.SizeOfImage;
                size_t patternLength = strlen(mask);

                for (size_t i = 0; i < size - patternLength; i++) {
                    bool found = true;
                    for (size_t j = 0; j < patternLength; j++) {
                        if (mask[j] != '?' && pattern[j] != base[i + j]) {
                            found = false;
                            break;
                        }
                    }
                    if (found)
                        return &base[i];
                }
                return nullptr;
            }
            MODDING_API void* PatternScanReverse(HMODULE module, void* from, const char* pattern, const char* mask) {
                char* base = (char*)module;
                size_t patternLength = strlen(mask);

                for (size_t i = (uintptr_t)from - (uintptr_t)base; i > 0; i--) {
                    bool found = true;
                    for (size_t j = 0; j < patternLength; j++) {
                        if (mask[j] != '?' && pattern[j] != base[i + j]) {
                            found = false;
                            break;
                        }
                    }
                    if (found)
                        return &base[i];
                }
                return nullptr;
            }

            MODDING_API void* FollowPointer(void* basePtr, int offset) {
                if (!basePtr)
                    return nullptr;

                void* ptr = *(void**)basePtr;
                if (!ptr)
                    return nullptr;

                return (void*)((uintptr_t)ptr + offset);
			}
			MODDING_API uintptr_t GetMethodOffset(const char* method) {
				MethodAddress addr = Internal::methods[std::string(method)];
				return addr.addr;
			}
			MODDING_API size_t GetMethodSize(const char* method) {
				MethodAddress addr = Internal::methods[std::string(method)];
				return addr.size;
			}
			MODDING_API uintptr_t GetStaticVariableOffset(const char* variable) {
				return Internal::staticVariables[std::string(variable)];
			}
			MODDING_API uintptr_t GetVariableOffset(const char* structure, const char* variable) {
				return Internal::structs[std::string(structure)][std::string(variable)];
			}
        }
    }
}