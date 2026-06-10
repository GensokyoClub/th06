#pragma once

#include "AnmVm.hpp"
#include "Chain.hpp"
#include "ZunTimer.hpp"
#include "inttypes.hpp"
// #include "zwave.hpp"
// #include <d3d8.h>
// #include <d3dx8math.h>

struct RawStageHeader
{
    LE<i16> nbObjects;
    LE<i16> nbFaces;
    LE<i32> facesOffset;
    LE<i32> scriptOffset;
    LE<i32> unk_c;
    char stageName[128];
    char songNames[4][128];
    char songPaths[4][128];
};

struct RawStageQuadBasic
{
    LE<i16> type;
    LE<i16> byteSize;
    LE<i16> anmScript;
    LE<i16> vmIdx;
    ZunVec3Raw position;
    ZunVec2Raw size;
};

struct RawStageObject
{
    LE<i16> id;
    i8 zLevel;
    i8 flags;
    ZunVec3Raw position;
    ZunVec3Raw size;
    RawStageQuadBasic firstQuad;
};

struct RawStageObjectInstance
{
    LE<i16> id;
    LE<i16> unk2;
    ZunVec3Raw position;
};

struct RawStageInstr
{
    LE<i32> frame;
    LE<i16> opcode;
    LE<i16> size;
    i32 args[3];
};

struct StageCameraSky
{
    f32 nearPlane;
    f32 farPlane;
    ZunColor color;
};

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
    static ZunResult RegisterChain(u32 stage);
    static void CutChain();
    static ChainCallbackResult OnUpdate(Stage *stage);
    static ChainCallbackResult OnDrawHighPrio(Stage *stage);
    static ChainCallbackResult OnDrawLowPrio(Stage *stage);
    static ZunResult AddedCallback(Stage *stage);
    static ZunResult DeletedCallback(Stage *stage);

    ZunResult LoadStageData(const char *anmpath, const char *stdpath);
    ZunResult UpdateObjects();
    ZunResult RenderObjects(i32 zLevel);

    AnmVm *quadVms;
    const RawStageHeader *stdData;
    i32 quadCount;
    i32 objectsCount;
    RawStageObject **objects;
    const RawStageObjectInstance *objectInstances;
    const RawStageInstr *beginningOfScript;
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

extern Stage g_Stage;
