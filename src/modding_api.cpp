#include "modding_api.h"
#include <psapi.h>
#include <stdio.h>

#pragma comment(lib, "psapi.lib")

namespace ModAPI {
// Constant addresses
void *g_PlayerStaminaAddr = nullptr;
void *g_PlayerVelocityXAddr = nullptr;
void *g_PlayerVelocityYAddr = nullptr;

// Base address for pointer chain
void **g_PlayerLocationBasePtr = nullptr;
void **g_PlayerHealthBasePtr = nullptr;
void **g_PlayerNacreBasePtr = nullptr;

void ***g_SaveArrayPtr = nullptr;
u32 *g_SaveArraySize = nullptr;

void *g_menuStateAddr = nullptr;

// Offsets from Cheat Engine pointer scan
const int PLAYER_X_OFFSET = 0x18;
const int PLAYER_Y_OFFSET = 0x1C; // Usually Y is 4 bytes after X
const int PLAYER_Z_OFFSET = 0x20; // Usually Z is 4 bytes after Y
const int PLAYER_HEALTH_OFFSET = 0xD4;
const int PLAYER_LIQUID_NACRE_OFFSET = 0x754;

// Custom save entry hashmap stuff
const int ENTRY_SIZE = 88; // sizeof(SaveEntry)
const int HASHMAP_CAPACITY = 8192;
SaveEntry *customHashmap = nullptr;
u32 customHashmapSize = 0;
bool hashmapInitialized = false;
MenuState lastMenuState = MenuState::Start; // Track menu state changes

char *stringBuffer = nullptr;
const size_t STRING_BUFFER_SIZE = 1024 * 1024; // 1MB
size_t stringBufferOffset = 0;
} // namespace ModAPI

// Helper function to follow pointer chain
void *FollowPointer(void *basePtr, int offset) {
  if (!basePtr)
    return nullptr;

  void *ptr = *(void **)basePtr;
  if (!ptr)
    return nullptr;

  return (void *)((uintptr_t)ptr + offset);
}

// ========================================
// Memory Functions
// ========================================

extern "C" {
MODDING_API bool WriteMemory(void *address, const void *data, size_t size) {
  DWORD oldProtect;
  if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
    return false;
  }
  memcpy(address, data, size);
  VirtualProtect(address, size, oldProtect, &oldProtect);
  return true;
}

MODDING_API bool ReadMemory(void *address, void *buffer, size_t size) {
  DWORD oldProtect;
  if (!VirtualProtect(address, size, PAGE_READONLY, &oldProtect)) {
    return false;
  }
  memcpy(buffer, address, size);
  VirtualProtect(address, size, oldProtect, &oldProtect);
  return true;
}

MODDING_API bool PatchBytes(void *address, const char *bytes, size_t size) {
  DWORD oldProtect;
  if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
    return false;
  }
  memcpy(address, bytes, size);
  VirtualProtect(address, size, oldProtect, &oldProtect);
  return true;
}

MODDING_API bool NopBytes(void *address, size_t count) {
  DWORD oldProtect;
  if (!VirtualProtect(address, count, PAGE_EXECUTE_READWRITE, &oldProtect)) {
    return false;
  }
  memset(address, 0x90, count);
  VirtualProtect(address, count, oldProtect, &oldProtect);
  return true;
}

MODDING_API void *PatternScan(const char *pattern, const char *mask) {
  MODULEINFO moduleInfo;
  GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &moduleInfo,
                       sizeof(MODULEINFO));

  char *base = (char *)moduleInfo.lpBaseOfDll;
  size_t size = moduleInfo.SizeOfImage;
  size_t patternLength = strlen(mask);

  for (size_t i = 0; i < size - patternLength; i++) {
    bool found = true;
    for (size_t j = 0; j < patternLength; j++) {
      if (mask[j] != '?' && pattern[j] != base[i + j]) {
        found = false;
        break;
      }
    }
    if (found)
      return &base[i];
  }
  return nullptr;
}

// ========================================
// Menu Functions
// ========================================

