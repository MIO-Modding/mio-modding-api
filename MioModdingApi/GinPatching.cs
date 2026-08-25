using MioGame;
using MioGame.Shader;
using MioGame.std;
using MioModLoader;
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
        public static event System.Action PatchGins;
        public static unsafe void ApplyHooks()
        {
            On.MioGame.On_Gin_read.read_section_data_1.Hook += Read_section_data_1_Hook;
        }
        public static Dictionary<string, Dictionary<int, GinPatch>> patches = new Dictionary<string, Dictionary<int, GinPatch>>();
        public static unsafe void AddGinPatch(string file, string patch)
        {

            MioGame.String ginStr = Util.StringToMioString(patch);

            MioGame.String origGinStr = Util.StringToMioString(file);

            Gin_read ginRead = Gin_read.from_file(&ginStr);
            Gin_read origGinRead = Gin_read.from_file(&origGinStr);
            ginRead.read_header(true);
            origGinRead.read_header(true);

            Dictionary<int, GinPatch> lPatches = patches.GetValueOrDefault(Util.MioStringToString(origGinRead.path), new Dictionary<int, GinPatch>());
            for (uint i = 0; i < ginRead.header.section_count; i++)
            {
                var data = ((Gin_section_header*)ginRead.sections.data.data)[i];
                uint size = 0;
                for (int j = 0; j < 64; j++)
                {
                    if (data.name[j] == 0x0)
                    {
                        break;
                    }
                    size++;
                }
                var nameStr = new MioGame.String()
                {
                    data = new Ptr() {
                        data = (byte*)&data.name
                    },
                    size = size
                };

                int sectionIndex = origGinRead.find_section(&nameStr);
                GinPatch ginPatch = new GinPatch(&ginRead, i, data.size);
                lPatches.TryAdd(sectionIndex, ginPatch);
            }
            if (!patches.TryAdd(Util.MioStringToString(origGinRead.path), lPatches))
            {
                patches[Util.MioStringToString(origGinRead.path)] = lPatches;
            }
        }
        public static unsafe void PatchAllGins()
        {
            PatchGins.Invoke();
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
                    ModLoader.LogMessage("1");
                    var batcher = __this->batcher;
                    ModLoader.LogMessage("2");
                    uint next_subsection = batcher.next_subsection;
                    ModLoader.LogMessage("3");

                    bool is_bit_set = (((Ordered_gin_read*)batcher.ordered_reads.data.data)[batcher.next_idx].flags & Ordered_gin_read.Section_flags.Serialized) != 0;
                    ModLoader.LogMessage("4");
                    uint next;
                    ModLoader.LogMessage("5");
                    if (!is_bit_set || next_subsection > 1)
                    {
                        batcher.next_idx = batcher.next_idx + 1;
                        next = 0;
                    } else
                    {
                        next = next_subsection + 1;
                    }
                    ModLoader.LogMessage("6");
                    batcher.next_subsection = next;
                    ModLoader.LogMessage("7");
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
