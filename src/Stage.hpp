#pragma once
#include "AnmVm.hpp"
#include "Chain.hpp"
#include "decomp.hpp"
#include <d3d8.h>
#include <d3dx8math.h>

namespace th06
{
struct RawStageHeader
{
    i16 nbObjects;
    i16 nbFaces;
    i32 facesOffset;
    i32 scriptOffset;
    unreferenced_fields(0x4);
    char stageName[128];
    char songNames[4][128];
    char songPaths[4][128];
};
ZUN_ASSERT_TYPE(RawStageHeader, 0x490, 4);

struct RawStageQuadBasic
{
    i16 type;
    i16 byteSize;
    i16 anmScript;
    i16 vmIdx;
    D3DXVECTOR3 position;
    ZunVec2 size;
};
ZUN_ASSERT_TYPE(RawStageQuadBasic, 0x1c, 4);

struct RawStageObject
{
    i16 id;
    i8 zLevel;
    i8 flags;
    D3DXVECTOR3 position;
    D3DXVECTOR3 size;
    // Records end at a negative type and are stepped by byteSize.
    RawStageQuadBasic quads[];
};
ZUN_ASSERT_TYPE(RawStageObject, 0x1c, 4);

struct RawStageObjectInstance
{
    i16 id;
    alignment_padding(0x2);
    D3DXVECTOR3 position;
};
ZUN_ASSERT_TYPE(RawStageObjectInstance, 0x10, 4);

struct StdRawInstr
{
    i32 time;
    i16 opcode;
    i16 size;
    i32 args[3]; // Args are fixed at 3
};
ZUN_ASSERT_TYPE(StdRawInstr, 0x14, 4);

struct StageSky
{
    f32 nearPlane;
    f32 farPlane;
    D3DCOLOR fogColor;
};
ZUN_ASSERT_TYPE(StageSky, 0xc, 4);

enum SpellcardState
{
    SPELL_STATE_NOT_ACTIVE,
    SPELL_STATE_FADING_IN,
    SPELL_STATE_ACTIVE
};

#define SPELL_BACKGROUND_FADE_IN_DURATION 60

struct StageFile
{
    const char *anmFile;
    const char *stdFile;
};
ZUN_ASSERT_TYPE(StageFile, 0x8, 4);

enum StdOpcode
{
    STD_OPCODE_CAMERA_POSITION_KEY,
    STD_OPCODE_FOG,
    STD_OPCODE_CAMERA_FACING,
    STD_OPCODE_CAMERA_FACING_INTERP_LINEAR,
    STD_OPCODE_FOG_INTERP,
    STD_OPCODE_STD_PAUSE,
};

struct Stage
{
    ZunResult LoadStageData(const char *anmpath, const char *stdpath);
    ZunResult UpdateObjects();
    ZunResult RenderObjects(i32 zLevel);

    AnmVm *quadVms;
    RawStageHeader *stdData;
    i32 quadCount;
    i32 objectsCount;
    RawStageObject **objects;
    RawStageObjectInstance *objectInstances;
    StdRawInstr *beginningOfScript;
    ZunTimer scriptTime;
    i32 instructionIndex;
    ZunTimer timer;
    u32 stage;
    D3DXVECTOR3 position;
    StageSky sky;
    StageSky skyInterpInitial;
    StageSky skyInterpFinal;
    i32 skyInterpDuration;
    ZunTimer skyInterpTime;
    i8 skyNeedsSetup;
    alignment_padding(0x3);
    i32 spellcardState;
    i32 spellcardTicks;
    AnmVm spellcardBackground;
    unused_field(AnmVm);
    u8 unpauseFlag;
    alignment_padding(0x3);
    D3DXVECTOR3 facingInterpInitial;
    D3DXVECTOR3 facingInterpFinal;
    i32 facingInterpDuration;
    ZunTimer facingInterpTime;
    D3DXVECTOR3 positionInterpFinal;
    i32 positionInterpEndTime;
    D3DXVECTOR3 positionInterpInitial;
    i32 positionInterpStartTime;
};
ZUN_ASSERT_TYPE(Stage, 0x2f4, 4);

ZunResult Stage_RegisterChain(u32 stage);
void Stage_CutChain();

extern Stage g_Stage;
} // namespace th06
