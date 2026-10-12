using MioGame;
using TNode_Row = MioGame.TNode_Ui_interactive_node;
using Widget = MioGame.Ui_settings.Widget;

namespace MioModdingApi
{
    /// <summary>
    /// The "Mods" tab in Settings' left column.
    ///
    /// How the game makes its tabs (Ui_settings::pre_sim_update, first update): for each of pages
    /// 0-5 it instantiates a tab prefab under the tab column, places it one step below the previous
    /// tab, sets its label to the page's loca id, then links the six tabs up/down in a loop. Page 6
    /// (Controller_binding) gets no tab, and every per-frame tab behaviour (hover sound, previewing
    /// the page while its tab is hovered, selecting it) is in a loop that skips page 6.
    ///
    /// So the Mods tab is made the same way, from the game's own tab prefab (captured while Settings
    /// builds its tabs), placed below Assists and linked into the loop; and the per-frame behaviour
    /// is done here. Navigation needs nothing more: a hovered tab follows its up/down links itself
    /// (Ui_interactive_node::ui_update).
    ///
    /// If the tab can't be made, the "Mods" row on the Game page is kept instead.
    /// </summary>
    internal static unsafe partial class ModMenuUi
    {
        private const string ModsTabKey = ModMenu.KeyPrefix + "MODS_TAB";

        /// <summary>Label id the game gives the Game tab; identifies which child of a tab is its label.</summary>
        private const string GameTabTitleId = "UI_MENU_SETTINGS_GAME_TITLE";

        /// <summary>Fade the game uses when showing a page's contents.</summary>
        private const float ContentsFade = 0.3f;

        private static bool capturing;
        private static readonly List<(Prefab_ref prefab, nint world, nint node)> instantiated = [];
        private static nint gameTabLabel;

        private static void ApplyTabHooks()
        {
            Localization.AddString(ModsTabKey, "Mods");
            On.MioGame.On_World.instantiate_tree.Hook += Instantiate_tree_Hook;
            On.MioGame.GlobalFunctions.game.On_ui.set_text_1.Prefix += Set_text_Prefix;
        }

        // ============================================================ capture

        private static void BeginTabCapture()
        {
            capturing = true;
            instantiated.Clear();
            gameTabLabel = 0;
        }

        private static Node2* Instantiate_tree_Hook(On.MioGame.On_World.orig_instantiate_tree orig, World* self, Prefab_ref* path, Node2* parent, Zone* zone)
        {
            Node2* node = orig(self, path, parent, zone);
            if (capturing && path != null) instantiated.Add((*path, (nint)self, (nint)node));
            return node;
        }

        private static void Set_text_Prefix(Node2* node, MioGame.String text)
        {
            if (capturing && gameTabLabel == 0 && StringAllocator.FromMioString(text) == GameTabTitleId)
            {
                gameTabLabel = (nint)node;
            }
        }

        /// <summary>After Settings' first update: make the Mods tab from what was captured.</summary>
        private static void EndTabCapture(Ui_settings* self)
        {
            if (!capturing) return;
            capturing = false;

            try
            {
                if (menus.TryGetValue((nint)self, out Menu? menu) && menu.Built) BuildTab(self, menu);
            }
            catch (Exception ex)
            {
                ModMenu.Log($"creating the Mods tab failed, keeping the Game page row: {ex}");
            }
            finally
            {
                instantiated.Clear();
                gameTabLabel = 0;
            }
        }

        // ============================================================ build

