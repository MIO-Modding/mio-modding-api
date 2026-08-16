using MioGame;
using MioGame.Shader;
using MioGame.std;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace MioModdingApi
{
    public static class GinPatching
    {
        public static unsafe void ApplyHooks()
        {
            On.MioGame.On_Gin_read.read_section_data_1.Hook += Read_section_data_1_Hook;
        }
        public static Dictionary<string, Dictionary<int, GinPatch>> patches = new Dictionary<string, Dictionary<int, GinPatch>>();
        public static unsafe void AddGinPatch(string file, string patch)
        {

            IntPtr patchStr = Marshal.StringToHGlobalAnsi(patch);
            var patchList = Array_unsigned_int.from_data((byte*)patchStr, (uint)patch.Length);
            MioGame.String ginStr = MioGame.String.from_unicode_2(&patchList);

            IntPtr origStr = Marshal.StringToHGlobalAnsi(file);
            var origList = Array_unsigned_int.from_data((byte*)origStr, (uint)file.Length);
            MioGame.String origGinStr = MioGame.String.from_unicode_2(&origList);

            Gin_read ginRead = Gin_read.from_file(&ginStr);
            Gin_read origGinRead = Gin_read.from_file(&origGinStr);
            ginRead.read_header(true);
            origGinRead.read_header(true);

            Dictionary<int, GinPatch> lPatches = patches.GetValueOrDefault(Util.MioStringToString(origGinRead.path), new Dictionary<int, GinPatch>());
            for (uint i = 0; i < ginRead.header.section_count; i++)
            {
                var data = ginRead.sections[i];
                var nameStr = data->name_string();

                int sectionIndex = origGinRead.find_section_1(&nameStr);
                GinPatch ginPatch = new GinPatch(&ginRead, i, data->size);
                lPatches.TryAdd(sectionIndex, ginPatch);
            }
            if (!patches.TryAdd(Util.MioStringToString(origGinRead.path), lPatches))
            {
                patches[Util.MioStringToString(origGinRead.path)] = lPatches;
            }
        }
        public static void PatchAllGins()
        {

        }

        public static bool PatchedGins;
        private unsafe static void Read_section_data_1_Hook(On.MioGame.On_Gin_read.orig_read_section_data_1 orig, MioGame.Gin_read* __this, uint section_index, byte* mem, uint size)
        {
            if (!PatchedGins)
            {
                PatchAllGins();
                PatchedGins = true;
            }
            var pathStr = Util.MioStringToString(__this->path);
            if (patches.ContainsKey(pathStr) && patches[pathStr].ContainsKey((int)section_index))
            {
                if (__this->batcher.status == Gin_read_batcher.Status.Read_batching)
                {
                    var batcher = __this->batcher;
                    uint next_subsection = batcher.next_subsection;

                    bool is_bit_set = (((Ordered_gin_read*)batcher.ordered_reads.data.data[batcher.next_idx * sizeof(Ordered_gin_read)])->flags & Ordered_gin_read.Section_flags.Serialized) != 0;
                    uint next;
                    if (!is_bit_set || next_subsection > 1)
                    {
                        batcher.next_idx = batcher.next_idx + 1;
                        next = 0;
                    } else
                    {
                        next = next_subsection + 1;
                    }
                    batcher.next_subsection = next;
                    __this->batcher = batcher;
                }
                GinPatch patch = patches[pathStr][(int)section_index];
                orig(patch.targetRead, patch.targetIndex, mem, patch.size);
            } else
            {
                orig(__this, section_index, mem, size);
            }
        }
        public unsafe class GinPatch
        {
            public Gin_read* targetRead;
            public uint targetIndex;
            public uint size;
            public GinPatch(Gin_read* targetRead, uint targetIndex, uint size)
            {
                this.targetRead = targetRead;
                this.targetIndex = targetIndex;
                this.size = size;
            }
        }
    }
}
