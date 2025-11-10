#pragma once
#include "GameErrorContext.hpp"
#include "ZunMath.hpp"
#include "inttypes.hpp"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define ARRAY_SIZE_SIGNED(x) ((i32)sizeof(x) / (i32)sizeof(x[0]))

#define ZUN_BIT(a) (1 << (a))
#define ZUN_MASK(a) (ZUN_BIT(a) - 1)
#define ZUN_RANGE(a, count) (ZUN_MASK((a) + (count)) & ~ZUN_MASK(a))
#define ZUN_CLEAR_BITS(a, keep_mask) (a & ~keep_mask)

#define IS_PRESSED(key) (g_CurFrameInput & (key))
#define WAS_PRESSED(key) (((g_CurFrameInput & (key)) != 0) && (g_CurFrameInput & (key)) != (g_LastFrameInput & (key)))
#define WAS_PRESSED_PERIODIC(key)                                                                                      \
    (WAS_PRESSED(key) || (((g_CurFrameInput & (key)) != 0) && (g_IsEigthFrameOfHeldInput != 0)))

namespace th06
{
namespace utils
{
void DebugPrint(const char *fmt, ...);
void DebugPrint2(const char *fmt, ...);

f32 AddNormalizeAngle(f32 a, f32 b);
void Rotate(ZunVec3 *outVector, ZunVec3 *point, f32 angle);
}; // namespace utils
}; // namespace th06
