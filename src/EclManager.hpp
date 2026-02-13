#pragma once

#include "ItemManager.hpp"
#include "SoundPlayer.hpp"
#include "ZunBool.hpp"
#include "ZunColor.hpp"
#include "ZunMath.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include <Windows.h>
#include <d3dx8math.h>

namespace th06
{
// Forward declaration to avoid include loop.
struct Enemy;
struct EnemyEclContext;
struct EnemyManager;

enum EclVarId
{
    ECL_VAR_I32_0 = -10001,
    ECL_VAR_I32_1 = -10002,
    ECL_VAR_I32_2 = -10003,
    ECL_VAR_I32_3 = -10004,
    ECL_VAR_F32_0 = -10005,
    ECL_VAR_F32_1 = -10006,
    ECL_VAR_F32_2 = -10007,
    ECL_VAR_F32_3 = -10008,
    ECL_VAR_I32_4 = -10009,
    ECL_VAR_I32_5 = -10010,
    ECL_VAR_I32_6 = -10011,
    ECL_VAR_I32_7 = -10012,
    ECL_VAR_DIFFICULTY = -10013,
    ECL_VAR_RANK = -10014,
    ECL_VAR_ENEMY_POS_X = -10015,
    ECL_VAR_ENEMY_POS_Y = -10016,
    ECL_VAR_ENEMY_POS_Z = -10017,
    ECL_VAR_PLAYER_POS_X = -10018,
    ECL_VAR_PLAYER_POS_Y = -10019,
    ECL_VAR_PLAYER_POS_Z = -10020,
    ECL_VAR_PLAYER_ANGLE = -10021,
    ECL_VAR_ENEMY_TIMER = -10022,
    ECL_VAR_PLAYER_DISTANCE = -10023,
    ECL_VAR_ENEMY_LIFE = -10024,
    ECL_VAR_PLAYER_SHOT = -10025,
};

struct EclTimelineInstrArgs
{
    u32 uintVar1;
    u32 uintVar2;
    u32 uintVar3;
    u16 ushortVar1;
    u16 ushortVar2;
    u32 uintVar4;