        private static void BuildTab(Ui_settings* self, Menu menu)
        {
            Ui_settings.Page* pages = Pages(self);
            Node2* gameTab = pages[0].cat_choice.n;
            Node2* lastTab = pages[5].cat_choice.n;
            Node2* prevTab = pages[4].cat_choice.n;
            if (gameTab == null || lastTab == null || prevTab == null || gameTab->parent == null)
            {
                ModMenu.Log("Settings tabs not found; keeping the Game page row");
                return;
            }

            // The game's tab prefab: whatever was instantiated as the Game tab.
            int index = instantiated.FindIndex(e => e.node == (nint)gameTab);
            if (index < 0)
            {
                ModMenu.Log("didn't see the Settings tabs being made; keeping the Game page row");
                return;
            }
            Prefab_ref prefab = instantiated[index].prefab;
            var world = (World*)instantiated[index].world;

            Node2* tab = world->instantiate_tree(&prefab, gameTab->parent, null);
            if (tab == null)
            {
                ModMenu.Log("instantiating the Mods tab failed; keeping the Game page row");
                return;
            }

            // Label: the child with the same name as the Game tab's label.
            Node2* label = gameTabLabel != 0 ? FindDescendant(tab, ((Node2*)gameTabLabel)->name.hash, 3) : null;
            if (label != null) MioGame.GlobalFunctions.game.ui.set_text(label, *StringAllocator.GetMioString(ModsTabKey));
            else ModMenu.Log("couldn't find the tab's label node; the Mods tab may show no text");

            // One step below Assists, as the game spaces its tabs.
            Vec_float_3 last = lastTab->_transform.translation, prev = prevTab->_transform.translation;
            SetTranslation(world, tab, new Vec_float_3
            {
                x = last.x + (last.x - prev.x),
                y = last.y + (last.y - prev.y),
                z = last.z + (last.z - prev.z),
            });

            // Into the up/down loop: Assists -> Mods -> Game.
            var tabRow = new TNode_Row { n = tab };
            var lastRow = new TNode_Row { n = lastTab };
            var gameRow = new TNode_Row { n = gameTab };
            if (tabRow.d == null || lastRow.d == null || gameRow.d == null)
            {
                MioGame.GlobalFunctions.game.common.toggle_node(tab, false);
                ModMenu.Log("the new tab isn't an interactive node; keeping the Game page row");
                return;
            }
            lastRow.d->on_down = tabRow;
            tabRow.d->on_up = lastRow;
            tabRow.d->on_down = gameRow;
            gameRow.d->on_up = tabRow;
            tabRow.d->rt_flags &= ~HoverFlags;

            menu.Tab = tabRow;
            RemoveModsRow(self, menu);
        }

        /// <summary>
        /// Moves a node the way the game moves a fresh tab: write the translation, mark it dirty, and
        /// queue it on the world's dirty list if it isn't already there.
        /// </summary>
        private static void SetTranslation(World* world, Node2* node, Vec_float_3 translation)
        {
            node->_transform.translation = translation;
            node->flags |= Node2.Flags.Is_dirty;
            if ((node->flags & Node2.Flags.In_dirty_list) == 0)
            {
                world->dirty_nodes.push(node);
                node->flags |= Node2.Flags.In_dirty_list;
            }
        }

        private static Node2* FindDescendant(Node2* node, uint nameHash, int depth)
        {
            if (node == null || depth < 0) return null;
            var children = (Node2**)node->children.data.data;
            for (uint i = 0; i < node->children.count; i++)
            {
                if (children[i] == null) continue;
                if (children[i]->name.hash == nameHash) return children[i];
                Node2* found = FindDescendant(children[i], nameHash, depth - 1);
                if (found != null) return found;
            }
            return null;
        }

        /// <summary>The tab replaces the Game page's "Mods" row, which is always its last row.</summary>
        private static void RemoveModsRow(Ui_settings* self, Menu menu)
        {
            Ui_settings.Page* game = &Pages(self)[0];
            uint count = game->widget_list.count;
            if (count == 0 || menu.ModsRow.n == null) return;

            Widget* rows = Rows(game->widget_list);
            if (rows[count - 1].node.n != menu.ModsRow.n) return;

            MioGame.GlobalFunctions.game.common.toggle_node(menu.ModsRow.n, false);
            game->widget_list.count = count - 1;
            LinkRows(rows, count - 1);
            FixDividers(rows, count - 1);
            menu.ModsRow = default;
        }

        // ============================================================ per frame

