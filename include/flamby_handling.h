#pragma once
#include <unordered_map>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

extern "C" {
	void OverwriteFlambyFile(fs::path file, std::vector<char> data);
	void RestoreFlambyOriginalData();
	void ApplyFlambyData();
	void LoadFlambyData(fs::path directory);
}