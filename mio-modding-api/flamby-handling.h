#pragma once
#include <map>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

struct File {
	void* fd;
};

struct Gin_section_header {
	char name[0x40];
	uint64_t offset;
	uint32_t size;
	uint32_t compressed_size;
	uint32_t flags;
	uint32_t params[0x4];
	uint32_t section_version;
	char section_id[0x10];
	uint64_t checksum[0x2];
};
struct Gin_header {
	uint32_t magic;
	uint32_t format_version;
	uint32_t reserved[0x2];
	char file_id[0x10];
	uint32_t reserved_2;
	char file_path[0x100];
	uint32_t section_count;
	uint64_t checksum[0x2];
};

template <typename T>
struct GameArray {
	uint32_t count;
	uint32_t capacity;
	T* data;
};
struct Ordered_gin_read {
	uint32_t flags;
	int32_t section;
	void* asset;
};

struct Gin_read_batcher {
	int32_t status;
	uint8_t pad_0[4];
	GameArray<Ordered_gin_read> ordered_reads;
	uint32_t next_idx;
	uint32_t next_subsection;
	GameArray<uint8_t> file_chunk;
	uint64_t chunk_file_offset;
};

struct Gin_read {
	ModAPI::SaveData::GameString path;
	File fd;
	uint64_t file_size;
	Gin_header header;
	GameArray<Gin_section_header> sections;
	uint8_t header_auto_updated;
	uint32_t original_version;
	Gin_read_batcher batcher;
};


struct GinPatch {
	Gin_read* targetRead;
	uint32_t targetIndex;
	uint32_t size;
};

void AddGinPatch(fs::path file, fs::path patch);
void PatchChecksum();
GinPatch GetPatch(fs::path file, int32_t index);
bool HasPatch(fs::path file, int32_t index);