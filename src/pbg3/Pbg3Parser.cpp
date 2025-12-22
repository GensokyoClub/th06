#include "pbg3/Pbg3Parser.hpp"


Pbg3Parser::Pbg3Parser() : IPbg3Parser(), FileAbstraction()
{
}

i32 Pbg3Parser::OpenArchive(char *path)
{
    this->Close();
    this->Reset();
    if (!FileAbstraction::Open(path, "r"))
    {
        return false;
    }
    this->fileSize = GetSize();
    return true;
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
        return false;
    }

    if (this->bitIdxInCurByte == 0x80)
    {
        this->curByte = FileAbstraction::ReadByte();
        if (this->curByte == -1)
        {
            return false;
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
                return false;
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

i32 Pbg3Parser::ReadByteAssumeAligned()
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
        return false;
    }

    if (!this->SeekToNextByte())
    {
        return false;
    }

    if (!FileAbstraction::Seek(fileOffset, SEEK_SET))
    {
        return false;
    }

    this->offsetInFile = fileOffset;
    this->crc = 0;
    return true;
}

i32 Pbg3Parser::SeekToNextByte()
{
    if (!this->HasNonNullHandle())
    {
        return false;
    }

    while (this->bitIdxInCurByte != 0x80)
    {
        this->ReadBit();
    }
    return true;
}

i32 Pbg3Parser::ReadByteAlignedData(u8 *data, u32 bytesToRead)
{
    u32 numBytesRead;

    this->SeekToNextByte();
    return FileAbstraction::Read(data, bytesToRead, &numBytesRead);
}

i32 Pbg3Parser::GetLastWriteTime(std::filesystem::file_time_type &lastWriteTime)
{
    if (!this->HasNonNullHandle())
    {
        return false;
    }

    // EWWWW abstraction violation much? (Maybe this is an inlined function?)
    return FileAbstraction::GetLastWriteTime(lastWriteTime);
}

i32 Pbg3Parser::ReadByte()
{
    // MSVC generates an add -0x18 instruction to get the caller base here, while the original binary uses a sub 0x18?
    return Pbg3Parser::ReadByteAssumeAligned();
}

Pbg3Parser::~Pbg3Parser()
{
    this->Close();
}
