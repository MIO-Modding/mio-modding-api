using MioGame;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace MioModdingApi
{
    public static class Util
    {
        public static unsafe MioGame.String StringToMioString(string str)
        {
            IntPtr strPtr = Marshal.StringToHGlobalAnsi(str);
            var strList = Array_unsigned_int.from_data((byte*)strPtr, (uint)str.Length);
            MioGame.String mioStr = MioGame.String.from_unicode_2(&strList);
            return mioStr;
        }
        public static unsafe string MioStringToString(MioGame.String mioStr)
        {
            return Marshal.PtrToStringAuto((nint)mioStr.data.data);
        }
    }
}
