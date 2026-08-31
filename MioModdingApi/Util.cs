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
            byte[] utf32Bytes = System.Text.Encoding.UTF32.GetBytes(str);
            uint[] uintArray = new uint[utf32Bytes.Length / 4];
            Buffer.BlockCopy(utf32Bytes, 0, uintArray, 0, utf32Bytes.Length);
            fixed (uint* pinnedPtr = uintArray)
            {
                var strList = Array_unsigned_int.from_data((byte*)pinnedPtr, (uint)uintArray.Length);
                MioGame.String mioStr = MioGame.String.from_unicode(&strList);
                return mioStr;
            }
        }
        public static unsafe MioGame.String* StringToMioStringPtr(string str)
        {
            MioGame.String ptr = StringToMioString(str);
            return &ptr;
        }
        public static unsafe string MioStringToString(MioGame.String mioStr)
        {
            return Marshal.PtrToStringUTF8((nint)mioStr.data.data, (int)mioStr.size);
        }
    }
}
