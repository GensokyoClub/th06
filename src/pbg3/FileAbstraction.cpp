#include "pbg3/FileAbstraction.hpp"
#include <FileSystem.hpp>

namespace th06
{
FileAbstraction::FileAbstraction()
{
    handle = NULL;
    access = ACCESS_INVALID;
}

i32 FileAbstraction::Open(char *filename, char *mode)
{
    char openMode[] = "*b";

    this->Close();

    char *curMode;
    for (curMode = mode; *curMode != '\0'; curMode += 1)
    {
        if (*curMode == 'r')
        {
            this->access = ACCESS_READ;
            openMode[0] = 'r';
            break;
        }
        else if (*curMode == 'w')
        {
            this->access = ACCESS_WRITE;
            openMode[0] = 'w';
            break;
        }
        else if (*curMode == 'a')
        {
            this->access = ACCESS_WRITE;
            openMode[0] = 'a';
            break;
        }
    }

    if (*curMode == '\0')
    {
        return 0;
    }

    this->handle = FileSystem::FopenUTF8(filename, openMode);

    if (this->handle == NULL)
        return 0;

    this->path = new std::filesystem::path(filename);

    return 1;
}

void FileAbstraction::Close()
{
    if (this->handle != NULL)
    {
        std::fclose(this->handle);
        this->handle = NULL;
        this->access = ACCESS_INVALID;
        delete this->path;
    }
}

i32 FileAbstraction::Read(u8 *data, u32 dataLen, u32 *numBytesRead)
{
    if (this->access != ACCESS_READ)
    {
        return false;
    }

    *numBytesRead = std::fread(data, 1, dataLen, this->handle);

    return !(dataLen != 0 && *numBytesRead < dataLen);
}

i32 FileAbstraction::Write(u8 *data, u32 dataLen, u32 *outWritten)
{
    if (this->access != ACCESS_WRITE)
    {
        return false;
    }

    *outWritten = std::fwrite(data, 1, dataLen, this->handle);

    return !(dataLen != 0 && *outWritten < dataLen);
}

i32 FileAbstraction::ReadByte()
{
    u8 data;
    u32 outBytesRead;

    if (!this->Read(&data, 1, &outBytesRead))
    {
        return -1;
    }
    else
    {
        if (outBytesRead == 0)
        {
            return -1;
        }
        return data;
    }
}

i32 FileAbstraction::WriteByte(u32 b)
{
    u8 outByte;
    u32 outBytesWritten;

    outByte = b;
    if (!this->Write(&outByte, 1, &outBytesWritten))
    {
        return -1;
    }
    else
    {
        if (outBytesWritten == 0)
        {
            return -1;
        }
        return b;
    }
}

i32 FileAbstraction::Seek(u32 amount, u32 seekFrom)
{
    if (this->handle == NULL)
    {
        return 0;
    }

    std::fseek(this->handle, amount, seekFrom);
    return 1;
}

u32 FileAbstraction::Tell()
{
    if (this->handle == NULL)
    {
        return 0;
    }

    return std::ftell(this->handle);
}

u32 FileAbstraction::GetSize()
{
    if (this->handle == NULL)
    {
        return 0;
    }

    return std::filesystem::file_size(*this->path);
}

u8 *FileAbstraction::ReadWholeFile(u32 maxSize)
{
    if (this->access != ACCESS_READ)
    {
        return NULL;
    }

    u32 dataLen = this->GetSize();
    u32 outDataLen;
    if (dataLen <= maxSize)
    {
        u8 *data = new u8[dataLen];
        if (data != NULL)
        {
            u32 oldLocation = this->Tell();
            // Pretty sure the plan here was to seek to 0, but woops the code
            // is buggy.
            if (this->Seek(oldLocation, SEEK_SET) != 0)
            {
                if (this->Read(data, dataLen, &outDataLen) == 0)
                {
                    delete[] data;
                    return NULL;
                }
                this->Seek(oldLocation, SEEK_SET);
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
}; // namespace th06
