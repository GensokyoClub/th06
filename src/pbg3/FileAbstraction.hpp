#pragma once
#include "decomp.hpp"
#include <Windows.h>

namespace th06
{
#ifndef PBG_EOF
#define PBG_EOF (-1)
#endif
#ifndef PBG3_MAGIC
#define PBG3_MAGIC mmioFOURCC('P', 'B', 'G', '3')
#endif

class FileAbstraction
{
  public:
    virtual BOOL Open(const char *filename, char *mode);
    virtual void Close(void);
    virtual BOOL Read(void *data, DWORD dataLen, DWORD *numBytesRead);
    virtual BOOL Write(void *data, DWORD dataLen, DWORD *outWritten);
    virtual int ReadByte(void);
    virtual int WriteByte(int b);
    virtual BOOL Seek(DWORD amount, DWORD seekFrom);
    virtual DWORD Tell(void);
    virtual DWORD GetSize(void);

    BOOL WriteString(void *buffer);

    virtual LPVOID ReadWholeFile(DWORD maxSize);

    FileAbstraction();
    ~FileAbstraction();

  protected:
    HANDLE handle;

  private:
    DWORD access;
};
ZUN_ASSERT_SIZE(FileAbstraction, 0xc);
} // namespace th06
