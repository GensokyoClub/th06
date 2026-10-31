#include "pbg3/MemoryPbg3Reader.hpp"
#include <string.h>

namespace th06
{
MemoryPbg3Reader::MemoryPbg3Reader() : IPbg3Reader()
{
    this->data = NULL;
}

MemoryPbg3Reader::~MemoryPbg3Reader()
{
    this->Detach();
}

BOOL MemoryPbg3Reader::Attach(void *buffer, DWORD length, const FILETIME *time)
{
    this->Detach();
    if (buffer == NULL)
    {
        return FALSE;
    }
    if (length == 0)
    {
        return FALSE;
    }
    this->fileSize = length;
    this->data = (u8 *)buffer;
    if (time != NULL)
    {
        this->lastWriteTime = *time;
    }
    else
    {
        SYSTEMTIME now;
        GetSystemTime(&now);
        SystemTimeToFileTime(&now, &this->lastWriteTime);
    }
    return TRUE;
}

void MemoryPbg3Reader::Detach()
{
    this->SeekToNextByte();
    this->data = NULL;
    this->fileSize = 0;
    this->Reset();
}

BOOL MemoryPbg3Reader::ReadBit()
{
    if (this->data == NULL)
    {
        return FALSE;
    }
    if (this->bitMask == 0x80)
    {
        this->curByte = this->ReadByte();
        if (this->curByte == PBG_EOF)
        {
            return FALSE;
        }
        this->checksum += this->curByte;
    }
    i32 result = this->curByte & this->bitMask;
    this->bitMask >>= 1;
    if (this->bitMask == 0)
    {
        this->bitMask = 0x80;
    }
    return result != 0;
}

u32 MemoryPbg3Reader::ReadBits(u32 numBits)
{
    u32 mask = 1u << (numBits - 1);
    u32 result = 0;
    if (this->data == NULL)
    {
        return 0;
    }
    while (mask != 0)
    {
        if (this->bitMask == 0x80)
        {
            this->curByte = this->ReadByte();
            if (this->curByte == PBG_EOF)
            {
                return 0;
            }
            this->checksum += this->curByte;
        }
        if (this->curByte & this->bitMask)
        {
            result |= mask;
        }
        mask >>= 1;
        this->bitMask >>= 1;
        if (this->bitMask == 0)
        {
            this->bitMask = 0x80;
        }
    }
    return result;
}

i32 MemoryPbg3Reader::ReadByte()
{
    if (this->data == NULL)
    {
        return PBG_EOF;
    }
    if (this->offset >= this->fileSize)
    {
        return PBG_EOF;
    }
    return this->data[this->offset++];
}

BOOL MemoryPbg3Reader::SeekToOffset(DWORD fileOffset)
{
    if (fileOffset >= this->fileSize)
    {
        return FALSE;
    }
    if (FALSE == this->SeekToNextByte())
    {
        return FALSE;
    }
    this->offset = fileOffset;
    this->checksum = 0;
    return TRUE;
}

BOOL MemoryPbg3Reader::SeekToNextByte()
{
    if (this->data == NULL)
    {
        return FALSE;
    }
    while (this->bitMask != 0x80)
    {
        this->ReadBit();
    }
    return TRUE;
}

BOOL MemoryPbg3Reader::Read(void *out, DWORD length)
{
    if (this->SeekToNextByte() == FALSE)
    {
        return FALSE;
    }
    if (this->data == NULL)
    {
        return FALSE;
    }
    if (this->offset >= this->fileSize)
    {
        return FALSE;
    }
    u32 copied = this->fileSize - this->offset;
    if (copied >= length)
    {
        copied = length;
    }
    memcpy(out, this->data + this->offset, copied);
    this->offset += copied;
    return length == copied;
}

BOOL MemoryPbg3Reader::GetLastWriteTime(LPFILETIME time)
{
    if (time == NULL)
    {
        return FALSE;
    }
    *time = this->lastWriteTime;
    return TRUE;
}
} // namespace th06