MODDING_API MenuState GetGameMenuState() {
  if (!ModAPI::g_menuStateAddr) {
    return MenuState::Start;
  }

  volatile int *ptr = reinterpret_cast<volatile int *>(ModAPI::g_menuStateAddr);

  return static_cast<MenuState>(*ptr);
}

MODDING_API bool SetGameMenuState(MenuState state) {
  if (!ModAPI::g_menuStateAddr) {
    return false;
  }

  int *ptr = reinterpret_cast<int *>(ModAPI::g_menuStateAddr);
  int new_value = state;

  return WriteMemoryTyped(ptr, new_value);
}

MODDING_API bool CheckIfSaveLoaded() {
  if (!ModAPI::g_menuStateAddr) {
    return false;
  }

  return (MenuState::Game == GetGameMenuState());
}

MODDING_API void WaitForSaveLoad() {
  while (!ModAPI::g_menuStateAddr) {
    Sleep(100); // Prevent CPU burn
  }

  while (true) {
    if (CheckIfSaveLoaded()) {
      break;
    }

    Sleep(100); // Prevent CPU burn
  }

  // small extra delay to let subsystems finish initializing
  Sleep(500);
}

// ========================================
// Player Functions
// ========================================

MODDING_API f32x3 GetPlayerLocation() {
  f32x3 loc = {-1.0f, -1.0f, -1.0f};

  if (!ModAPI::g_PlayerLocationBasePtr) {
    return loc;
  }

  // Follow the pointer chain
  void *playerXAddr =
      FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_X_OFFSET);
  void *playerYAddr =
      FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_Y_OFFSET);
  void *playerZAddr =
      FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_Z_OFFSET);

  if (playerXAddr) {
    loc.x = *(float *)playerXAddr;
  }
  if (playerYAddr) {
    loc.y = *(float *)playerYAddr;
  }
  if (playerZAddr) {
    loc.z = *(float *)playerZAddr;
  }

  return loc;
}

MODDING_API bool SetPlayerLocation(f32x3 location) {
  if (!ModAPI::g_PlayerLocationBasePtr) {
    return false;
  }

  void *playerXAddr =
      FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_X_OFFSET);
  void *playerYAddr =
      FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_Y_OFFSET);
  void *playerZAddr =
      FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_Z_OFFSET);

  bool success = true;
  if (playerXAddr) {
    success &= WriteMemoryTyped(playerXAddr, location.x);
  }
  if (playerYAddr) {
    success &= WriteMemoryTyped(playerYAddr, location.y);
  }
  if (playerZAddr) {
    success &= WriteMemoryTyped(playerZAddr, location.z);
  }

  return success;
}

MODDING_API f32x2 GetPlayerVelocity() {
  f32x2 vel = {-1.0f, -1.0f};

  if (!ModAPI::g_PlayerVelocityXAddr || !ModAPI::g_PlayerVelocityYAddr) {
    return vel;
  }

  vel.x = *(float *)ModAPI::g_PlayerVelocityXAddr;
  vel.y = *(float *)ModAPI::g_PlayerVelocityYAddr;

  return vel;
}

MODDING_API bool SetPlayerVelocity(f32x2 velocity) {
  if (!ModAPI::g_PlayerVelocityXAddr || !ModAPI::g_PlayerVelocityYAddr) {
    return false;
  }

  bool success = true;
  success &= WriteMemoryTyped(ModAPI::g_PlayerVelocityXAddr, velocity.x);
  success &= WriteMemoryTyped(ModAPI::g_PlayerVelocityYAddr, velocity.y);

  return success;
}

MODDING_API int GetPlayerHealth() {
  if (!ModAPI::g_PlayerHealthBasePtr) {
    return -1;
  }

  void *playerHealthAddr = FollowPointer(ModAPI::g_PlayerHealthBasePtr,
                                         ModAPI::PLAYER_HEALTH_OFFSET);

  if (playerHealthAddr) {
    return *(int *)playerHealthAddr;
  } else {
    return -1;
  }
}

MODDING_API bool SetPlayerHealth(int health) {
  if (!ModAPI::g_PlayerHealthBasePtr) {
    return false;
  }

  void *playerHealthAddr = FollowPointer(ModAPI::g_PlayerHealthBasePtr,
                                         ModAPI::PLAYER_HEALTH_OFFSET);

  bool success = true;
  if (playerHealthAddr) {
    success &= WriteMemoryTyped(playerHealthAddr, health);
  }

  return success;
}

