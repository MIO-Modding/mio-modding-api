#pragma once
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

typedef struct GinKey {
	uint32_t ver;
	uint32_t res[2];
	char id[16];
	uint32_t res2;
	char path[256];
	uint64_t check[2];
} GinKey;
typedef struct GinSectionInfo {
    char name[64];
    uint64_t offset;
    uint32_t size;
    uint32_t c_size;
    uint32_t flags;
    uint32_t params[4];
    uint32_t ver;
    char id[16];
    uint64_t check[2];
} GinSectionInfo;

std::pair<GinKey, std::unordered_map<std::string, std::pair<GinSectionInfo, std::vector<char>>>> DecompileGin(fs::path file);
std::vector<char> RecompileGin(std::pair<GinKey, std::unordered_map<std::string, std::pair<GinSectionInfo, std::vector<char>>>> data);