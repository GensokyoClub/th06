#pragma once

#include "AnmVm.hpp"
#include "Chain.hpp"
#include "ZunTimer.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"

namespace th06
{
struct RawStageHeader
{
    i16 nbObjects;
    i16 nbFaces;
    i32 facesOffset;
    i32 scriptOffset;
    i32 unk_c;
    char stageName[128];
    char songNames[4][128];
    char songPaths[4][128];
};
ZUN_ASSERT_SIZE(RawStageHeader, 0x490);

struct RawStageQuadBasic
{
    i16 type;
    i16 byteSize;
    i16 anmScript;
    i16 vmIdx;
    ZunVec3 position;
    ZunVec2 size;
};
ZUN_ASSERT_SIZE(RawStageQuadBasic, 0x1c);

struct RawStageObject
{
    i16 id;
    i8 zLevel;
    i8 flags;
    ZunVec3 position;
    ZunVec3 size;
    RawStageQuadBasic firstQuad;
};
ZUN_ASSERT_SIZE(RawStageObject, 0x38);

struct RawStageObjectInstance
{
    i16 id;
    i16 unk2;
    ZunVec3 position;
};
ZUN_ASSERT_SIZE(RawStageObjectInstance, 0x10);

struct RawStageInstr
{
    i32 frame;
    i16 opcode;
    i16 size;
    i32 args[3];
};
ZUN_ASSERT_SIZE(RawStageInstr, 0x14);

struct StageCameraSky
{
    f32 nearPlane;
    f32 farPlane;
    ZunColor color;
};
ZUN_ASSERT_SIZE(StageCameraSky, 0xc);

enum SpellcardState
{
    NOT_RUNNING,
    RUNNING,
    RAN_FOR_60_FRAMES
};

struct StageFile
{
    const char *anmFile;
    const char *stdFile;
};
ZUN_ASSERT_SIZE(StageFile, 0x8);

enum StageOpcode
{
    STDOP_CAMERA_POSITION_KEY,
    STDOP_FOG,
    STDOP_CAMERA_FACING,
    STDOP_CAMERA_FACING_INTERP_LINEAR,
    STDOP_FOG_INTERP,
    STDOP_PAUSE,
};

struct Stage
{
    Stage();
    static bool RegisterChain(u32 stage);
    static void CutChain();
    static ChainCallbackResult OnUpdate(Stage *stage);
    static ChainCallbackResult OnDrawHighPrio(Stage *stage);
    static ChainCallbackResult OnDrawLowPrio(Stage *stage);
    static bool AddedCallback(Stage *stage);
    static bool DeletedCallback(Stage *stage);

    bool LoadStageData(const char *anmpath, const char *stdpath);
    bool UpdateObjects();
    bool RenderObjects(i32 zLevel);

    AnmVm *quadVms;
    RawStageHeader *stdData;
    i32 quadCount;
    i32 objectsCount;
    RawStageObject **objects;
    RawStageObjectInstance *objectInstances;
    RawStageInstr *beginningOfScript;
    ZunTimer scriptTime;
    i32 instructionIndex;
    ZunTimer timer;
    u32 stage;
    ZunVec3 position;
    StageCameraSky skyFog;
    StageCameraSky skyFogInterpInitial;
    StageCameraSky skyFogInterpFinal;
    i32 skyFogInterpDuration;
    ZunTimer skyFogInterpTimer;
    i8 skyFogNeedsSetup;
    SpellcardState spellcardState;
    i32 ticksSinceSpellcardStarted;
    AnmVm spellcardBackground;
    AnmVm unk2;
    u8 unpauseFlag;
    ZunVec3 facingDirInterpInitial;
    ZunVec3 facingDirInterpFinal;
    i32 facingDirInterpDuration;
    ZunTimer facingDirInterpTimer;
    ZunVec3 positionInterpFinal;
    i32 positionInterpEndTime;
    ZunVec3 positionInterpInitial;
    i32 positionInterpStartTime;
};
ZUN_ASSERT_SIZE(Stage, 0x2f4);

extern Stage g_Stage;
}; // namespace th06
