using MioGame;
using TNode_Row = MioGame.TNode_Ui_interactive_node;
using Widget = MioGame.Ui_settings.Widget;
using WidgetArray = MioGame.Array_Ui_settings__Widget;

namespace MioModdingApi
{
    /// <summary>
    /// Builds the Mods menu out of the game's own Settings rows.
    ///
    /// LAYOUT. Settings keeps its pages in a fixed <c>Page[7]</c> indexed by category, so there is no
    /// spare slot for a new page. Slot 6 (Controller_binding) has no tab and is only reached through a
    /// goto row, which makes it a usable host: after the game builds it, every row the Mods menu needs
    /// (the mod list, then each mod's options) is appended to its row list behind the vanilla rows,
    /// and the list is trimmed back so vanilla only sees its own. Each screen is then shown by
    /// pointing the page's list header at a range of that one buffer.
    ///
    /// RULES the engine imposes (each learned from a crash):
    ///  - An array finds its allocator through its existing buffer. Never push into an array the
    ///    engine didn't create, or that has no buffer: it asserts 'area' or faults. So rows are only
    ///    appended to lists the game built, and the screens are read-only views.
    ///  - add_choice copies its values array and needs that array to be engine-allocated. Rows are
    ///    built from a vanilla Yes/No row's values, then the row's own copy is resized and its entries
    ///    pointed at our strings.
    ///  - Row text must be loca ids. Ours are registered with <see cref="Localization"/>.
    ///
    /// NAVIGATION is driven by watching <c>page_opened</c> after the game's update:
    ///  - Game page goto "Mods" (0 -> 6): show the mod list.
    ///  - A mod-list row or a button is a goto aimed at the Game page (6 -> 0): open the mod that was
    ///    hovered, or run the button.
    ///  - Back from slot 6 always goes to Controller (6 -> 4): go up one level instead.
    /// Per Ui_settings::pre_sim_update, page visibility is applied at the start of the update and a
    /// goto or back is applied at its end, so correcting page_opened in the Suffix lands before any
    /// page is shown or hidden: the detour is never visible. The game focuses rows by flag, so every
    /// screen change moves the hover itself.
    /// </summary>
    internal static unsafe partial class ModMenuUi
    {
        private const int GamePage = (int)Ui_settings.Cat.Game;
        private const int ControllerPage = (int)Ui_settings.Cat.Controller;
        private const int HostPage = (int)Ui_settings.Cat.Controller_binding;
        private const int PageCount = HostPage + 1;

        /// <summary>Where mod-list goto rows point. The jump is caught and turned into "open that mod".</summary>
        private const int OpenModTarget = GamePage;

        private const string ModsRowKey = ModMenu.KeyPrefix + "MODS";

        /// <summary>Row placement: row n of a page sits at base_pos + n * increment. Refreshed from the game's own calls.</summary>
        private static Vec_float_3 basePos = new() { x = 95, y = -36, z = 0 };
        private static Vec_float_3 increment = new() { x = 0, y = -60, z = 0 };

        private enum Screen { Closed, ModList, ModPage }

        /// <summary>A run of rows inside slot 6's buffer.</summary>
        private readonly record struct View(uint Start, uint Count);

        /// <summary>Mods-menu state for one Settings instance (main menu and pause menu each build their own).</summary>
        private sealed class Menu
        {
            public ModMenuPage[] Pages = [];
            public TNode_Row ModsRow;            // the "Mods" row on the Game page (fallback when there's no tab)
            public TNode_Row Tab;                // the "Mods" tab in the left column, once created
            public bool Previewing;              // the tab is hovered at tab level: mod list shown as a preview
            public float PreviewFade;            // the preview's fade-in, driven by us (see DrivePreviewFade)

            public bool Built;
            public uint VanillaCount;            // slot 6's own rows, at the front of its buffer
            public uint TotalCount;              // everything in the buffer, ours included
            public View ModList;
            public View[] ModPages = [];
            public ModMenuOption[][] PageRows = [];   // per mod page, the option behind each row
            public string?[][] PageRowDescriptions = [];   // per mod page, each row's description

            public Screen Screen;
            public int OpenPage = -1;
            public int LastPageOpened = int.MinValue;
            public int HoveredBeforeUpdate = -1;
            public WidgetArray VanillaHeader;    // slot 6's real header, while a view is in
            public readonly List<nint> HiddenVanilla = [];

            /// <summary>
            /// The screen left on display after closing. Slot 6 fades out over several frames, so its
            /// vanilla content stays hidden and our rows stay up until slot 6 is next entered.
            /// </summary>
            public View? Lingering;

            /// <summary>Slot 6's own bottom scroll padding, restored with its content.</summary>
            public float? VanillaBottomPad;

            /// <summary>The reset confirmation window is open for the shown mod page.</summary>
            public bool ResetPending;
        }

