using MioModLoader;
using System.Reflection;
namespace MioModdingApi
{
    public class MioModdingApi : Mod
    {
        public MioModdingApi(Assembly assembly) : base(assembly)
        {
        }

        public override string[] GetDependencies()
        {
            return [];
        }

        public override string GetId()
        {
            return "io.github.mio-modding.mio-modding-api";
        }

        public override string GetName()
        {
            return "Mio Modding Api";
        }

        public override void Initialize()
        {
            LogMessage("Loaded Mio Modding Api");
        }
        public static void CoolMethod()
        {
            ModLoader.LogMessage("COOL METHOD CALLED");
        }
    }
}
