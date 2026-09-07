using MioGame;
using System.Runtime.InteropServices;

namespace MioModdingApi
{
    public static class Trinkets
    {
        public static readonly Dictionary<string, Trinket> trinkets = [];
        public static Trinket RegisterTrinket(string id, int cost, string placeAnchor, Placement placement)
        {
            Trinket trinket = new(id, cost, placeAnchor, placement);
            trinkets.Add(id, trinket);
            return trinket;
        }
        public static unsafe void ApplyHooks()
        {
            On.MioGame.On_Tab_trinkets.add_col.Hook += Add_col_Hook;
            On.MioGame.On_Tab_trinkets.add_row.Hook += Add_row_Hook;
            On.MioGame.GlobalFunctions.game.On_game.trinket_slot_cost.Hook += Trinket_slot_cost_Hook;
        }

        private static unsafe int Trinket_slot_cost_Hook(On.MioGame.GlobalFunctions.game.On_game.orig_trinket_slot_cost orig, MioGame.String* name)
        {
            foreach (var trinket in trinkets.Values)
            {
                if (trinket.ItemId.Equals(name))
                {
                    return trinket.Cost;
                }
            }
            return orig(name);
        }

        private static unsafe void Add_row_Hook(On.MioGame.On_Tab_trinkets.orig_add_row orig, Tab_trinkets* self, Array_TNode_Ui_interactive_node* retBuffer, TNode_Ui_group_node slots, sbyte** names, uint count, Vec_float_2 center)
        {
            AddColOrRowHook(names, count, out sbyte** newNames, out uint length);
            orig(self, retBuffer, slots, newNames, length, center);
            Marshal.FreeHGlobal((nint)newNames);
        }

        private static unsafe void Add_col_Hook(On.MioGame.On_Tab_trinkets.orig_add_col orig, Tab_trinkets* self, Array_TNode_Ui_interactive_node* retBuffer, TNode_Ui_group_node slots, sbyte** names, uint count, Vec_float_2 start)
        {
            AddColOrRowHook(names, count, out sbyte** newNames, out uint length);
            orig(self, retBuffer, slots, newNames, length, start);
            Marshal.FreeHGlobal((nint)newNames);
        }

        private static readonly Dictionary<string, nint> Strs = new();
        private static unsafe void AddColOrRowHook(sbyte** names, uint count, out sbyte** newNames, out uint length)
        {
            List<string> strings = new List<string>();
            for (int i = 0; i < count; i++)
            {
                string str = new(names[i]);
                str += '\0';
                strings.Add(str);
            }
            foreach (var trinket in trinkets.Values)
            {
                var str = strings.FirstOrDefault(j => j.Replace("\0", "") == trinket.PlaceAnchor);
                if (str != null)
                {
                    int index = strings.IndexOf(str) + (trinket.Placement == Placement.AFTER ? 1 : 0);
                    strings.Insert(index, trinket.Id + "\0");
                }
            }
            newNames = (sbyte**)Marshal.AllocHGlobal(sizeof(sbyte*) * strings.Count);
            length = (uint)strings.Count;
            for (int i = 0; i < strings.Count; i++)
            {
                string key = strings[i];
                if (Strs.TryGetValue(key, out nint ptr))
                {
                    newNames[i] = (sbyte*)ptr;
                    continue;
                }

                sbyte* str = (sbyte*)Marshal.AllocHGlobal(key.Length + 1);
                int bytesWritten = System.Text.Encoding.UTF8.GetBytes(key, new Span<byte>(str, key.Length));
                str[bytesWritten] = 0; // Null-terminate the string
                Strs.Add(key, (nint)str);
                newNames[i] = str;
            }
        }
        public enum Placement
        {
            AFTER,
            BEFORE
        }
        public class Trinket
        {
            public readonly string Id;
            public readonly int Cost;
            public readonly string PlaceAnchor;
            public readonly Placement Placement;
            public readonly string ItemId;

            public Trinket(string id, int cost, string placeAnchor, Placement placement)
            {
                Id = id;
                Cost = cost;
                PlaceAnchor = placeAnchor;
                Placement = placement;
                ItemId = "TRINKET:" + id;
            }
        }
    }
}
