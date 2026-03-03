#include <unordered_map>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <modding_api.h>
#include <zstd.h>
#include <lz4.h>
#include <gin_serializer.h>

namespace fs = std::filesystem;

std::unordered_map<fs::path, std::vector<char>> originalFlambyFileData;
std::unordered_map<fs::path, std::vector<char>> newFlambyFileData;
std::unordered_map<fs::path, std::unordered_map<std::string, std::vector<char>>> ginData;
std::vector<fs::path> overwrittenFlambyData;

extern "C" {
	void OverwriteFlambyFile(fs::path file, std::vector<char> data) {
		if (fs::exists(file) && std::count(overwrittenFlambyData.begin(), overwrittenFlambyData.end(), file) <= 0) {
			overwrittenFlambyData.push_back(file);
		}
		newFlambyFileData[file] = data;
	}
    void ModifyGin(fs::path file, std::string section, std::vector<char> data) {
        if (fs::exists(file) && ginData.count(file) <= 0) {
			ginData[file] = DecompileGin(file);
        }
        ginData[file][section] = data;
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
		for (fs::path key : overwrittenFlambyData) {
			std::vector<char> value = originalFlambyFileData[key];
			fs::remove(key);
			std::ofstream stream(key, std::ios::out | std::ios::binary);
			stream.write(reinterpret_cast<const char*>(value.data()), value.size());
			stream.close();
		}
	}
	void ApplyFlambyData() {
		for (std::pair<fs::path, std::vector<char>> i : newFlambyFileData) {
			fs::path key = i.first;
			std::vector<char> value = i.second;
			fs::remove(key);
			std::ofstream stream(key, std::ios::out | std::ios::binary);
			stream.write(reinterpret_cast<const char*>(value.data()), value.size());
			stream.close();
		}
	}
}