MODDING_API int GetPlayerLiquidNacre() {
  i32 *count = nullptr;
  GetSaveEntryValueCount("RESOURCE:PEARL_SHARDS", count);
  return (int)*count;
}

MODDING_API bool SetPlayerLiquidNacre(int nacre) {
  return SetSaveEntryValueCount("RESOURCE:PEARL_SHARDS", (i32)nacre);
}

MODDING_API int GetPlayerCrystalNacre() {
  i32 *count = nullptr;
  GetSaveEntryValueCount("RESOURCE:SOLID_DROPLETS", count);
  return (int)*count;
}

MODDING_API bool SetPlayerCrystalNacre(int nacre) {
  return SetSaveEntryValueCount("RESOURCE:SOLID_DROPLETS", (i32)nacre);
}

MODDING_API float GetPlayerStamina() {
  if (!ModAPI::g_PlayerStaminaAddr) {
    return -1.0f;
  }

  if (ModAPI::g_PlayerStaminaAddr) {
    return *(float *)ModAPI::g_PlayerStaminaAddr;
  } else {
    return -1.0f;
  }
}

MODDING_API bool SetPlayerStamina(float stamina) {
  if (!ModAPI::g_PlayerStaminaAddr) {
    return false;
  }

  bool success = true;
  if (ModAPI::g_PlayerStaminaAddr) {
    success &= WriteMemoryTyped(ModAPI::g_PlayerStaminaAddr, stamina);
  }

  return success;
}

// ========================================
// Save Entry Functions
// ========================================

MODDING_API int GetSaveEntryCount() {
  if (!ModAPI::g_SaveArraySize) {
    return 0;
  }
  return *ModAPI::g_SaveArraySize;
}

// Returns a pointer to the actual entry in memory
MODDING_API SaveEntry *GetSaveEntryByIndex(int index) {
  if (!ModAPI::g_SaveArrayPtr || !*ModAPI::g_SaveArrayPtr) {
    return nullptr;
  }

  int count = GetSaveEntryCount();
  if (index < 0 || index >= count) {
    return nullptr;
  }

  SaveEntry *entries = (SaveEntry *)*ModAPI::g_SaveArrayPtr;
  return (SaveEntry *)((char *)entries + (index * 88));
}

// Returns a pointer to the actual entry in memory by name
// We don't know the hash algorithm, so we just compare names directly
MODDING_API SaveEntry *GetSaveEntry(const char *name) {
  if (!ModAPI::g_SaveArrayPtr || !*ModAPI::g_SaveArrayPtr || !name) {
    return nullptr;
  }

  SaveEntry *entries = (SaveEntry *)*ModAPI::g_SaveArrayPtr;
  int count = GetSaveEntryCount();

  for (int i = 0; i < count; i++) {
    SaveEntry *entry = (SaveEntry *)((char *)entries + (i * 88));
    // Skip empty entries
    if (!entry->entry_name.data) {
      continue;
    }
    // Compare the actual string names
    if (strcmp(entry->entry_name.data, name) == 0) {
      return entry;
    }
  }

  return nullptr;
}

MODDING_API bool SaveEntryExists(const char *name) {
  return GetSaveEntry(name) != nullptr;
}

MODDING_API const char *GetSaveEntryName(int index) {
  SaveEntry *entry = GetSaveEntryByIndex(index);
  if (!entry || !entry->entry_name.data) {
    return nullptr;
  }
  return entry->entry_name.data;
}

// Helper functions for reading/writing count
MODDING_API bool GetSaveEntryValueCount(const char *name, i32 *outCount) {
  SaveEntry *entry = GetSaveEntry(name);
  if (!entry || !outCount) {
    return false;
  }
  *outCount = entry->value.count;
  return true;
}

MODDING_API bool SetSaveEntryValueCount(const char *name, i32 count) {
  SaveEntry *entry = GetSaveEntry(name);
  if (!entry) {
    return false;
  }
  entry->value.count = count;
  return true;
}

