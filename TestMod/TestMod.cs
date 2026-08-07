using MioModLoader;
using PolyHook2.API;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
namespace TestMod
{
    public class TestMod : Mod
    {
        public TestMod(Assembly assembly, string name, string id, string[] dependencies) : base(assembly, name, id, dependencies)
        {
        }

        public override void Initialize()
        {
            Hooks();
        }
        private unsafe void Hooks()
        {
            On.MioGame.On_Game.fixed_update.Prefix += Fixed_update_Prefix;
        }

        private unsafe void Fixed_update_Prefix(ref MioGame.Game self)
        {
            var mio = self.mio;
            if (mio.node != null && !mio.cutscene.active && !mio.walk_bot.active && mio.hook.state._value == MioGame.Mio.Hook.State.Inactive)
            {
                mio.move_by_slide(new MioGame.Vec_float_3() { Base = new MioGame._vec_storage_float_3() { x = 0.1f } });
            }
        }
    }
}
