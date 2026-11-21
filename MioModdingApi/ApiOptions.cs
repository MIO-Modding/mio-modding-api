using MioGame;
using System.Diagnostics;

namespace MioModdingApi
{
    /// <summary>
    /// The modding api's own page in the Mods menu.
    /// </summary>
    internal static class ApiOptions
    {
        private static ToggleOption? debugMode;
        private static ButtonOption? openMods;
        private static bool startupApplied;
        private static string modsPath = "";

        /// <summary>Registers the api's page. Called from the api's Initialize, so it is listed first.</summary>
        internal static void Register(MioModdingApi api)
        {
            var page = ModMenu.RegisterMod(api, displayName: "Modding API", description: "Core API for modifying MIO.");

            debugMode = page.RegisterToggle("debug_mode", "Start in debug mode", defaultValue: false,
                description: "Turns on the game's debug mode automatically, without the d-pad input. " +
                             "Turning it off takes effect the next time the game starts.");

            // Turning it on takes effect straight away; there's no clean way to turn debug mode off.
            debugMode.Changed += enabled =>
            {
                if (enabled) EnableDebug();
            };

            modsPath = AppContext.BaseDirectory + "mods\\";
            openMods = page.RegisterButton("open_mods", "Open mods folder",
                description: "Opens the mods folder in your filesystem.");
            openMods.Pressed += OpenMods_Pressed;
        }

        internal static unsafe void ApplyHooks()
        {
            On.MioGame.On_Metagame.update.Suffix += Metagame_update_Suffix;
        }

        /// <summary>
        /// Applies "Start in debug mode" once the game reaches the main menu: the point where the
        /// player could first use the d-pad input themselves.
        /// </summary>
        private static unsafe void Metagame_update_Suffix(Metagame* self)
        {
            if (startupApplied || self->state < Metagame.State.Main_menu) return;
            startupApplied = true;

            try
            {
                if (debugMode?.Value == true) EnableDebug();
            }
            catch (Exception ex)
            {
                ModMenu.Log($"enabling debug mode failed: {ex}");
            }
        }

        private static void EnableDebug()
        {
            ref Metagame metagame = ref Globals.metagame;
            if (metagame.debug_enabled) return;

            // The game only allows debug mode on installs with its debug key, and the d-pad input
            // respects that. This option deliberately doesn't: mark it allowed first, in case
            // enable_debug checks the flag itself.
            if (!metagame.debug_allowed)
            {
                metagame.debug_allowed = true;
                ModMenu.Log("debug mode wasn't allowed on this install (no debug key); allowing it");
            }

            MioGame.GlobalFunctions.game.metagame.enable_debug();
            ModMenu.Log($"debug mode turned on (debug_enabled={metagame.debug_enabled})");
        }

        private static void OpenMods_Pressed()
        {
            try
            {
                Process.Start(new ProcessStartInfo()
                {
                    FileName = modsPath,
                    UseShellExecute = true,
                    Verb = "open"
                });
            }
            catch (Exception ex) { ModMenu.Log("could not open mods folder: " + ex); }
        }
    }
}