        private static readonly Dictionary<nint, Menu> menus = [];

        /// <summary>True while we call the game's row builders, so our shifted cursor isn't captured.</summary>
        private static bool addingOwnRows;

        internal static void ApplyHooks()
        {
            Localization.AddString(ModsRowKey, "Mods");

            On.MioGame.On_Ui_settings.init_page.Suffix += Init_page_Suffix;
            On.MioGame.On_Ui_settings__init_page___lambda_5.OperatorInvoke.Prefix += Add_choice_Prefix;
            On.MioGame.On_Ui_settings.pre_sim_update.Prefix += Pre_sim_update_Prefix;
            On.MioGame.On_Ui_settings.pre_sim_update.Suffix += Pre_sim_update_Suffix;
            On.MioGame.On_Ui_settings.open_confirm_window.Hook += Open_confirm_window_Hook;
            ApplyTabHooks();
            On.MioGame.On_Ui_settings.apply_new_keyboard_mouse_binding.Hook += Apply_new_keyboard_mouse_binding_Hook;
            On.MioGame.On_Ui_settings.apply_new_button_binding.Hook += Apply_new_button_binding_Hook;
        }

        // ============================================================ build

        /// <summary>Keeps the row cursor in step with the game's own rows.</summary>
        private static void Add_choice_Prefix(Ui_settings__init_page___lambda_5* closure, MioGame.String* name, Array_String* values, MioGame.String* description)
        {
            if (addingOwnRows) return;
            if (closure->base_pos != null) basePos = *closure->base_pos;
            if (closure->increment != null) increment = *closure->increment;
        }

        private static void Init_page_Suffix(Ui_settings* self, Ui_settings.Page* page, Ui_settings.Cat c)
        {
            try
            {
                addingOwnRows = true;
                if (c == Ui_settings.Cat.Game) BuildModsRow(self, page);
                else if (c == Ui_settings.Cat.Controller_binding) BuildScreens(self, page);
            }
            catch (Exception ex)
            {
                ModMenu.Log($"building Settings rows failed: {ex}");
            }
            finally
            {
                addingOwnRows = false;
            }
        }

        /// <summary>Game is the first page built, so this is also where a Settings instance starts.</summary>
        private static void BuildModsRow(Ui_settings* self, Ui_settings.Page* page)
        {
            var menu = new Menu { Pages = ModMenu.VisiblePages() };
            menus[(nint)self] = menu;
            if (menu.Pages.Length == 0) return;

            // Appended below the Game page's own rows, so no shift.
            Widget* row = AddGoto(page, ModsRowKey, HostPage, screenStart: 0);
            menu.ModsRow = row->node;

            LinkRows(Rows(page->widget_list), page->widget_list.count);
            FixDividers(Rows(page->widget_list), page->widget_list.count);
        }

