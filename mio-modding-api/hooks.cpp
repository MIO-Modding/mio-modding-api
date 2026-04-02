#include "Windows.h"
#include "mio-modding-api.h"
#include "polyhook2/Detour/NatDetour.hpp"
#include <vector>
#include <functional>

namespace ModAPI {
	namespace Hooks {
		namespace Combat {
			// Functions hooked to OnHitEnemy
			std::vector<std::function<void(uintptr_t, uintptr_t)>> hitenemy_hooks;

			uintptr_t hitenemy_trampoline = NULL;

			NOINLINE void __cdecl ResolveOnHitEnemyHook(uintptr_t enemy_info_address, uintptr_t hit_info_address) {
				// Run all functions connected to this event
				for (int i = 0; i < hitenemy_hooks.size(); i++) {
					hitenemy_hooks[i](enemy_info_address, hit_info_address);
				}
				// Run the original Enemy Hit function (at the address provided in enemyhit_trampoline)
				typedef int func(uintptr_t, uintptr_t);
				func* trampoline = (func*)(hitenemy_trampoline);
				int i = trampoline(enemy_info_address, hit_info_address);
			}

			// Enter a function to call it when an enemy is hit. Parameters are the addresses of enemy data and hit data.
			MODDING_API void RunOnHitEnemy(std::function<void(uintptr_t, uintptr_t)> callback) {
				hitenemy_hooks.push_back(callback);
			}
		}

		namespace Time {
			float g_timeScale = 1.0f;
			uintptr_t rtlqpc_trampoline = NULL;
			std::vector<std::function<void(float)>> ontick_hooks;

			NOINLINE BOOL __stdcall ResolveRtlQPCHook(PLARGE_INTEGER PerformanceCount) {
				typedef BOOL(*func)(PLARGE_INTEGER);
				func trampoline = (func)(rtlqpc_trampoline);

				BOOL result = trampoline(PerformanceCount);

				static bool initialized = false;
				static LONGLONG lastRealCount = 0;
				static double accumulatedCount = 0.0;

				if (!initialized) {
					lastRealCount = PerformanceCount->QuadPart;
					accumulatedCount = (double)PerformanceCount->QuadPart;
					initialized = true;
					return result;
				}

				LONGLONG delta = PerformanceCount->QuadPart - lastRealCount;
				accumulatedCount += (double)delta * g_timeScale;
				lastRealCount = PerformanceCount->QuadPart;

				PerformanceCount->QuadPart = (LONGLONG)accumulatedCount;

				for (auto& hook : ontick_hooks) {
					hook((float)delta * g_timeScale);
				}

				return result;
			}

			MODDING_API void SetTimeScale(float scale) {
				g_timeScale = scale;
			}

			MODDING_API float GetTimeScale() {
				return g_timeScale;
			}

			MODDING_API void RunOnTick(std::function<void(float)> callback) {
				ontick_hooks.push_back(callback);
			}
		}

		// Initialize and connect hooks
		MODDING_API void InitializeHooks() {
			static PLH::NatDetour enemyhit_hook_detour = PLH::NatDetour((uintptr_t)ModAPI::Addresses::g_HitEnemyAddress, (uintptr_t)Combat::ResolveOnHitEnemyHook, &Combat::hitenemy_trampoline);
			enemyhit_hook_detour.hook();

			static PLH::NatDetour rtlqpc_detour = PLH::NatDetour(
				(uintptr_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlQueryPerformanceCounter"),
				(uintptr_t)Time::ResolveRtlQPCHook,
				&Time::rtlqpc_trampoline);
			rtlqpc_detour.hook();
		}
	}
}