MODDING_API bool GetSaveEntryValueFlags(const char *name, u32 *outFlags) {
  SaveEntry *entry = GetSaveEntry(name);
  if (!entry || !outFlags) {
    return false;
  }
  *outFlags = entry->value.flags;
  return true;
}

MODDING_API bool SetSaveEntryValueFlags(const char *name, u32 flags) {
  SaveEntry *entry = GetSaveEntry(name);
  if (!entry) {
    return false;
  }
  entry->value.flags = flags;
  return true;
}

// ========================================
// Initialization
// ========================================

MODDING_API void InitializeAddresses() {
  HMODULE hModule = GetModuleHandleA("mio.exe");
  if (!hModule) {
    LogMessage("ERROR: Failed to get mio.exe module handle!");
    return;
  }

  // Get the base address of mio.exe
  uintptr_t baseAddr = (uintptr_t)hModule;

  // Add the offset from Cheat Engine
  uintptr_t playerLocationBasePtrAddr = baseAddr + 0x010EDF48;
  uintptr_t playerHealthBasePtrAddr = baseAddr + 0x010EE2E8;
  uintptr_t playerNacreBasePtrAddr = baseAddr + 0x01114AD0;
  uintptr_t playerStaminaPtrAddr = baseAddr + 0x110F9A8;

  uintptr_t menuStateAddr = baseAddr + 0x10DFC88;

  uintptr_t saveArrayPtrAddr = baseAddr + 0x01114AD0;
  uintptr_t saveArraySizeAddr = baseAddr + 0x01114AC8;

  // Store the address
  ModAPI::g_PlayerLocationBasePtr = (void **)playerLocationBasePtrAddr;
  ModAPI::g_PlayerHealthBasePtr = (void **)playerHealthBasePtrAddr;
  ModAPI::g_PlayerNacreBasePtr = (void **)playerNacreBasePtrAddr;

  ModAPI::g_PlayerStaminaAddr = (void *)playerStaminaPtrAddr;

  ModAPI::g_menuStateAddr = (void *)menuStateAddr;

  ModAPI::g_SaveArrayPtr = (void ***)saveArrayPtrAddr;
  ModAPI::g_SaveArraySize = (uint32_t *)saveArraySizeAddr;
  ModAPI::g_PlayerVelocityXAddr = (void *)(baseAddr + 0x10EE0C8);
  ModAPI::g_PlayerVelocityYAddr = (void *)(baseAddr + 0x10EE0CC);
}

// MODDING_API void InitializeSaveEntryHashMap() {
// WaitForSaveLoad();
//
// char msg[256];
//
// sprintf_s(msg, "Initializing custom save entry hashmap...");
// LogMessage(msg);
//
// sprintf_s(msg, "Custom hashmap address: %p", (void *)ModAPI::customHashmap);
// LogMessage(msg);
//
// sprintf_s(msg, "Custom hashmap capacity: %d entries",
// ModAPI::HASHMAP_CAPACITY);
// LogMessage(msg);
//
// sprintf_s(msg, "Custom hashmap size: %zu bytes",
// sizeof(ModAPI::customHashmap));
// LogMessage(msg);
// sprintf_s(msg, "Original HashMapPtr: %p", (void *)ModAPI::g_SaveArrayPtr);
// LogMessage(msg);
//
// Zero out the custom hashmap memset(ModAPI::customHashmap, 0,
//  sizeof(ModAPI::customHashmap));
//
// Get the original save array info
// if (!ModAPI::g_SaveArrayPtr ||
// !*ModAPI::g_SaveArrayPtr ||
// !ModAPI::g_SaveArraySize) {
// LogMessage("ERROR: Save array pointers not
// initialized!"); LogMessage( "We probably
// need to wait until the save is loaded.");
// // THIS!!! LOOK
//  HERE MARS!!!
//  THIS IS PAST
//  YOU TRYING TO
//  SAVE NOW
//  PRESENT TIME
//  YOU HOURS OF
//  PAIN AND
//  MULTIPLE
//  MENTAL
//  BREAKDOWNS!!!
//  CHECK IF
//  CARNAVAL HAS
//  SENT YOU ANY
//  IDEAS!!!
// if (!ModAPI::g_SaveArrayPtr) {
// LogMessage("\t!ModAPI::g_SaveArrayPtr");
// }
// if (!*ModAPI::g_SaveArrayPtr) {
// LogMessage("\t!*ModAPI::g_SaveArrayPtr");
// }
// if (!ModAPI::g_SaveArraySize) {
// LogMessage("\t!ModAPI:g_SazeArraySize");
// }
// return;
// }

