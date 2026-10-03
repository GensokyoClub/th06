#include "pbg3/FileAbstraction.hpp"

namespace th06
{
FileAbstraction::FileAbstraction()
{
    handle = INVALID_HANDLE_VALUE;
    access = 0;
}

// DUMMY FUNCTIONS FOR IAT
struct MappedFileView
{
    HANDLE file;
    HANDLE mapping;
    void *data;
    DWORD size;
    BOOL writable;
};
BOOL CloseMappedFileView(MappedFileView *view)
{
    FlushViewOfFile(view->data, 0);
    UnmapViewOfFile(view->data);
    CloseHandle(view->mapping);
    CloseHandle(view->file);
    return TRUE;
}
BOOL OpenMappedFileView(const WCHAR *filename, BOOL writable, MappedFileView *view)
{
    GetFileAttributesW(filename);
    CreateFileW(filename, 0, 0, NULL, 0, 0, NULL);
    GetFileSize(view->file, NULL);
    CreateFileMappingA(view->file, NULL, 0, 0, 0, NULL);
    MapViewOfFile(view->mapping, 0, 0, 0, 0);
    return TRUE;
}
BOOL Exists_Dummy(const char *filename)
{
    GetFileAttributesA(filename);
    return TRUE;
}
// END DUMMY FUNCTIONS

BOOL FileAbstraction::Open(const char *filename, const char *mode)
{
    u32 creationDisposition;
    BOOL isAppendMode = FALSE;

    this->Close();

    const char *curMode;
    for (curMode = mode; *curMode != '\0'; curMode++)
    {
        if (*curMode == 'r')
        {
            this->access = GENERIC_READ;
            creationDisposition = OPEN_EXISTING;
            break;
        }
        else if (*curMode == 'w')
        {
            DeleteFile(filename);
            this->access = GENERIC_WRITE;
            creationDisposition = OPEN_ALWAYS;
            break;
        }
        else if (*curMode == 'a')
        {
            isAppendMode = TRUE;
            this->access = GENERIC_WRITE;
            creationDisposition = OPEN_ALWAYS;
            break;
        }
    }

    if (*curMode == '\0')
    {
        return FALSE;
    }
    this->handle = CreateFile(filename, this->access, FILE_SHARE_READ, NULL, creationDisposition,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (this->handle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (isAppendMode)
    {
        SetFilePointer(this->handle, 0, NULL, FILE_END);
    }
    return TRUE;
}

void FileAbstraction::Close()
{
    if (this->handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(this->handle);
        this->handle = INVALID_HANDLE_VALUE;
        this->access = 0;
    }
}

BOOL FileAbstraction::Read(void *data, u32 dataLen, DWORD *numBytesRead)
{
    if (this->access != GENERIC_READ)
    {
        return FALSE;
    }

    return ReadFile(this->handle, data, dataLen, numBytesRead, NULL);
}

BOOL FileAbstraction::Write(void *data, u32 dataLen, DWORD *outWritten)
{
    if (this->access != GENERIC_WRITE)
    {
        return FALSE;
    }

    return WriteFile(this->handle, data, dataLen, outWritten, NULL);
}

i32 FileAbstraction::ReadByte()
{
    u8 data;
    DWORD outBytesRead;

    if (this->Read(&data, 1, &outBytesRead) == FALSE)
    {
        return PBG_EOF;
    }
    if (outBytesRead == 0)
    {
        return PBG_EOF;
    }
    else
    {
        return data;
    }
}

i32 FileAbstraction::WriteByte(i32 b)
{
    u8 outByte;
    DWORD outBytesWritten;

    outByte = b;
    if (this->Write(&outByte, 1, &outBytesWritten) == FALSE)
    {
        return PBG_EOF;
    }
    if (outBytesWritten == 0)
    {
        return PBG_EOF;
    }
    else
    {
        return b;
    }
}

BOOL FileAbstraction::Seek(u32 amount, u32 seekFrom)
{
    if (this->handle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    SetFilePointer(this->handle, amount, NULL, seekFrom);
    return TRUE;
}

u32 FileAbstraction::Tell()
{
    if (this->handle == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    return SetFilePointer(this->handle, 0, NULL, FILE_CURRENT);
}

u32 FileAbstraction::GetSize()
{
    if (this->handle == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    return GetFileSize(this->handle, NULL);
}

BOOL FileAbstraction::WriteString(void *buffer)
{
    DWORD Length;
    DWORD temp;

    if (this->access != GENERIC_WRITE)
    {
        return FALSE;
    }

    Length = strlen((char *)buffer);
    return Write(buffer, Length, &temp);
}

LPVOID FileAbstraction::ReadWholeFile(u32 maxSize)
{
    if (this->access != GENERIC_READ)
    {
        return NULL;
    }

    u32 dataLen = this->GetSize();
    DWORD outDataLen;
    if (dataLen <= maxSize)
    {
        LPVOID data = (LPVOID)LocalAlloc(LPTR, dataLen);
        if (data != NULL)
        {
            u32 oldLocation = this->Tell();
            // Pretty sure the plan here was to seek to 0, but woops the code
            // is buggy.
            if (this->Seek(oldLocation, FILE_BEGIN) != FALSE)
            {
                if (this->Read(data, dataLen, &outDataLen) == FALSE)
                {
                    LocalFree(data);
                    return NULL;
                }
                this->Seek(oldLocation, FILE_BEGIN);
                return data;
            }
            // Yes, this case leaks the data. Amazing, I know.
        }
    }
    return NULL;
}

FileAbstraction::~FileAbstraction()
{
    this->Close();
}

} // namespace th06
