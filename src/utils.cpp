#ifdef DEBUG
#include <cstdarg>
#include <cstdio>
#endif

#include "ZunMath.hpp"
#include "i18n.hpp"
#include "utils.hpp"

namespace th06
{
namespace utils
{
void DebugPrint(const char *fmt, ...)
{
#ifdef DEBUG
    char tmpBuffer[512];
    std::va_list args;

    va_start(args, fmt);
    std::vsnprintf(tmpBuffer, 511, fmt, args);
    va_end(args);

    std::printf("DEBUG2: %s\n", tmpBuffer);
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

void Rotate(ZunVec3 *outVector, ZunVec3 *point, f32 angle)
{
    f32 sinOut;
    f32 cosOut;

    sinOut = ZUN_SINF(angle);
    cosOut = ZUN_COSF(angle);
    outVector->x = cosOut * point->x + sinOut * point->y;
    outVector->y = cosOut * point->y - sinOut * point->x;
}

void DebugPrint2(const char *fmt, ...)
{
#ifdef DEBUG
    char tmpBuffer[512];
    std::va_list args;

    va_start(args, fmt);
    std::vsnprintf(tmpBuffer, 511, fmt, args);
    va_end(args);

    std::printf("DEBUG2: %s\n", tmpBuffer);
#endif
}
}; // namespace utils
}; // namespace th06