        private static void BuildScreens(Ui_settings* self, Ui_settings.Page* host)
        {
            if (!menus.TryGetValue((nint)self, out Menu? menu) || menu.Pages.Length == 0) return;

            Array_String* seed = FindYesNoValues(self);
            if (seed == null)
            {
                ModMenu.Log("no vanilla Yes/No row to build from; Mods menu disabled");
                return;
            }

            uint vanillaCount = host->widget_list.count;

            // Mod list: one goto row per mod.
            uint listStart = host->widget_list.count;
            foreach (ModMenuPage page in menu.Pages)
            {
                Widget* row = AddGoto(host, page.TitleKey, OpenModTarget, listStart);
                row->description = *StringAllocator.GetMioString(page.DescriptionKey);
            }
            var modList = new View(listStart, host->widget_list.count - listStart);

            // One screen per mod. Pushes can move the buffer, so each row is configured right away.
            var modPages = new View[menu.Pages.Length];
            var pageRows = new ModMenuOption[menu.Pages.Length][];
            var pageRowDescriptions = new string?[menu.Pages.Length][];
            for (int p = 0; p < menu.Pages.Length; p++)
            {
                uint start = host->widget_list.count;
                var rowOptions = new List<ModMenuOption>();
                var rowDescriptions = new List<string?>();
                foreach (ModMenuOption option in menu.Pages[p].Options)
                {
                    switch (option.RowKind)
                    {
                        case ModMenuRowKind.Button:
                            {
                                // Same trick as the mod list: a goto whose jump is caught (see Navigate).
                                Widget* button = AddGoto(host, option.TitleKey, OpenModTarget, start);
                                button->description = *StringAllocator.GetMioString(option.DescriptionKey);
                                rowOptions.Add(option);
                                rowDescriptions.Add(option.Description);
                                break;
                            }
                        case ModMenuRowKind.Keybind:
                            {
                                // The game's own binding rows, both pointing at the option's binding.
                                var keybind = (KeybindOption)option;
                                Widget* keys = AddKeyBinding(host, keybind.TitleKey, keybind.Binding, start);
                                keys->description = *StringAllocator.GetMioString(keybind.KeyboardDescriptionKey);
                                Widget* pad = AddButtonBinding(host, keybind.ControllerTitleKey, keybind.Binding, start);
                                pad->description = *StringAllocator.GetMioString(option.DescriptionKey);
                                keybinds[(nint)keybind.Binding] = keybind;
                                rowOptions.Add(option);
                                rowOptions.Add(option);
                                rowDescriptions.Add(keybind.KeyboardDescription);
                                rowDescriptions.Add(option.Description);
                                break;
                            }
                        default:
                            {
                                Widget* row = AddChoice(host, seed, option.TitleKey, option.ValueKeys, option.DescriptionKey, start);
                                if (option.IsToggle) row->p_bool = (bool*)option.Cell;
                                else row->p_choice = option.Cell;
                                row->current_choice = option.Index;
                                rowOptions.Add(option);
                                rowDescriptions.Add(option.Description);
                                break;
                            }
                    }
                }
                modPages[p] = new View(start, host->widget_list.count - start);
                pageRows[p] = rowOptions.ToArray();
                pageRowDescriptions[p] = rowDescriptions.ToArray();
            }

            menu.VanillaCount = vanillaCount;
            menu.TotalCount = host->widget_list.count;
            menu.ModList = modList;
            menu.ModPages = modPages;
            menu.PageRows = pageRows;
            menu.PageRowDescriptions = pageRowDescriptions;

            Widget* all = Rows(host->widget_list);
            foreach (View view in modPages.Prepend(modList))
            {
                LinkRows(all + view.Start, view.Count);
                FixDividers(all + view.Start, view.Count);
            }
            SetVisible(all + vanillaCount, menu.TotalCount - vanillaCount, false);

            host->widget_list.count = vanillaCount;   // vanilla sees only its own rows
            menu.Built = true;
        }

        /// <summary>
        /// Values of a vanilla Yes/No row on an already-built page: engine-allocated and alive as long
        /// as Settings, so a safe array for add_choice to copy.
        /// </summary>
        private static Array_String* FindYesNoValues(Ui_settings* self)
        {
            var pages = (Ui_settings.Page*)&self->pages;
            for (int p = 0; p < HostPage; p++)
            {
                Widget* rows = Rows(pages[p].widget_list);
                for (uint i = 0; i < pages[p].widget_list.count; i++)
                {
                    if (rows[i].kind == Widget.Kind.Choice && rows[i].p_bool != null && rows[i].choices.count == 2)
                    {
                        return &rows[i].choices;
                    }
                }
            }
            return null;
        }

        /// <summary>
        /// The game's add_choice, with the values replaced afterwards: it copies <paramref name="seed"/>
        /// into the new row, then that engine-owned copy is resized (growing a non-empty engine array
        /// is safe) and pointed at our loca ids.
        /// </summary>
        private static Widget* AddChoice(Ui_settings.Page* page, Array_String* seed, string nameKey, string[] valueKeys, string descriptionKey, uint screenStart)
        {
            Ui_settings.Page* pagePtr = page;
            Vec_float_3 screenBase = ScreenBase(screenStart), step = increment;
            var add = new Ui_settings__init_page___lambda_5 { page = &pagePtr, base_pos = &screenBase, increment = &step };

            Widget* row = add.OperatorInvoke(StringAllocator.GetMioString(nameKey), seed, StringAllocator.GetMioString(descriptionKey));

            if (row->choices.count != (uint)valueKeys.Length) row->choices.resize((uint)valueKeys.Length);
            var values = (MioGame.String*)row->choices.data.data;
            for (int i = 0; i < valueKeys.Length; i++) values[i] = *StringAllocator.GetMioString(valueKeys[i]);
            return row;
        }

        /// <summary>The game's keyboard/mouse binding row (built by init_page's &lt;lambda_4&gt;).</summary>
        private static Widget* AddKeyBinding(Ui_settings.Page* page, string nameKey, Input_state.Binding* binding, uint screenStart)
        {
            Ui_settings.Page* pagePtr = page;
            Vec_float_3 screenBase = ScreenBase(screenStart), step = increment;
            var add = new Ui_settings__init_page___lambda_4 { page = &pagePtr, base_pos = &screenBase, increment = &step };
            return add.OperatorInvoke(StringAllocator.GetMioString(nameKey), binding);
        }

