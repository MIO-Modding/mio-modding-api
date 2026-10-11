using System.Globalization;
using System.Runtime.InteropServices;
using System.Text.Json.Nodes;

namespace MioModdingApi
{
    /// <summary>How an option appears in Settings.</summary>
    internal enum ModMenuRowKind
    {
        /// <summary>One choice row (toggle, choice, number).</summary>
        Choice,
        /// <summary>One goto row that runs code (button).</summary>
        Button,
        /// <summary>A keyboard/mouse binding row followed by a controller binding row (keybind).</summary>
        Keybind,
    }

    /// <summary>
    /// A setting shown on a mod's page. Every option is stored as an index into its list of values:
    /// the game's Settings rows write the selected index straight into native memory owned by the
    /// option, which is polled for changes while Settings is open.
    /// </summary>
    public abstract unsafe class ModMenuOption
    {
        public ModMenuPage Page { get; }
        public string Id { get; }
        public string Title { get; }
        public string? Description { get; }

        /// <summary>Native cell the Settings row writes into. Allocated once, never freed.</summary>
        internal readonly int* Cell;

        /// <summary>Index last seen in <see cref="Cell"/>, for change detection.</summary>
        private int knownIndex;

        /// <summary>Set when code changes the value, so an open Settings row can be redrawn.</summary>
        internal bool RowNeedsSync;

        internal string TitleKey { get; }
        internal string DescriptionKey { get; }

        /// <summary>Loca ids for each value, in order.</summary>
        internal abstract string[] ValueKeys { get; }

        /// <summary>Toggles use the row's bool pointer; everything else its choice index.</summary>
        internal virtual bool IsToggle => false;

        /// <summary>False for rows that hold no value (buttons): nothing to poll, save or reset.</summary>
        internal virtual bool HasValue => true;

        /// <summary>Which kind of Settings row(s) the option is shown as.</summary>
        internal virtual ModMenuRowKind RowKind => ModMenuRowKind.Choice;

        internal abstract int DefaultIndex { get; }

        private protected ModMenuOption(ModMenuPage page, string id, string title, string? description)
        {
            ArgumentException.ThrowIfNullOrWhiteSpace(id);
            ArgumentException.ThrowIfNullOrWhiteSpace(title);

            Page = page;
            Id = id;
            Title = title;
            Description = description;

            Cell = (int*)NativeMemory.AllocZeroed(sizeof(int));

            string baseKey = ModMenu.KeyPrefix + page.ModId + "." + id;
            TitleKey = baseKey + ".TITLE";
            Localization.AddString(TitleKey, title);

            // Rows without a description pass an empty one, as the game's own rows do.
            DescriptionKey = "";
            if (!string.IsNullOrEmpty(description))
            {
                DescriptionKey = baseKey + ".DESCRIPTION";
                Localization.AddString(DescriptionKey, description);
            }
        }

        /// <summary>Called once the subclass has its values, to apply the default.</summary>
        private protected void InitIndex()
        {
            WriteIndex(DefaultIndex);
            knownIndex = DefaultIndex;
        }

        /// <summary>Index of the current value.</summary>
        internal int Index => ReadIndex();

        private int ReadIndex()
        {
            if (!HasValue) return 0;
            // A toggle's row writes a C++ bool (one byte) through p_bool.
            int raw = IsToggle ? *(byte*)Cell : *Cell;
            return Math.Clamp(raw, 0, ValueKeys.Length - 1);
        }

        private void WriteIndex(int index)
        {
            if (IsToggle)
            {
                *Cell = 0;
                *(byte*)Cell = (byte)(index != 0 ? 1 : 0);
            }
            else
            {
                *Cell = index;
            }
        }

        /// <summary>Sets the value from code (or from the saved config).</summary>
        private protected void SetIndex(int index, bool save)
        {
            index = Math.Clamp(index, 0, ValueKeys.Length - 1);
            if (index == knownIndex && ReadIndex() == index) return;

            WriteIndex(index);
            knownIndex = index;
            RowNeedsSync = true;
            NotifyChanged();
            if (save) ModMenuConfig.Changed(this, saveNow: true);
        }

