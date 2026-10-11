using System.Text.Json;
using System.Text.Json.Nodes;

namespace MioModdingApi
{
    /// <summary>
    /// Stores Mods-menu values in the modding api's own config folder
    /// (<c>modconfig/&lt;api id&gt;/mod-menu.json</c>), never in the game save:
    /// <code>
    /// { "some.mod.id": { "fast_hook": true, "difficulty": "Hard", "hook_speed": 120 } }
    /// </code>
    /// Entries for mods that aren't installed right now are kept, so uninstalling a mod for a while
    /// doesn't lose its settings.
    /// </summary>
    internal static class ModMenuConfig
    {
        private const string FileName = "mod-menu.json";

        private static JsonObject? root;
        private static bool dirty;

        private static string? FilePath
        {
            get
            {
                MioModdingApi? api = MioModdingApi.Instance;
                return api == null ? null : Path.Combine(api.GetModConfigPath(), FileName);
            }
        }

        private static JsonObject Root
        {
            get
            {
                if (root != null) return root;
                root = Load() ?? new JsonObject();
                return root;
            }
        }

        private static JsonObject? Load()
        {
            string? path = FilePath;
            if (path == null || !File.Exists(path)) return null;

            try
            {
                return JsonNode.Parse(File.ReadAllText(path)) as JsonObject;
            }
            catch (Exception ex)
            {
                // Keep the broken file for the player to inspect, and start from defaults.
                ModMenu.Log($"could not read {path}, using defaults: {ex.Message}");
                try { File.Copy(path, path + ".broken", overwrite: true); } catch { /* best effort */ }
                return null;
            }
        }

        /// <summary>Applies the saved value for a newly registered option, if there is one.</summary>
        internal static void ApplySaved(ModMenuOption option)
        {
            if (Root[option.Page.ModId] is JsonObject mod && mod[option.Id] is JsonNode saved)
            {
                option.FromJson(saved);
            }
        }

        /// <summary>Records an option's new value. Saves now, or on the next <see cref="SaveIfDirty"/>.</summary>
        internal static void Changed(ModMenuOption option, bool saveNow)
        {
            if (!option.HasValue) return;
            if (Root[option.Page.ModId] is not JsonObject mod)
            {
                mod = new JsonObject();
                Root[option.Page.ModId] = mod;
            }
            mod[option.Id] = option.ToJson();
            dirty = true;

            if (saveNow) SaveIfDirty();
        }

        internal static void SaveIfDirty()
        {
            if (!dirty) return;
            dirty = false;

            string? path = FilePath;
            if (path == null) return;

            try
            {
                Directory.CreateDirectory(Path.GetDirectoryName(path)!);

                // Write beside the real file and swap, so a crash mid-write can't truncate it.
                string temp = path + ".tmp";
                File.WriteAllText(temp, Root.ToJsonString(new JsonSerializerOptions { WriteIndented = true }));
                File.Move(temp, path, overwrite: true);
            }
            catch (Exception ex)
            {
                ModMenu.Log($"could not save {path}: {ex.Message}");
            }
        }
    }
}