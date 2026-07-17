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
        static X64Detour? fixedUpdateDetour;
        private static unsafe void Hooks()
        {
            delegate* unmanaged[Cdecl]<MioGame.Game*, void> fixedUpdateHook = &FixedUpdateHook;
            fixedUpdateDetour = new X64Detour((ulong)MioGame.Game.Pointers.fixed_update, (ulong)fixedUpdateHook);
            fixedUpdateDetour.Hook();
        }
        [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
        private static unsafe void FixedUpdateHook(MioGame.Game* game)
        {
            var mio = game->mio;
            if (mio.node != null && !mio.cutscene.active && !mio.walk_bot.active && mio.hook.state._value == MioGame.Mio.Hook.State.Inactive)
            {
                mio.move_by_slide(new MioGame.Vec_float_3() { Base = new MioGame._vec_storage_float_3() { x = 0.1f } });
            }
            ulong trampAddr = fixedUpdateDetour!.TrampolineAddress;
            var originalFunc = (delegate* unmanaged[Cdecl]<MioGame.Game*, void>)trampAddr;
            originalFunc(game);
        }
    }
}
