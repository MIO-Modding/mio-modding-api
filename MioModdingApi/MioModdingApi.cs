using MioGame;
using MioModLoader;
using PolyHook2.API;
using System.Reflection;
using System.Runtime.InteropServices;
namespace MioModdingApi
{
    public class MioModdingApi : Mod
    {
        public unsafe override void Initialize()
        {
            NativeMod.NativeModule.MemoryAddress = (ulong)ModLoader.MioMemoryAddress;
            LogMessage("Loaded Mio Modding Api");
            PatchChecksum();
            GinPatching.ApplyHooks();
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
