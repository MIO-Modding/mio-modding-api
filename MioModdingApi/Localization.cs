using MioGame;
using System.Runtime.InteropServices;
using System.Text.Json.Nodes;

namespace MioModdingApi
{
    public static class Localization
    {
        public static readonly Dictionary<string, Dictionary<string, nint>> Strings = new();
        private static readonly string[] LanguageNames = Enum.GetNames(typeof(Language));

        private static readonly string English = Enum.GetName(typeof(Language), Language.EN)!;

        public static unsafe void LoadLanguageFile(string path)
        {
            JsonObject obj = (JsonNode.Parse(System.IO.File.ReadAllText(path)) as JsonObject)!;
            foreach (var i in obj)
            {
                ref var strs = ref CollectionsMarshal.GetValueRefOrAddDefault(Strings, i.Key, out _);
                strs ??= new Dictionary<string, nint>();

                foreach (var j in (i.Value as JsonObject)!)
                {
                    string value = (string)j.Value!;
                    var str = StringAllocator.GetMioString(value);
                    strs[j.Key] = (nint)str;
                }
            }
        }
        public static unsafe void ApplyHooks()
        {
            On.MioGame.On_Loca.try_translate.Hook += Try_translate_Hook;
        }

        private static unsafe MioGame.String* Try_translate_Hook(On.MioGame.On_Loca.orig_try_translate orig, Loca* self, MioGame.String* id)
        {
            var lang = LanguageNames[self->current_txt_lang];
            var str = StringAllocator.FromMioString(id)!;
            if (Strings.TryGetValue(lang, out var strs))
            {
                if (strs.TryGetValue(str, out nint value))
                {
                    return (MioGame.String*)value;
                }
            }
            if (Strings.TryGetValue(English, out strs))
            {
                if (strs.TryGetValue(str, out nint value))
                {
                    return (MioGame.String*)value;
                }
            }
            return orig(self, id);
        }
    }
}
