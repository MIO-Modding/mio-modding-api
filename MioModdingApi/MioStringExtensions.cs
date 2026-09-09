using String = MioGame.String;

namespace MioModdingApi;

public static class StringExtensions
{
    public static unsafe bool Equals(this String str, String* other) {
        return Util.MioStringToString(str) == Util.MioStringToString(other[0]);
    }

    public static unsafe bool Equals(this string? str, String* other)
    {
        if (str is null)
        {
            return other == null || other->data.data == null;
        }
        return str == Util.MioStringToString(other[0]);
    }
}
