#include "pch.h"
#include "mio-modding-api.h"

extern "C" {
MODDING_API Vector3 GetPlayerLocation() {
    Vector3 loc = { -1.0f, -1.0f, -1.0f };

    if (!ModAPI::g_PlayerLocationBasePtr) {
        return loc;
    }

    // Follow the pointer chain
    void* playerXAddr =
        FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_X_OFFSET);
    void* playerYAddr =
        FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_Y_OFFSET);
    void* playerZAddr =
        FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_Z_OFFSET);

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
    if (!ModAPI::g_PlayerLocationBasePtr) {
        return false;
    }

    void* playerXAddr =
        FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_X_OFFSET);
    void* playerYAddr =
        FollowPointer(ModAPI::g_PlayerLocationBasePtr, ModAPI::PLAYER_Y_OFFSET);
    void* playerZAddr =
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

MODDING_API Vector2 GetPlayerVelocity() {
    Vector2 vel = { -1.0f, -1.0f };

    if (!ModAPI::g_PlayerVelocityXAddr || !ModAPI::g_PlayerVelocityYAddr) {
        return vel;
    }

    vel.x = *(float*)ModAPI::g_PlayerVelocityXAddr;
    vel.y = *(float*)ModAPI::g_PlayerVelocityYAddr;

    return vel;
}

MODDING_API bool SetPlayerVelocity(Vector2 velocity) {
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

    void* playerHealthAddr = FollowPointer(ModAPI::g_PlayerHealthBasePtr,
        ModAPI::PLAYER_HEALTH_OFFSET);

    if (playerHealthAddr) {
        return *(int*)playerHealthAddr;
    }
    else {
        return -1;
    }
}

MODDING_API bool SetPlayerHealth(int health) {
    if (!ModAPI::g_PlayerHealthBasePtr) {
        return false;
    }

    void* playerHealthAddr = FollowPointer(ModAPI::g_PlayerHealthBasePtr,
        ModAPI::PLAYER_HEALTH_OFFSET);

    bool success = true;
    if (playerHealthAddr) {
        success &= WriteMemoryTyped(playerHealthAddr, health);
    }

    return success;
}

MODDING_API int GetPlayerLiquidNacre() {
    int32_t* count = nullptr;
    GetSaveEntryValueCount("RESOURCE:PEARL_SHARDS", count);
    return (int)*count;
}

MODDING_API bool SetPlayerLiquidNacre(int nacre) {
    return SetSaveEntryValueCount("RESOURCE:PEARL_SHARDS", (uint32_t)nacre);
}

MODDING_API int GetPlayerCrystalNacre() {
    int32_t* count = nullptr;
    GetSaveEntryValueCount("RESOURCE:SOLID_DROPLETS", count);
    return (int)*count;
}

MODDING_API bool SetPlayerCrystalNacre(int nacre) {
    return SetSaveEntryValueCount("RESOURCE:SOLID_DROPLETS", (uint32_t)nacre);
}

MODDING_API float GetPlayerStamina() {
    if (!ModAPI::g_PlayerStaminaAddr) {
        return -1.0f;
    }

    if (ModAPI::g_PlayerStaminaAddr) {
        return *(float*)ModAPI::g_PlayerStaminaAddr;
    }
    else {
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
MODDING_API int64_t MovePlayer(Vector2 vector) {
    //I am still unsure what this array is for lol
    float array[0x4];
    int64_t vec = vector.CreateXYInt64();
    //Im also unsure what the return even is meant to be so im just passing it through
    int64_t ret = CallAssembly<int64_t>(ModAPI::g_MoveByMethodAddr, ModAPI::g_PlayerObjAddr, &vec, array);
    return ret;
}
}