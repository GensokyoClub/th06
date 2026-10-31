#pragma once
#include "decomp.hpp"
#include <Windows.h>

namespace th06
{
// Bit writer interface, the counterpart of IPbg3Reader. Reconstructed from Kioh Gyoku.
// TH06 never writes archives, so the linker discards all of this.
class IPbg3Writer
{
  public:
    IPbg3Writer()
    {
        this->Reset();
    }
    void Reset();
    BOOL WriteVarInt(u32 value);
    BOOL WriteString(const char *value);
    BOOL WriteChar(u8 value);
    BOOL WriteInt(u32 value);
    u32 GetChecksum() const
    {
        return this->checksum;
    }
    void ResetChecksum()
    {
        this->checksum = 0;
    }
    virtual BOOL WriteBit(BOOL bit) = 0;
    virtual BOOL WriteBits(u32 value, u32 numBits) = 0;
    virtual i32 WriteByte(i32 value) = 0;
    virtual BOOL SeekToOffset(DWORD fileOffset) = 0;
    virtual BOOL SeekToNextByte() = 0;
    virtual BOOL Write(void *data, DWORD length) = 0;
    virtual ~IPbg3Writer();

  protected:
    u32 offset;
    u32 fileSize;
    u32 curByte;
    u8 bitMask;
    alignment_padding(0x3);
    u32 checksum;
};
ZUN_ASSERT_SIZE(IPbg3Writer, 0x18);
} // namespace th06
