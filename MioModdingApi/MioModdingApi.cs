using MioModLoader;
using System.Runtime.InteropServices;
namespace MioModdingApi
{
    public class MioModdingApi : Mod
    {
        /// <summary>The loaded api mod, for its config folder and logging.</summary>
        internal static MioModdingApi? Instance { get; private set; }

        public override unsafe void Initialize()
        {
            Instance = this;
            NativeMod.NativeModule.MemoryAddress = (ulong)ModLoader.MioMemoryAddress;
            LogMessage("Loaded Mio Modding Api");
            PatchChecksum();
            GinPatching.ApplyHooks();
            Trinkets.ApplyHooks();
            Localization.ApplyHooks();
            ModMenu.ApplyHooks();
            ApiOptions.ApplyHooks();
            ApiOptions.Register(this);

            // Loader and mod log lines into the game's log file (the loader handles the window).
            GameLogFile.ApplyHooks();
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