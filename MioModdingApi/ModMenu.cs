using MioModLoader;

namespace MioModdingApi
{
    /// <summary>
    /// Adds a native "Mods" entry to the game's Settings menu (Game page). Selecting it lists the
    /// mods that registered options; selecting a mod shows its options, which the player can change
    /// like any other setting. Values are saved in the modding api's own config folder, not the game
    /// save.
    /// <para />
    /// Register from your mod's <c>Initialize</c>. Settings is built when the main menu loads and
    /// again each time it is opened from the pause menu, so anything registered later appears the
    /// next time Settings is built.
    /// </summary>
    /// <example>
    /// <code>
    /// var page = ModMenu.RegisterMod(this, description: "Tweaks for the hook.");
    /// var fastHook = page.RegisterToggle("fast_hook", "Fast hook", defaultValue: true);
    /// var speed = page.RegisterNumber("hook_speed", "Hook speed", min: 50, max: 200, step: 10, defaultValue: 100, format: "{0}%");
    ///
    /// if (fastHook.Value) { ... }
    /// speed.Changed += value => LogMessage($"hook speed is now {value}%");
    /// </code>
    /// </example>
    public static class ModMenu
    {
        private static readonly List<ModMenuPage> pages = [];

        /// <summary>Every registered mod page, in registration order.</summary>
        public static IReadOnlyList<ModMenuPage> Pages => pages;

        /// <summary>
        /// Adds your mod to the Mods list. The display name defaults to the name in your mod.json.
        /// Registering the same mod twice returns the existing page.
        /// </summary>
        public static ModMenuPage RegisterMod(Mod mod, string? displayName = null, string? description = null)
        {
            ArgumentNullException.ThrowIfNull(mod);
            return RegisterMod(mod.Id, displayName ?? mod.Name, description);
        }

        /// <summary>
        /// Adds a page to the Mods list under an id of your choosing. Prefer
        /// <see cref="RegisterMod(Mod, string?, string?)"/>, which uses your mod's id.
        /// Registering the same id twice returns the existing page.
        /// </summary>
        public static ModMenuPage RegisterMod(string modId, string displayName, string? description = null)
        {
            ArgumentException.ThrowIfNullOrWhiteSpace(modId);
            ArgumentException.ThrowIfNullOrWhiteSpace(displayName);

            if (TryGetPage(modId, out ModMenuPage? existing))
            {
                return existing;
            }

            var page = new ModMenuPage(modId, displayName, description);
            pages.Add(page);
            return page;
        }

        /// <summary>The page registered under <paramref name="modId"/>, if any.</summary>
        public static bool TryGetPage(string modId, [System.Diagnostics.CodeAnalysis.NotNullWhen(true)] out ModMenuPage? page)
        {
            page = pages.FirstOrDefault(p => p.ModId == modId);
            return page != null;
        }

        /// <summary>
        /// Looks up an option by mod id and option id, e.g. to read another mod's setting.
        /// Null if either isn't registered or the option is a different type.
        /// </summary>
        public static T? GetOption<T>(string modId, string optionId) where T : ModMenuOption
        {
            return TryGetPage(modId, out ModMenuPage? page) ? page.GetOption<T>(optionId) : null;
        }

        public static void ApplyHooks()
        {
            ModMenuUi.ApplyHooks();
        }

        /// <summary>Pages that have something to show, in registration order.</summary>
        internal static ModMenuPage[] VisiblePages() => pages.Where(p => p.Options.Count > 0).ToArray();

        /// <summary>Picks up values the game wrote into option storage. Called once per Settings frame.</summary>
        internal static void PollValues()
        {
            foreach (ModMenuPage page in pages)
            {
                foreach (ModMenuOption option in page.Options)
                {
                    option.PollNative();
                }
            }
        }

        internal static void Log(string message)
        {
            MioModdingApi.Instance?.LogMessage("[ModMenu] " + message);
        }

        /// <summary>Loca id prefix for every string the menu shows.</summary>
        internal const string KeyPrefix = "MIO_MODDING_API.MOD_MENU.";
    }

    /// <summary>One mod's entry in the Mods list and the options shown when it is selected.</summary>
    public sealed class ModMenuPage
    {
        private readonly List<ModMenuOption> options = [];

        public string ModId { get; }
        public string DisplayName { get; }
        public string? Description { get; }

        /// <summary>Options in the order they appear.</summary>
        public IReadOnlyList<ModMenuOption> Options => options;

        internal string TitleKey { get; }
        internal string DescriptionKey { get; }

        internal ModMenuPage(string modId, string displayName, string? description)
        {
            ModId = modId;
            DisplayName = displayName;
            Description = description;

            TitleKey = ModMenu.KeyPrefix + modId + ".TITLE";
            Localization.AddString(TitleKey, displayName);

            DescriptionKey = "";
            if (!string.IsNullOrEmpty(description))
            {
                DescriptionKey = ModMenu.KeyPrefix + modId + ".DESCRIPTION";
                Localization.AddString(DescriptionKey, description);
            }
        }

        /// <summary>An on/off option, shown with the game's own Yes/No.</summary>
        public ToggleOption RegisterToggle(string id, string title, bool defaultValue = false, string? description = null)
        {
            return Add(new ToggleOption(this, id, title, description, defaultValue));
        }

        /// <summary>
        /// An option that picks one of several labelled values, cycled with left/right like the
        /// game's own choices.
        /// </summary>
        public ChoiceOption RegisterChoice(string id, string title, IReadOnlyList<string> choices, int defaultIndex = 0, string? description = null)
        {
            return Add(new ChoiceOption(this, id, title, description, choices, defaultIndex));
        }

        /// <summary>
        /// A whole number from <paramref name="min"/> to <paramref name="max"/> in steps of
        /// <paramref name="step"/>, cycled with left/right. <paramref name="format"/> is a .NET format
        /// string for the label, e.g. "{0}%" or "{0} s".
        /// </summary>
        public NumberOption RegisterNumber(string id, string title, int min, int max, int step = 1, int? defaultValue = null, string? description = null, string format = "{0}")
        {
            return Add(new NumberOption(this, id, title, description, min, max, step, defaultValue ?? min, format));
        }

        /// <summary>
        /// A row that runs <paramref name="onPressed"/> when the player selects it. Buttons can sit
        /// anywhere among the other options; they hold no value and aren't saved.
        /// </summary>
        public ButtonOption RegisterButton(string id, string title, Action? onPressed = null, string? description = null)
        {
            return Add(new ButtonOption(this, id, title, description, onPressed));
        }

        /// <summary>Puts every option on this page back to its default and saves.</summary>
        public void ResetToDefaults()
        {
            foreach (ModMenuOption option in options) option.ResetToDefault();
        }

        /// <summary>This page's option with the given id, or null if missing or a different type.</summary>
        public T? GetOption<T>(string id) where T : ModMenuOption
        {
            return options.FirstOrDefault(o => o.Id == id) as T;
        }

        private T Add<T>(T option) where T : ModMenuOption
        {
            if (options.Any(o => o.Id == option.Id))
            {
                throw new ArgumentException($"Mod '{ModId}' already has an option with id '{option.Id}'.", "id");
            }

            options.Add(option);
            if (option.HasValue) ModMenuConfig.ApplySaved(option);
            return option;
        }
    }
}