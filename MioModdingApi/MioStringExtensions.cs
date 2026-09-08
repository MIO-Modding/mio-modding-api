using String = MioGame.String;

namespace MioModdingApi;

public static class StringExtensions
{
    public static unsafe bool Equals(this String str, String* other) {
        return str == (sbyte*)other;
    }

    public static unsafe bool Equals(this string? str, String* other)
    {
        if (str is null)
        {
            return other == null || other->data.data == null;
        }
        using TempString temp = new(str);
        return temp.MioString == (sbyte*)other;
    }
}