        /// <summary>Picks up a change the Settings row wrote into <see cref="Cell"/>.</summary>
        internal virtual void PollNative()
        {
            if (!HasValue) return;
            int index = ReadIndex();
            if (index == knownIndex) return;

            knownIndex = index;
            NotifyChanged();
            ModMenuConfig.Changed(this, saveNow: false);   // written once the row's edit closes
        }

        /// <summary>A throwing handler must not break the Settings hook that noticed the change.</summary>
        private protected void NotifyChanged()
        {
            try { RaiseChanged(); }
            catch (Exception ex) { ModMenu.Log($"a Changed handler for {Page.ModId}/{Id} threw: {ex}"); }
        }

        /// <summary>Puts the option back to its default value and saves.</summary>
        public virtual void ResetToDefault()
        {
            if (HasValue) SetIndex(DefaultIndex, save: true);
        }

        private protected abstract void RaiseChanged();

        internal abstract JsonNode ToJson();

        /// <summary>Applies a saved value. Ignores values that no longer fit (e.g. a removed choice).</summary>
        internal abstract void FromJson(JsonNode node);

        private protected string ValueKey(int index) => ModMenu.KeyPrefix + Page.ModId + "." + Id + ".VALUE." + index;
    }

    /// <summary>An on/off option, shown with the game's own Yes/No.</summary>
    public sealed class ToggleOption : ModMenuOption
    {
        private static readonly string[] YesNoKeys = ["UI_MENU_SETTINGS_BOOL_NO", "UI_MENU_SETTINGS_BOOL_YES"];

        public bool DefaultValue { get; }

        /// <summary>Raised with the new value whenever it changes, from Settings or from code.</summary>
        public event Action<bool>? Changed;

        internal ToggleOption(ModMenuPage page, string id, string title, string? description, bool defaultValue)
            : base(page, id, title, description)
        {
            DefaultValue = defaultValue;
            InitIndex();
        }

        public bool Value
        {
            get => Index != 0;
            set => SetIndex(value ? 1 : 0, save: true);
        }

        internal override string[] ValueKeys => YesNoKeys;
        internal override bool IsToggle => true;
        internal override int DefaultIndex => DefaultValue ? 1 : 0;

        private protected override void RaiseChanged() => Changed?.Invoke(Value);

        internal override JsonNode ToJson() => JsonValue.Create(Value);

        internal override void FromJson(JsonNode node)
        {
            if (node is JsonValue v && v.TryGetValue(out bool b)) SetIndex(b ? 1 : 0, save: false);
        }
    }

    /// <summary>An option that picks one of several labelled values.</summary>
    public sealed class ChoiceOption : ModMenuOption
    {
        private readonly string[] choices;
        private readonly string[] valueKeys;
        private readonly int defaultIndex;

        public IReadOnlyList<string> Choices => choices;

        /// <summary>Raised with the new index whenever it changes, from Settings or from code.</summary>
        public event Action<int>? Changed;

        internal ChoiceOption(ModMenuPage page, string id, string title, string? description, IReadOnlyList<string> choices, int defaultIndex)
            : base(page, id, title, description)
        {
            ArgumentNullException.ThrowIfNull(choices);
            if (choices.Count == 0) throw new ArgumentException("A choice option needs at least one choice.", nameof(choices));
            if (defaultIndex < 0 || defaultIndex >= choices.Count) throw new ArgumentOutOfRangeException(nameof(defaultIndex));

            this.choices = choices.ToArray();
            this.defaultIndex = defaultIndex;
            valueKeys = new string[this.choices.Length];
            for (int i = 0; i < this.choices.Length; i++)
            {
                valueKeys[i] = ValueKey(i);
                Localization.AddString(valueKeys[i], this.choices[i]);
            }
            InitIndex();
        }

        /// <summary>Index of the selected choice.</summary>
        public int SelectedIndex
        {
            get => Index;
            set => SetIndex(value, save: true);
        }

        /// <summary>The selected choice's label. Setting it selects the first choice with that label.</summary>
        public string Value
        {
            get => choices[Index];
            set
            {
                int index = Array.IndexOf(choices, value);
                if (index < 0) throw new ArgumentException($"'{value}' is not one of the choices.", nameof(value));
                SetIndex(index, save: true);
            }
        }

        internal override string[] ValueKeys => valueKeys;
        internal override int DefaultIndex => defaultIndex;

