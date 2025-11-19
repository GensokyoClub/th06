#ifdef DEBUG
#include <cstdarg>
#include <stdio.h>
#endif

#include <windows.h>

#include "ZunMath.hpp"
#include "i18n.hpp"
#include "utils.hpp"

namespace th06
{
DIFFABLE_STATIC(HANDLE, g_ExclusiveMutex)
namespace utils
{
ZunResult CheckForRunningGameInstance(void)
{
    g_ExclusiveMutex = CreateMutex(NULL, TRUE, TEXT("Touhou Koumakyou App"));

    if (g_ExclusiveMutex == NULL)
    {
        return ZUN_ERROR;
    }
    else if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        g_GameErrorContext.Fatal(TH_ERR_ALREADY_RUNNING);
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

void DebugPrint(const char *fmt, ...)
{
#ifdef DEBUG
    char tmpBuffer[512];
    std::va_list args;

    va_start(args, fmt);
    vsprintf(tmpBuffer, fmt, args);
    va_end(args);

    printf("DEBUG2: %s\n", tmpBuffer);
#endif
}

f32 AddNormalizeAngle(f32 a, f32 b)
{
    i32 i;

    i = 0;
    a += b;
    while (a > ZUN_PI)
    {
        a -= ZUN_2PI;
        if (i++ > 16)
            break;
    }
    while (a < -ZUN_PI)
    {
        a += ZUN_2PI;
        if (i++ > 16)
            break;
    }
    return a;
}

#pragma var_order(sinOut, cosOut)
void Rotate(D3DXVECTOR3 *outVector, D3DXVECTOR3 *point, f32 angle)
{
    f32 sinOut;
    f32 cosOut;

    sinOut = sinf(angle);
    cosOut = cosf(angle);
    outVector->x = cosOut * point->x + sinOut * point->y;
    outVector->y = cosOut * point->y - sinOut * point->x;
}

void DebugPrint2(const char *fmt, ...)
{
#ifdef DEBUG
    char tmpBuffer[512];
    std::va_list args;

    va_start(args, fmt);
    vsprintf(tmpBuffer, fmt, args);
    va_end(args);

    printf("DEBUG2: %s\n", tmpBuffer);
#endif
}
}; // namespace utils
}; // namespace th06