        /// <summary>The game's controller binding row (built by init_page's &lt;lambda_3&gt;, as on Remap gamepad).</summary>
        private static Widget* AddButtonBinding(Ui_settings.Page* page, string nameKey, Input_state.Binding* binding, uint screenStart)
        {
            Ui_settings.Page* pagePtr = page;
            Vec_float_3 screenBase = ScreenBase(screenStart), step = increment;
            var add = new Ui_settings__init_page___lambda_3 { page = &pagePtr, base_pos = &screenBase, increment = &step };
            return add.OperatorInvoke(StringAllocator.GetMioString(nameKey), binding);
        }

        private static Widget* AddGoto(Ui_settings.Page* page, string nameKey, int targetPage, uint screenStart)
        {
            Ui_settings.Page* pagePtr = page;
            Vec_float_3 screenBase = ScreenBase(screenStart), step = increment;
            var add = new Ui_settings__init_page___lambda_2 { page = &pagePtr, base_pos = &screenBase, increment = &step };
            return add.OperatorInvoke(StringAllocator.GetMioString(nameKey), targetPage);
        }

        /// <summary>
        /// The game places row n at base_pos + n * increment. Moving the base up by the rows already
        /// in the buffer puts a screen's first row where a page's first row goes.
        /// </summary>
        private static Vec_float_3 ScreenBase(uint rowsBefore) => new()
        {
            x = basePos.x - increment.x * rowsBefore,
            y = basePos.y - increment.y * rowsBefore,
            z = basePos.z - increment.z * rowsBefore,
        };

        // ============================================================ navigation

        private static void Pre_sim_update_Prefix(Ui_settings* self, Node2* n, float dt)
        {
            // Settings builds its tabs on its first update; watch that to learn how to make ours.
            if (!self->initialized) BeginTabCapture();

            // A goto row (mod-list entry or button) moves the hover as it's pressed, so note which one
            // it was beforehand.
            if (menus.TryGetValue((nint)self, out Menu? menu) && menu.Screen != Screen.Closed)
            {
                menu.HoveredBeforeUpdate = HoveredIndex(Host(self)->widget_list);
            }
        }

        private static void Pre_sim_update_Suffix(Ui_settings* self, Node2* n, float dt)
        {
            try
            {
                ModMenu.PollValues();
                EndTabCapture(self);

                if (menus.TryGetValue((nint)self, out Menu? menu) && menu.Built)
                {
                    FinishReset(self, menu);
                    Navigate(self, menu, dt);
                    if (menu.Screen == Screen.ModPage) SyncChangedRows(self, menu);
                }

                // Values are written once the row being edited closes, like the game's own settings.
                if (self->widget_opened < 0) ModMenuConfig.SaveIfDirty();
            }
            catch (Exception ex)
            {
                ModMenu.Log($"Settings update failed: {ex}");
            }
        }

        private static void Navigate(Ui_settings* self, Menu menu, float dt)
        {
            int previous = menu.LastPageOpened;
            int now = self->page_opened;
            menu.LastPageOpened = now;

            if (menu.Tab.n != null && now < 0 && !self->exit)
            {
                UpdateTabLevel(self, menu, dt);
                return;
            }

            switch (menu.Screen)
            {
                case Screen.Closed:
                    if (previous == GamePage && now == HostPage)
                    {
                        Show(self, menu, Screen.ModList, page: -1, hover: 0);
                    }
                    else if (now == HostPage && previous != HostPage && menu.Lingering != null)
                    {
                        // Entered slot 6 the game's own way (Remap gamepad): hand back its content
                        // before it's drawn.
                        RestoreVanilla(self, menu);
                    }
                    break;

                case Screen.ModList:
                    if (self->exit || now < 0)
                    {
                        Close(self, menu);
                    }
                    else if (now == ControllerPage)
                    {
                        if (menu.Tab.n != null)
                        {
                            // Back to the tab bar, on the Mods tab, still previewing the list.
                            GoTo(self, menu, -1);
                            ReturnToTab(self, menu);
                        }
                        else
                        {
                            Close(self, menu);
                            GoTo(self, menu, GamePage);
                            SetHover(menu.ModsRow);
                        }
                    }
                    else if (now == OpenModTarget && previous == HostPage)
                    {
                        int index = menu.HoveredBeforeUpdate;
                        if (index >= 0 && index < menu.Pages.Length)
                        {
                            Show(self, menu, Screen.ModPage, page: index, hover: 0);
                            GoTo(self, menu, HostPage);
                        }
                        else
                        {
                            Close(self, menu);
                        }
                    }
                    else if (now != HostPage)
                    {
                        Close(self, menu);
                    }
                    break;

                case Screen.ModPage:
                    if (self->exit || now < 0)
                    {
                        Close(self, menu);
                    }
                    else if (now == ControllerPage)
                    {
                        Show(self, menu, Screen.ModList, page: -1, hover: menu.OpenPage);
                        GoTo(self, menu, HostPage);
                    }
                    else if (now == OpenModTarget && previous == HostPage)
                    {
                        // A button was selected: stay on this page and run it.
                        GoTo(self, menu, HostPage);
                        int index = menu.HoveredBeforeUpdate;
                        ModMenuOption[] options = menu.PageRows[menu.OpenPage];
                        Refocus(self, menu, Math.Max(index, 0));
                        if (index >= 0 && index < options.Length && options[index] is ButtonOption button)
                        {
                            button.Press();
                        }
                    }
                    else if (now != HostPage)
                    {
                        Close(self, menu);
                    }
                    break;
            }
        }

