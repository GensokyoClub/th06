#pragma once
#include "decomp.hpp"
#include "pbg3/FileAbstraction.hpp"
#include "pbg3/IPbg3Writer.hpp"

namespace th06
{
// File-backed bit writer, the counterpart of Pbg3Reader. Decomped from Kioh Gyoku.
class Pbg3Writer : public IPbg3Writer, public FileAbstraction
{
  public:
    BOOL OpenArchive(const char *path);
    virtual void Close();
    virtual BOOL WriteBit(BOOL bit);
    virtual BOOL WriteBits(u32 value, u32 numBits);
    virtual i32 WriteByte(i32 value);
    virtual BOOL SeekToOffset(DWORD fileOffset);
    virtual BOOL SeekToNextByte();
    virtual BOOL Write(void *data, DWORD length);
    Pbg3Writer();
    virtual ~Pbg3Writer();
};
ZUN_ASSERT_SIZE(Pbg3Writer, 0x24);
} // namespace th06
