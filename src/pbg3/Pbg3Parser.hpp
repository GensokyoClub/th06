#pragma once

#include "inttypes.hpp"
#include "pbg3/FileAbstraction.hpp"
#include "pbg3/IPbg3Parser.hpp"

class Pbg3Parser : public IPbg3Parser, public FileAbstraction
{
  public:
    Pbg3Parser();
    i32 OpenArchive(char *path);
    i32 ReadBit();
    u32 ReadInt(u32 numBitsAsPowersOf2);
    i32 ReadByteAssumeAligned();
    i32 SeekToOffset(u32 fileOffset);
    i32 SeekToNextByte();
    i32 ReadByteAlignedData(u8 *data, u32 bytesToRead);

    void Close();
    i32 ReadByte();

    ~Pbg3Parser();
};