MODDING_API void InitializeSaveEntryHashMap() {
  char msg[256];

  sprintf_s(msg, "Initializing custom save entry hashmap...");
  LogMessage(msg);

  // Allocate the hashmap in the game's memory space if not already allocated
  if (!ModAPI::customHashmap) {
    size_t hashmapSize = ModAPI::ENTRY_SIZE * ModAPI::HASHMAP_CAPACITY;

    LogMessage("Allocating custom hashmap in game memory space...");
    ModAPI::customHashmap = (SaveEntry *)VirtualAlloc(
        nullptr,                  // Let system choose address
        hashmapSize,              // Size
        MEM_COMMIT | MEM_RESERVE, // Commit and reserve
        PAGE_READWRITE            // Read/write access
    );

    if (!ModAPI::customHashmap) {
      LogMessage("ERROR: Failed to allocate custom hashmap!");
      return;
    }

    sprintf_s(msg, "Allocated custom hashmap at: %p (size: %zu bytes)",
              (void *)ModAPI::customHashmap, hashmapSize);
    LogMessage(msg);
  }

  // Allocate string buffer if not already allocated
  if (!ModAPI::stringBuffer) {
    LogMessage("Allocating string buffer in game memory space...");
    ModAPI::stringBuffer =
        (char *)VirtualAlloc(nullptr, ModAPI::STRING_BUFFER_SIZE,
                             MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!ModAPI::stringBuffer) {
      LogMessage("ERROR: Failed to allocate string buffer!");
      return;
    }

    sprintf_s(msg, "Allocated string buffer at: %p (size: %zu bytes)",
              (void *)ModAPI::stringBuffer, ModAPI::STRING_BUFFER_SIZE);
    LogMessage(msg);

    ModAPI::stringBufferOffset = 0;
  }

  sprintf_s(msg, "Custom hashmap capacity: %d entries",
            ModAPI::HASHMAP_CAPACITY);
  LogMessage(msg);

  sprintf_s(msg, "Custom hashmap size: %zu bytes",
            sizeof(ModAPI::customHashmap));
  LogMessage(msg);

  // Zero out the custom hashmap
  memset(ModAPI::customHashmap, 0, sizeof(ModAPI::customHashmap));
  ModAPI::customHashmapSize = 0;
  ModAPI::stringBufferOffset = 0;

  // Get the original save array info
  if (!ModAPI::g_SaveArrayPtr || !*ModAPI::g_SaveArrayPtr ||
      !ModAPI::g_SaveArraySize) {
    LogMessage("WARNING: Save array pointers not ready yet");
    return;
  }

  SaveEntry *originalEntries = (SaveEntry *)*ModAPI::g_SaveArrayPtr;
  u32 originalCount = *ModAPI::g_SaveArraySize;

  // Check if save data is actually loaded
  if (originalCount == 0 || originalEntries == nullptr) {
    LogMessage("WARNING: Save data not loaded yet");
    if (!ModAPI::g_SaveArrayPtr) {
      LogMessage("\t!ModAPI::g_SaveArrayPtr");
    }
    if (!*ModAPI::g_SaveArrayPtr) {
      LogMessage("\t!*ModAPI::g_SaveArrayPtr");
    }
    if (!ModAPI::g_SaveArraySize) {
      LogMessage("\t!ModAPI:g_SazeArraySize");
    }
    return;
  }

  sprintf_s(msg, "Original hashmap has %d entries", originalCount);
  LogMessage(msg);

  // Copy existing entries to custom hashmap
  if (originalCount > ModAPI::HASHMAP_CAPACITY) {
    sprintf_s(msg,
              "WARNING: Original hashmap has more entries (%d) than custom "
              "capacity (%d)!",
              originalCount, ModAPI::HASHMAP_CAPACITY);
    LogMessage(msg);
    originalCount = ModAPI::HASHMAP_CAPACITY;
  }

  sprintf_s(msg, "Copying %d entries from original hashmap...", originalCount);
  LogMessage(msg);

  // Use try-catch to handle potential crashes
  __try {
    for (u32 i = 0; i < originalCount; i++) {
      SaveEntry *originalEntry =
          (SaveEntry *)((char *)originalEntries + (i * ModAPI::ENTRY_SIZE));
      memcpy(&ModAPI::customHashmap[i], originalEntry, ModAPI::ENTRY_SIZE);
    }
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    LogMessage("ERROR: Exception while copying entries!");
    ModAPI::hashmapInitialized = false;
    return;
  }

  ModAPI::customHashmapSize = originalCount;

  sprintf_s(msg, "Copied %d entries to custom hashmap",
            ModAPI::customHashmapSize);
  LogMessage(msg);

  // Redirect game pointers to use custom hashmap
  sprintf_s(msg, "Redirecting game pointers to custom hashmap...");
  LogMessage(msg);

  // Update the pointer to point to our custom hashmap
  *ModAPI::g_SaveArrayPtr = (void **)ModAPI::customHashmap;

  // Update the size
  *ModAPI::g_SaveArraySize = ModAPI::HASHMAP_CAPACITY;

  sprintf_s(msg, "Game now using custom hashmap at %p with %d max entries",
            (void *)ModAPI::customHashmap, ModAPI::HASHMAP_CAPACITY);
  LogMessage(msg);

  ModAPI::hashmapInitialized = true;
  LogMessage("Custom save entry hashmap initialized successfully!");
}