        private static void GoTo(Ui_settings* self, Menu menu, int page)
        {
            self->page_opened = page;
            menu.LastPageOpened = page;
        }

        /// <summary>Puts one of our screens into slot 6.</summary>
        private static void Show(Ui_settings* self, Menu menu, Screen screen, int page, int hover)
        {
            Ui_settings.Page* host = Host(self);

            if (menu.Screen == Screen.Closed && menu.Lingering is View lingering)
            {
                // Slot 6's vanilla content is still hidden from last time; just drop the old screen.
                SetVisible(Rows(menu.VanillaHeader) + lingering.Start, lingering.Count, false);
                menu.Lingering = null;
            }
            else if (menu.Screen == Screen.Closed)
            {
                // First screen: hide what the game put in slot 6 and keep its real header.
                menu.HiddenVanilla.Clear();
                Widget* vanilla = Rows(host->widget_list);
                for (uint i = 0; i < host->widget_list.count; i++) HideIfShown(vanilla[i].node.n, menu);
                HideIfShown(host->controller.root.n, menu);
                HideIfShown(host->controller_xb.root.n, menu);
                HideIfShown(host->controller_ps4.root.n, menu);
                HideIfShown(host->controller_ps5.root.n, menu);
                menu.VanillaHeader = host->widget_list;
            }
            else
            {
                SetVisible(Rows(host->widget_list), host->widget_list.count, false);
            }

            View view = screen == Screen.ModList ? menu.ModList : menu.ModPages[page];
            host->widget_list = Header(menu, view);
            Widget* rows = Rows(host->widget_list);

            SetVisible(rows, view.Count, true);
            FixDividers(rows, view.Count);
            if (screen == Screen.ModPage) SyncRows(rows, menu.PageRows[page]);

            FitAboveDescription(host, menu, screen, page);

            // The game focuses whichever row is flagged hovered; clear every candidate first.
            ClearHover(Rows(menu.VanillaHeader), menu.TotalCount);
            for (int p = 0; p < HostPage; p++) ClearHover(Rows(Pages(self)[p].widget_list), Pages(self)[p].widget_list.count);
            if (view.Count > 0)
            {
                hover = Math.Clamp(hover, 0, (int)view.Count - 1);
                SetHover(rows[hover].node);

                // The game only refreshes the description panel when the hover moves; ours moved by
                // flag, so the panel would keep its placeholder text. Set it for the focused row.
                string? description = screen == Screen.ModList
                    ? menu.Pages[hover].Description
                    : menu.PageRowDescriptions[page][hover];
                SetDescription(host, description);
            }

            menu.Screen = screen;
            menu.OpenPage = page;
        }

        /// <summary>Puts the hover back on a row of the open screen after a caught goto.</summary>
        private static void Refocus(Ui_settings* self, Menu menu, int index)
        {
            Ui_settings.Page* host = Host(self);
            Widget* rows = Rows(host->widget_list);
            uint count = host->widget_list.count;
            if (count == 0) return;
            index = Math.Clamp(index, 0, (int)count - 1);

            ClearHover(Rows(menu.VanillaHeader), menu.TotalCount);
            for (int p = 0; p < HostPage; p++) ClearHover(Rows(Pages(self)[p].widget_list), Pages(self)[p].widget_list.count);
            SetHover(rows[index].node);

            if (menu.Screen == Screen.ModPage) SetDescription(host, menu.PageRowDescriptions[menu.OpenPage][index]);
        }

        /// <summary>
        /// Gives slot 6's row list back to the game. What's on screen is left alone: slot 6 fades out
        /// over the next frames, and showing its vanilla content now would flash the gamepad page.
        /// The content is handed back by <see cref="RestoreVanilla"/> when slot 6 is next entered.
        /// </summary>
        private static void Close(Ui_settings* self, Menu menu)
        {
            if (menu.Screen == Screen.Closed) return;
            Ui_settings.Page* host = Host(self);

            menu.Lingering = new View((uint)(Rows(host->widget_list) - Rows(menu.VanillaHeader)), host->widget_list.count);
            host->widget_list = menu.VanillaHeader;
            ClearHover(Rows(menu.VanillaHeader) + menu.VanillaCount, menu.TotalCount - menu.VanillaCount);

            menu.Screen = Screen.Closed;
            menu.OpenPage = -1;
            ModMenuConfig.SaveIfDirty();
        }

