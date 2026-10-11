using MioGame;
using System.Runtime.InteropServices;
using System.Text.Json.Nodes;
using Binding = MioGame.Input_state.Binding;

namespace MioModdingApi
{
    /// <summary>
    /// A hotkey: one keyboard key or mouse button, plus one controller button. Shown as two of the
    /// game's own binding rows (keyboard/mouse, then controller), which the player rebinds exactly
    /// like the game's controls. Hotkeys may share keys with the game's controls; both then react.
    /// </summary>
    /// <example>
    /// <code>
    /// var noclip = page.RegisterKeybind("noclip", "Toggle noclip", defaultKey: Key.Key_N, defaultButton: Gamepad_button.Btn_leftstick);
    /// // every frame, e.g. in a Game::fixed_update hook:
    /// if (noclip.WasPressed) ToggleNoclip();
    /// </code>
    /// </example>
    public sealed unsafe class KeybindOption : ModMenuOption
    {
        /// <summary>Native binding both rows point at; the game writes rebinds straight into it.</summary>
        internal readonly Binding* Binding;

        private Binding known;

        public Key DefaultKey { get; }
        public Mouse_button DefaultMouseButton { get; }
        public Gamepad_button DefaultButton { get; }

        /// <summary>Raised whenever the binding changes, from Settings or from code.</summary>
        public event Action<KeybindOption>? Changed;

        /// <summary>Loca id for the controller row's title.</summary>
        internal string ControllerTitleKey { get; }

        /// <summary>Pressing this while the keyboard/mouse row is listening clears it instead of binding it.</summary>
        public const Key ClearKey = Key.Key_delete;

        private const string ClearHint = "While rebinding, press Delete to clear.";

        /// <summary>The keyboard/mouse row's description: the option's own, plus how to clear it.</summary>
        internal string KeyboardDescription { get; }

        /// <summary>Loca id for <see cref="KeyboardDescription"/>.</summary>
        internal string KeyboardDescriptionKey { get; }

        internal KeybindOption(ModMenuPage page, string id, string title, string? description,
            Key defaultKey, Mouse_button defaultMouse, Gamepad_button defaultButton)
            : base(page, id, title, description)
        {
            if (defaultKey != Key.Key_dummy && defaultMouse != Mouse_button.Mouse_none)
            {
                throw new ArgumentException("A keybind's default is a key or a mouse button, not both.", nameof(defaultMouse));
            }

            DefaultKey = defaultKey;
            DefaultMouseButton = defaultMouse;
            DefaultButton = defaultButton;

            ControllerTitleKey = TitleKey + ".CONTROLLER";
            Localization.AddString(ControllerTitleKey, title + " (Controller)");

            KeyboardDescription = string.IsNullOrEmpty(description) ? ClearHint : description + "\n" + ClearHint;
            KeyboardDescriptionKey = TitleKey + ".KEYBOARD_DESCRIPTION";
            Localization.AddString(KeyboardDescriptionKey, KeyboardDescription);

            Binding = (Binding*)NativeMemory.AllocZeroed((nuint)sizeof(Binding));
            Write(defaultKey, defaultMouse, defaultButton);
            known = *Binding;
        }

        /// <summary>The bound keyboard key, or <c>Key_dummy</c> if none (or a mouse button is bound instead).</summary>
        public Key Key
        {
            get => Binding->key_primary;
            set => Set(value, value != Key.Key_dummy ? Mouse_button.Mouse_none : Binding->mouse_btn, Binding->gamepad_button);
        }

        /// <summary>The bound mouse button, or <c>Mouse_none</c> if none (or a key is bound instead).</summary>
        public Mouse_button MouseButton
        {
            get => Binding->mouse_btn;
            set => Set(value != Mouse_button.Mouse_none ? Key.Key_dummy : Binding->key_primary, value, Binding->gamepad_button);
        }

        /// <summary>The bound controller button, or <c>Btn_none</c> if none.</summary>
        public Gamepad_button Button
        {
            get => Binding->gamepad_button;
            set => Set(Binding->key_primary, Binding->mouse_btn, value);
        }

        // ---------------------------------------------------------------- reading input

        /// <summary>True while the bound key, mouse button or controller button is held.</summary>
        public bool IsDown => Any(static b => b.down);

        /// <summary>True on the frame the binding is first pressed.</summary>
        public bool WasPressed => Any(static b => b.pressed);

        /// <summary>True on the frame the binding is released.</summary>
        public bool WasReleased => Any(static b => b.released);

