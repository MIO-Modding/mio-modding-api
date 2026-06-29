using MioModLoader;
using System.Reflection;
namespace MioModdingApi
{
    public class MioModdingApi : Mod
    {
        public MioModdingApi(Assembly assembly, string name, string id, string[] dependencies) : base(assembly, name, id, dependencies)
        {
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
