#pragma once
#include <unordered_map>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

void OverwriteFlambyFile(fs::path file, std::vector<char> data);
void RestoreFlambyOriginalData();
void ApplyFlambyData();
void LoadFlambyData(fs::path directory);
void ModifyGin(fs::path file, std::string section, std::vector<char> data);
void ModifyGin(fs::path file, std::string section, fs::path);