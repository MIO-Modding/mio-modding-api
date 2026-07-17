using MioModLoader;
using PolyHook2.API;
using System.Reflection;
using System.Runtime.InteropServices;
namespace MioModdingApi
{
    public class MioModdingApi : Mod
    {
        public MioModdingApi(Assembly assembly, string name, string id, string[] dependencies) : base(assembly, name, id, dependencies)
        {
        }

        public override void Initialize()
        {
            MioGame.MioGameModule.MioGameMemoryAddress = (ulong)ModLoader.mioMemoryAddress;
            LogMessage("Loaded Mio Modding Api");
        }
    }
}
