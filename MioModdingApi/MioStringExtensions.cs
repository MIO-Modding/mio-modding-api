using System.Runtime.InteropServices;
using String = MioGame.String;

namespace MioModdingApi;

public static class StringExtensions
{
    public static unsafe String ToMioString(this ReadOnlySpan<byte> str, bool isStatic = false)
    {
        fixed (byte* b = str)
        {
            return new String
            {
                data = new MioGame.Ptr
                {
                    data = b
                },
                size = (uint)str.Length,
                is_static = isStatic ? 1u : 0
            };
        }
    }

    public static unsafe string? ToCsString(this String str)
    {
        byte* data = str.data.data;
        if (data != null)
        {
            return Marshal.PtrToStringUTF8((IntPtr)data, (int)str.size);
        }

        return null;
    }

    public static unsafe bool Equals(this String str, String* other)
    {
        if (other == null)
        {
            return false;
        }
        if (str.size != other->size)
        {
            return false;
        }

        for (int i = 0; i < str.size; i++)
        {
            if (str.data.data[i] != other->data.data[i])
            {
                return false;
            }
        }

        return true;
    }

    public static unsafe bool Equals(this string str, String* other)
    {
        using TempString temp = new(str);
        return temp.MioString.Equals(other);
    }
}
