#pragma once
#include "Windows.h"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <filesystem>

#ifdef MODDING_API_EXPORTS
#define MODDING_API __declspec(dllexport)
#else
#define MODDING_API __declspec(dllimport)
#endif

/**
 * @brief A 3D vector with float components.
 */
struct Vector3 {
	float x; ///< X component.
	float y; ///< Y component.
	float z; ///< Z component.

	/**
	 * @brief Constructs a Vector3 from three float components.
	 * @param x X component.
	 * @param y Y component.
	 * @param z Z component.
	 */
	Vector3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};

/**
 * @brief A 2D vector with float components.
 */
struct Vector2 {
	float x; ///< X component.
	float y; ///< Y component.

	/**
	 * @brief Constructs a Vector2 from two float components.
	 * @param x X component.
	 * @param y Y component.
	 */
	Vector2(float x, float y) {
		this->x = x;
		this->y = y;
	}

	/**
	 * @brief Constructs a Vector2 from a packed int64.
	 *
	 * Unpacks two floats from a 64-bit integer where the raw IEEE 754 bit
	 * representation of Y occupies the high 32 bits and X occupies the low
	 * 32 bits.
	 *
	 * @param xyint Packed 64-bit integer containing both float components.
	 * @see CreateXYInt64()
	 */
	Vector2(int64_t xyint) {
		uint32_t b1 = static_cast<uint32_t>(static_cast<uint64_t>(xyint) >> 32);
		uint32_t b2 = static_cast<uint32_t>(xyint & 0xFFFFFFFF);

		float y;
		float x;
		memcpy(&y, &b1, sizeof(float));
		memcpy(&x, &b2, sizeof(float));
		this->x = x;
		this->y = y;
	}

	/**
	 * @brief Packs this Vector2 into a single int64.
	 *
	 * The raw IEEE 754 bit representation of Y is placed in the high 32 bits
	 * and X in the low 32 bits.
	 *
	 * @return Packed int64 representation of this vector.
	 * @see Vector2(int64_t)
	 */
	int64_t CreateXYInt64() {
		uint32_t u1;
		uint32_t u2;
		memcpy(&u1, &y, sizeof(y));
		memcpy(&u2, &x, sizeof(x));

		uint64_t combined_u64 = (static_cast<uint64_t>(u1) << 32) | u2;
		int64_t combined_s64 = static_cast<int64_t>(combined_u64);

		return combined_s64;
	}
};

/**
 * @brief Root namespace for the Modding API.
 */
namespace ModAPI {
	/**
	 * @brief Base pointers used internally to resolve addresses via pointer chains.
	 *
	 * @warning These are internal to the API. Use the provided API functions
	 * instead of accessing these directly.
	 */
	namespace Pointers {
		extern void** g_PlayerLocationBasePtr; ///< Base pointer for the player's location pointer chain.
		extern void** g_PlayerHealthBasePtr;   ///< Base pointer for the player's health pointer chain.
		extern void** g_PlayerNacreBasePtr;	   ///< Base pointer for the player's nacre pointer chain.
		extern void*** g_SaveArrayPtr;		   ///< Pointer to the save data array.
	} // namespace Pointers

	/**
	 * @brief Resolved memory addresses for game values that do not require
	 * pointer chains.
	 *
	 * @warning These are internal to the API. Use the provided API functions
	 * instead of accessing these directly.
	 */
	namespace Addresses {
		extern uintptr_t g_BaseAddr;		///< Base Address for the mio.exe.

		extern void* g_PlayerStaminaAddr;	///< Direct address of the player's stamina value.
		extern void* g_PlayerVelocityXAddr; ///< Direct address of the player's X velocity value, offset from the player object.
		extern void* g_PlayerVelocityYAddr; ///< Direct address of the player's Y velocity value, offset from the player object.
		extern void* g_MoveByMethodAddr;	///< Address of the game's internal move by method function.
		extern void* g_PlayerObjAddr;		///< Direct address of the player object.
		extern void* g_HitEnemyAddress;		///< Address of the game's internal hit enemy function.
		extern void* g_MenuStateAddr;		///< Direct address of the current menu state value.
		extern void* g_GiveFlagAddress;		///< Address of the game's internal give flag function.
		extern void* g_GameAddr;
	} // namespace Addresses

