#include <stddef.h>

#include "pbg3/Pbg3Archive.hpp"

namespace th06
{
DIFFABLE_STATIC(Pbg3Archive **, g_Pbg3Archives)

Pbg3Archive::Pbg3Archive()
{
    this->fileTableOffset = 0;
    this->numOfEntries = 0;
    this->entries = NULL;
    this->parser = NULL;
    this->unk = 0;
}

i32 Pbg3Archive::ParseHeader()
{
    if (this->parser->ReadMagic() != 0x33474250)
    {
        if (this->parser != NULL)
        {
            delete this->parser;
            this->parser = NULL;
        }
        return FALSE;
    }

    this->numOfEntries = this->parser->ReadVarInt();
    this->fileTableOffset = this->parser->ReadVarInt();
    if (this->parser->SeekToOffset(this->fileTableOffset) == FALSE)
    {
        if (this->parser != NULL)
        {
            delete this->parser;
            this->parser = NULL;
        }
        return FALSE;
    }

    this->entries = new Pbg3Entry[this->numOfEntries];
    if (this->entries == NULL)
    {
        if (this->parser != NULL)
        {
            delete this->parser;
            this->parser = NULL;
        }
        return FALSE;
    }

    for (u32 idx = 0; idx < this->numOfEntries; idx += 1)
    {
        this->entries[idx].unk2 = this->parser->ReadVarInt();
        this->entries[idx].unk1 = this->parser->ReadVarInt();
        this->entries[idx].checksum = this->parser->ReadVarInt();
        this->entries[idx].dataOffset = this->parser->ReadVarInt();
        this->entries[idx].uncompressedSize = this->parser->ReadVarInt();
        if (this->parser->ReadString(this->entries[idx].filename, sizeof(this->entries[idx].filename)) == FALSE)
        {
            if (this->parser != NULL)
            {
                delete this->parser;
                this->parser = NULL;
            }
            if (this->entries != NULL)
            {
                delete[] this->entries;
                this->entries = NULL;
            }

            return FALSE;
        }
    }

    return TRUE;
}

i32 Pbg3Archive::Release()
{
    this->fileTableOffset = 0;
    this->numOfEntries = 0;
    if (this->parser != NULL)
    {
        delete this->parser;
        this->parser = NULL;
    }
    if (this->entries != NULL)
    {
        delete[] this->entries;
        this->entries = NULL;
    }
    free(this->unk);
    return TRUE;
}

i32 Pbg3Archive::FindEntry(char *path)
{
    for (u32 entryIdx = 0; entryIdx < this->numOfEntries; entryIdx += 1)
    {
        char *entryFilename = this->entries[entryIdx].filename;
        i32 res = strcmp(path, entryFilename);
        if (res == 0)
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

u8 *Pbg3Archive::ReadEntryRaw(u32 *outSize, u32 *outChecksum, i32 entryIdx)
{
    if (this->parser == NULL)
    {
        return NULL;
    }

    if (entryIdx >= this->numOfEntries)
        return NULL;

    if (outSize == NULL)
        return NULL;

    if (outChecksum == NULL)
        return NULL;

    if (this->parser->SeekToOffset(this->entries[entryIdx].dataOffset) == FALSE)
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

    if (this->parser->ReadByteAlignedData(data, size) == FALSE)
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

i32 Pbg3Archive::Load(char *path)
{
    if (this->Release() == FALSE)
    {
        return FALSE;
    }

    this->parser = new Pbg3Parser();
    if (this->parser == NULL)
    {
        return FALSE;
    }

    if (this->parser->OpenArchive(path) == FALSE)
    {
        if (this->parser != NULL)
        {
            delete this->parser;
            this->parser = NULL;
        }
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
        if (inCursor - rawData >= (i32)size)                                                                           \
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
    outBitMask = 0x01 << (bitsCount - 1);                                                                              \
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

u8 *Pbg3Archive::ReadDecompressEntry(u32 entryIdx, char *filename)
{
    if (entryIdx >= this->numOfEntries || this->parser == NULL)
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
    u32 dictHead = 1;

    u8 dict[LZSS_DICTSIZE];

    u32 currByte;
    u32 inBits;
    u32 outBitMask;
    u32 matchOffset;
    u32 opcode;

    // Memset doesn't produce matching assembly
    for (i32 i = 0; i < LZSS_DICTSIZE; i++)
    {
        dict[i] = 0;
    }

    for (;;)
    {
        DEC_READ_FLAG_BIT();

        // Read literal byte from next 8 bits
        if (opcode != 0)
        {
            DEC_READ_BITS(8);
            DEC_WRITE_BYTE(inBits);
            continue;
        }
        // Copy from dictionary, 13 bit offset, then 4 bit length

        DEC_READ_BITS(13);

        matchOffset = inBits;
        if (matchOffset == 0)
        {
            break;
        }

        DEC_READ_BITS(4);

        for (i32 i = 0; i <= (i32)inBits + 2; i++)
        {
            u32 c = dict[(matchOffset + i) & LZSS_DICTSIZE_MASK];
            DEC_WRITE_BYTE(c);
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
}; // namespace th06
