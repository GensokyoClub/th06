#pragma once

#include "diffbuild.hpp"
#include "inttypes.hpp"
#include <Windows.h>

namespace th06
{

class FileAbstraction
{
  public:
    virtual i32 Open(const char *filename, const char *mode);
    virtual void Close();
    virtual i32 Read(u8 *data, u32 dataLen, u32 *numBytesRead);
    virtual i32 Write(u8 *data, u32 dataLen, u32 *outWritten);
    virtual i32 ReadByte();
    virtual i32 WriteByte(u32 b);
    virtual i32 Seek(u32 amount, u32 seekFrom);
    virtual u32 Tell();
    virtual u32 GetSize();
    virtual u8 *ReadWholeFile(u32 maxSize);

    FileAbstraction();
    ~FileAbstraction();

    BOOL HasNonNullHandle()
    {
        return this->handle != NULL;
    }
    BOOL HasValidHandle()
    {
        return this->handle != INVALID_HANDLE_VALUE;
    }

  protected:
    HANDLE handle;

  private:
    DWORD access;
};
ZUN_ASSERT_SIZE(FileAbstraction, 0xc);
}; // namespace th06
