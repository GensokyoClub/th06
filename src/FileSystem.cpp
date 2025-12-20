#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <new>
#include <windows.h>
#endif

#include "FileSystem.hpp"
#include "utils.hpp"

namespace th06 {
u32 g_LastFileSize;

FILE *FileSystem::FopenUTF8(const char *filepath, const char *mode) {
#ifndef _WIN32
    return std::fopen(filepath, mode);
#else
    u32 filepathWLen =
        MultiByteToWideChar(CP_UTF8, 0, filepath, -1, NULL, 0) * 2;
    u32 modeWLen = MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0) * 2;

    if (filepathWLen == 0 || modeWLen == 0) {
        return NULL;
    }

    wchar_t *filepathW = new wchar_t[filepathWLen];
    wchar_t *modeW = new wchar_t[modeWLen];

    MultiByteToWideChar(CP_UTF8, 0, filepath, -1, filepathW, filepathWLen / 2);
    MultiByteToWideChar(CP_UTF8, 0, mode, -1, modeW, modeWLen / 2);

    FILE *f = _wfopen(filepathW, modeW);

    delete[] filepathW;
    delete[] modeW;

    return f;
#endif
}

u8 *FileSystem::OpenPath(const char *filepath) {
    u8 *data;
    FILE *file;
    size_t fsize;

    utils::DebugPrint2("%s Load ... \n", filepath);
    file = std::fopen(filepath, "rb");
    if (file == NULL) {
        utils::DebugPrint2("error : %s is not found.\n", filepath);
        return NULL;
    } else {
        std::fseek(file, 0, SEEK_END);
        fsize = std::ftell(file);
        g_LastFileSize = fsize;
        std::fseek(file, 0, SEEK_SET);
        data = (u8 *)std::malloc(fsize);
        std::fread(data, 1, fsize, file);
        std::fclose(file);
    }
    return data;
}

int FileSystem::WriteDataToFile(const char *path, void *data, size_t size) {
    FILE *f;

    f = FopenUTF8(path, "wb");
    if (f == NULL) {
        return -1;
    } else {
        if (std::fwrite(data, 1, size, f) != size) {
            std::fclose(f);
            return -2;
        } else {
            std::fclose(f);
            return 0;
        }
    }
}
}; // namespace th06
