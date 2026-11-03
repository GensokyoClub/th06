#include "pbg3/Pbg3Reader.hpp"

namespace th06
{

Pbg3Reader::Pbg3Reader() : IPbg3Reader(), FileAbstraction()
{
}

Pbg3Reader::~Pbg3Reader()
{
    this->Close();
}

BOOL Pbg3Reader::OpenArchive(const char *path)
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

void Pbg3Reader::Close()
{
    FileAbstraction::Close();
    this->Reset();
}

i32 Pbg3Reader::ReadBit()
{
    if (this->handle == NULL)
    {
        return FALSE;
    }

    if (this->bitMask == 0x80)
    {
        this->curByte = FileAbstraction::ReadByte();
        if (this->curByte == PBG_EOF)
        {
            return FALSE;
        }
        this->offset += 1;
        this->checksum += this->curByte;
    }

    i32 res = this->curByte & this->bitMask;
    this->bitMask >>= 1;
    if (this->bitMask == 0)
    {
        this->bitMask = 0x80;
    }
    return res != 0;
}

u32 Pbg3Reader::ReadBits(u32 numBits)
{
    u32 remainingBits = 1u << (numBits - 1);
    u32 result = 0;

    if (this->handle == NULL)
    {
        return 0;
    }

    while (remainingBits != 0)
    {
        if (this->bitMask == 0x80)
        {
            this->curByte = FileAbstraction::ReadByte();
            if (this->curByte == PBG_EOF)
            {
                return 0;
            }
            this->offset += 1;
            this->checksum += this->curByte;
        }
        u32 bitIdx = this->bitMask;
        if ((bitIdx & this->curByte) != 0)
        {
            result |= remainingBits;
        }
        remainingBits >>= 1;
        this->bitMask >>= 1;
        if (this->bitMask == 0)
        {
            this->bitMask = 0x80;
        }
    }

    return result;
}

i32 Pbg3Reader::ReadByte()
{
    if (this->offset < this->fileSize)
    {
        this->offset += 1;
    }

    return FileAbstraction::ReadByte();
}

BOOL Pbg3Reader::SeekToOffset(DWORD fileOffset)
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

    this->offset = fileOffset;
    this->checksum = 0;
    return TRUE;
}

BOOL Pbg3Reader::SeekToNextByte()
{
    if (this->handle == NULL)
    {
        return FALSE;
    }

    while (this->bitMask != 0x80)
    {
        this->ReadBit();
    }
    return TRUE;
}

BOOL Pbg3Reader::Read(void *data, DWORD bytesToRead)
{
    DWORD numBytesRead;

    this->SeekToNextByte();
    return FileAbstraction::Read(data, bytesToRead, &numBytesRead);
}

BOOL Pbg3Reader::GetLastWriteTime(LPFILETIME lastWriteTime)
{
    if (this->handle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    return GetFileTime(this->handle, NULL, NULL, lastWriteTime);
}
} // namespace th06
