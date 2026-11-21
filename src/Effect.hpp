#pragma once
#include "AnmVm.hpp"
#include "ZunTimer.hpp"
#include "decomp.hpp"

namespace th06
{
enum EffectCallbackResult
{
    EFFECT_CALLBACK_RESULT_STOP = 0,
    EFFECT_CALLBACK_RESULT_DONE = 1
};

struct Effect;

typedef EffectCallbackResult (*EffectUpdateCallback)(Effect *);
struct Effect
{
    AnmVm vm;
    D3DXVECTOR3 position;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 acceleration;
    D3DXVECTOR3 offset;
    D3DXVECTOR3 direction;
    D3DXQUATERNION quaternion;
    f32 radius;
    f32 angle;
    ZunTimer timer;
    unreferenced_fields(0x4);
    EffectUpdateCallback updateCallback;
    i8 inUseFlag;
    i8 effectId;
    i8 isFadingOut;
    i8 fadeOutTimer;
};
ZUN_ASSERT_TYPE(Effect, 0x17c, 4);

struct EffectInfo
{
    i32 anmScript;
    EffectUpdateCallback updateCallback;
};
ZUN_ASSERT_TYPE(EffectInfo, 0x8, 4);
} // namespace th06