        /// <summary>Puts slot 6's own content back on display and takes ours down.</summary>
        private static void RestoreVanilla(Ui_settings* self, Menu menu)
        {
            if (menu.Lingering is View lingering)
            {
                SetVisible(Rows(menu.VanillaHeader) + lingering.Start, lingering.Count, false);
                menu.Lingering = null;
            }

            foreach (nint node in menu.HiddenVanilla)
            {
                MioGame.GlobalFunctions.game.common.toggle_node((Node2*)node, true);
            }
            menu.HiddenVanilla.Clear();

            Ui_scroll_rect* scroll = Host(self)->scroll.d;
            if (menu.VanillaBottomPad is float pad && scroll != null) scroll->pad.w = pad;
            menu.VanillaBottomPad = null;
        }

        /// <summary>A header for one screen: a range of slot 6's buffer. Never pushed to.</summary>
        private static WidgetArray Header(Menu menu, View view)
        {
            WidgetArray header = default;
            header.count = view.Count;
            header.capacity = view.Count;
            header.data.data = (byte*)(Rows(menu.VanillaHeader) + view.Start);
            return header;
        }

        // ============================================================ values

        /// <summary>Shows each option's current value on its row.</summary>
        private static void SyncRows(Widget* rows, ModMenuOption[] rowOptions)
        {
            for (int i = 0; i < rowOptions.Length; i++)
            {
                ModMenuOption option = rowOptions[i];
                if (option.RowKind != ModMenuRowKind.Choice) continue;
                rows[i].current_choice = option.Index;
                option.RowNeedsSync = false;
            }
        }

        /// <summary>Redraws rows whose value was changed from code while the page is open.</summary>
        private static void SyncChangedRows(Ui_settings* self, Menu menu)
        {
            ModMenuOption[] rowOptions = menu.PageRows[menu.OpenPage];
            Widget* rows = Rows(Host(self)->widget_list);
            for (int i = 0; i < rowOptions.Length; i++)
            {
                ModMenuOption option = rowOptions[i];
                if (option.RowKind != ModMenuRowKind.Choice || !option.RowNeedsSync) continue;
                rows[i].current_choice = option.Index;
                option.RowNeedsSync = false;
            }
        }

        // ============================================================ row helpers

        private static Ui_settings.Page* Pages(Ui_settings* self) => (Ui_settings.Page*)&self->pages;
        private static Ui_settings.Page* Host(Ui_settings* self) => &Pages(self)[HostPage];
        private static Widget* Rows(WidgetArray list) => (Widget*)list.data.data;

        /// <summary>
        /// Vertical navigation follows explicit on_up / on_down links, which the game sets once its
        /// rows are built (the last row wraps to the first). Rows added afterwards need linking.
        /// </summary>
        private static void LinkRows(Widget* rows, uint count)
        {
            if (count == 0) return;
            for (uint i = 0; i < count; i++)
            {
                Ui_interactive_node* node = rows[i].node.d;
                if (node == null) continue;
                node->on_up = rows[(i + count - 1) % count].node;
                node->on_down = rows[(i + 1) % count].node;
            }
        }

        /// <summary>
        /// Each row is a node with children [highlight, "value", "name", "line"]; the game hides the
        /// last row's "line" (the divider). Rows added afterwards need the same.
        /// </summary>
        private const uint LineNameHash = 0xC481A465;

        private static void FixDividers(Widget* rows, uint count)
        {
            for (uint i = 0; i < count; i++)
            {
                Node2* line = FindChild(rows[i].node.n, LineNameHash);
                if (line != null) MioGame.GlobalFunctions.game.common.toggle_node(line, i != count - 1);
            }
        }

        private static Node2* FindChild(Node2* node, uint nameHash)
        {
            if (node == null) return null;
            var children = (Node2**)node->children.data.data;
            for (uint i = 0; i < node->children.count; i++)
            {
                if (children[i] != null && children[i]->name.hash == nameHash) return children[i];
            }
            return null;
        }

        private static void SetVisible(Widget* rows, uint count, bool visible)
        {
            for (uint i = 0; i < count; i++)
            {
                if (rows[i].node.n != null) MioGame.GlobalFunctions.game.common.toggle_node(rows[i].node.n, visible);
            }
        }

        private static void HideIfShown(Node2* node, Menu menu)
        {
            if (node == null) return;
            // Already hidden by the game (e.g. art for a controller that isn't connected): leave it.
            if ((node->flags & (Node2.Flags.Disabled | Node2.Flags.Disabled_recursive)) != 0) return;
            MioGame.GlobalFunctions.game.common.toggle_node(node, false);
            menu.HiddenVanilla.Add((nint)node);
        }

        // ============================================================ layout

