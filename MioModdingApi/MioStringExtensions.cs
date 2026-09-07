using String = MioGame.String;

namespace MioModdingApi;

public static class StringExtensions
{
    public static unsafe bool Equals(this String str, String* other)
    {
        if (str.data.data == null)
        {
            return other->data.data == null;
        }
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

    public static unsafe bool Equals(this string? str, String* other)
    {
        if (str is null)
        {
            return other == null || other->data.data == null;
        }
        using TempString temp = new(str);
        return temp.MioString.Equals(other);
    }
}
