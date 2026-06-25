#include "Windows.h"
#include "mio-modding-api.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <psapi.h>
#include "saved-addresses.h"

using namespace ModAPI::Util;
namespace fs = std::filesystem;

namespace Internal {
	std::unordered_map<std::string, uintptr_t> staticVariables;
	std::unordered_map<std::string, MethodAddress> methods;
	std::unordered_map<std::string, std::unordered_map<std::string, uintptr_t>> structs;

	uintptr_t GetSectionHeader(const char* sectionName) {
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
			if(strncmp(reinterpret_cast<const char*>(section->Name), sectionName, 8) == 0) {
				void* actualAddress = (PBYTE)section->VirtualAddress;
				return (uintptr_t)actualAddress;
			}
		}
		return 0;
	}

	void ReloadSavedAddresses() {
		uintptr_t textHeader = GetSectionHeader(".text");
		uintptr_t dataHeader = GetSectionHeader(".data");

		fs::path basePath = GetFolderPathForMod("mio-modding-api");
		fs::path methodsPath = basePath / fs::path("methods.json");
		fs::path variablesPath = basePath / fs::path("variables.json");
		fs::path structsPath = basePath / fs::path("structs.json");

		std::ifstream file(methodsPath);
		nlohmann::json data = nlohmann::json::parse(file);
		file.close();
		for(const auto& item : data.items()) {
			nlohmann::json data2 = item.value().get<nlohmann::json>();
			methods[item.key()] = MethodAddress(textHeader + (uintptr_t)data2["address"].get<uint64_t>(), (size_t)data2["address"].get<uint64_t>());
		}

		file = std::ifstream(variablesPath);
		data = nlohmann::json::parse(file);
		file.close();
		for(const auto& item : data.items()) {
			staticVariables[item.key()] = dataHeader+(uintptr_t) item.value().get<uint64_t>();
		}

		file = std::ifstream(structsPath);
		data = nlohmann::json::parse(file);
		file.close();
		for(const auto& item : data.items()) {
			std::unordered_map<std::string, uintptr_t> map;
			nlohmann::json data2 = item.value().get<nlohmann::json>();
			for(const auto& item2 : data2.items()) {
				nlohmann::json data3 = item2.value().get<nlohmann::json>();
				if (!data3.contains("offset")) {
					continue;
				}
				map[item2.key()] = (uintptr_t)data3["offset"].get<uint64_t>();
			}
			structs[item.key()] = map;
		}
	}
}