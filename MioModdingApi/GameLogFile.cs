using MioGame;
using MioModLoader;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace MioModdingApi
{
    /// <summary>
    /// Writes the loader's and mods' log lines into the game's own log file, as plain text.
    ///
    /// The loader shows every line in the log window itself; this only provides the file. Lines go
    /// straight to the game's log file handle rather than through win32_io::write_console, which would
    /// also send them to the game's console output (now separated from the window by the loader).
    /// Until the game opens its log file the loader keeps lines, and they're written here once it has.
    ///
    /// Loaders before rich logging have no FileLogSink, so it's looked up by reflection (a direct
    /// reference would fail to compile into a method the old loader can run). On those loaders the
    /// old route is used: the loader's logOverride, sending lines through write_console.
    /// </summary>
    internal static class GameLogFile
    {
        private static readonly object Lock = new();
        private static bool flushedOnce;

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern unsafe bool WriteFile(void* file, byte* buffer, uint bytesToWrite, out uint bytesWritten, nint overlapped);

        private static MethodInfo? flushFileLog;

        internal static unsafe void ApplyHooks()
        {
            PropertyInfo? sink = typeof(ModLoader).GetProperty("FileLogSink", BindingFlags.Public | BindingFlags.Static);
            if (sink == null)
            {
                // Older loader: route mod lines through the game's console writer, which also writes the file.
                On.MioGame.GlobalFunctions.platform.win32.On_entrypoint.main.Prefix += Main_Prefix_Legacy;
                return;
            }

            sink.SetValue(null, (Func<string, bool>)Write);
            flushFileLog = typeof(ModLoader).GetMethod("FlushFileLog", BindingFlags.Public | BindingFlags.Static);

            // Lines logged before the game opened its log file are written on the first update after.
            On.MioGame.On_Metagame.update.Suffix += Metagame_update_Suffix;
        }

        private static unsafe void Metagame_update_Suffix(Metagame* self)
        {
            if (flushedOnce || Globals.log_output_file.fd == null) return;
            flushedOnce = true;
            flushFileLog?.Invoke(null, null);
        }

        private static unsafe void Main_Prefix_Legacy(int argc, sbyte** argv, sbyte** envp)
        {
            // Can't be applied until the game's entrypoint runs; earlier lines aren't in the file.
#pragma warning disable CS0618 // logOverride is obsolete in newer loaders, which never reach this
            ModLoader.logOverride = message =>
            {
                using TempString str = new(message + "\n");
                MioGame.GlobalFunctions.core.win32_io.write_console(&str.MioString, false, true);
            };
#pragma warning restore CS0618
        }

        /// <summary>Writes one line; false while the game's log file isn't open yet.</summary>
        private static unsafe bool Write(string line)
        {
            void* fd = Globals.log_output_file.fd;
            if (fd == null) return false;

            byte[] bytes = Encoding.UTF8.GetBytes(line + "\n");
            lock (Lock)
            {
                fixed (byte* p = bytes)
                {
                    // Large writes are split, as the game does.
                    uint offset = 0;
                    while (offset < bytes.Length)
                    {
                        if (!WriteFile(fd, p + offset, (uint)bytes.Length - offset, out uint written, 0) || written == 0) break;
                        offset += written;
                    }
                }
            }
            return true;
        }
    }
}