#include "pbg3/IPbg3Parser.hpp"

void IPbg3Parser::Reset()
{
    this->bitIdxInCurByte = 128;
    this->offsetInFile = 0;
    this->fileSize = 0;
    this->curByte = 0;
    this->crc = 0;
}

u32 IPbg3Parser::ReadVarInt()
{
    u32 res = 0;
    i32 varintHdr = 0;

    if (this->ReadBit())
    {
        varintHdr = 2;
    }
    if (this->ReadBit())
    {
        varintHdr |= 1;
    }

    u32 intLen;
    switch (varintHdr)
    {
    case 0:
        intLen = 0x80;
        break;
    case 1:
        intLen = 0x8000;
        break;
    case 2:
        intLen = 0x800000;
        break;
    case 3:
        intLen = 0x80000000;
        break;
    default:
        // TODO: There's probably a way to match without goto, but
        // I can't figure it out... a simple `return 0;` won't share
        // the function epilogue with the other return res.
        goto end;
    }

    do
    {
        if (this->ReadBit())
        {
            res |= intLen;
        }
        intLen >>= 1;
    } while (intLen != 0);
end:
    return res;
}

u32 IPbg3Parser::ReadMagic()
{
    u32 b0 = this->ReadInt(8);
    u32 b1 = b0 + (this->ReadInt(8) << 8);
    u32 b2 = b1 + (this->ReadInt(8) << 16);
    u32 b3 = b2 + (this->ReadInt(8) << 24);

    return b3;
}

u32 IPbg3Parser::ReadString(char *out, u32 maxSize)
{
    if (out == NULL)
        return false;

    for (u32 idx = 0; idx < maxSize; idx++)
    {
        out[idx] = this->ReadInt(8);
        if (out[idx] == '\0')
        {
            return true;
        }
    }

    return false;
}
