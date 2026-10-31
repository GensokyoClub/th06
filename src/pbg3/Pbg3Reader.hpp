#pragma once
#include "decomp.hpp"
#include "pbg3/FileAbstraction.hpp"
#include "pbg3/IPbg3Reader.hpp"

namespace th06
{
class Pbg3Reader : public IPbg3Reader, public FileAbstraction
{
  public:
    BOOL OpenArchive(const char *path);
    virtual void Close();
    virtual BOOL ReadBit();
    virtual u32 ReadBits(u32 numBits);
    virtual i32 ReadByte();
    virtual BOOL SeekToOffset(DWORD fileOffset);
    virtual BOOL SeekToNextByte();
    virtual BOOL Read(void *data, DWORD bytesToRead);
    virtual BOOL GetLastWriteTime(LPFILETIME lastWriteTime);

    Pbg3Reader();
    virtual ~Pbg3Reader();
};
ZUN_ASSERT_SIZE(Pbg3Reader, 0x24);
} // namespace th06
