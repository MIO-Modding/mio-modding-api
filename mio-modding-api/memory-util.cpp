#include "Windows.h"
#include <psapi.h>
#include "mio-modding-api.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

uintptr_t GetDataSectionHeader() {
	HMODULE hModule = GetModuleHandle(NULL);
	if(!hModule)
		return 0;
	auto baseAddress = reinterpret_cast<PBYTE>(hModule);
	auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(baseAddress);
	if(dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return 0;
	auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(baseAddress + dosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);

	for(WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++, section++) {
		if(strncmp(reinterpret_cast<const char*>(section->Name), ".data", 8) == 0) {
			void* actualAddress = (PBYTE)section->VirtualAddress;
			return (uintptr_t)actualAddress;
		}
	}
	return 0;
}

namespace ModAPI {
    namespace Util {
        extern "C" {
            MODDING_API bool WriteMemory(void* address, const void* data, size_t size) {
                DWORD oldProtect;
                if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
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
				fs::path modJsonPath = GetFolderPathForMod("mio-modding-api") / fs::path("methods.json");
				std::ifstream file(modJsonPath);
				nlohmann::json data = nlohmann::json::parse(file);
				file.close();
				return (uintptr_t)data[method].get<uint64_t>();
			}
			MODDING_API uintptr_t GetStaticVariableOffset(const char* variable) {
				uintptr_t dataHeader = GetDataSectionHeader();
				fs::path modJsonPath = GetFolderPathForMod("mio-modding-api") / fs::path("variables.json");
				std::ifstream file(modJsonPath);
				nlohmann::json data = nlohmann::json::parse(file);
				file.close();
				return dataHeader+(uintptr_t)data[variable].get<uint64_t>();
			}
			MODDING_API uintptr_t GetVariableOffset(const char* structure, const char* variable) {
				fs::path modJsonPath = GetFolderPathForMod("mio-modding-api") / fs::path("structs.json");
				std::ifstream file(modJsonPath);
				nlohmann::json data = nlohmann::json::parse(file);
				file.close();
				nlohmann::json variables = data[structure].get<nlohmann::json>();
				return (uintptr_t)variables[variable].get<uint64_t>();
			}
        }
    }
}