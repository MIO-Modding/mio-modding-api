#include "pch.h"
#include "mio-modding-api.h"

namespace ModAPI {
    namespace Player {
        extern "C" {
            MODDING_API Vector3 GetPlayerLocation() {
                Vector3 loc = { -1.0f, -1.0f, -1.0f };

                if (!ModAPI::Pointers::g_PlayerLocationBasePtr) {
                    return loc;
                }

                // Follow the pointer chain
                void* playerXAddr =
                    ModAPI::Util::FollowPointer(ModAPI::Pointers::g_PlayerLocationBasePtr, ModAPI::Offsets::PLAYER_X_OFFSET);
                void* playerYAddr =
                    ModAPI::Util::FollowPointer(ModAPI::Pointers::g_PlayerLocationBasePtr, ModAPI::Offsets::PLAYER_Y_OFFSET);
                void* playerZAddr =
                    ModAPI::Util::FollowPointer(ModAPI::Pointers::g_PlayerLocationBasePtr, ModAPI::Offsets::PLAYER_Z_OFFSET);

                if (playerXAddr) {
                    loc.x = *(float*)playerXAddr;
                }
                if (playerYAddr) {
                    loc.y = *(float*)playerYAddr;
                }
                if (playerZAddr) {
                    loc.z = *(float*)playerZAddr;
                }

                return loc;
            }

            MODDING_API bool SetPlayerLocation(Vector3 location) {
                if (!ModAPI::Pointers::g_PlayerLocationBasePtr) {
                    return false;
                }

                void* playerXAddr =
                    ModAPI::Util::FollowPointer(ModAPI::Pointers::g_PlayerLocationBasePtr, ModAPI::Offsets::PLAYER_X_OFFSET);
                void* playerYAddr =
                    ModAPI::Util::FollowPointer(ModAPI::Pointers::g_PlayerLocationBasePtr, ModAPI::Offsets::PLAYER_Y_OFFSET);
                void* playerZAddr =
                    ModAPI::Util::FollowPointer(ModAPI::Pointers::g_PlayerLocationBasePtr, ModAPI::Offsets::PLAYER_Z_OFFSET);

                bool success = true;
                if (playerXAddr) {
                    success &= ModAPI::Util::WriteMemoryTyped(playerXAddr, location.x);
                }
                if (playerYAddr) {
                    success &= ModAPI::Util::WriteMemoryTyped(playerYAddr, location.y);
                }
                if (playerZAddr) {
                    success &= ModAPI::Util::WriteMemoryTyped(playerZAddr, location.z);
                }

                return success;
            }

            MODDING_API Vector2 GetPlayerVelocity() {
                Vector2 vel = { -1.0f, -1.0f };

                if (!ModAPI::Addresses::g_PlayerVelocityXAddr || !ModAPI::Addresses::g_PlayerVelocityYAddr) {
                    return vel;
                }

                vel.x = *(float*)ModAPI::Addresses::g_PlayerVelocityXAddr;
                vel.y = *(float*)ModAPI::Addresses::g_PlayerVelocityYAddr;

                return vel;
            }

            MODDING_API bool SetPlayerVelocity(Vector2 velocity) {
                if (!ModAPI::Addresses::g_PlayerVelocityXAddr || !ModAPI::Addresses::g_PlayerVelocityYAddr) {
                    return false;
                }

                bool success = true;
                success &= ModAPI::Util::WriteMemoryTyped(ModAPI::Addresses::g_PlayerVelocityXAddr, velocity.x);
                success &= ModAPI::Util::WriteMemoryTyped(ModAPI::Addresses::g_PlayerVelocityYAddr, velocity.y);

                return success;
            }

            MODDING_API int GetPlayerHealth() {
                if (!ModAPI::Pointers::g_PlayerHealthBasePtr) {
                    return -1;
                }

                void* playerHealthAddr = ModAPI::Util::FollowPointer(ModAPI::Pointers::g_PlayerHealthBasePtr,
                    ModAPI::Offsets::PLAYER_HEALTH_OFFSET);

                if (playerHealthAddr) {
                    return *(int*)playerHealthAddr;
                }
                else {
                    return -1;
                }
            }

            MODDING_API bool SetPlayerHealth(int health) {
                if (!ModAPI::Pointers::g_PlayerHealthBasePtr) {
                    return false;
                }

                void* playerHealthAddr = ModAPI::Util::FollowPointer(ModAPI::Pointers::g_PlayerHealthBasePtr,
                    ModAPI::Offsets::PLAYER_HEALTH_OFFSET);

                bool success = true;
                if (playerHealthAddr) {
                    success &= ModAPI::Util::WriteMemoryTyped(playerHealthAddr, health);
                }

                return success;
            }

            MODDING_API int GetPlayerLiquidNacre() {
                int32_t* count = nullptr;
                ModAPI::SaveData::GetSaveEntryValueCount("RESOURCE:PEARL_SHARDS", count);
                return (int)*count;
            }

            MODDING_API bool SetPlayerLiquidNacre(int nacre) {
                return ModAPI::SaveData::SetSaveEntryValueCount("RESOURCE:PEARL_SHARDS", (uint32_t)nacre);
            }

            MODDING_API int GetPlayerCrystalNacre() {
                int32_t* count = nullptr;
                ModAPI::SaveData::GetSaveEntryValueCount("RESOURCE:SOLID_DROPLETS", count);
                return (int)*count;
            }

            MODDING_API bool SetPlayerCrystalNacre(int nacre) {
                return ModAPI::SaveData::SetSaveEntryValueCount("RESOURCE:SOLID_DROPLETS", (uint32_t)nacre);
            }

            MODDING_API float GetPlayerStamina() {
                if (!ModAPI::Addresses::g_PlayerStaminaAddr) {
                    return -1.0f;
                }

                if (ModAPI::Addresses::g_PlayerStaminaAddr) {
                    return *(float*)ModAPI::Addresses::g_PlayerStaminaAddr;
                }
                else {
                    return -1.0f;
                }
            }

            MODDING_API bool SetPlayerStamina(float stamina) {
                if (!ModAPI::Addresses::g_PlayerStaminaAddr) {
                    return false;
                }

                bool success = true;
                if (ModAPI::Addresses::g_PlayerStaminaAddr) {
                    success &= ModAPI::Util::WriteMemoryTyped(ModAPI::Addresses::g_PlayerStaminaAddr, stamina);
                }

                return success;
            }
            MODDING_API int64_t MovePlayer(Vector2 vector) {
                //I am still unsure what this array is for lol
                float array[0x4];
                int64_t vec = vector.CreateXYInt64();
                //Im also unsure what the return even is meant to be so im just passing it through
                int64_t ret = ModAPI::Util::CallAssembly<int64_t>(ModAPI::Addresses::g_MoveByMethodAddr, ModAPI::Addresses::g_PlayerObjAddr, &vec, array);
                return ret;
            }
        }
    }
}