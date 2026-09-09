using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using System.Text;
using String = MioGame.String;

namespace MioModdingApi;

/// <summary>
/// A utility class for allocating and managing <see cref="String"/> instances in unmanaged memory.
/// These allocated <see cref="String"/> are intended to be reused throughout the lifetime of the ModLoader.
/// </summary>
public static class StringAllocator
{
    /// <summary>
    /// Dictionary that maps C# strings to their corresponding unmanaged <see cref="String"/> pointers.
    /// </summary>
    private static readonly Dictionary<string, nint> Strings = [];

    /// <summary>
    /// Dictionary that maps static/permanent unmanaged <see cref="String"/> data pointers to their corresponding C# strings.
    /// </summary>
    private static readonly Dictionary<nint, string> StringsReverse = [];

    /// <summary>
    /// Blocks of unmanaged memory used for allocating static/permanent mod <see cref="String"/> instances and their <c>byte*</c> data.
    /// </summary>
    private static readonly List<StringBlock> Blocks = [];

    private static readonly object Lock = new();

    /// <summary>
    /// Gets the unmanaged <see cref="String"/> pointer that represents the given C# <see cref="string"/>.
    /// <para />
    /// If a <see cref="String"/> has already been created for the specified C# <see cref="string"/>,
    /// this returns a pointer to the existing <see cref="String"/> value.
    /// Otherwise, this allocates a new unmanaged <see cref="String"/> and returns a pointer to it.
    /// </summary>
    /// <param name="str">The C# <see cref="string"/> to get or create a <see cref="String"/> for.</param>
    /// <returns>A pointer to the allocated unmanaged <see cref="String"/>.</returns>
    /// <exception cref="ArgumentNullException">Thrown if <paramref name="str"/> is <see langword="null"/>.</exception>
    /// <exception cref="ArgumentException">Thrown if <paramref name="str"/> is too large to allocate in a single 1MB block.</exception>
    /// <exception cref="InvalidOperationException">Thrown if the allocation fails for any reason.</exception>
    public static unsafe String* GetMioString(string str)
    {
        ArgumentNullException.ThrowIfNull(str);
        if (Strings.TryGetValue(str, out nint value))
        {
            return (String*)value;
        }

        return CreateString(str);
    }

    /// <inheritdoc cref="FromMioString(String)"/>
    public static unsafe string? FromMioString(String* mioStr) {
        return mioStr == null ? null : FromMioString(*mioStr);
    }

    /// <summary>
    /// Converts an unmanaged <see cref="String"/> to a C# <see cref="string"/>.
    /// If <paramref name="mioStr"/>'s <see cref="MioGame.String.is_static"/> field is not <c>0</c>, this will cache the resulting C# string.
    /// </summary>
    /// <param name="mioStr">The unmanaged <see cref="String"/> to convert.</param>
    /// <returns>
    /// The corresponding C# <see cref="string"/>,
    /// or <see langword="null"/> if <paramref name="mioStr"/>'s data is <see langword="null"/>.
    /// </returns>
    public static unsafe string? FromMioString(String mioStr)
    {
        if (mioStr.data.data == null)
        {
            return null;
        }

        if (mioStr.size == 0)
        {
            return string.Empty;
        }

        nint ptr = (nint)mioStr.data.data;
        if (mioStr.is_static == 0)
        {
            return Marshal.PtrToStringUTF8(ptr, (int)mioStr.size);
        }

        if (StringsReverse.TryGetValue(ptr, out string? value))
        {
            return value;
        }

        string result = Marshal.PtrToStringUTF8(ptr, (int)mioStr.size);
        StringsReverse[ptr] = result;
        return result;
    }

    private static unsafe String* CreateString(string str)
    {
        if (!StringBlock.CanAllocateAny(str))
        {
            throw new ArgumentException("String is too large to allocate in a single 1MB block.", nameof(str));
        }

        lock (Lock)
        {
            String* mioStr;
            // Attempt to allocate on an existing block first
            foreach (StringBlock block in Blocks)
            {
                if (block.TryAllocString(str, out mioStr))
                {
                    Strings[str] = (nint)mioStr;
                    StringsReverse[(nint)mioStr] = str;
                    return mioStr;
                }
            }

            StringBlock newBlock = AllocNewBlock();
            if (newBlock.TryAllocString(str, out mioStr))
            {
                Strings[str] = (nint)mioStr;
                StringsReverse[(nint)mioStr] = str;
                return mioStr;
            }
        }

        throw new InvalidOperationException("Failed to allocate string.");
    }