	/**
	 * @brief Offsets used to resolve fields within game objects via pointer chains.
	 *
	 * @warning These are internal to the API. Use the provided API functions
	 * instead of accessing these directly.
	 */
	namespace Offsets {
		extern const int PLAYER_X_OFFSET;			 ///< Offset to the player's X position.
		extern const int PLAYER_Y_OFFSET;			 ///< Offset to the player's Y position.
		extern const int PLAYER_Z_OFFSET;			 ///< Offset to the player's Z position.
		extern const int PLAYER_HEALTH_OFFSET;		 ///< Offset to the player's health value.
		extern const int PLAYER_LIQUID_NACRE_OFFSET; ///< Offset to the player's liquid nacre value.
	} // namespace Offsets

	/**
	 * @brief Enumerations used by the API.
	 */
	namespace Enums {
		/**
		 * @brief Represents the current high-level state of the game's menu
		 * system. The names match the names used internally by the game.
		 *
		 * Read via Util::GetGameMenuState() and written via
		 * Util::SetGameMenuState().
		 */
		enum MenuState {
			Start,			///< Initial startup state.
			Splashes,		///< Splash screen sequence.
			Shader_spinner, ///< Shader compilation loading screen.
			Main_menu,		///< Main menu is active.
			Game,			///< A save is loaded and the game is running.
			Credits,		///< Credits sequence.
			Exit			///< Game is shutting down.
		};
	} // namespace Enums

	/**
	 * @brief Functions for reading and manipulating player state.
	 */
	namespace Player {
		extern "C" {
			/**
			 * @brief Gets the player's current position.
			 * @return A Vector3 containing the player's X, Y, and Z coordinates.
			 */
			MODDING_API Vector3 GetPlayerLocation();

			/**
			 * @brief Sets the player's position.
			 * @param location Target position as a Vector3.
			 * @return True on success, false if the base location pointer fails to resolve, or if there is an error setting the values.
			 */
			MODDING_API bool SetPlayerLocation(Vector3 location);

			/**
			 * @brief Get's the player's current velocity.
			 * @return A Vector2 containing the player's X and Y velocity components.
			 */
			MODDING_API Vector2 GetPlayerVelocity();

			/**
			 * @brief Sets the player's velocity.
			 * @param velocity Target velocity as a Vector2.
			 * @return True on success, false if either velocity pointer fails to resolve, or if there is an error setting the values.
			 */
			MODDING_API bool SetPlayerVelocity(Vector2 velocity);

			/**
			 * @brief Gets the player's current health.
			 * @return The player's current health as an integer.
			 */
			MODDING_API int GetPlayerHealth();

			/**
			 * @brief Sets the player's health.
			 * @param health The health value to set.
			 * @return True on success, false if the health pointer fails to resolve, or if there is an error setting the value.
			 */
			MODDING_API bool SetPlayerHealth(int health);

			/**
			 * @brief Gets the player's current liquid nacre amount.
			 * @return The player's current liquid nacre as an integer.
			 */
			MODDING_API int GetPlayerLiquidNacre();

			/**
			 * @brief Sets the player's liquid nacre amount.
			 * @param nacre The liquid nacre value to be set.
			 * @return True on success, false if there is an error setting the value.
			 */
			MODDING_API bool SetPlayerLiquidNacre(int nacre);

			/**
			 * @brief Gets the player's current crystal nacre amount.
			 * @return The player's current crystal nacre as an integer.
			 */
			MODDING_API int GetPlayerCrystalNacre();

			/**
			 * @brief Sets the player's crystal nacre amount.
			 * @param nacre The crystal nacre value to be set.
			 * @return True on success, false if there is an error setting the value.
			 */
			MODDING_API bool SetPlayerCrystalNacre(int nacre);

			/**
			 * @brief Gets the player's current energy/stamina.
			 * @return The player's current energy/stamina as a float, or -1.0 if there stamina pointer fails to resolve.
			 */
			MODDING_API float GetPlayerStamina();

			/**
			 * @brief Sets the player's energy/stamina.
			 * @param stamina The energy/stamina value to set.
			 * @return True on success, false, if the stamina address fails to resolve.
			 */
			MODDING_API bool SetPlayerStamina(float stamina);

			/**
			 * @brief Moves the player by the given vector using the game's internal move method.
			 * @param vector The X and Y offsets to apply.
			 * @return The game's internal return value from the move method.
			 */
			MODDING_API int64_t MovePlayer(Vector2 vector);
		}
	} // namespace Player

	/**
	 * @brief Functions for reading and manipulating entries in the internal save data array.
	 */
	namespace SaveData {
		extern uint32_t* g_SaveArraySize; ///<  Pointer to the number of entries in the save data array.

