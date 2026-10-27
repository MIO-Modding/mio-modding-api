using MioGame;
using System.Runtime.InteropServices;
using System.Text.Json.Nodes;
using Binding = MioGame.Input_state.Binding;

namespace MioModdingApi
{
    /// <summary>
    /// A hotkey: one keyboard key or mouse button, plus one controller button. Shown as the game's
    /// own binding rows (keyboard/mouse, then controller; either can be left out), which the player
    /// rebinds exactly like the game's controls, and clears with Delete. Hotkeys may share keys with
    /// the game's controls; both then react.
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

        /// <summary>True if the keyboard/mouse row is shown and a key or mouse button can be bound.</summary>
        public bool HasKeyboard { get; }

        /// <summary>True if the controller row is shown and a controller button can be bound.</summary>
        public bool HasController { get; }

        /// <summary>
        /// Pressing this while either binding row is listening clears that row instead of binding it.
        /// On the controller row it's the keyboard's Delete, since every spare pad button is bindable.
        /// </summary>
        public const Key ClearKey = Key.Key_delete;

        private const string ClearHint = "While rebinding, press Delete to clear.";
        private const string ControllerClearHint = "While rebinding, press Delete on the keyboard to clear.";

        /// <summary>Each row's description: which input it binds, the option's own text, and how to clear it.</summary>
        internal string KeyboardDescription { get; }
        internal string ControllerDescription { get; }

        /// <summary>Loca ids for <see cref="KeyboardDescription"/> and <see cref="ControllerDescription"/>.</summary>
        internal string KeyboardDescriptionKey { get; }
        internal string ControllerDescriptionKey { get; }

        internal KeybindOption(ModMenuPage page, string id, string title, string? description,
            Key defaultKey, Mouse_button defaultMouse, Gamepad_button defaultButton, bool keyboard, bool controller)
            : base(page, id, title, description)
        {
            if (!keyboard && !controller)
            {
                throw new ArgumentException("A keybind needs a keyboard row, a controller row, or both.", nameof(keyboard));
            }
            if (defaultKey != Key.Key_dummy && defaultMouse != Mouse_button.Mouse_none)
            {
                throw new ArgumentException("A keybind's default is a key or a mouse button, not both.", nameof(defaultMouse));
            }

            HasKeyboard = keyboard;
            HasController = controller;

            // A side that isn't shown can't be bound, so its default is dropped.
            DefaultKey = keyboard ? defaultKey : Key.Key_dummy;
            DefaultMouseButton = keyboard ? defaultMouse : Mouse_button.Mouse_none;
            DefaultButton = controller ? defaultButton : Gamepad_button.Btn_none;

            KeyboardDescription = RowDescription("(Keyboard)", description, ClearHint);
            KeyboardDescriptionKey = TitleKey + ".KEYBOARD_DESCRIPTION";
            Localization.AddString(KeyboardDescriptionKey, KeyboardDescription);

            ControllerDescription = RowDescription("(Controller)", description, ControllerClearHint);
            ControllerDescriptionKey = TitleKey + ".CONTROLLER_DESCRIPTION";
            Localization.AddString(ControllerDescriptionKey, ControllerDescription);

            Binding = (Binding*)NativeMemory.AllocZeroed((nuint)sizeof(Binding));
            Write(DefaultKey, DefaultMouseButton, DefaultButton);
            known = *Binding;
        }

        private static string RowDescription(string device, string? description, string hint) =>
            string.IsNullOrEmpty(description) ? $"{device} {hint}" : $"{device} {description}\n{hint}";

        /// <summary>The bound keyboard key, or <c>Key_dummy</c> if none, a mouse button is bound instead, or there's no keyboard row.</summary>
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

        /// <summary>The bound controller button, or <c>Btn_none</c> if none (always, without a controller row).</summary>
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
            Binding->key_primary = HasKeyboard ? key : Key.Key_dummy;
            Binding->mouse_btn = HasKeyboard ? mouse : Mouse_button.Mouse_none;
            Binding->gamepad_button = HasController ? button : Gamepad_button.Btn_none;
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

        /// <summary>Unbinds the controller button (Delete on a listening controller row).</summary>
        internal void ClearButton()
        {
            Binding->gamepad_button = Gamepad_button.Btn_none;
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