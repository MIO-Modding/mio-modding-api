using MioModLoader;
using PdbToCSharp;
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
            Task.Run(async () =>
            {
                await Task.Delay(100);
                Thread thread = new Thread(Loop);
                thread.UnsafeStart();
            });
        }
        private unsafe void Loop()
        {
            while (true)
            {
                Thread.Sleep(1000 / 60);
                if (MioTestClass.game == null) continue;
                var node = MioTestClass.game->mio.node;
                if (node == null) continue;
                node->_transform.translation += new System.Numerics.Vector3(0.5f, 0, 0);
            }
        }
    }
}
