using MioModLoader;
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
            Localization.ApplyHooks();

            //Apply log hook
            On.MioGame.GlobalFunctions.platform.win32.On_entrypoint.main.Prefix += Main_Prefix;
        }
        private static unsafe void Main_Prefix(int argc, sbyte** argv, sbyte** envp)
        {
            //This CANNOT be applied until the games entrypoint method is called so unfortunately anything before then is Gone
            ModLoader.logOverride = message =>
            {
                using TempString str = new(message + "\n");
                MioGame.GlobalFunctions.core.win32_io.write_console(&str.MioString, false, true);
            };
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool VirtualProtect(IntPtr lpAddress, uint dwSize, uint flNewProtect, out uint lpflOldProtect);
        private const uint PAGE_EXECUTE_READWRITE = 0x40;
        private unsafe void PatchChecksum()
        {
            IntPtr baseAddress = new(ModLoader.MioMemoryAddress);
            IntPtr targetAddress = IntPtr.Add(baseAddress, 0x31678);

            ReadOnlySpan<byte> bytes = [0x39, 0xc0, 0x90, 0x90, 0x90];
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
                var msg = Marshal.GetLastPInvokeErrorMessage();
                LogMessage("Failed to change memory protection for gin patching: - " + msg);
            }
        }
    }
}
