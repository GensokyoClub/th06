#pragma once
#include "AnmVm.hpp"
#include "BulletManager.hpp"
#include "EclManager.hpp"
#include "Effect.hpp"
#include "ItemManager.hpp"
#include "SoundPlayer.hpp"
#include "ZunBool.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"
#include <Windows.h>
#include <d3d8.h>
#include <d3dx8math.h>
#include <string.h>

namespace th06
{
struct Enemy;

struct EnemyBulletShooter
{
    EnemyBulletShooter()
    {
        memset(this, 0, sizeof(EnemyBulletShooter));
    }
    i16 sprite;
    i16 spriteOffset;
    D3DXVECTOR3 position;
    f32 angle1;
    f32 angle2;
    f32 speed1;
    f32 speed2;
    f32 exFloats[4];
    i32 exInts[4];
    unreferenced_fields(0x4);
    i16 count1;
    i16 count2;
    u16 aimMode;
    u16 unk_4a;
    u32 flags;
    SoundIdx sfx;
};
ZUN_ASSERT_TYPE(EnemyBulletShooter, 0x54, 4);

struct EnemyLaserShooter
{
    EnemyLaserShooter()
    {
        memset(this, 0, sizeof(EnemyLaserShooter));
    }
    i16 sprite;
    i16 spriteOffset;
    D3DXVECTOR3 position;
    f32 angle;
    unreferenced_fields(0x4);
    f32 speed;
    unreferenced_fields(0x4);
    f32 startOffset;
    f32 endOffset;
    f32 startLength;
    f32 width;
    i32 startTime;
    i32 duration;
    i32 despawnDuration;
    i32 hitboxStartTime;
    i32 hitboxEndDelay;
    unreferenced_fields(0x4);
    u16 type;
    u32 flags;
    unreferenced_fields(0x4);
};
ZUN_ASSERT_TYPE(EnemyLaserShooter, 0x54, 4);

struct EnemyEclContext
{
    EclRawInstr *currentInstr;
    ZunTimer time;
    void (*funcSetFunc)(Enemy *, EclRawInstr *);
    i32 int0;
    i32 int1;
    i32 int2;
    i32 int3;
    f32 float0;
    f32 float1;
    f32 float2;
    f32 float3;
    i32 counter0;
    i32 counter1;
    i32 counter2;
    i32 counter3;
    i32 compareRegister;
    u16 subId;
};
ZUN_ASSERT_TYPE(EnemyEclContext, 0x4c, 4);

enum EnemyMoveMode
{
    EnemyMove_AxisSpeed,
    EnemyMove_Velocity,
    EnemyMove_Interp
};

enum EnemyInterpMode
{
    EnemyInterp_Linear,
    EnemyInterp_Decelerate,
    EnemyInterp_DecelerateFast,
    EnemyInterp_Accelerate,
    EnemyInterp_AccelerateFast,
};

enum EnemyDeathMode
{
    EnemyDeath_DespawnNoCallback,
    EnemyDeath_DisableInteraction,
    EnemyDeath_DropItemsOnly,
    EnemyDeath_SetHpTo1
};

struct EnemyFlags
{
    // First byte
    u8 movementMode : 2;   // EnemyMoveMode
    u8 moveInterpMode : 3; // EnemyInterpMode
    u8 shootingDisabled : 1;
    u8 mirrored : 1;
    u8 isSlotOccupied : 1;

    // Second byte
    u8 isInteractable : 1;
    u8 isCollidable : 1;
    u8 hasBeenInBounds : 1;
    u8 isBoss : 1;
    u8 isDamageable : 1;
    u8 deathMode : 3; // EnemyDeathMode

