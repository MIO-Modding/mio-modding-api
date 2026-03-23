#include <map>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "mio-modding-api.h"
#include "gin-serialization.h"
#include "mio-modding-api-internal.h"

namespace fs = std::filesystem;

std::map<fs::path, std::vector<char>> originalFlambyFileData;
std::map<fs::path, std::vector<char>> newFlambyFileData;
std::map<fs::path, std::pair<GinKey, std::map<std::string, std::pair<GinSectionInfo, std::vector<char>>>>> ginData;
std::vector<fs::path> overwrittenFlambyData;

void OverwriteFlambyFile(fs::path file, std::vector<char> data) {
	if (fs::exists(file)) {
		overwrittenFlambyData.push_back(file);
	}
	newFlambyFileData[file] = data;
}
void ModifyGin(fs::path file, std::string section, std::vector<char> data) {
	if (fs::exists(file) && ginData.count(file) <= 0) {
		ginData[file] = DecompileGin(file);
	}
	ginData[file].second[section].second = data;
}
void ModifyGin(fs::path file, std::string section, fs::path path) {
	std::ifstream fileData(path, std::ios::binary);
	std::vector<char> bytes(
		(std::istreambuf_iterator<char>(fileData)),
		(std::istreambuf_iterator<char>())
	);
	fileData.close();
	ModifyGin(file, section, bytes);
}
void LoadFlambyData(fs::path directory) {
	for (const fs::directory_entry entry : fs::directory_iterator(directory)) {
		if (entry.is_regular_file()) {
			fs::path file = entry.path();
			std::ifstream fileData(file, std::ios::binary);
			std::vector<char> bytes(
				(std::istreambuf_iterator<char>(fileData)),
				(std::istreambuf_iterator<char>())
			);
			fileData.close();
			originalFlambyFileData[file] = bytes;
		}
		else if (entry.is_directory()) {
			LoadFlambyData(entry.path());
		}
	}
}
void RestoreFlambyOriginalData() {
	fs::remove_all("flamby_modified");
	fs::create_directory("flamby_modified");
	for (fs::path key : overwrittenFlambyData) {
		std::vector<char> value = originalFlambyFileData[key];
		fs::copy(key, fs::path("flamby_modified") / key);
		fs::remove(key);
		std::ofstream stream(key, std::ios::out | std::ios::binary);
		stream.write(reinterpret_cast<const char*>(value.data()), value.size());
		stream.close();
	}
}
void ApplyFlambyData() {
	LogMessage("Recompiling Gin");
	for (std::pair<fs::path, std::pair<GinKey, std::map<std::string, std::pair<GinSectionInfo, std::vector<char>>>>> i : ginData) {
		OverwriteFlambyFile(i.first, RecompileGin(i.second));
	}
	LogMessage("Finished Recompiling Gin");
	LogMessage("Writing Gin Data");
	for (std::pair<fs::path, std::vector<char>> i : newFlambyFileData) {
		fs::path key = i.first;
		std::vector<char> value = i.second;
		fs::remove(key);
		std::ofstream stream(key, std::ios::out | std::ios::binary);
		stream.write(value.data(), value.size());
		stream.close();
	}
	LogMessage("Finished Writing Gin Data");
}
void PatchChecksum() {
	HMODULE hModule = GetModuleHandleA("mio.exe");
	uintptr_t baseAddr = (uintptr_t)hModule;
	uintptr_t addr = baseAddr + 0x31678;
	std::vector<char> bytes = { (char)0x39, (char)0xc0, (char)0x90, (char)0x90, (char)0x90 };
	for (char i : bytes) {
		ModAPI::Util::WriteMemoryTyped((void*)addr, i);
		addr += 1;
	}
}