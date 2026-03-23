#include <filesystem>
#include "mio-modding-api.h"
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <zstd.h>
#include <lz4.h>
#include "gin-serialization.h"
#include <MurmurHash3.h>
#include "mio-modding-api-internal.h"

namespace fs = std::filesystem;

template <typename T>
void WriteToVector(std::vector<char>& byteVector, const T& value) {
    const char* bytes = reinterpret_cast<const char*>(&value);
    byteVector.insert(byteVector.end(), bytes, bytes + sizeof(T));
}

const uint32_t gin_magic = 0x004E4947;
const uint32_t gin_version = 2; 
std::vector<char> RecompileGin(std::pair<GinKey, std::map<std::string, std::pair<GinSectionInfo, std::vector<char>>>> data) {
    std::vector<char> output;
    WriteToVector<uint32_t>(output, gin_magic);
    WriteToVector<uint32_t>(output, data.first.ver);
    WriteToVector<uint32_t[2]>(output, data.first.res);
    WriteToVector<char[16]>(output, data.first.id);
    WriteToVector<uint32_t>(output, data.first.res2);
    WriteToVector<char[256]>(output, data.first.path);
    uint32_t sections = data.second.size();
    WriteToVector(output, sections);
    //uint32_t checkInd = output.size();
    WriteToVector<uint64_t[2]>(output, data.first.check);
    std::vector<std::vector<char>> newSections;
    uint64_t offset = output.size() + (sections * sizeof(GinSectionInfo));
    //std::vector<char> checksumData;
    for (auto& i : data.second) {
        GinSectionInfo info = GinSectionInfo();
        std::copy(std::begin(i.second.first.name), std::end(i.second.first.name), std::begin(info.name));
        uint32_t flags = i.second.first.flags;
        std::vector<char> data = i.second.second;
        std::vector<char> finalData;
        size_t uncompressedSize = data.size();
        size_t compressedSize = 0;

        if (flags & (1 << 4)) { // ZSTD
            size_t maxCompressedSize = ZSTD_compressBound(uncompressedSize);
            finalData.resize(maxCompressedSize);
            compressedSize = ZSTD_compress(finalData.data(), maxCompressedSize, data.data(), uncompressedSize, 3);
            finalData.resize(compressedSize);
        }
        else if (flags & (1 << 5)) { // LZ4
            size_t maxCompressedSize = LZ4_compressBound(uncompressedSize);
            finalData.resize(maxCompressedSize);
            compressedSize = LZ4_compress_default(
                data.data(),
                finalData.data(),
                uncompressedSize,
                maxCompressedSize
            );
            finalData.resize(compressedSize);
        }
        else {
            finalData = data;
        }
        newSections.push_back(finalData);
        info.size = uncompressedSize;
        info.c_size = compressedSize;
        info.flags = flags;
        std::copy(std::begin(i.second.first.params), std::end(i.second.first.params), std::begin(info.params));
        info.ver = i.second.first.ver;
        std::copy(std::begin(i.second.first.id), std::end(i.second.first.id), std::begin(info.id));
        std::copy(std::begin(i.second.first.check), std::end(i.second.first.check), std::begin(info.check));
        info.offset = offset;

        uint64_t sectionCheck[2] = { 0, 0 };
        MurmurHash3_x64_128(finalData.data(), static_cast<int>(finalData.size()), 0, sectionCheck);

        std::vector<char> sectionVector;
        WriteToVector<char[64]>(sectionVector, info.name);
        WriteToVector<uint64_t>(sectionVector, info.offset);
        WriteToVector<uint32_t>(sectionVector, info.size);
        WriteToVector<uint32_t>(sectionVector, info.c_size);
        WriteToVector<uint32_t>(sectionVector, info.flags);
        WriteToVector<uint32_t[4]>(sectionVector, info.params);
        WriteToVector<uint32_t>(sectionVector, info.ver);
        WriteToVector<char[16]>(sectionVector, info.id);
        WriteToVector<uint64_t[2]>(sectionVector, sectionCheck);
        output.insert(output.end(), sectionVector.begin(), sectionVector.end());
        offset += finalData.size();
    }
    for (auto& i : newSections) {
        output.insert(output.end(), i.begin(), i.end());
        //checksumData.insert(checksumData.end(), i.begin(), i.end());
    }

    //uint64_t check[2] = { 0, 0 };
    //MurmurHash3_x64_128(checksumData.data(), static_cast<int>(checksumData.size()), 0, check);
    //std::memcpy(&output[checkInd], check, 16);

    return output;
}
std::pair<GinKey, std::map<std::string, std::pair<GinSectionInfo, std::vector<char>>>> DecompileGin(fs::path file) {
    std::map<std::string, std::pair<GinSectionInfo, std::vector<char>>> output;
    if (!fs::exists(file)) {
        LogMessage("Failed to decompile gin: File doesnt exist");
        return std::pair<GinKey, std::map<std::string, std::pair<GinSectionInfo, std::vector<char>>>>();
    }
    std::ifstream fileData(file, std::ios::binary);
    if (!fileData.is_open()) {
        LogMessage("Failed to decompile gin: File couldnt open");
        return std::pair<GinKey, std::map<std::string, std::pair<GinSectionInfo, std::vector<char>>>>();
    }
    uint32_t magic;
    fileData.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (magic != gin_magic) {
        LogMessage("Failed to decompile gin: File isnt a gin file");
        return std::pair<GinKey, std::map<std::string, std::pair<GinSectionInfo, std::vector<char>>>>();
    }
    GinKey ginKey = GinKey();
    uint32_t ver;
    fileData.read(reinterpret_cast<char*>(&ver), sizeof(ver));
    ginKey.ver = ver;
    fileData.read(reinterpret_cast<char*>(&ginKey.res), sizeof(ginKey.res));
    fileData.read(reinterpret_cast<char*>(&ginKey.id), sizeof(ginKey.id));
    fileData.read(reinterpret_cast<char*>(&ginKey.res2), sizeof(ginKey.res2));
    fileData.read(reinterpret_cast<char*>(&ginKey.path), sizeof(ginKey.path));
    uint32_t sectionCount;
    fileData.read(reinterpret_cast<char*>(&sectionCount), sizeof(sectionCount));
    fileData.read(reinterpret_cast<char*>(&ginKey.check), sizeof(ginKey.check));

    std::vector<GinSectionInfo> sections = std::vector<GinSectionInfo>();
    for (int i = 0; i < sectionCount; i++) {
        GinSectionInfo current = GinSectionInfo();

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
        GinSectionInfo current = sections[i];
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
        std::string finalName = name;
        output[finalName] = std::pair<GinSectionInfo, std::vector<char>>(current, finalData);
    }
    LogMessage((std::string("Finished Decompiling ") + std::to_string(sectionCount) + " sections").c_str());
    fileData.close();
    return std::pair<GinKey, std::map<std::string, std::pair<GinSectionInfo, std::vector<char>>>>(ginKey, output);
}