using MioModLoader;
using System.Reflection;
namespace TestMod
{
    public class TestMod : Mod
    {
        public TestMod(Assembly assembly, string name, string id, string[] dependencies) : base(assembly, name, id, dependencies)
        {
        }

        public override void Initialize()
        {
            MioModdingApi.MioModdingApi.CoolMethod();
        }
    }
}
