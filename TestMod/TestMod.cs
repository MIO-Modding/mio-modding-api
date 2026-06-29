using MioModLoader;
using System.Reflection;
namespace TestMod
{
    public class TestMod : Mod
    {
        public TestMod(Assembly assembly) : base(assembly)
        {
        }

        public override string[] GetDependencies()
        {
            return new string[] { "io.github.mio-modding.mio-modding-api" };
        }

        public override string GetId()
        {
            return "io.github.mio-modding.mio-modding-api.test-mod";
        }

        public override string GetName()
        {
            return "Test Mod";
        }

        public override void Initialize()
        {
            MioModdingApi.MioModdingApi.CoolMethod();
        }
    }
}