        private protected override void RaiseChanged() => Changed?.Invoke(Index);

        // Saved by label, so reordering or adding choices in a later version keeps the player's pick.
        internal override JsonNode ToJson() => JsonValue.Create(Value);

        internal override void FromJson(JsonNode node)
        {
            if (node is not JsonValue v || !v.TryGetValue(out string? label)) return;
            int index = Array.IndexOf(choices, label);
            if (index >= 0) SetIndex(index, save: false);
        }
    }

    /// <summary>A whole number in a fixed range and step.</summary>
    public sealed class NumberOption : ModMenuOption
    {
        /// <summary>Settings shows every value as its own entry; beyond this it gets unusable.</summary>
        private const int MaxValueCount = 1000;

        private readonly string[] valueKeys;
        private readonly int defaultIndex;

        public int Min { get; }
        public int Max { get; }
        public int Step { get; }
        public int DefaultValue { get; }

        /// <summary>Raised with the new value whenever it changes, from Settings or from code.</summary>
        public event Action<int>? Changed;

        internal NumberOption(ModMenuPage page, string id, string title, string? description, int min, int max, int step, int defaultValue, string format)
            : base(page, id, title, description)
        {
            if (step <= 0) throw new ArgumentOutOfRangeException(nameof(step), "Step must be positive.");
            if (max < min) throw new ArgumentException("Max must be at least min.", nameof(max));

            long count = ((long)max - min) / step + 1;
            if (count > MaxValueCount) throw new ArgumentException($"Too many values ({count}); Settings can show at most {MaxValueCount}. Use a larger step.", nameof(step));

            Min = min;
            Max = min + (int)(count - 1) * step;   // the last value actually reachable with this step
            Step = step;
            defaultIndex = ToIndex(defaultValue);
            DefaultValue = FromIndex(defaultIndex);

            valueKeys = new string[count];
            for (int i = 0; i < count; i++)
            {
                valueKeys[i] = ValueKey(i);
                Localization.AddString(valueKeys[i], string.Format(CultureInfo.InvariantCulture, format, FromIndex(i)));
            }
            InitIndex();
        }

        public int Value
        {
            get => FromIndex(Index);
            set => SetIndex(ToIndex(value), save: true);
        }

        private int FromIndex(int index) => Min + index * Step;

        /// <summary>Nearest index for a value, clamped to the range.</summary>
        private int ToIndex(int value)
        {
            long index = (long)Math.Round(((double)value - Min) / Step);
            return (int)Math.Clamp(index, 0, ((long)Max - Min) / Step);
        }

        internal override string[] ValueKeys => valueKeys;
        internal override int DefaultIndex => defaultIndex;

        private protected override void RaiseChanged() => Changed?.Invoke(Value);

        internal override JsonNode ToJson() => JsonValue.Create(Value);

        internal override void FromJson(JsonNode node)
        {
            if (node is JsonValue v && v.TryGetValue(out int n)) SetIndex(ToIndex(n), save: false);
        }
    }

    /// <summary>
    /// A row that runs code when the player selects it, like the game's own "Remap gamepad" row.
    /// It holds no value, so nothing is saved and Reset to Default leaves it alone.
    /// </summary>
    public sealed class ButtonOption : ModMenuOption
    {
        /// <summary>
        /// Raised when the player selects the button. Runs on the game thread during the Settings
        /// update, so keep it quick; start longer work elsewhere.
        /// </summary>
        public event Action? Pressed;

        internal ButtonOption(ModMenuPage page, string id, string title, string? description, Action? onPressed)
            : base(page, id, title, description)
        {
            if (onPressed != null) Pressed += onPressed;
        }

        internal override bool HasValue => false;
        internal override ModMenuRowKind RowKind => ModMenuRowKind.Button;
        internal override string[] ValueKeys => [];
        internal override int DefaultIndex => 0;

        /// <summary>Runs the button's handlers. A throwing handler is logged, not passed to the game.</summary>
        internal void Press()
        {
            try { Pressed?.Invoke(); }
            catch (Exception ex) { ModMenu.Log($"button {Page.ModId}/{Id} threw: {ex}"); }
        }

        private protected override void RaiseChanged() { }
        internal override JsonNode ToJson() => JsonValue.Create(0);
        internal override void FromJson(JsonNode node) { }
    }
}