		/**
		 * @brief A null-terminated string as stored internally by the game.
		 * @note Do not modify the fields of this struct directly.
		 */
		typedef struct GameString {
			char* data;		 ///< Pointer to the string data.
			uint32_t size;	 ///< Length of the string in bytes.
			uint32_t unused; ///< Padding - do not use.
			GameString(char* data) {
				this->data = data;
				size = strlen(data);
			}
		} GameString;
		static_assert(sizeof(GameString) == 16, "GameString size mismatch — struct layout may be wrong");

		/**
		 * @brief The value payload of a save data entry.
		 * @note Do not modify the fields of this struct directly.
		 */
		typedef struct SaveEntryValue {
			uint32_t flags;	 ///< Flags associated with this entry.
			int32_t count;	 ///< The count/amount of this entry.
			BYTE unused[56]; ///< Padding - do not use.
		} SaveEntryValue;
		static_assert(sizeof(SaveEntryValue) == 64, "SaveEntryValue size mismatch — struct layout may be wrong");

		/**
		 * @brief A single save data entry as it appears in the game's save array.
		 *
		 * Retrieve instances via GetSaveEntry() or GetSaveEntryByIndex().
		 * Do not construct this struct manually.
		 */
		typedef struct SaveEntry {
			uint32_t hash;		   ///< Hash of the entry. Currently unknown how this is calculated - do not use.
			uint32_t padding;	   ///< Alignment padding - do not use.
			GameString entry_name; ///< The key/name of this save entry.
			SaveEntryValue value;  ///< The value payload of this entry.
		} SaveEntry;
		static_assert(sizeof(SaveEntry) == 88, "SaveEntry size mismatch — struct layout may be wrong");

		extern "C" {
			/**
			 * @brief Gets the total number of entries in the save data array.
			 * @return The number of save entries as an integer.
			 */
			MODDING_API int GetSaveEntryCount();

			/**
			 * @brief Gets a save entry by its index in the save data array.
			 * @param index Zero-based index of the entry
			 * @return Pointer to the SaveEntry, or nullptr if the index is out of range.
			 */
			MODDING_API SaveEntry* GetSaveEntryByIndex(int index);

			/**
			 * @brief Gets a save entry by its key/name.
			 * @param name The key/name of the the entry to find.
			 * @return Pointer to the SaveEntry, or nullptr if not found.
			 */
			MODDING_API SaveEntry* GetSaveEntry(const char* name);

			/**
			 * @brief Checks whether a save entry with the given key/name exists.
			 * @param name The key/name to check.
			 * @return True if the entry exists, false otherwise.
			 */
			MODDING_API bool SaveEntryExists(const char* name);

			/**
			 * @brief Gets the key/name of a save entry by its index.
			 * @param index Zero-based index of the entry.
			 * @return The name of the entry, or nullptr if the index is out of range.
			 */
			MODDING_API const char* GetSaveEntryName(int index);

			/**
			 * @brief Gets the count value of a save entry.
			 * @param name The key/name of the entry.
			 * @param outCount Pointer to an int32_t to receive the count value.
			 * @return True on success, false if the entry was not found.
			 */
			MODDING_API bool GetSaveEntryValueCount(const char* name, int32_t* outCount);

			/**
			 * @brief Sets the count value of a save entry.
			 * @param name The key/name of the entry.
			 * @param count The count value to set.
			 * @return True on success, false if the entry was not found.
			 */
			MODDING_API bool SetSaveEntryValueCount(const char* name, int32_t count);

			/**
			 * @brief Gets the flags of a save entry.
			 * @param name The key/name of the entry.
			 * @param outFlags Pointer to a uint32_t to receive the flags value.
			 * @return True on success, false if the entry was not found.
			 */
			MODDING_API bool GetSaveEntryValueFlags(const char* name, uint32_t* outFlags);

			/**
			 * @brief Sets the flags of a save entry.
			 * @param name The key/name of the entry.
			 * @param flags The flags value to set.
			 * @return True on success, false if the entry was not found.
			 */
			MODDING_API bool SetSaveEntryValueFlags(const char* name, uint32_t flags);
			/**
			 * @brief Increases the amount for a flag
			 * @param flag The flag to increase.
			 * @param amount The amount to increase the flag by.
			 * @return The game's internal return value from the give flag method.
			 */
			MODDING_API char* GiveFlag(const char* flag, int32_t amount);
		}
	} // namespace SaveData