    D3DXVECTOR3 *Var1AsVec()
    {
        return (D3DXVECTOR3 *)&this->uintVar1;
    }
};

struct EclTimelineInstr
{
    i16 time;
    i16 arg0;
    i16 opCode;
    i16 size;
    EclTimelineInstrArgs args;
};

union EclRawInstrArg {
    struct
    {
        i8 a;
        i8 b;
        i8 c;
        i8 d;
    } by;
    struct
    {
        i16 lo;
        i16 hi;
    } sh;
    f32 f32;
    i32 i32;
    EclVarId id;
};

struct EclRawInstrAluArgs
{
    EclVarId res;
    EclRawInstrArg arg1;
    EclRawInstrArg arg2;
    EclRawInstrArg arg3;
    EclRawInstrArg arg4;
};

struct EclRawInstrJumpArgs
{
    i32 time;
    i32 offset;
    EclVarId var;
};

struct EclRawInstrCallArgs
{
    i32 eclSub;
    i32 var0;
    f32 float0;
    EclVarId cmpLhs;
    i32 cmpRhs;
};

struct EclRawInstrCmpArgs
{
    EclRawInstrArg lhs;
    EclRawInstrArg rhs;
};

struct EclRawInstrMoveArgs
{
    ZunVec3 pos;
};

struct EclRawInstrAnmSetMainArgs
{
    i32 scriptIdx;
};

struct EclRawInstrAnmSetSlotArgs
{
    i32 vmIdx;
    i32 scriptIdx;
};

struct EclRawInstrAnmSetDeathArgs
{
    i8 deathAnm1;
    i8 deathAnm2;
    i8 deathAnm3;
};

struct EclRawInstrBulletArgs
{
    i16 sprite;
    i16 color;
    EclVarId count1;
    EclVarId count2;
    f32 speed1;
    f32 speed2;
    f32 angle1;
    f32 angle2;
    i32 flags;
};

struct EclRawInstrLaserArgs
{
    i16 sprite;
    i16 color;
    f32 angle;
    f32 speed;
    f32 startOffset;
    f32 endOffset;
    f32 startLength;
    f32 width;
    i32 startTime;
    i32 duration;
    i32 despawnDuration;
    i32 hitboxStartTime;
    i32 hitboxEndDelay;
    i32 flags;
};

struct EclRawInstrLaserOpArgs
{
    i32 laserIdx;
    ZunVec3 arg1;
};

struct EclRawInstrBulletEffectsArgs
{
    EclVarId ivar1;
    EclVarId ivar2;
    EclVarId ivar3;
    EclVarId ivar4;
    f32 fvar1;
    f32 fvar2;
    f32 fvar3;
    f32 fvar4;
};

struct EclRawInstrSetInt
{
    i32 i32;
};

struct EclRawInstrSpellcardEffectArgs
{
    i32 effectColorId;
    ZunVec3 pos;
    f32 effectDistance;
};

struct EclRawInstrMoveBoundSetArgs
{
    ZunVec2 lowerMoveLimit;
    ZunVec2 upperMoveLimit;
};

struct EclRawInstrAnmSetPosesArgs
{
    i16 anmExDefault;
    i16 anmExFarLeft;
    i16 anmExFarRight;
    i16 anmExLeft;
    i16 anmExRight;
};

struct EclRawInstrSetInterruptArgs
{
    i32 interruptSub;
    i32 interruptId;
};

struct EclRawInstrSpellcardStartArgs
{
    i16 spellcardSprite;
    i16 spellcardId;
    char spellcardName[1];
};

struct EclRawInstrEffectParticleArgs
{
    i32 effectId;
    i32 numParticles;
    ZunColor particleColor;
};

struct EclRawInstrTimeSetArgs
{
    EclVarId timeToSet;
};

struct EclRawInstrDropItemArgs
{
    ItemType itemId;
};

struct EclRawInstrEnemyCreateArgs
{
    i32 subId;
    ZunVec3 pos;
    i16 life;
    i16 itemDrop;
    i32 score;
};
ZUN_ASSERT_SIZE(EclRawInstrEnemyCreateArgs, 0x18);

struct EclRawInstrAnmInterruptSlotArgs
{
    i32 vmId;
    i32 interruptId;
};

struct EclRawInstrBulletSoundArgs
{
    SoundIdx bulletSfx;
};

struct EclRawInstrBulletRankInfluenceArgs
{
    f32 bulletRankSpeedLow;
    f32 bulletRankSpeedHigh;
    i32 bulletRankAmount1Low;
    i32 bulletRankAmount1High;
    i32 bulletRankAmount2Low;
    i32 bulletRankAmount2High;
};

struct EclRawInstrExInstrArgs
{
    u32 exInstrIndex;
    union {
        i32 i32Param;
        u8 u8Param;
    };
};

union EclRawInstrArgs {
    EclRawInstrAluArgs alu;
    EclRawInstrCmpArgs cmp;
    EclRawInstrJumpArgs jump;
    EclRawInstrCallArgs call;
    EclRawInstrAnmSetMainArgs anmSetMain;
    EclRawInstrAnmSetPosesArgs anmSetPoses;
    EclRawInstrAnmSetSlotArgs anmSetSlot;
    EclRawInstrAnmSetDeathArgs anmSetDeath;
    EclRawInstrMoveArgs move;
    EclRawInstrBulletArgs bullet;
    EclRawInstrLaserArgs laser;
    EclRawInstrLaserOpArgs laserOp;
    EclRawInstrBulletEffectsArgs bulletEffects;
    EclRawInstrSpellcardEffectArgs spellcardEffect;
    EclRawInstrMoveBoundSetArgs moveBoundSet;
    EclRawInstrSetInterruptArgs setInterrupt;
    EclRawInstrSpellcardStartArgs spellcardStart;
    EclRawInstrEffectParticleArgs effectParticle;
    EclRawInstrTimeSetArgs timeSet;
    EclRawInstrDropItemArgs dropItem;
    EclRawInstrEnemyCreateArgs enemyCreate;
    EclRawInstrAnmInterruptSlotArgs anmInterruptSlot;
    EclRawInstrBulletSoundArgs bulletSound;
    EclRawInstrBulletRankInfluenceArgs bulletRankInfluence;
    EclRawInstrExInstrArgs exInstr;
    i32 setInt;

