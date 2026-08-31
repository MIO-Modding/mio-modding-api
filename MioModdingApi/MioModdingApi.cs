using MioGame;
using MioModLoader;
using PolyHook2.API;
using System.Reflection;
using System.Runtime.InteropServices;
namespace MioModdingApi
{
    public class MioModdingApi : Mod
    {
        public override unsafe void Initialize()
        {
            NativeMod.NativeModule.MemoryAddress = (ulong)ModLoader.MioMemoryAddress;
            LogMessage("Loaded Mio Modding Api");
            PatchChecksum();
            GinPatching.ApplyHooks();
            Trinkets.ApplyHooks();

            //Apply log hook
            On.MioGame.GlobalFunctions.platform.win32.On_entrypoint.main.Prefix += Main_Prefix;
        }
        private unsafe void Main_Prefix(int argc, sbyte** argv, sbyte** envp)
        {
            //This CANNOT be applied until the games entrypoint method is called so unfortunately anything before then is Gone
            ModLoader.logOverride = new Action<string>((message) =>
            {
                MioGame.String* str = (MioGame.String*)Marshal.AllocHGlobal(Marshal.SizeOf<MioGame.String>());
                str[0] = Util.StringToMioString(message + "\n");
                MioGame.GlobalFunctions.core.win32_io.write_console(str, false, true);
                Marshal.FreeHGlobal((nint)str);
            });
        }

        [DllImport("kernel32.dll")]
        static extern bool VirtualProtect(IntPtr lpAddress, uint dwSize, uint flNewProtect, out uint lpflOldProtect);
        const uint PAGE_EXECUTE_READWRITE = 0x40;
        private unsafe void PatchChecksum()
        {

            IntPtr baseAddress = new IntPtr(ModLoader.MioMemoryAddress);
            IntPtr targetAddress = IntPtr.Add(baseAddress, 0x31678);

            byte[] bytes = [0x39, 0xc0, 0x90, 0x90, 0x90];
            uint size = (uint)bytes.Length;

            if (VirtualProtect(targetAddress, size, PAGE_EXECUTE_READWRITE, out uint oldProtect))
            {
                byte* ptr = (byte*)targetAddress.ToPointer();
                for (int i = 0; i < bytes.Length; i++)
                {
                    ptr[i] = bytes[i];
                }
                VirtualProtect(targetAddress, size, oldProtect, out _);
            }
            else
            {
                int error = Marshal.GetLastWin32Error();
                LogMessage("Failed to change memory protection for gin patching: " + error);
            }
        }
    }
}
