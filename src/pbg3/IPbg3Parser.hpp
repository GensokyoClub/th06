#pragma once

#include "inttypes.hpp"

class IPbg3Parser
{
  public:
    IPbg3Parser()
    {
        this->Reset();
    }
    void Reset();
    u32 ReadVarInt();
    u32 ReadMagic();
    u32 ReadString(char *out, u32 maxSize);
    virtual i32 ReadBit() = 0;
    virtual u32 ReadInt(u32 numBitsAsPowersOf2) = 0;
    virtual i32 ReadByteAssumeAligned() = 0;
    virtual i32 SeekToOffset(u32 fileOffset) = 0;
    virtual i32 SeekToNextByte() = 0;
    virtual i32 ReadByteAlignedData(u8 *data, u32 bytesToRead) = 0;
    virtual ~IPbg3Parser()
    {
    }

  protected:
    u32 offsetInFile;
    u32 fileSize;
    u32 curByte;
    u8 bitIdxInCurByte;
    u32 crc;
};
