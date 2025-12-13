#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

#include "ZunColor.hpp"
#include "ZunMath.hpp"
#include "ZunResult.hpp"
#include "ZunTimer.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"

namespace th06
{
struct AnmLoadedSprite
{
    i32 sourceFileIndex;
    ZunVec2 startPixelInclusive;
    ZunVec2 endPixelInclusive;
    f32 textureHeight;
    f32 textureWidth;
    ZunVec2 uvStart;
    ZunVec2 uvEnd;
    f32 heightPx;
    f32 widthPx;
    i32 spriteId;
};
ZUN_ASSERT_SIZE(AnmLoadedSprite, 0x38);

#define AnmOpcode_Exit 0
#define AnmOpcode_SetActiveSprite 1
#define AnmOpcode_SetScale 2
#define AnmOpcode_SetAlpha 3
#define AnmOpcode_SetColor 4
#define AnmOpcode_Jump 5
#define AnmOpcode_Nop 6
#define AnmOpcode_FlipX 7
#define AnmOpcode_FlipY 8
#define AnmOpcode_SetRotation 9
#define AnmOpcode_SetAngleVel 10
#define AnmOpcode_SetScaleSpeed 11
#define AnmOpcode_Fade 12
#define AnmOpcode_SetBlendAdditive 13
#define AnmOpcode_SetBlendDefault 14
#define AnmOpcode_ExitHide 15
#define AnmOpcode_SetRandomSprite 16
#define AnmOpcode_SetPosition 17
#define AnmOpcode_PosTimeLinear 18
#define AnmOpcode_PosTimeDecel 19
#define AnmOpcode_PosTimeAccel 20
#define AnmOpcode_Stop 21
#define AnmOpcode_InterruptLabel 22
#define AnmOpcode_AnchorTopLeft 23
#define AnmOpcode_StopHide 24
#define AnmOpcode_UsePosOffset 25
#define AnmOpcode_SetAutoRotate 26
#define AnmOpcode_UVScrollX 27
#define AnmOpcode_UVScrollY 28
#define AnmOpcode_SetVisibility 29
#define AnmOpcode_ScaleTime 30
#define AnmOpcode_SetZWriteDisable 31

struct AnmRawInstr
{
    i16 time;
    u8 opcode;
    u8 argsCount;
    u32 args[10];
};

enum AnmVmFlagsEnum
{
    AnmVmFlags_Visible = 1 << 0,
    AnmVmFlags_1 = 1 << 1,
    AnmVmFlags_BlendMode = 1 << 2,
    AnmVmFlags_ColorOp = 1 << 3,
    AnmVmFlags_4 = 1 << 4,
    AnmVmFlags_UsePosOffset = 1 << 5,
    AnmVmFlags_FlipX = 1 << 6,
    AnmVmFlags_FlipY = 1 << 7,
    AnmVmFlags_AnchorLeft = 1 << 8,
    AnmVmFlags_AnchorTop = 1 << 9,
    /* posTime missing because it is not really a flag */
    AnmVmFlags_ZWriteDisable = 1 << 12,
    AnmVmFlags_IsStopped = 1 << 13,
};

#define ANM_VM_INITIAL_FLAGS 0x3

enum AnmVmBlendMode
{
    AnmVmBlendMode_InvSrcAlpha,
    AnmVmBlendMode_One,
};

enum AnmVmColorOp
{
    AnmVmColorOp_Modulate,
    AnmVmColorOp_Add,
};

enum AnmVmAnchor
{
    AnmVmAnchor_Center,
    AnmVmAnchor_Left,
    AnmVmAnchor_Top,
    AnmVmAnchor_TopLeft,
};

union AnmVmFlags {
    u16 flags;
    struct
    {
        u32 isVisible : 1;
        u32 flag1 : 1;
        u32 blendMode : 1;
        u32 colorOp : 1;
        u32 flag4 : 1;
        u32 usePosOffset : 1;
        u32 flip : 2;
        u32 anchor : 2;
        u32 posTime : 2;
        u32 zWriteDisable : 1;
        u32 isStopped : 1;
    };
};

struct AnmVm
{
    void Initialize()
    {
        this->uvScrollPos.y = 0.0;
        this->uvScrollPos.x = 0.0;
        this->scaleInterpFinalX = 0.0;
        this->scaleInterpFinalY = 0.0;
        this->angleVel.z = 0.0;
        this->angleVel.y = 0.0;
        this->angleVel.x = 0.0;
        this->rotation.z = 0.0;
        this->rotation.y = 0.0;
        this->rotation.x = 0.0;
        this->scaleX = 1.0;
        this->scaleY = 1.0;
        this->scaleInterpEndTime = 0;
        this->alphaInterpEndTime = 0;
        this->color = D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0xff);
        D3DXMatrixIdentity(&this->matrix);
        this->flags.flags = AnmVmFlags_Visible | AnmVmFlags_1;
        this->autoRotate = 0;
        this->pendingInterrupt = 0;
        this->posInterpEndTime = 0;
        this->currentTimeInScript.Initialize();
    }

    AnmVm()
    {
        this->activeSpriteIndex = -1;
    }

    void SetInvisible()
    {
        this->flags.isVisible = 0;
    }

    D3DXVECTOR3 rotation;
    D3DXVECTOR3 angleVel;
    f32 scaleY;
    f32 scaleX;
    f32 scaleInterpFinalY;
    f32 scaleInterpFinalX;
    D3DXVECTOR2 uvScrollPos;
    ZunTimer currentTimeInScript;
    D3DXMATRIX matrix;
    ZunColor color;
    AnmVmFlags flags;

    i16 alphaInterpEndTime;
    i16 scaleInterpEndTime;
    i16 autoRotate;
    i16 pendingInterrupt;
    i16 posInterpEndTime;
    // Two padding bytes
    D3DXVECTOR3 pos;
    f32 scaleInterpInitialY;
    f32 scaleInterpInitialX;
    ZunTimer scaleInterpTime;
    i16 activeSpriteIndex;
    i16 baseSpriteIndex;
    i16 anmFileIndex;
    // Two padding bytes
    AnmRawInstr *beginingOfScript;
    AnmRawInstr *currentInstruction;
    AnmLoadedSprite *sprite;
    D3DCOLOR alphaInterpInitial;
    D3DCOLOR alphaInterpFinal;
    D3DXVECTOR3 posInterpInitial;
    D3DXVECTOR3 posInterpFinal;
    D3DXVECTOR3 posOffset;
    ZunTimer posInterpTime;
    i32 timeOfLastSpriteSet;
    ZunTimer alphaInterpTime;
    u8 fontWidth;
    u8 fontHeight;
    // Two final padding bytes
};
ZUN_ASSERT_SIZE(AnmVm, 0x110);
}; // namespace th06
