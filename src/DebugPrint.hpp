#pragma once
#include <stdarg.h>
#include <stdio.h>

namespace th06
{
static void DebugPrint(const char *fmt, ...)
{
#ifdef DEBUG
    char tmpBuffer[512];
    va_list args;

    va_start(args, fmt);
    vsprintf(tmpBuffer, fmt, args);
    va_end(args);

    printf("DEBUG: %s\n", tmpBuffer);
#endif
}
} // namespace th06
