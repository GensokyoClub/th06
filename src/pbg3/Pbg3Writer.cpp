#include "pbg3/Pbg3Writer.hpp"

namespace th06
{
Pbg3Writer::Pbg3Writer() : IPbg3Writer(), FileAbstraction()
{
}

Pbg3Writer::~Pbg3Writer()
{
    this->Close();
}

BOOL Pbg3Writer::OpenArchive(const char *path)
{
    this->Close();
    this->Reset();
    return FileAbstraction::Open(path, "w") != FALSE;
}

void Pbg3Writer::Close()
{
    if (this->handle != NULL)
    {
        this->SeekToNextByte();
        FileAbstraction::Close();
    }
    this->Reset();
}

BOOL Pbg3Writer::WriteBit(BOOL bit)
{
    if (this->handle == NULL)
    {
        return FALSE;
    }
    if (bit != FALSE)
    {
        this->curByte |= this->bitMask;
    }
    this->bitMask >>= 1;
    if (this->bitMask == 0)
    {
        if (FileAbstraction::WriteByte(this->curByte) != this->curByte)
        {
            return FALSE;
        }
        ++this->offset;
        this->fileSize = max(this->fileSize, this->GetPosition());
        this->checksum += this->curByte;
        this->curByte = 0;
        this->bitMask = 0x80;
    }
    return TRUE;
}

BOOL Pbg3Writer::WriteBits(u32 value, u32 numBits)
{
    u32 mask = 1u << (numBits - 1);
    if (this->handle == NULL)
    {
        return FALSE;
    }
    while (mask != 0)
    {
        if (value & mask)
        {
            this->curByte |= this->bitMask;
        }
        this->bitMask >>= 1;
        if (this->bitMask == 0)
        {
            if (FileAbstraction::WriteByte(this->curByte) != this->curByte)
            {
                return FALSE;
            }
            ++this->offset;
            this->fileSize = max(this->fileSize, this->GetPosition());
            this->checksum += this->curByte;
            this->curByte = 0;
            this->bitMask = 0x80;
        }
        mask >>= 1;
    }
    return TRUE;
}

i32 Pbg3Writer::WriteByte(i32 value)
{
    if (FileAbstraction::WriteByte(value) == PBG_EOF)
    {
        return PBG_EOF;
    }
    ++this->offset;
    this->fileSize = max(this->fileSize, this->GetPosition());
    return value;
}

BOOL Pbg3Writer::SeekToOffset(DWORD fileOffset)
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
    this->checksum = 0;
    this->offset = fileOffset;
    this->fileSize = max(this->fileSize, this->GetPosition());
    return TRUE;
}

BOOL Pbg3Writer::SeekToNextByte()
{
    if (this->handle == NULL)
    {
        return FALSE;
    }
    while (this->bitMask != 0x80)
    {
        this->WriteBit(0);
    }
    return TRUE;
}

BOOL Pbg3Writer::Write(void *data, DWORD length)
{
    DWORD written;
    if (this->SeekToNextByte() == FALSE)
    {
        return FALSE;
    }
    return FileAbstraction::Write(data, length, &written);
}

} // namespace th06
