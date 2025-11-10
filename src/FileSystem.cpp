#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "FileSystem.hpp"
#include "utils.hpp"

namespace th06
{
u32 g_LastFileSize;

u8 *FileSystem::OpenPath(const char *filepath)
{
    u8 *data;
    FILE *file;
    size_t fsize;

    utils::DebugPrint2("%s Load ... \n", filepath);
    file = std::fopen(filepath, "rb");
    if (file == NULL) {
        utils::DebugPrint2("error : %s is not found.\n", filepath);
        return NULL;
    }
    else {
        std::fseek(file, 0, SEEK_END);
        fsize = std::ftell(file);
        g_LastFileSize = fsize;
        std::fseek(file, 0, SEEK_SET);
        data = (u8*)std::malloc(fsize);
        std::fread(data, 1, fsize, file);
        std::fclose(file);
    }
    return data;
}

int FileSystem::WriteDataToFile(const char *path, void *data, size_t size)
{
    FILE *f;

    f = std::fopen(path, "wb");
    if (f == NULL)
    {
        return -1;
    }
    else
    {
        if (std::fwrite(data, 1, size, f) != size)
        {
            std::fclose(f);
            return -2;
        }
        else
        {
            std::fclose(f);
            return 0;
        }
    }
}
}; // namespace th06