char *AllocateString(const char *str) {
  if (!ModAPI::stringBuffer) {
    LogMessage("ERROR: String buffer not allocated!");
    return nullptr;
  }

  size_t len = strlen(str) + 1;
  if (ModAPI::stringBufferOffset + len >= ModAPI::STRING_BUFFER_SIZE) {
    LogMessage("ERROR: String buffer full!");
    return nullptr;
  }

  char *allocated = &ModAPI::stringBuffer[ModAPI::stringBufferOffset];
  strcpy(allocated, str);
  ModAPI::stringBufferOffset += len;

  return allocated;
}

MODDING_API void MonitorSaveLoads() {
  LogMessage("Save load monitor thread started");

  while (true) {
    Sleep(100); // Check every 100ms

    MenuState currentState = GetGameMenuState();

    // Detect transition from main menu -> game (save load)
    if (ModAPI::lastMenuState != MenuState::Game &&
        currentState == MenuState::Game) {
      LogMessage("==============================================");
      LogMessage("Detected save load! Reinitializing hashmap...");
      LogMessage("==============================================");

      // Wait a moment for save data to fully load
      Sleep(1000);

      // Mark as uninitialized
      ModAPI::hashmapInitialized = false;

      // Reinitialize the hashmap with the new save data
      InitializeSaveEntryHashMap();

      if (ModAPI::hashmapInitialized) {
        LogMessage("Hashmap reinitialized successfully!");
      } else {
        LogMessage("WARNING: Failed to reinitialize hashmap");
      }
    }

    // Detect transition from game -> main menu (save unload)
    if (ModAPI::lastMenuState == MenuState::Game &&
        currentState != MenuState::Game) {
      LogMessage("==============================================");
      LogMessage("Detected save unload (returned to menu)");
      LogMessage("==============================================");
      ModAPI::hashmapInitialized = false;
    }

    ModAPI::lastMenuState = currentState;
  }
}

// ========================================
// Utility Functions
// ========================================

MODDING_API void LogMessage(const char *message) {
  printf("[MOD API] %s\n", message);
}

MODDING_API APIVersion GetAPIVersion() {
  APIVersion v = {MODDING_API_VERSION_MAJOR, MODDING_API_VERSION_MINOR,
                  MODDING_API_VERSION_PATCH};
  return v;
}
}