        // Every Settings page has a scroll area 825 tall, centred on the panel, with 50 of padding
        // top and bottom; the description is anchored to the panel's bottom at y = -375 and grows
        // upward, wrapping at 900 wide. Vanilla pages never have enough rows to reach it. Ours can,
        // so each screen raises the scroll area's bottom padding to clear its tallest description.

        /// <summary>Line height of the description text, in panel units, per unit of font size.</summary>
        private const float LineHeightPerFontSize = 1.4f;

        /// <summary>Clear space between the last visible row and the description.</summary>
        private const float DescriptionGap = 30f;

        /// <summary>Average glyph width per unit of font size, for estimating where text wraps.</summary>
        private const float CharWidthPerFontSize = 0.5f;

        private static void FitAboveDescription(Ui_settings.Page* host, Menu menu, Screen screen, int page)
        {
            Ui_scroll_rect* scroll = host->scroll.d;
            Ui_text_node* text = host->description_node.d;
            if (scroll == null || host->scroll.n == null) return;

            menu.VanillaBottomPad ??= scroll->pad.w;
            float vanillaPad = menu.VanillaBottomPad.Value;

            IEnumerable<string?> descriptions = screen == Screen.ModList
                ? menu.Pages.Select(p => p.Description)
                : menu.PageRowDescriptions[page];

            float fontSize = text != null && text->font_size > 0 ? text->font_size : 30f;
            float maxWidth = text != null && text->max_width > 0 ? text->max_width : 900f;
            int lines = descriptions.Max(d => EstimateLines(d, fontSize, maxWidth));
            if (lines == 0 || host->description_node.n == null)
            {
                scroll->pad.w = vanillaPad;
                return;
            }

            // Both nodes share a parent; the scroll area is centred on its node.
            float scrollBottom = host->scroll.n->_transform.translation.y - scroll->dim.y / 2;
            float descriptionBottom = host->description_node.n->_transform.translation.y;
            float descriptionTop = descriptionBottom + lines * fontSize * LineHeightPerFontSize;

            scroll->pad.w = Math.Max(vanillaPad, descriptionTop + DescriptionGap - scrollBottom);
        }

        /// <summary>Lines <paramref name="text"/> wraps to, by word, at an average glyph width.</summary>
        private static int EstimateLines(string? text, float fontSize, float maxWidth)
        {
            if (string.IsNullOrWhiteSpace(text)) return 0;

            int perLine = Math.Max(1, (int)(maxWidth / (fontSize * CharWidthPerFontSize)));
            int lines = 0;
            foreach (string paragraph in text.Split('\n'))
            {
                lines++;
                int used = 0;
                foreach (string word in paragraph.Split(' ', StringSplitOptions.RemoveEmptyEntries))
                {
                    int needed = used == 0 ? word.Length : used + 1 + word.Length;
                    if (needed > perLine && used > 0)
                    {
                        lines++;
                        used = word.Length;
                    }
                    else
                    {
                        used = needed;
                    }
                }
            }
            return lines;
        }

        // ============================================================ keybinds

        /// <summary>Every keybind row built so far, by the native binding it points at.</summary>
        private static readonly Dictionary<nint, KeybindOption> keybinds = [];

        // When a binding row is rebound, the game resolves conflicts with its own controls:
        //  - apply_new_keyboard_mouse_binding clears that key or mouse button from every row on the
        //    Keyboard page (slot 3), i.e. it would unbind the player's own controls;
        //  - apply_new_button_binding swaps buttons with the other controller rows in slot 6's current
        //    list, which on our screens is the other hotkeys on the same mod page.
        // Mod hotkeys are allowed to share keys with the game. So for our rows the game's function
        // still writes the new binding, but every other binding it can reach is put back afterwards.

        private static void Apply_new_keyboard_mouse_binding_Hook(On.MioGame.On_Ui_settings.orig_apply_new_keyboard_mouse_binding orig,
            Ui_settings* self, Widget* widget, Input_state.Binding newBinding)
        {
            if (!TryGetKeybind(widget, out KeybindOption? keybind))
            {
                orig(self, widget, newBinding);
                return;
            }

            using var guard = new BindingGuard(self, widget);
            orig(self, widget, newBinding);
            keybind.ApplyKeyboardMouse(newBinding.key_primary, newBinding.mouse_btn);
        }

        private static void Apply_new_button_binding_Hook(On.MioGame.On_Ui_settings.orig_apply_new_button_binding orig,
            Ui_settings* self, Widget* widget, Input_state.Binding newBinding)
        {
            if (!TryGetKeybind(widget, out KeybindOption? keybind))
            {
                orig(self, widget, newBinding);
                return;
            }

            using var guard = new BindingGuard(self, widget);
            orig(self, widget, newBinding);
            keybind.ApplyButton(newBinding.gamepad_button);
        }

