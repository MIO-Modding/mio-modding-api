#pragma once
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

extern "C" {
	std::unordered_map<std::string, std::vector<char>> DecompileGin(fs::path file);
}