    private static StringBlock AllocNewBlock()
    {
        StringBlock newBlock = StringBlock.Allocate();
        Blocks.Add(newBlock);
        return newBlock;
    }

    internal static void FreeAllBlocks()
    {
        foreach (var block in Blocks)
        {
            block.Dispose();
        }

        Blocks.Clear();
        Strings.Clear();
        StringsReverse.Clear();
    }

    /// <summary>
    /// A private class that represents a block of unmanaged memory used for allocating <see cref="String"/> instances.
    /// Both the <see cref="String"/> struct and the <c>byte*</c> data are stored in this block.
    /// <para />
    /// This class does not list all the strings allocated in the block.
    /// Instead, these are stored in the <see cref="Strings"/> dictionary in the <see cref="StringAllocator"/> class.
    /// </summary>
    private unsafe class StringBlock : IDisposable
    {
        private StringBlock(void* blockPtr)
        {
            _blockPtr = (byte*)blockPtr;
        }

        internal static StringBlock Allocate()
        {
            void* blockPtr = NativeMemory.AlignedAlloc(BlockSize, Alignment);
            return new StringBlock(blockPtr);
        }

        public void Dispose()
        {
            NativeMemory.AlignedFree(_blockPtr);
        }

        private const int BlockSize = 1024 * 1024; // 1 MB
        private const int Alignment = 16;
        private static readonly int MioStringStructSize = sizeof(String);

        private readonly byte* _blockPtr;
        private int _offset;

        private byte* CurrentPtr => _blockPtr + _offset;

        public static bool CanAllocateAny(string str)
        {
            int size = Encoding.UTF8.GetMaxByteCount(str.Length);
            return MioStringStructSize + size + 1 <= BlockSize;
        }

        private bool CanAllocate(string str)
        {
            int size = Encoding.UTF8.GetMaxByteCount(str.Length);
            int misalignment = _offset % Alignment;
            if (misalignment != 0)
            {
                size += Alignment - misalignment;
            }

            return _offset + MioStringStructSize + size + 1 <= BlockSize;
        }

        private bool CanAllocate<T>() where T : unmanaged
        {
            int size = sizeof(T);
            return _offset + MioStringStructSize + size <= BlockSize;
        }

        public bool TryAllocString(string str, [NotNullWhen(true)] out String* mioStr)
        {
            if (!CanAllocate(str))
            {
                mioStr = null;
                return false; // Not enough space in this block
            }

            AllocString(str, out mioStr);
            return true;
        }

        private void AllocString(string str, out String* mioStr)
        {
            // We first allocate the string, and have the byte* data allocated right next to it in memory.
            mioStr = AllocStruct<String>();

            Encoding utf8 = Encoding.UTF8;
            int maxLen = utf8.GetMaxByteCount(str.Length);
            byte* bufferPtr = CurrentPtr;
            var buffer = new Span<byte>(bufferPtr, maxLen);
            int bytesWritten = utf8.GetBytes(str, buffer);
            _offset += bytesWritten;

            // Null-terminate the string
            *CurrentPtr = 0;
            _offset++;
            Align();

            mioStr->data.data = bufferPtr;
            mioStr->size = (uint)bytesWritten;
            mioStr->is_static = 1;
        }

        private T* AllocStruct<T>() where T : unmanaged
        {
            if (!CanAllocate<T>())
            {
                throw new InvalidOperationException("Not enough space in the block to allocate the struct.");
            }

            var structPtr = (T*)CurrentPtr;
            _offset += sizeof(T);
            Align();
            return structPtr;
        }

        private void Align()
        {
            const int mask = Alignment - 1;
            _offset = (_offset + mask) & ~mask;
        }
    }
}

public readonly unsafe struct TempString : IDisposable
{
    public readonly String MioString;
    public readonly string? String;
    private readonly byte* _buffer;
    private readonly bool _allocated;

    public TempString(string? value)
    {
        if (value is null)
        {
            return;
        }

        String = value;
        _allocated = true;
        int exactByteCount = checked(Encoding.UTF8.GetByteCount(value) + 1); // + 1 for null terminator
        _buffer = (byte*)NativeMemory.Alloc((nuint)exactByteCount);

        int byteCount = Encoding.UTF8.GetBytes(value, new Span<byte>(_buffer, exactByteCount));
        _buffer[byteCount] = 0; // null-terminate

        MioString = new String
        {
            data = new MioGame.Ptr
            {
                data = _buffer
            },
            size = (uint)byteCount,
            is_static = 0
        };
    }

    public void Dispose()
    {
        if (_allocated && _buffer != null)
        {
            NativeMemory.Free(_buffer);
        }
    }
}