        /// <summary>
        /// At tab level (no page open): the game handles its six tabs and skips page 6, so the Mods
        /// tab's hover sound, preview and selection happen here.
        /// </summary>
        private static void UpdateTabLevel(Ui_settings* self, Menu menu, float dt)
        {
            Ui_interactive_node* tab = menu.Tab.d;
            if (tab == null || self->confirm_window.window_open) return;

            if ((tab->rt_flags & Ui_interactive_node.Rt_flags.Hover_trigger) != 0)
            {
                Globals.audio.game.play_sfx_ui(Sfx_ui.MainMenu_navigate);
            }

            bool hovered = (tab->rt_flags & Ui_interactive_node.Rt_flags.Hovered) != 0;
            if (!hovered)
            {
                if (menu.Previewing)
                {
                    // The game fades slot 6's contents out (page 6 isn't open), with our rows still in.
                    menu.Previewing = false;
                    Close(self, menu);
                }
                return;
            }

            // Preview: the mod list in the panel.
            if (menu.Screen == Screen.Closed) Show(self, menu, Screen.ModList, page: -1, hover: 0);
            Ui_group_node* contents = Host(self)->contents.d;
            if (contents != null)
            {
                Ui_element* element = contents->as_ui_element();
                if (!menu.Previewing) menu.PreviewFade = element->status == Ui_element.Status.Hidden ? 0f : element->fade;
                DrivePreviewFade(element, menu, dt);
            }
            menu.Previewing = true;

            if (ConfirmPressed())
            {
                Globals.input.game.reset(MioGame.Action.UI_confirm, false);
                Globals.audio.game.play_sfx_ui(Sfx_ui.MainMenu_validate);

                menu.Previewing = false;
                GoTo(self, menu, HostPage);
                self->page_choice_first_sfx_passed = false;

                // Focus the first mod, as the game focuses a page's first row when its tab is selected.
                Widget* rows = Rows(Host(self)->widget_list);
                if (Host(self)->widget_list.count > 0)
                {
                    SetHover(rows[0].node);
                    SetDescription(Host(self), menu.Pages[0].Description);
                }
            }
        }

        /// <summary>
        /// Fades the previewed contents in. Every frame the game starts fading slot 6's contents out
        /// (page 6 isn't open) and its UI update then advances that fade, all before this runs; so
        /// rather than toggling (which would only ever advance the fade-out), the fade is driven here,
        /// after the game's update and before drawing. Once the tab loses hover the game takes over
        /// and fades out from wherever this left it.
        /// </summary>
        private static void DrivePreviewFade(Ui_element* element, Menu menu, float dt)
        {
            menu.PreviewFade = Math.Min(1f, menu.PreviewFade + dt / ContentsFade);
            element->fade = menu.PreviewFade;
            element->fade_per_sec = 0f;
            element->status = menu.PreviewFade >= 1f ? Ui_element.Status.Shown : Ui_element.Status.Fadeing_in;
        }

        /// <summary>Back from the mod list: to the tab bar, on the Mods tab.</summary>
        private static void ReturnToTab(Ui_settings* self, Menu menu)
        {
            Ui_settings.Page* pages = Pages(self);
            for (int p = 0; p < HostPage; p++)
            {
                ClearHover(Rows(pages[p].widget_list), pages[p].widget_list.count);
                if (pages[p].cat_choice.n != null) ClearHover(pages[p].cat_choice);
            }
            ClearHover(Rows(Host(self)->widget_list), Host(self)->widget_list.count);
            SetHover(menu.Tab);
            menu.Previewing = true;
        }

        private static void ClearHover(TNode_Row node)
        {
            Ui_interactive_node* d = node.d;
            if (d != null) d->rt_flags &= ~HoverFlags;
        }

        /// <summary>
        /// "Confirm" pressed this frame, read the way the game reads an action (Input_state::Game::
        /// operator[]): any of the action's bound key, mouse button, second key or pad button.
        /// </summary>
        private static bool ConfirmPressed()
        {
            ref Input_state.Game game = ref Globals.input.game;
            Input_state.Binding binding = game.bindings.data[(int)MioGame.Action.UI_confirm];

            if (binding.key_primary != Key.Key_dummy && game.kb.keys.keys.data[(int)binding.key_primary].pressed) return true;
            if (binding.mouse_btn != Mouse_button.Mouse_none && game.mouse.buttons.data[(int)binding.mouse_btn].pressed) return true;
            if (binding.key_secondary != Key.Key_dummy && game.kb.keys.keys.data[(int)binding.key_secondary].pressed) return true;
            if (binding.gamepad_button != Gamepad_button.Btn_none && game.pad.buttons.data[(int)binding.gamepad_button].pressed) return true;
            return false;
        }
    }
}