    i32 GetBossLifeCount()
    {
        return this->setInt;
    }
};

struct EclRawInstr
{
    i32 time;
    i16 opCode;
    i16 offsetToNext;
    u8 unk_8;
    // Bitfield where each bit tells us whether we should skip this instruction
    // on that difficulty (1) or run it (0).
    u8 skipForDifficulty;
    u8 unk_a;
    u8 unk_b;
    EclRawInstrArgs args;
};

struct EclRawHeader
{
    i16 subCount;
    i16 mainCount;
    EclTimelineInstr *timelineOffsets[3];
    EclRawInstr *subOffsets[0];
};
ZUN_ASSERT_SIZE(EclRawHeader, 0x10);

enum EclRawInstrOpcode
{
    ECL_OPCODE_NOP,
    ECL_OPCODE_UNIMP,
    ECL_OPCODE_JUMP,
    ECL_OPCODE_JUMPDEC,
    ECL_OPCODE_SETINT,
    ECL_OPCODE_SETFLOAT,
    ECL_OPCODE_SETINTRAND,
    ECL_OPCODE_SETINTRANDMIN,
    ECL_OPCODE_SETFLOATRAND,
    ECL_OPCODE_SETFLOATRANDMIN,
    ECL_OPCODE_SETVARSELFX,
    ECL_OPCODE_SETVARSELFY,
    ECL_OPCODE_SETVARSELFZ,
    ECL_OPCODE_MATHINTADD,
    ECL_OPCODE_MATHINTSUB,
    ECL_OPCODE_MATHINTMUL,
    ECL_OPCODE_MATHINTDIV,
    ECL_OPCODE_MATHINTMOD,
    ECL_OPCODE_MATHINC,
    ECL_OPCODE_MATHDEC,
    ECL_OPCODE_MATHFLOATADD,
    ECL_OPCODE_MATHFLOATSUB,
    ECL_OPCODE_MATHFLOATMUL,
    ECL_OPCODE_MATHFLOATDIV,
    ECL_OPCODE_MATHFLOATMOD,
    ECL_OPCODE_MATHATAN2,
    ECL_OPCODE_MATHNORMANGLE,
    ECL_OPCODE_CMPINT,
    ECL_OPCODE_CMPFLOAT,
    ECL_OPCODE_JUMPLSS,
    ECL_OPCODE_JUMPLEQ,
    ECL_OPCODE_JUMPEQU,
    ECL_OPCODE_JUMPGRE,
    ECL_OPCODE_JUMPGEQ,
    ECL_OPCODE_JUMPNEQ,
    ECL_OPCODE_CALL,
    ECL_OPCODE_RET,
    ECL_OPCODE_CALLLSS,
    ECL_OPCODE_CALLLEQ,
    ECL_OPCODE_CALLEQU,
    ECL_OPCODE_CALLGRE,
    ECL_OPCODE_CALLGEQ,
    ECL_OPCODE_CALLNEQ,
    ECL_OPCODE_MOVEPOSITION,
    ECL_OPCODE_MOVEAXISVELOCITY,
    ECL_OPCODE_MOVEVELOCITY,
    ECL_OPCODE_MOVEANGULARVELOCITY,
    ECL_OPCODE_MOVESPEED,
    ECL_OPCODE_MOVEACCELERATION,
    ECL_OPCODE_MOVERAND,
    ECL_OPCODE_MOVERANDINBOUND,
    ECL_OPCODE_MOVEATPLAYER,
    ECL_OPCODE_MOVEDIRTIMEDECELERATE, // 0x34 / 52
    ECL_OPCODE_MOVEDIRTIMEDECELERATEFAST,
    ECL_OPCODE_MOVEDIRTIMEACCELERATE,
    ECL_OPCODE_MOVEDIRTIMEACCELERATEFAST,
    ECL_OPCODE_MOVEPOSITIONTIMELINEAR,
    ECL_OPCODE_MOVEPOSITIONTIMEDECELERATE,
    ECL_OPCODE_MOVEPOSITIONTIMEDECELERATEFAST,
    ECL_OPCODE_MOVEPOSITIONTIMEACCELERATE,
    ECL_OPCODE_MOVEPOSITIONTIMEACCELERATEFAST,
    ECL_OPCODE_MOVETIMEDECELERATE,
    ECL_OPCODE_MOVETIMEDECELERATEFAST,
    ECL_OPCODE_MOVETIMEACCELERATE,
    ECL_OPCODE_MOVETIMEACCELERATEFAST,
    ECL_OPCODE_MOVEBOUNDSSET,
    ECL_OPCODE_MOVEBOUNDSDISABLE,
    ECL_OPCODE_BULLETFANAIMED,          // 0x43 / 67
    ECL_OPCODE_BULLETFAN,               // 0x44 / 68
    ECL_OPCODE_BULLETCIRCLEAIMED,       // 0x45 / 69
    ECL_OPCODE_BULLETCIRCLE,            // 0x46 / 70
    ECL_OPCODE_BULLETOFFSETCIRCLEAIMED, // 0x47 / 71
    ECL_OPCODE_BULLETOFFSETCIRCLE,      // 0x48 / 72
    ECL_OPCODE_BULLETRANDOMANGLE,       // 0x49 / 73
    ECL_OPCODE_BULLETRANDOMSPEED,       // 0x4a / 74
    ECL_OPCODE_BULLETRANDOM,            // 0x4b / 75
    ECL_OPCODE_SHOOTINTERVAL,           // 0x4c / 76
    ECL_OPCODE_SHOOTINTERVALDELAYED,    // 0x4d / 77
    ECL_OPCODE_SHOOTDISABLED,           // 0x4e / 78
    ECL_OPCODE_SHOOTENABLED,            // 0x4f / 79
    ECL_OPCODE_SHOOTNOW,                // 0x50 / 80
    ECL_OPCODE_SHOOTOFFSET,             // 0x51 / 81
    ECL_OPCODE_BULLETEFFECTS,           // 0x52 / 82
    ECL_OPCODE_BULLETCANCEL,
    ECL_OPCODE_BULLETSOUND,
    ECL_OPCODE_LASERCREATE,           // 0x55 / 85
    ECL_OPCODE_LASERCREATEAIMED,      // 0x56 / 86
    ECL_OPCODE_LASERINDEX,            // 0x57 / 87
    ECL_OPCODE_LASERROTATE,           // 0x58 / 88
    ECL_OPCODE_LASERROTATEFROMPLAYER, // 0x59 / 89
    ECL_OPCODE_LASEROFFSET,           // 0x5a / 90
    ECL_OPCODE_LASERTEST,             // 0x5b / 91
    ECL_OPCODE_LASERCANCEL,           // 0x5c / 92
    ECL_OPCODE_SPELLCARDSTART,        // 0x5d / 93
    ECL_OPCODE_SPELLCARDEND,          // 0x5e / 94
    ECL_OPCODE_ENEMYCREATE,
    ECL_OPCODE_ENEMYKILLALL,
    ECL_OPCODE_ANMSETMAIN,             // 0x61 / 97
    ECL_OPCODE_ANMSETPOSES,            // 0x62 / 98
    ECL_OPCODE_ANMSETSLOT,             // 0x63 / 99
    ECL_OPCODE_ANMSETDEATH,            // 0x64 / 100
    ECL_OPCODE_BOSSSET,                // 0x65 / 101
    ECL_OPCODE_SPELLCARDEFFECT,        // 0x66 / 102
    ECL_OPCODE_ENEMYSETHITBOX,         // 0x67 / 103
    ECL_OPCODE_ENEMYFLAGCOLLISION,     // 0x68 / 104
    ECL_OPCODE_ENEMYFLAGCANTAKEDAMAGE, // 0x69 / 105
    ECL_OPCODE_EFFECTSOUND,            // 0x6a / 106
    ECL_OPCODE_ENEMYFLAGDEATH,         // 0x6b / 107
    ECL_OPCODE_DEATHCALLBACKSUB,       // 0x6c / 108
    ECL_OPCODE_ENEMYINTERRUPTSET,      // 0x6d / 109
    ECL_OPCODE_ENEMYINTERRUPT,         // 0x6e / 110
    ECL_OPCODE_ENEMYLIFESET,           // 0x6f / 111
    ECL_OPCODE_BOSSTIMERSET,           // 0x70 / 112
    ECL_OPCODE_LIFECALLBACKTHRESHOLD,
    ECL_OPCODE_LIFECALLBACKSUB,
    ECL_OPCODE_TIMERCALLBACKTHRESHOLD,
    ECL_OPCODE_TIMERCALLBACKSUB,
    ECL_OPCODE_ENEMYFLAGINTERACTABLE,
    ECL_OPCODE_EFFECTPARTICLE,
    ECL_OPCODE_DROPITEMS,
    ECL_OPCODE_ANMFLAGROTATION,
    ECL_OPCODE_EXINSCALL,
    ECL_OPCODE_EXINSREPEAT,
    ECL_OPCODE_TIMESET,
    ECL_OPCODE_DROPITEMID,
    ECL_OPCODE_STDUNPAUSE,
    ECL_OPCODE_BOSSSETLIFECOUNT,
    ECL_OPCODE_DEBUGWATCH,
    ECL_OPCODE_ANMINTERRUPTMAIN,
    ECL_OPCODE_ANMINTERRUPTSLOT,
    ECL_OPCODE_ENEMYFLAGDISABLECALLSTACK,
    ECL_OPCODE_BULLETRANKINFLUENCE,
    ECL_OPCODE_ENEMYFLAGINVISIBLE,
    ECL_OPCODE_BOSSTIMERCLEAR,
    ECL_OPCODE_LASERCLEARALL,
    ECL_OPCODE_SPELLCARDFLAGTIMEOUT,
};

struct EclManager
{
    ZunResult Load(char *ecl);
    void Unload();
    ZunResult RunEcl(Enemy *enemy);
    ZunResult CallEclSub(EnemyEclContext *enemyEcl, i16 subId);
    f32 AngleProvokedPlayer(D3DXVECTOR3 *pos, u8 playerType);

    EclRawHeader *eclFile;
    EclRawInstr **subTable;
    EclTimelineInstr *timeline;
};
ZUN_ASSERT_SIZE(EclManager, 0xc);

DIFFABLE_EXTERN(EclManager, g_EclManager);
}; // namespace th06