        private static bool TryGetKeybind(Widget* widget, [System.Diagnostics.CodeAnalysis.NotNullWhen(true)] out KeybindOption? keybind)
        {
            keybind = null;
            return widget != null && widget->p_key_binding != null && keybinds.TryGetValue((nint)widget->p_key_binding, out keybind);
        }

        /// <summary>
        /// Snapshots every binding the game's conflict resolution can reach (the Keyboard page's rows
        /// and slot 6's current rows), except the one being rebound, and restores them on dispose.
        /// </summary>
        private readonly struct BindingGuard : IDisposable
        {
            private readonly List<(nint binding, Input_state.Binding value)> saved;

            public BindingGuard(Ui_settings* self, Widget* rebound)
            {
                saved = [];
                Save(Pages(self)[(int)Ui_settings.Cat.Keyboard].widget_list, rebound->p_key_binding);
                Save(Host(self)->widget_list, rebound->p_key_binding);
            }

            private void Save(WidgetArray list, Input_state.Binding* skip)
            {
                Widget* rows = Rows(list);
                for (uint i = 0; i < list.count; i++)
                {
                    Input_state.Binding* binding = rows[i].p_key_binding;
                    if (binding == null || binding == skip) continue;
                    saved.Add(((nint)binding, *binding));
                }
            }

            public void Dispose()
            {
                foreach ((nint binding, Input_state.Binding value) in saved)
                {
                    *(Input_state.Binding*)binding = value;
                }
            }
        }

        // ============================================================ reset to default

        // How the game does it (Ui_settings::pre_sim_update): pressing Reset sets wait_reset and calls
        // open_confirm_window. When the window closes it sets result / result_triggered, and that same
        // update, if wait_reset is still set, resets the settings of whatever page_opened is. On our
        // screens that's slot 6, which means the Controller page's settings and every gamepad binding.
        //
        // So on a mod page the window opens as normal (same text, sounds and Yes/No), but wait_reset is
        // cleared straight away so the game never resets anything, and the answer is read here once
        // the window has closed. On the mod list there is nothing to reset, so no window opens.

        private static void Open_confirm_window_Hook(On.MioGame.On_Ui_settings.orig_open_confirm_window orig, Ui_settings* self)
        {
            try
            {
                // At tab level (our list only previewed) this is the game's own "Reset All": leave it be.
                if (menus.TryGetValue((nint)self, out Menu? menu) && menu.Screen != Screen.Closed && self->page_opened >= 0)
                {
                    if (menu.Screen == Screen.ModPage)
                    {
                        orig(self);
                        menu.ResetPending = true;
                    }
                    self->wait_reset = false;
                    return;
                }
            }
            catch (Exception ex)
            {
                ModMenu.Log($"reset failed: {ex}");
                self->wait_reset = false;   // never let the game reset slot 6 while our screen is up
                return;
            }
            orig(self);
        }

        /// <summary>Acts on the reset window's answer once it has closed.</summary>
        private static void FinishReset(Ui_settings* self, Menu menu)
        {
            if (!menu.ResetPending || self->confirm_window.window_open) return;
            menu.ResetPending = false;

            if (self->confirm_window.result && menu.Screen == Screen.ModPage)
            {
                ModMenuPage page = menu.Pages[menu.OpenPage];
                page.ResetToDefaults();
                ModMenu.Log($"reset {page.ModId} to defaults");
            }
        }

        private static void SetDescription(Ui_settings.Page* page, string? text)
        {
            if (page->description_node.n == null) return;
            MioGame.GlobalFunctions.game.ui.set_text(page->description_node.n, *StringAllocator.GetMioString(text ?? ""));
        }

        private const Ui_interactive_node.Rt_flags HoverFlags =
            Ui_interactive_node.Rt_flags.Hovered | Ui_interactive_node.Rt_flags.Hovered_last_update | Ui_interactive_node.Rt_flags.Hover_trigger;

        private static void ClearHover(Widget* rows, uint count)
        {
            for (uint i = 0; i < count; i++)
            {
                Ui_interactive_node* node = rows[i].node.d;
                if (node != null) node->rt_flags &= ~HoverFlags;
            }
        }

        private static void SetHover(TNode_Row row)
        {
            if (row.n == null) return;
            Ui_interactive_node* node = row.d;
            if (node != null) node->rt_flags |= Ui_interactive_node.Rt_flags.Hovered;
        }

        private static int HoveredIndex(WidgetArray list)
        {
            Widget* rows = Rows(list);
            for (uint i = 0; i < list.count; i++)
            {
                Ui_interactive_node* node = rows[i].node.d;
                if (node != null && (node->rt_flags & Ui_interactive_node.Rt_flags.Hovered) != 0) return (int)i;
            }
            return -1;
        }
    }
}