	/**
	 * @brief General utility functions for memory manipulation and logging.
	 */
	namespace Util {
		extern "C" {
			/**
			 * @brief Writes raw bytes to an arbitrary memory address.
			 * @param address Target memory address.
			 * @param data Pointer to the data to write.
			 * @param size Number of bytes to write.
			 * @return True on success, false if the write failed.
			 * @warning Incorrect use can cause crashes.
			 */
			MODDING_API bool WriteMemory(void* address, const void* data, size_t size);

			/**
			 * @brief Reads raw bytes from an arbitrary memory address.
			 * @param address Source memory address.
			 * @param buffer Pointer to the buffer to read into.
			 * @param size Number of bytes to read.
			 * @return True on success, false if the read failed.
			 * @warning Incorrect use can cause crashes.
			 */
			MODDING_API bool ReadMemory(void* address, void* buffer, size_t size);

			/**
			 * @brief Patches bytes at a memory address with the given byte string.
			 * @param address Target memory address.
			 * @param bytes Byte string to write.
			 * @param size Number of bytes to write.
			 * @return True on success, false if the patch failed.
			 * @warning Incorrect use can cause crashes.
			 */
			MODDING_API bool PatchBytes(void* address, const char* bytes, size_t size);

			/**
			 * @brief Overwrites bytes at a memory address with NOP instructions.
			 * @param address Target memory address.
			 * @param count Number of bytes to NOP.
			 * @return True on success, false if the operation failed.
			 * @warning Incorrect use can cause crashes.
			 */
			MODDING_API bool NopBytes(void* address, size_t count);

			/**
			 * @brief Scans a module's memory for a byte pattern.
			 * @param module Handle to the module to scan.
			 * @param pattern Byte pattern to search for.
			 * @param mask Mask string where 'x' matches the byte and '?' is a wildcard.
			 * @return Pointer to the first match, or nullptr if not found.
			 */
			MODDING_API void* PatternScan(HMODULE module, const char* pattern,
										  const char* mask);

			/**
			 * @brief Scans a module's memory in reverse for a byte pattern, starting from a given address.
			 * @param module Handle to the module to scan.
			 * @param from Address to begin scanning backwards from.
			 * @param pattern Byte pattern to search for.
			 * @param mask Mask string where 'x' matches the byte and '?' is a wildcard.
			 * @return Pointer to the first match, or nullptr if not found.
			 */
			MODDING_API void* PatternScanReverse(HMODULE module, void* from,
												 const char* pattern, const char* mask);

			/**
			 * @brief Follows a pointer and applies an offset to resolve an address.
			 * @param basePtr The base pointer to dereference.
			 * @param offset The offset to apply after dereferencing.
			 * @return The resolved address, or nullptr if the pointer could not be followed.
			 */
			MODDING_API void* FollowPointer(void* basePtr, int offset);

			/**
			 * @brief Logs a message to the console, prefixed with the mod ID.
			 * @param modId The ID of the mod sending the message.
			 * @param message The message to log.
			 */
			MODDING_API void LogMessage(char* modId, const char* message);

			/**
			 * @brief Gets the current menu state of the game.
			 * @return The current MenuState value.
			 * @see Enums::MenuState
			 */
			MODDING_API Enums::MenuState GetGameMenuState();

			/**
			 * @brief Sets the current menu state of the game.
			 * @param state The MenuState value to set.
			 * @return True on success, false if the menu state address failed to resolve.
			 * @see Enums::MenuState
			 */
			MODDING_API bool SetGameMenuState(Enums::MenuState state);

			/**
			 * @brief Checks whether a save is currently loaded.
			 * @return True if a save is loaded, false otherwise.
			 */
			MODDING_API bool CheckIfSaveLoaded();

			/**
			 * @brief Blocks until a save is loaded.
			 *
			 * @note This is a blocking call. Do not call this on the main thread.
			 */
			MODDING_API void WaitForSaveLoaded();

			/**
			 * @brief Loads a method offset from the methods.json
			 * @param Key of method
			 * @return Offset of the method
			 */
			MODDING_API uintptr_t GetMethodOffset(const char* method);
			/**
			 * @brief Loads a static variable offset from the variable.json
			 * @param Key of variable
			 * @return Offset of the static variable
			 */
			MODDING_API uintptr_t GetStaticVariableOffset(const char* variable);
			/**
			 * @brief Loads a variable offset in a struct from the structs.json
			 * @param Struct to get offset from
			 * @param Variable in struct
			 * @return Offset of the variable
			 */
			MODDING_API uintptr_t GetVariableOffset(const char* structure, const char* variable);

			/**
			 * @brief Gets the path to a mods folder
			 * @param Mod to get path to
			 * @return The path of the mods folder
			 */
			MODDING_API std::filesystem::path GetFolderPathForMod(const char* mod);
		}

