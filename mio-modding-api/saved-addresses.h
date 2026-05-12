#pragma once

#include "Windows.h"
#include "mio-modding-api.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <psapi.h>

struct MethodAddress {
	uintptr_t addr;
	size_t size;
	MethodAddress(uintptr_t addr, size_t size) {
		this->addr = addr;
		this->size = size;
	}
	MethodAddress() {}
};
namespace Internal {

	extern std::unordered_map<std::string, uintptr_t> staticVariables;
	extern std::unordered_map<std::string, MethodAddress> methods;
	extern std::unordered_map<std::string, std::unordered_map<std::string, uintptr_t>> structs;

	void ReloadSavedAddresses();
}