        /// <summary>
        /// Checks the game's gameplay input (the same state its own controls read) for the bound key,
        /// mouse button and controller button.
        /// </summary>
        private bool Any(Func<Input_btn, bool> test)
        {
            ref Input_state.Game game = ref Globals.input.game;

            Key key = Binding->key_primary;
            if (key != Key.Key_dummy && (uint)key < Fixed_array_Input_btn_512.count && test(game.kb.keys.keys.data[(int)key])) return true;

            Mouse_button mouse = Binding->mouse_btn;
            if (mouse != Mouse_button.Mouse_none && (uint)mouse < Fixed_array_Input_btn_6.count && test(game.mouse.buttons.data[(int)mouse])) return true;

            Gamepad_button button = Binding->gamepad_button;
            if (button != Gamepad_button.Btn_none && (uint)button < Fixed_array_Input_btn_22.count && test(game.pad.buttons.data[(int)button])) return true;

            return false;
        }

        // ---------------------------------------------------------------- value plumbing

        internal override ModMenuRowKind RowKind => ModMenuRowKind.Keybind;
        internal override string[] ValueKeys => [];
        internal override int DefaultIndex => 0;

        private void Write(Key key, Mouse_button mouse, Gamepad_button button)
        {
            Binding->key_primary = key;
            Binding->mouse_btn = mouse;
            Binding->gamepad_button = button;
            Binding->key_secondary = Key.Key_dummy;
        }

        private void Set(Key key, Mouse_button mouse, Gamepad_button button, bool save = true)
        {
            Write(key, mouse, button);
            if (SameAs(known)) return;

            known = *Binding;
            NotifyChanged();
            if (save) ModMenuConfig.Changed(this, saveNow: true);
        }

        private bool SameAs(Binding other) =>
            Binding->key_primary == other.key_primary &&
            Binding->mouse_btn == other.mouse_btn &&
            Binding->gamepad_button == other.gamepad_button;

        /// <summary>Picks up a rebind the Settings row wrote into <see cref="Binding"/>.</summary>
        internal override void PollNative()
        {
            if (SameAs(known)) return;
            known = *Binding;
            NotifyChanged();
            ModMenuConfig.Changed(this, saveNow: false);
        }

        public override void ResetToDefault() => Set(DefaultKey, DefaultMouseButton, DefaultButton);

        /// <summary>Unbinds the key, mouse button and controller button, and saves.</summary>
        public void Clear() => Set(Key.Key_dummy, Mouse_button.Mouse_none, Gamepad_button.Btn_none);

        /// <summary>
        /// Applies a new keyboard/mouse binding from the Settings row. A key replaces a mouse button
        /// and vice versa, so a hotkey is always one or the other. <see cref="ClearKey"/> unbinds both.
        /// </summary>
        internal void ApplyKeyboardMouse(Key key, Mouse_button mouse)
        {
            if (key == ClearKey) Write(Key.Key_dummy, Mouse_button.Mouse_none, Binding->gamepad_button);
            else if (key != Key.Key_dummy) Write(key, Mouse_button.Mouse_none, Binding->gamepad_button);
            else if (mouse != Mouse_button.Mouse_none) Write(Key.Key_dummy, mouse, Binding->gamepad_button);
        }

        /// <summary>Applies a new controller binding from the Settings row.</summary>
        internal void ApplyButton(Gamepad_button button)
        {
            if (button != Gamepad_button.Btn_none) Binding->gamepad_button = button;
        }

        private protected override void RaiseChanged() => Changed?.Invoke(this);

        // Saved by enum name, so the file stays readable and survives renumbering.
        internal override JsonNode ToJson() => new JsonObject
        {
            ["key"] = Enum.GetName(Binding->key_primary) ?? nameof(Key.Key_dummy),
            ["mouse"] = Enum.GetName(Binding->mouse_btn) ?? nameof(Mouse_button.Mouse_none),
            ["button"] = Enum.GetName(Binding->gamepad_button) ?? nameof(Gamepad_button.Btn_none),
        };

        internal override void FromJson(JsonNode node)
        {
            if (node is not JsonObject o) return;
            Key key = Parse(o["key"], Binding->key_primary);
            Mouse_button mouse = Parse(o["mouse"], Binding->mouse_btn);
            Gamepad_button button = Parse(o["button"], Binding->gamepad_button);
            if (key != Key.Key_dummy) mouse = Mouse_button.Mouse_none;
            Set(key, mouse, button, save: false);
        }

        private static T Parse<T>(JsonNode? node, T fallback) where T : struct, Enum =>
            node is JsonValue v && v.TryGetValue(out string? name) && Enum.TryParse(name, out T parsed) ? parsed : fallback;
    }
}