#pragma once

#include "inttypes.hpp"
#include <cstdio>
#include <filesystem>
#include <system_error>

enum AccessMode
{
    ACCESS_READ,
    ACCESS_WRITE,
    ACCESS_INVALID
};

class IFileAbstraction
{
  public:
    virtual i32 Open(char *filename, char *mode) = 0;
    virtual void Close() = 0;
    virtual i32 Read(u8 *data, u32 dataLen, u32 *numBytesRead) = 0;
    virtual i32 Write(u8 *data, u32 dataLen, u32 *outWritten) = 0;
    virtual i32 ReadByte() = 0;
    virtual i32 WriteByte(u32 b) = 0;
    virtual i32 Seek(u32 amount, u32 seekFrom) = 0;
    virtual u32 Tell() = 0;
    virtual u32 GetSize() = 0;
    virtual u8 *ReadWholeFile(u32 maxSize) = 0;
};

class FileAbstraction : public IFileAbstraction
{
  public:
    FileAbstraction();
    ~FileAbstraction();

    virtual i32 Open(char *filename, char *mode);
    virtual void Close();
    virtual i32 Read(u8 *data, u32 dataLen, u32 *numBytesRead);
    virtual i32 Write(u8 *data, u32 dataLen, u32 *outWritten);
    virtual i32 ReadByte();
    virtual i32 WriteByte(u32 b);
    virtual i32 Seek(u32 amount, u32 seekFrom);
    virtual u32 Tell();
    virtual u32 GetSize();
    virtual u8 *ReadWholeFile(u32 maxSize);

    bool HasNonNullHandle()
    {
        return this->handle != NULL;
    }

  protected:
    std::FILE *handle;
    std::filesystem::path *path;

  private:
    AccessMode access;
};
