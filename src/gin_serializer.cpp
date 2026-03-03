#include <filesystem>
#include <modding_api.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <zstd.h>
#include <lz4.h>

namespace fs = std::filesystem;

template <typename T> inline void WriteToVector(std::vector<char>* vector, T value) {
    const char* bytes = reinterpret_cast<const char*>(&value);
    vector->insert(vector->end(), bytes, bytes + sizeof(value));
}

extern "C" {
    const uint32_t gin_magic = 0x004E4947;
    const uint32_t gin_version = 2;
    std::vector<char> CompileGin(std::unordered_map<std::string, std::vector<char>> data) {
        std::vector<char> output;
        WriteToVector<uint32_t>(&output, gin_magic);
        WriteToVector<uint32_t>(&output, gin_version);
    }
    std::unordered_map<std::string, std::vector<char>> DecompileGin(fs::path file) {
        std::unordered_map<std::string, std::vector<char>> output;
        if (!fs::exists(file)) {
            LogMessage("Failed to decompile gin: File doesnt exist");
            return;
        }
        std::ifstream fileData(file, std::ios::binary);
        if (!fileData.is_open()) {
            LogMessage("Failed to decompile gin: File couldnt open");
            return;
        }
        uint32_t magic;
        fileData.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        if (magic != gin_magic) {
            LogMessage("Failed to decompile gin: File isnt a gin file");
            return;
        }
        uint32_t ver;
        fileData.read(reinterpret_cast<char*>(&ver), sizeof(ver));
        fileData.ignore(sizeof(uint32_t[2]));
        fileData.ignore(sizeof(char[16]));
        fileData.ignore(sizeof(uint32_t));
        fileData.ignore(sizeof(char[256]));
        uint32_t sectionCount;
        fileData.read(reinterpret_cast<char*>(&sectionCount), sizeof(sectionCount));
        fileData.ignore(sizeof(uint64_t[2]));

        struct section {
            uint8_t name[64];
            uint64_t offset;
            uint32_t size;
            uint32_t c_size;
            uint32_t flags;
            uint32_t params[4];
            uint32_t ver;
            char id[16];
            uint64_t check[2];
        };
        std::vector<section> sections = std::vector<section>();
        for (int i = 0; i < sectionCount; i++) {
            section current = section();

            fileData.read(reinterpret_cast<char*>(&current.name), sizeof(current.name));
            fileData.read(reinterpret_cast<char*>(&current.offset), sizeof(current.offset));
            fileData.read(reinterpret_cast<char*>(&current.size), sizeof(current.size));
            fileData.read(reinterpret_cast<char*>(&current.c_size), sizeof(current.c_size));
            fileData.read(reinterpret_cast<char*>(&current.flags), sizeof(current.flags));
            fileData.read(reinterpret_cast<char*>(&current.params), sizeof(current.params));
            fileData.read(reinterpret_cast<char*>(&current.ver), sizeof(current.ver));
            fileData.read(reinterpret_cast<char*>(&current.id), sizeof(current.id));
            fileData.read(reinterpret_cast<char*>(&current.check), sizeof(current.check));

            sections.push_back(current);
        }

        LogMessage((std::string("Decompiling ") + std::to_string(sectionCount) + " sections").c_str());
        for (int i = 0; i < sections.size(); i++) {
            section current = sections[i];
            std::string name(reinterpret_cast<const char*>(current.name), sizeof(current.name));
            name = name.substr(0, name.find('\x0'));
            uint64_t offset = current.offset;
            uint32_t sizeUncompressed = current.size;
            uint32_t sizeCompressed = current.c_size;
            uint32_t flags = current.flags;
            uint32_t readSize = sizeCompressed > 0 ? sizeCompressed : sizeUncompressed;

            std::streampos currentPos = fileData.tellg();
            fileData.seekg(offset);
            std::vector<char> bytes(readSize);
            fileData.read(reinterpret_cast<char*>(bytes.data()), readSize);
            fileData.seekg(currentPos);

            std::vector<char> finalData(sizeUncompressed);
            if (flags & (1 << 4)) { // ZSTD
                size_t const dSize = ZSTD_decompress(finalData.data(), sizeUncompressed,
                    bytes.data(), bytes.size());
            }
            else if (flags & (1 << 5)) { // LZ4
                int rv = LZ4_decompress_safe(
                    bytes.data(),
                    finalData.data(),
                    bytes.size(),
                    sizeUncompressed
                );
            }
            else {
                finalData = bytes;
            }
            std::string finalName = std::to_string(i) + "_" + name;
            output[finalName] = finalData;
        }
        LogMessage((std::string("Finished Decompiling ") + std::to_string(sectionCount) + " sections").c_str());
        fileData.close();
        return output;
    }
}