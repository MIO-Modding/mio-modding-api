using MioGame;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json.Nodes;
using System.Threading.Tasks;
using static On.MioGame.On_GA_spare_part;

namespace MioModdingApi
{
    public static class Localization
    {
        public static Dictionary<string, Dictionary<string, nint>> strings = new();
        public static unsafe void LoadLanguageFile(string path)
        {
            JsonObject obj = (JsonObject.Parse(System.IO.File.ReadAllText(path)) as JsonObject)!;
            foreach (var i in obj)
            {
                Dictionary<string, nint> strs = new Dictionary<string, nint>();
                foreach (var j in (i.Value as JsonObject)!)
                {
                    var str = (MioGame.String*)Marshal.AllocHGlobal(Marshal.SizeOf(typeof(MioGame.String)));
                    str[0] = Util.StringToMioString(((string?)j.Value)!);
                    strs.Add(j.Key, (nint)str);
                }
                if (strings.ContainsKey(i.Key))
                {
                    foreach (var j in strs)
                    {
                        strings[i.Key].TryAdd(j.Key, j.Value);
                    }
                } else
                {
                    strings[i.Key] = strs;
                }
            }
        }
        public static unsafe void ApplyHooks()
        {
            On.MioGame.On_Loca.try_translate.Hook += Try_translate_Hook;
        }

        private static unsafe MioGame.String* Try_translate_Hook(On.MioGame.On_Loca.orig_try_translate orig, Loca* __this, MioGame.String* id)
        {
            var lang = Enum.GetNames(typeof(MioGame.Language))[__this->current_txt_lang];
            var str = Util.MioStringToString(id[0]);
            if (strings.ContainsKey(lang))
            {
                if (strings[lang].ContainsKey(str))
                {
                    return (MioGame.String*)strings[lang][str];
                }
            }
            var english = Enum.GetName(typeof(MioGame.Language), MioGame.Language.EN)!;
            if (strings.ContainsKey(english))
            {
                if (strings[english].ContainsKey(str))
                {
                    return (MioGame.String*)strings[english][str];
                }
            }
            return orig(__this, id);
        }
    }
}
