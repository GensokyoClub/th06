#include <stddef.h>

#include "pbg3/Pbg3Archive.hpp"
#include "pbg3/MemoryPbg3Reader.hpp"

namespace th06
{
Pbg3Archive::Pbg3Archive()
{
    this->fileTableOffset = 0;
    this->numOfEntries = 0;
    this->entries = NULL;
    this->reader = NULL;
    this->resData = NULL;
}

BOOL Pbg3Archive::ParseHeader()
{
    if (this->reader->ReadInt() != PBG3_MAGIC)
    {
        SAFE_DELETE(this->reader);
        return FALSE;
    }

    this->numOfEntries = this->reader->ReadVarInt();
    this->fileTableOffset = this->reader->ReadVarInt();
    if (this->reader->SeekToOffset(this->fileTableOffset) == FALSE)
    {
        SAFE_DELETE(this->reader);
        return FALSE;
    }

    this->entries = new Pbg3Entry[this->numOfEntries];
    if (this->entries == NULL)
    {
        SAFE_DELETE(this->reader);
        return FALSE;
    }

    for (u32 idx = 0; idx < this->numOfEntries; idx++)
    {
        this->entries[idx].unk2 = this->reader->ReadVarInt();
        this->entries[idx].unk1 = this->reader->ReadVarInt();
        this->entries[idx].checksum = this->reader->ReadVarInt();
        this->entries[idx].dataOffset = this->reader->ReadVarInt();
        this->entries[idx].uncompressedSize = this->reader->ReadVarInt();
        if (this->reader->ReadString(this->entries[idx].filename, sizeof(this->entries[idx].filename)) == FALSE)
        {
            SAFE_DELETE(this->reader);
            SAFE_DELETE_ARRAY(this->entries);
            return FALSE;
        }
    }

    return TRUE;
}

BOOL Pbg3Archive::Release()
{
    this->fileTableOffset = 0;
    this->numOfEntries = 0;
    SAFE_DELETE(this->reader);
    SAFE_DELETE_ARRAY(this->entries);
    free(this->resData);
    return TRUE;
}

i32 Pbg3Archive::FindEntry(const char *path)
{
    for (u32 entryIdx = 0; entryIdx < this->numOfEntries; entryIdx++)
    {
        if (strcmp(path, this->entries[entryIdx].filename) == 0)
        {
            return entryIdx;
        }
    }
    return -1;
}

u32 Pbg3Archive::GetEntrySize(u32 entryIdx)
{
    if (entryIdx >= this->numOfEntries)
    {
        return 0;
    }

    return this->entries[entryIdx].uncompressedSize;
}

// Dead KOG code
u32 Pbg3Archive::GetEntrySize(const char *path)
{
    return this->GetEntrySize(this->FindEntry(path));
}

u8 *Pbg3Archive::ReadEntryRaw(u32 *outSize, u32 *outChecksum, i32 entryIdx)
{
    if (this->reader == NULL)
    {
        return NULL;
    }

    if (entryIdx >= this->numOfEntries)
        return NULL;

    if (outSize == NULL)
        return NULL;

    if (outChecksum == NULL)
        return NULL;

    if (this->reader->SeekToOffset(this->entries[entryIdx].dataOffset) == FALSE)
        return NULL;

    u32 size;
    if (entryIdx == this->numOfEntries - 1)
    {
        size = this->fileTableOffset - this->entries[entryIdx].dataOffset;
    }
    else
    {
        size = this->entries[entryIdx + 1].dataOffset - this->entries[entryIdx].dataOffset;
    }

    u8 *data = (u8 *)malloc(size);
    if (data == NULL)
        return NULL;

    if (this->reader->Read(data, size) == FALSE)
    {
        free(data);
        return NULL;
    }

    *outChecksum = this->entries[entryIdx].checksum;
    *outSize = size;
    return data;
}

Pbg3Archive::~Pbg3Archive()
{
    this->Release();
}

BOOL Pbg3Archive::Load(const char *path)
{
    Pbg3Reader *fileReader;

    if (this->Release() == FALSE)
    {
        return FALSE;
    }

    fileReader = new Pbg3Reader();
    this->reader = fileReader;
    if (this->reader == NULL)
    {
        return FALSE;
    }

    if (fileReader->OpenArchive(path) == FALSE)
    {
        SAFE_DELETE(this->reader);
        return FALSE;
    }

    return this->ParseHeader();
}

// Dead KOG code: opens an archive stored as an RCDATA resource. KOG's
// CGrpLoader::LoadPR calls it as LzDecode::OpenR(NULL, name).
BOOL Pbg3Archive::LoadR(HMODULE module, const char *name)
{
    HRSRC res;
    HGLOBAL mem;
    void *data;
    DWORD size;
    MemoryPbg3Reader *memReader;

    if (this->Release() == FALSE)
    {
        return FALSE;
    }

    res = FindResource(module, name, RT_RCDATA);
    if (res == NULL)
    {
        return FALSE;
    }

    mem = LoadResource(module, res);
    if (mem == NULL)
    {
        return FALSE;
    }

    data = LockResource(mem);
    if (data == NULL)
    {
        FreeResource(data);
        return FALSE;
    }

    size = SizeofResource(module, res);
    this->resData = malloc(size);
    if (this->resData == NULL)
    {
        FreeResource(mem);
        return FALSE;
    }
    memcpy(this->resData, data, size);
    FreeResource(mem);

    memReader = new MemoryPbg3Reader();
    this->reader = memReader;
    if (this->reader == NULL)
    {
        return FALSE;
    }

    if (memReader->Attach(this->resData, size, NULL) == FALSE)
    {
        SAFE_DELETE(this->reader);
        return FALSE;
    }

    return this->ParseHeader();
}

#define LZSS_DICTSIZE 0x2000
#define LZSS_DICTSIZE_MASK 0x1fff
#define LZSS_MIN_MATCH 3

#define DEC_NEXT_BIT()                                                                                                 \
    inBitMask >>= 1;                                                                                                   \
    if (inBitMask == 0)                                                                                                \
    {                                                                                                                  \
        inBitMask = 0x80;                                                                                              \
    }

#define DEC_WRITE_BYTE(data)                                                                                           \
    *outCursor++ = data;                                                                                               \
    dict[dictHead] = data;                                                                                             \
    dictHead = (dictHead + 1) & LZSS_DICTSIZE_MASK;

#define DEC_HANDLE_FETCH_NEW_BYTE()                                                                                    \
    if (inBitMask == 0x80)                                                                                             \
    {                                                                                                                  \
        currByte = *inCursor;                                                                                          \
        if (inCursor - rawData >= inSize)                                                                              \
        {                                                                                                              \
            currByte = 0;                                                                                              \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            inCursor++;                                                                                                \
        }                                                                                                              \
        checksum += currByte;                                                                                          \
    }

#define DEC_READ_FLAG_BIT()                                                                                            \
    DEC_HANDLE_FETCH_NEW_BYTE();                                                                                       \
    opcode = currByte & inBitMask;                                                                                     \
    DEC_NEXT_BIT();

#define DEC_READ_BITS(bitsCount)                                                                                       \
    outBitMask = 0x01u << (bitsCount - 1);                                                                             \
    inBits = 0;                                                                                                        \
    while (outBitMask != 0)                                                                                            \
    {                                                                                                                  \
        DEC_HANDLE_FETCH_NEW_BYTE();                                                                                   \
        if ((currByte & inBitMask) != 0)                                                                               \
        {                                                                                                              \
            inBits |= outBitMask;                                                                                      \
        }                                                                                                              \
        outBitMask >>= 1;                                                                                              \
        DEC_NEXT_BIT();                                                                                                \
    }

u8 *Pbg3Archive::ReadDecompressEntry(u32 entryIdx, const char *filename)
{
    if (entryIdx >= this->numOfEntries)
        return NULL;

    if (this->reader == NULL)
        return NULL;

    u32 size = this->GetEntrySize(entryIdx);
    u8 *out = (u8 *)malloc(size);
    if (out == NULL)
        return NULL;

    u8 *outCursor = out;

    u32 expectedCsum;
    u8 *rawData = this->ReadEntryRaw(&size, &expectedCsum, entryIdx);

    if (rawData == NULL)
    {
        free(out);
        return NULL;
    }

    u8 *inCursor = rawData;
    u8 inBitMask = 0x80;
    u32 checksum = 0;
    i32 inSize = size;
    u32 dictHead = 1;

    u8 dict[LZSS_DICTSIZE];

    i32 i;
    // Memset doesn't produce matching assembly
    for (i = 0; i < LZSS_DICTSIZE; i++)
    {
        dict[i] = 0;
    }

    u32 currByte;
    u32 inBits;
    u32 outBitMask;
    u32 matchOffset;
    i32 matchLength;
    u32 opcode;

    for (;;)
    {
        DEC_READ_FLAG_BIT();

        // Read literal byte from next 8 bits
        if (opcode != 0)
        {
            DEC_READ_BITS(8);
            DEC_WRITE_BYTE(inBits);
        }
        // Copy from dictionary, 13 bit offset, then 4 bit length
        else
        {
            DEC_READ_BITS(13);

            matchOffset = inBits;
            if (matchOffset == 0)
            {
                break;
            }

            DEC_READ_BITS(4);

            matchLength = inBits + 2;
            for (i = 0; i <= matchLength; i++)
            {
                u32 c = dict[(matchOffset + i) & LZSS_DICTSIZE_MASK];
                DEC_WRITE_BYTE(c);
            }
        }
    }

    // Skip past any remaining bits in the data
    while (inBitMask != 0x80)
    {
        DEC_READ_FLAG_BIT();
    }

    free(rawData);

    if (this->entries[entryIdx].checksum != checksum)
    {
        free(out);
        return NULL;
    }

    return out;
}
} // namespace th06
