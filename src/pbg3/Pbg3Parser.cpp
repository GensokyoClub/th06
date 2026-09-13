#include "pbg3/Pbg3Parser.hpp"

namespace th06
{

Pbg3Parser::Pbg3Parser() : IPbg3Parser(), FileAbstraction()
{
}

i32 Pbg3Parser::OpenArchive(char *path)
{
    this->Close();
    this->Reset();
    if (FileAbstraction::Open(path, "r") == FALSE)
    {
        return FALSE;
    }
    this->fileSize = GetFileSize(this->handle, NULL);
    return TRUE;
}

void Pbg3Parser::Close()
{
    FileAbstraction::Close();
    this->Reset();
}

i32 Pbg3Parser::ReadBit()
{
    if (!this->HasNonNullHandle())
    {
        return FALSE;
    }

    if (this->bitIdxInCurByte == 0x80)
    {
        this->curByte = FileAbstraction::ReadByte();
        if (this->curByte == -1)
        {
            return FALSE;
        }
        this->offsetInFile += 1;
        this->crc += this->curByte;
    }

    i32 res = this->curByte & this->bitIdxInCurByte;
    this->bitIdxInCurByte >>= 1;
    if (this->bitIdxInCurByte == 0)
    {
        this->bitIdxInCurByte = 0x80;
    }
    return res != 0;
}

u32 Pbg3Parser::ReadInt(u32 numBitsAsPowersOf2)
{
    u32 remainingBits = 1 << (numBitsAsPowersOf2 - 1);
    u32 result = 0;

    if (!this->HasNonNullHandle())
    {
        return 0;
    }

    while (remainingBits != 0)
    {
        if (this->bitIdxInCurByte == 0x80)
        {
            this->curByte = FileAbstraction::ReadByte();
            if (this->curByte == -1)
            {
                return FALSE;
            }
            this->offsetInFile += 1;
            this->crc += this->curByte;
        }
        u32 bitIdx = this->bitIdxInCurByte;
        if ((bitIdx & this->curByte) != 0)
        {
            result |= remainingBits;
        }
        remainingBits >>= 1;
        this->bitIdxInCurByte >>= 1;
        if (this->bitIdxInCurByte == 0)
        {
            this->bitIdxInCurByte = 0x80;
        }
    }

    return result;
}

i32 Pbg3Parser::ReadByte()
{
    if (this->offsetInFile < this->fileSize)
    {
        this->offsetInFile += 1;
    }

    return FileAbstraction::ReadByte();
}

i32 Pbg3Parser::SeekToOffset(u32 fileOffset)
{
    if (fileOffset >= this->fileSize)
    {
        return FALSE;
    }

    if (this->SeekToNextByte() == FALSE)
    {
        return FALSE;
    }

    if (FileAbstraction::Seek(fileOffset, FILE_BEGIN) == FALSE)
    {
        return FALSE;
    }

    this->offsetInFile = fileOffset;
    this->crc = 0;
    return TRUE;
}

i32 Pbg3Parser::SeekToNextByte()
{
    if (!this->HasNonNullHandle())
    {
        return FALSE;
    }

    while (this->bitIdxInCurByte != 0x80)
    {
        this->ReadBit();
    }
    return TRUE;
}

i32 Pbg3Parser::ReadByteAlignedData(u8 *data, u32 bytesToRead)
{
    u32 numBytesRead;

    this->SeekToNextByte();
    return FileAbstraction::Read(data, bytesToRead, &numBytesRead);
}

i32 Pbg3Parser::GetLastWriteTime(LPFILETIME lastWriteTime)
{
    // Yes, this is comparing against INVALID_HANDLE_VALUE instead of NULL. Why?
    // Unclear.
    if (!this->HasValidHandle())
    {
        return FALSE;
    }

    // EWWWW abstraction violation much? (Maybe this is an inlined function?)
    return FileAbstraction::GetLastWriteTime(lastWriteTime);
}

Pbg3Parser::~Pbg3Parser()
{
    this->Close();
}
}; // namespace th06