    // Third byte
    u8 shouldClampPos : 1;
    u8 rotateAnm : 1;
    u8 disableCallStack : 1;
    u8 isInvisible : 1;
    u8 isTimeoutSpell : 1;
    alignment_bitfields(u8, 3);
};
ZUN_ASSERT_TYPE(EnemyFlags, 0x3, 1);

enum EclValueType
{
    ECL_VALUE_TYPE_INT,
    ECL_VALUE_TYPE_FLOAT,
    ECL_VALUE_TYPE_READONLY,
    ECL_VALUE_TYPE_UNDEFINED,
};

enum EnemyPose
{
    EnemyPose_Default = 0xFF,
    EnemyPose_Neutral = 0,
    EnemyPose_Left = 1,
    EnemyPose_Right = 2
};

#define ENEMY_ANM_SLOTS 8
#define MAX_ECL_STACK_DEPTH 7
#define MAX_LASERS_PER_ENEMY 32

struct Enemy
{
    void Move()
    {
        if (!this->flags.mirrored)
        {
            this->position.x += g_Supervisor.effectiveFramerateMultiplier * this->axisSpeed.x;
        }
        else
        {
            this->position.x -= g_Supervisor.effectiveFramerateMultiplier * this->axisSpeed.x;
        }
        this->position.y += g_Supervisor.effectiveFramerateMultiplier * this->axisSpeed.y;
        this->position.z += g_Supervisor.effectiveFramerateMultiplier * this->axisSpeed.z;
    }

    void ClampPos();
    ZunBool HandleLifeCallback();
    ZunBool HandleTimerCallback();
    void Despawn();

    static void ResetEffectArray(Enemy *enemy);

    f32 LifePercent()
    {
        return (f32)this->life / (f32)this->maxLife;
    }

    ZunBool HasPhaseTimerFinished()
    {
        return this->phaseTimer.current >= this->timerCallbackThreshold;
    }

    void ResetRank()
    {
        this->bulletRankSpeedLow = -0.5f;
        this->bulletRankSpeedHigh = 0.5f;
        this->bulletRankAmount1Low = 0;
        this->bulletRankAmount1High = 0;
        this->bulletRankAmount2Low = 0;
        this->bulletRankAmount2High = 0;
    }

    AnmVm primaryVm;
    AnmVm vms[ENEMY_ANM_SLOTS];
    EnemyEclContext currentContext;
    EnemyEclContext savedContextStack[MAX_ECL_STACK_DEPTH + 1]; // +1 dummy slot for overflow
    i32 stackDepth;
    unreferenced_fields(0x4);
    i32 deathCallbackSub;
    i32 interrupts[8];
    i32 runInterrupt;
    D3DXVECTOR3 position;
    D3DXVECTOR3 hitboxDimensions;
    D3DXVECTOR3 axisSpeed;
    f32 angle;
    f32 angularVelocity;
    f32 speed;
    f32 acceleration;
    D3DXVECTOR3 shootOffset;
    D3DXVECTOR3 moveInterp;
    D3DXVECTOR3 moveInterpStartPos;
    ZunTimer moveInterpTimer;
    i32 moveInterpStartTime;
    f32 bulletRankSpeedLow;
    f32 bulletRankSpeedHigh;
    i16 bulletRankAmount1Low;
    i16 bulletRankAmount1High;
    i16 bulletRankAmount2Low;
    i16 bulletRankAmount2High;
    i32 life;
    i32 maxLife;
    i32 score;
    ZunTimer phaseTimer;
    ZunColor color;
    EnemyBulletShooter bulletProps;
    i32 shootInterval;
    ZunTimer shootIntervalTimer;
    EnemyLaserShooter laserProps;
    Laser *lasers[MAX_LASERS_PER_ENEMY];
    i32 laserStore;
    u8 deathParticle1;
    u8 deathParticle2;
    u8 deathAnm3;
    i8 itemDrop;
    u8 bossId;
    u8 unk_e41;
    alignment_padding(0x2);
    ZunTimer exInsFunc10Timer;
    EnemyFlags flags;
    u8 anmPoseCurrent;
    i16 anmPoseDefault;
    i16 anmPoseNeutralFromLeft;
    i16 anmPoseNeutralFromRight;
    i16 anmPoseLeft;
    i16 anmPoseRight;
    D3DXVECTOR2 lowerMoveLimit;
    D3DXVECTOR2 upperMoveLimit;
    Effect *effectArray[12];
    i32 effectIdx;
    f32 effectDistance;
    i32 lifeCallbackThreshold;
    i32 lifeCallbackSub;
    i32 timerCallbackThreshold;
    i32 timerCallbackSub;
    f32 exInsFunc6Angle;
    ZunTimer exInsFunc6Timer;
};
ZUN_ASSERT_TYPE(Enemy, 0xec8, 4);
} // namespace th06
