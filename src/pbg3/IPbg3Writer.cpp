#include "pbg3/IPbg3Writer.hpp"

#define MAKE_DWORD(a, b, c, d) ((DWORD)(MAKEWORD(c, d) << 16) | (DWORD)MAKEWORD(a, b))
#define MAKE_INV_DWORD(a, b, c, d) MAKE_DWORD(d, c, b, a)

namespace th06
{

IPbg3Writer::~IPbg3Writer()
{
}

void IPbg3Writer::Reset()
{
    this->bitMask = 0x80;
    this->offset = 0;
    this->fileSize = 0;
    this->curByte = 0;
    this->checksum = 0;
}

BOOL IPbg3Writer::WriteVarInt(u32 value)
{
    u32 mask;
    if (value < 0x100)
    {
        this->WriteBits(0, 2);
        mask = 0x80;
    }
    else if (value < 0x10000)
    {
        this->WriteBits(1, 2);
        mask = 0x8000;
    }
    else if (value < 0x1000000)
    {
        this->WriteBits(2, 2);
        mask = 0x800000;
    }
    else
    {
        this->WriteBits(3, 2);
        mask = 0x80000000;
    }
    do
    {
        this->WriteBit(value & mask);
        mask >>= 1;
    } while (mask != 0);
    return TRUE;
}

BOOL IPbg3Writer::WriteChar(u8 value)
{
    return this->WriteBits(value, 8);
}

BOOL IPbg3Writer::WriteInt(u32 value)
{
    u8 *bytes = (u8 *)&value;
    return this->WriteBits(MAKE_INV_DWORD(bytes[0], bytes[1], bytes[2], bytes[3]), 32);
}

BOOL IPbg3Writer::WriteString(const char *value)
{
    if (value == NULL)
    {
        return FALSE;
    }
    while (*value != '\0')
    {
        this->WriteChar(*value);
        value++;
    }
    this->WriteChar(0);
    return TRUE;
}

} // namespace th06
