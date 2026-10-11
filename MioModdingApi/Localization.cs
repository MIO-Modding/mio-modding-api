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

        /// <summary>
        /// A loca id that is never in the game's table. Passed to Loca::translate in place of an
        /// overridden id, so the game falls back to the string we supply.
        /// </summary>
        private const string MissingId = "\u0001MIO_MODDING_API_MISSING_LOCA_ID";

        /// <summary>
        /// translate shows "UNKNOWN ID ..." when the fallback it is given is empty, so empty overrides
        /// are passed as a single space.
        /// </summary>
        private const string Blank = " ";

        private static unsafe MioGame.String* AsFallback(MioGame.String* value) =>
            value->size == 0 ? StringAllocator.GetMioString(Blank) : value;

        public static unsafe void LoadLanguageFile(string path)
        {
            JsonObject obj = (JsonNode.Parse(System.IO.File.ReadAllText(path)) as JsonObject)!;
            foreach (var i in obj)
            {
                foreach (var j in (i.Value as JsonObject)!)
                {
                    AddString(i.Key, j.Key, (string)j.Value!);
                }
            }
        }

        /// <summary>
        /// Adds or replaces the text for a loca id in one language. Works for new ids and for
        /// overriding the game's own text, on every lookup path the game uses.
        /// </summary>
        /// <param name="id">The loca id, e.g. "UI_MENU_SETTINGS_VSYNC" or a new id of your own.</param>
        /// <param name="text">The text to show.</param>
        /// <param name="language">The language this text is for. English is also the fallback for languages with no entry.</param>
        public static void AddString(string id, string text, Language language = Language.EN)
        {
            AddString(Enum.GetName(language) ?? English, id, text);
        }

        private static unsafe void AddString(string languageName, string id, string text)
        {
            ref var strs = ref CollectionsMarshal.GetValueRefOrAddDefault(Strings, languageName, out _);
            strs ??= new Dictionary<string, nint>();
            strs[id] = (nint)StringAllocator.GetMioString(text);
        }

        public static unsafe void ApplyHooks()
        {
            On.MioGame.On_Loca.try_translate.Hook += Try_translate_Hook;

            // Most UI text (Settings among it) is resolved through Loca::translate, which returns the
            // string by value rather than a pointer into the table.
            On.MioGame.On_Loca.translate_1.Hook += Translate_lang_Hook;
            On.MioGame.On_Loca.translate_2.Hook += Translate_Hook;
        }

        private static string LanguageName(uint language) =>
            language < (uint)LanguageNames.Length ? LanguageNames[language] : English;

        /// <summary>
        /// The registered string for <paramref name="id"/> in <paramref name="languageName"/>, falling
        /// back to English. Null when no mod registered this id.
        /// </summary>
        private static unsafe MioGame.String* FindOverride(MioGame.String* id, string languageName)
        {
            if (Strings.Count == 0 || id == null) return null;

            var str = StringAllocator.FromMioString(id);
            if (str == null) return null;

            if (Strings.TryGetValue(languageName, out var strs) && strs.TryGetValue(str, out nint value))
            {
                return (MioGame.String*)value;
            }
            if (Strings.TryGetValue(English, out strs) && strs.TryGetValue(str, out value))
            {
                return (MioGame.String*)value;
            }
            return null;
        }

        private static unsafe MioGame.String* Try_translate_Hook(On.MioGame.On_Loca.orig_try_translate orig, Loca* self, MioGame.String* id)
        {
            MioGame.String* value = FindOverride(id, LanguageName(self->current_txt_lang));
            return value != null ? value : orig(self, id);
        }

        // translate returns its String by value. Writing that return ourselves crashed the caller, so
        // the game builds it: we ask for an id that can't exist, with our text as the fallback.
        private static unsafe void Translate_lang_Hook(On.MioGame.On_Loca.orig_translate_1 orig, Loca* self,
            MioGame.String* retBuffer, MioGame.String* id, Language lang, MioGame.String* _default)
        {
            // Language is a flags enum, so name it by value rather than by position.
            MioGame.String* value = FindOverride(id, Enum.GetName(lang) ?? English);
            if (value != null)
            {
                orig(self, retBuffer, StringAllocator.GetMioString(MissingId), lang, AsFallback(value));
                return;
            }
            orig(self, retBuffer, id, lang, _default);
        }

        private static unsafe void Translate_Hook(On.MioGame.On_Loca.orig_translate_2 orig, Loca* self,
            MioGame.String* retBuffer, MioGame.String* id, MioGame.String* _default)
        {
            MioGame.String* value = FindOverride(id, LanguageName(self->current_txt_lang));
            if (value != null)
            {
                orig(self, retBuffer, StringAllocator.GetMioString(MissingId), AsFallback(value));
                return;
            }
            orig(self, retBuffer, id, _default);
        }
    }
}