		/**
		 * @brief Calls a game function at the given address using the __fastcall convention.
		 *
		 * @tparam ReturnType The return type of the function.
		 * @tparam Args The argument types of the function.
		 * @param address The address of the function to call.
		 * @param args The arguments to pass to the function.
		 * @return The return value of the called function.
		 * @warning Calling a function with the wrong address, return type, or argument types will cause a crash.
		 */
		template <typename ReturnType, typename... Args>
		ReturnType CallAssembly(void* address, Args... args) {
			uintptr_t addr = reinterpret_cast<uintptr_t>(address);
			typedef ReturnType(__fastcall * InternalFunc)(Args...);
			InternalFunc func = reinterpret_cast<InternalFunc>(addr);
			return func(args...);
		}

		/**
		 * @brief Reads a typed value from an arbitrary memory address.
		 * @tparam T The type to read. Must be trivially copyable.
		 * @param address Memory address to read from.
		 * @return The value at the given address interpreted as type T.
		 * @warning No bounds checking is performed. Ensure the address is valid.
		 */
		template <typename T>
		T ReadMemoryTyped(void* address) {
			return *(T*)address;
		}

		/**
		 * @brief Writes a typed value to an arbitrary memory address.
		 * @tparam T The type to write. Must be trivially copyable.
		 * @param address Memory address to write to.
		 * @param value The value to write.
		 * @return True on success, false if the write failed.
		 * @warning No bounds checking is performed. Ensure the address is valid.
		 */
		template <typename T>
		bool WriteMemoryTyped(void* address, T value) {
			return WriteMemory(address, &value, sizeof(T));
		}
	} // namespace Util

	/**
	 * @brief Hooks into game events, allowing mods to register callbacks that fire when those events occur.
	 */
	namespace Hooks {
		/**
		 * @brief Hooks related to combat events.
		 */
		namespace Combat {
			/**
			 * @brief Registers a callback to be invoked whenever the player hits an enemy.
			 *
			 * Multiple callbacks can be registered and all will fire on each hit.
			 *
			 * @param callback Function to call on enemy hit. Receives two arguments:
			 *                 - The player object pointer (uintptr_t)
			 *                 - The enemy object pointer (uintptr_t)
			 */
			MODDING_API void RunOnHitEnemy(std::function<void(uintptr_t, uintptr_t)> callback);
		} // namespace Combat

		/**
		 * @brief Hooks related to flags.
		 */
		namespace Flags {
			/**
			 * @brief Registers a callback to be invoked whenever the player is given a flag.
			 *
			 * Multiple callbacks can be registered and all will fire on each hit.
			 *
			 * @param callback Function to call on enemy hit. Receives two arguments:
			 *                 - An unknown pointer to an object (uintptr_t)
			 *                 - A pointer to the GameString for the flag type, You may modify this pointer (GameString*)
			 *                 - A pointer to the amount of the flag, You may modify this pointer (int32_t*)
			 */
			MODDING_API void RunOnGiveFlag(std::function<void(uintptr_t, ModAPI::SaveData::GameString*, int32_t*)> callback);
		} // namespace Flags

		/**
		 * @brief Hooks related to game time.
		 */
		namespace Time {
			/**
			 * @brief Sets the game's time scale multiplier.
			 *
			 * A value of 1.0 is normal speed, 0.5 is half speed, 2.0 is double speed.
			 *
			 * @param scale The time scale multiplier. Must be greater than 0.
			 */
			MODDING_API void SetTimeScale(float scale);

			/**
			 * @brief Gets the game's current time scale multiplier.
			 * @return The current time scale multiplier.
			 */
			MODDING_API float GetTimeScale();

			/**
			 * @brief Registers a callback to be invoked on each game tick.
			 *
			 * Multiple callbacks can be registered and all will fire on each
			 * tick.
			 *
			 * @param callback Function to call on each tick. Receives the delta time in seconds since the last tick as a float.
			 */
			MODDING_API void RunOnTick(std::function<void(float)> callback);
		} // namespace Time

		/**
		 * @brief Initializes all hooks. Called automatically by the API on load.
		 *
		 * @note Do not call this manually.
		 */
		MODDING_API void InitializeHooks();
	} // namespace Hooks
} // namespace ModAPI