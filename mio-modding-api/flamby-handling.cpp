#include <map>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "mio-modding-api.h"
#include "gin-serialization.h"
#include "mio-modding-api-internal.h"
#include "flamby-handling.h"

namespace fs = std::filesystem;

std::map<fs::path, std::map<int32_t, GinPatch>> patches;
void AddGinPatch(fs::path file, fs::path patch) {
	void* ginReadReadHeader = (void*)(ModAPI::Addresses::g_BaseAddr + ModAPI::Util::GetMethodOffset("public: void __cdecl Gin_read::read_header(bool)"));
	void* ginReadFromFile = (void*)(ModAPI::Addresses::g_BaseAddr + ModAPI::Util::GetMethodOffset("public: static struct Gin_read __cdecl Gin_read::from_file(struct String const &)"));
	void* ginReadFindSection = (void*)(ModAPI::Addresses::g_BaseAddr + ModAPI::Util::GetMethodOffset("public: int __cdecl Gin_read::find_section(struct String const &)"));
	
	Gin_read* ginRead = new Gin_read();
	Gin_read* origGinRead = new Gin_read();

	auto ginStr = ModAPI::SaveData::GameString((char*)patch.string().c_str());
	auto origGinStr = ModAPI::SaveData::GameString((char*)file.string().c_str());

	ModAPI::Util::CallAssembly<Gin_read*, Gin_read*, ModAPI::SaveData::GameString*>(ginReadFromFile, ginRead, &ginStr);

	ModAPI::Util::CallAssembly<Gin_read*, Gin_read*, ModAPI::SaveData::GameString*>(ginReadFromFile, origGinRead, &origGinStr);

	ModAPI::Util::CallAssembly<Gin_read*, Gin_read*, bool>(ginReadReadHeader, ginRead, 1);
	ModAPI::Util::CallAssembly<Gin_read*, Gin_read*, bool>(ginReadReadHeader, origGinRead, 1);

	std::map<int32_t, GinPatch> lPatches;
	for(int i = 0; i < ginRead->header.section_count; i++) {
		auto nameStr = ModAPI::SaveData::GameString(ginRead->sections.data[i].name);

		int32_t sectionIndex = ModAPI::Util::CallAssembly<int32_t, Gin_read*, ModAPI::SaveData::GameString*>(ginReadFindSection, origGinRead, &nameStr);
		LogMessage(std::to_string(sectionIndex).c_str());
		LogMessage(nameStr.data);

		GinPatch ginPatch = GinPatch();
		ginPatch.targetIndex = i;
		ginPatch.targetRead = ginRead;

		lPatches[sectionIndex] = ginPatch;
	}

	patches[std::string(origGinRead->path.data)] = lPatches;
	delete origGinRead;
}
GinPatch GetPatch(fs::path file, int32_t index) {
	return patches[file][index];
}
bool HasPatch(fs::path file, int32_t index) {
	if (patches.count(file) && patches[file].count(index)) {
		return true;
	}
	return false;
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