using MioGame;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace MioModdingApi
{
    public static class Trinkets
    {
        public static Dictionary<string, Trinket> trinkets = new Dictionary<string, Trinket>();
        public static Trinket RegisterTrinket(string id, int cost, string placeAnchor, Placement placement)
        {
            Trinket trinket = new Trinket(id, cost, placeAnchor, placement);
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
            foreach (var i in trinkets)
            {
                if (Util.MioStringToString(name[0]) == i.Value.GetItemId())
                {
                    return i.Value.cost;
                }
            }
            return orig(name);
        }

        private static unsafe void Add_row_Hook(On.MioGame.On_Tab_trinkets.orig_add_row orig, Tab_trinkets* __this, Array_TNode_Ui_interactive_node* __return, TNode_Ui_group_node slots, sbyte** names, uint count, Vec_float_2 center)
        {
            AddColOrRowHook(names, count, out sbyte** newNames, out uint length);
            orig(__this, __return, slots, newNames, length, center);
            Marshal.FreeHGlobal((nint)newNames);
        }

        private static unsafe void Add_col_Hook(On.MioGame.On_Tab_trinkets.orig_add_col orig, Tab_trinkets* __this, Array_TNode_Ui_interactive_node* __return, TNode_Ui_group_node slots, sbyte** names, uint count, Vec_float_2 start)
        {
            AddColOrRowHook(names, count, out sbyte** newNames, out uint length);
            orig(__this, __return, slots, newNames, length, start);
            Marshal.FreeHGlobal((nint)newNames);
        }

        private static Dictionary<string, nint> strs = new Dictionary<string, nint>();
        private static unsafe void AddColOrRowHook(sbyte** names, uint count, out sbyte** newNames, out uint length)
        {
            string[] strings = new string[count];
            sbyte** chars = names;
            for (int i = 0; i < count; i++)
            {
                string str = new string(chars[i]);
                str += '\0';
                strings[i] = str;
            }
            foreach (var i in trinkets)
            {
                var str = strings.FirstOrDefault(j => j.Replace("\0", "") == i.Value.placeAnchor);
                if (str != null)
                {
                    List<string> strs = strings.ToList();
                    strs.Insert(strs.IndexOf(str) + (i.Value.placement == Placement.AFTER ? 1 : 0), i.Value.id + "\0");
                    strings = strs.ToArray();
                }
            }
            sbyte** backToStr = (sbyte**)Marshal.AllocHGlobal((sizeof(sbyte*) * strings.Length));
            for (int i = 0; i < strings.Length; i++)
            {
                sbyte* str;
                if (strs.ContainsKey(strings[i]))
                {
                    str = (sbyte*)strs[strings[i]];
                }
                else
                {
                    str = (sbyte*)Marshal.AllocHGlobal(Marshal.SizeOf<sbyte>() * (strings[i].Length));
                    for (int k = 0; k < strings[i].Length; k++)
                    {
                        str[k] = (sbyte)strings[i][k];
                    }
                    strs.Add(strings[i], (nint)str);
                }
                backToStr[i] = str;
            }
            newNames = backToStr;
            length = (uint)strings.Length;
        }
        public enum Placement
        {
            AFTER,
            BEFORE
        }
        public class Trinket
        {
            public string id;
            public int cost;
            public string placeAnchor;
            public Placement placement;
            public string GetItemId()
            {
                return "TRINKET:" + id;
            }
            public Trinket(string id, int cost, string placeAnchor, Placement placement)
            {
                this.id = id;
                this.cost = cost;
                this.placeAnchor = placeAnchor;
                this.placement = placement;
            }
        }
    }
}
