#include "Windows.h"
#include "mio-modding-api.h"

namespace ModAPI {
    namespace Player {
        extern "C" {
            MODDING_API Vector3 GetPlayerLocation() {
                Vector3 loc = { -1.0f, -1.0f, -1.0f };

                if(!ModAPI::Addresses::g_PlayerLocationAddr) {
                    return loc;
                }

                return *(Vector3*)ModAPI::Addresses::g_PlayerLocationAddr;
            }

            MODDING_API bool SetPlayerLocation(Vector3 location) {
				if(!ModAPI::Addresses::g_PlayerLocationAddr) {
                    return false;
                }
				bool success = ModAPI::Util::WriteMemoryTyped(ModAPI::Addresses::g_PlayerLocationAddr, location);
                return success;
            }

            MODDING_API Vector3 GetPlayerVelocity() {
                Vector3 vel = { -1.0f, -1.0f, -1.0 };

                if (!ModAPI::Addresses::g_PlayerVelocityAddr) {
                    return vel;
                }

                return *(Vector3*)ModAPI::Addresses::g_PlayerVelocityAddr;
            }

            MODDING_API bool SetPlayerVelocity(Vector3 velocity) {
                if (!ModAPI::Addresses::g_PlayerVelocityAddr) {
                    return false;
                }

                bool success = ModAPI::Util::WriteMemoryTyped(ModAPI::Addresses::g_PlayerVelocityAddr, velocity);
                return success;
            }

            MODDING_API int GetPlayerHealth() {
				if(!ModAPI::Addresses::g_PlayerHealthAddress) {
                    return -1;
                }

				return *(int*)ModAPI::Addresses::g_PlayerHealthAddress;
            }

            MODDING_API bool SetPlayerHealth(int health) {
				if(!ModAPI::Addresses::g_PlayerHealthAddress) {
                    return false;
                }
				bool success = ModAPI::Util::WriteMemoryTyped(ModAPI::Addresses::g_PlayerHealthAddress, health);

                return success;
            }

            MODDING_API int GetPlayerLiquidNacre() {
                return ModAPI::SaveData::GetSaveEntryValueCount("RESOURCE:PEARL_SHARDS");
            }

            MODDING_API bool SetPlayerLiquidNacre(int nacre) {
                return ModAPI::SaveData::SetSaveEntryValueCount("RESOURCE:PEARL_SHARDS", (uint32_t)nacre);
            }

            MODDING_API int GetPlayerCrystalNacre() {
                return ModAPI::SaveData::GetSaveEntryValueCount("RESOURCE:SOLID_DROPLETS");
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
            MODDING_API void MovePlayer(Vector3 vector) {
                ModAPI::Util::CallAssembly<void>(ModAPI::Addresses::g_MoveByMethodAddr, ModAPI::Addresses::g_PlayerObjAddr, &vector);
            }
        }
    }
}