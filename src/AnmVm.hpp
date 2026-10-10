#pragma once
#include <d3d8.h>
#include <d3dx8math.h>

#include "ZunColor.hpp"
#include "ZunMath.hpp"
#include "ZunResult.hpp"
#include "ZunTimer.hpp"
#include "decomp.hpp"

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
ZUN_ASSERT_TYPE(AnmLoadedSprite, 0x38, 4);

enum AnmOpcode
{
    ANM_OPCODE_ANM_DELETE,
    ANM_OPCODE_SET_SPRITE,
    ANM_OPCODE_SCALE,
    ANM_OPCODE_ALPHA,
    ANM_OPCODE_COLOR,
    ANM_OPCODE_JUMP,
    ANM_OPCODE_NOP,
    ANM_OPCODE_SCALE_FLIP_X,
    ANM_OPCODE_SCALE_FLIP_Y,
    ANM_OPCODE_ROTATION,
    ANM_OPCODE_ROTATION_SPEED,
    ANM_OPCODE_SCALE_SPEED,
    ANM_OPCODE_ALPHA_INTERP_LINEAR,
    ANM_OPCODE_BLEND_MODE_ADDITIVE,
    ANM_OPCODE_BLEND_MODE_NORMAL,
    ANM_OPCODE_ANM_STATIC,
    ANM_OPCODE_SPRITE_SET_RAND,
    ANM_OPCODE_MOVE_POSITION,
    ANM_OPCODE_MOVE_POSITION_INTERP_LINEAR,
    ANM_OPCODE_MOVE_POSITION_INTERP_DECELERATE_SLOW,
    ANM_OPCODE_MOVE_POSITION_INTERP_ACCELERATE_SLOW,
    ANM_OPCODE_ANM_HALT,
    ANM_OPCODE_INTERRUPT_LABEL,
    ANM_OPCODE_ANCHOR_TOP_LEFT,
    ANM_OPCODE_ANM_HALT_INVISIBLE,
    ANM_OPCODE_POSITION_MODE,
    ANM_OPCODE_SET_AUTO_ROTATE,
    ANM_OPCODE_SCROLL_SET_X,
    ANM_OPCODE_SCROLL_SET_Y,
    ANM_OPCODE_ANM_FLAG_VISIBLE,
    ANM_OPCODE_SCALE_INTERP_LINEAR,
    ANM_OPCODE_FLAG_DISABLE_Z_WRITE
};

struct AnmRawInstr
{
    i16 time;
    u8 opcode;
    u8 argsSize;
    unsigned char args[];
};
ZUN_ASSERT_TYPE(AnmRawInstr, 0x4, 2);

enum AnmVmFlagsEnum
{
    AnmVmFlags_Visible = 1 << 0,
    AnmVmFlags_VisibleOverride = 1 << 1,
    AnmVmFlags_BlendMode = 1 << 2,
    AnmVmFlags_ColorOp = 1 << 3,
    AnmVmFlags_4 = 1 << 4,
    AnmVmFlags_UsePosOffset = 1 << 5,
    AnmVmFlags_FlipX = 1 << 6,
    AnmVmFlags_FlipY = 1 << 7,
    AnmVmFlags_AnchorLeft = 1 << 8,
    AnmVmFlags_AnchorTop = 1 << 9,
    /* moveInterpMode missing because it is not really a flag */
    AnmVmFlags_ZWriteDisable = 1 << 12,
    AnmVmFlags_IsStopped = 1 << 13,
};

enum AnmBlendMode
{
    AnmBlendMode_NotSet = -1,
    AnmBlendMode_Normal,
    AnmBlendMode_Additive,
};

enum AnmColorOp
{
    AnmColorOp_NotSet = -1,
    AnmColorOp_Modulate,
    AnmColorOp_Add,
};

enum AnmZWriteState
{
    AnmZWriteState_NotSet = -1,
    AnmZWriteState_On = false,
    AnmZWriteState_Off = true,
};

enum AnmVmMirror
{
    AnmVmMirror_None,
    AnmVmMirror_X,
    AnmVmMirror_Y
};

enum AnmVmAnchor
{
    AnmVmAnchor_Center,
    AnmVmAnchor_Left,
    AnmVmAnchor_Top,
    AnmVmAnchor_TopLeft,
};

enum AnmVmInterpMode
{
    AnmVmInterp_Linear,
    AnmVmInterp_DecelerateSlow,
    AnmVmInterp_AccelerateSlow,
};

enum AnmVertexShader
{
    AnmVertexShader_NotSet = -1,
    AnmVertexShader_0,
    AnmVertexShader_1,
    AnmVertexShader_2,
    AnmVertexShader_3,
};

union AnmVmFlags {
    u16 flags;
    struct
    {
        u32 isVisible : 1;
        u32 isVisibleOverride : 1; // Intended for the engine to override visibility set by scripts
        u32 blendMode : 1;         // AnmBlendMode
        u32 colorOp : 1;           // AnmColorOp
        unreferenced_bitfields(u32, 1);
        u32 usePosOffset : 1;
        u32 flip : 2;           // AnmVmMirror
        u32 anchor : 2;         // AnmVmAnchor
        u32 moveInterpMode : 2; // AnmVmInterpMode
        u32 zWriteDisable : 1;
        u32 isStopped : 1;
        alignment_bitfields(u32, 18);
    };
};
ZUN_ASSERT_TYPE(AnmVmFlags, 0x4, 4);

struct AnmVmBase
{
    ZunVec3 rotation;
    ZunVec3 angleVel;
    f32 scaleY;
    f32 scaleX;
    f32 scaleInterpFinalY;
    f32 scaleInterpFinalX;
    ZunVec2 uvScrollPos;
    ZunTimer currentTimeInScript;
    D3DXMATRIX matrix;
    ZunColor color;
    AnmVmFlags flags;
    i16 alphaInterpEndTime;
    i16 scaleInterpEndTime;
    i16 autoRotate;
    i16 pendingInterrupt;
    i16 posInterpEndTime;
    alignment_padding(0x2);
};
ZUN_ASSERT_TYPE(AnmVmBase, 0x90, 4);

#define DEFAULT_ANM_FONT_SIZE 15

struct AnmVm : AnmVmBase
{
    void Initialize()
    {
        this->uvScrollPos.x = this->uvScrollPos.y = 0.0f;
        this->scaleInterpFinalY = this->scaleInterpFinalX = 0.0f;
        this->angleVel.x = this->angleVel.y = this->angleVel.z = 0.0f;
        this->rotation.x = this->rotation.y = this->rotation.z = 0.0f;
        this->scaleY = this->scaleX = 1.0f;
        this->scaleInterpEndTime = 0;
        this->alphaInterpEndTime = 0;
        this->color = COLOR_WHITE;
        D3DXMatrixIdentity(&this->matrix);
        this->flags.flags = AnmVmFlags_Visible | AnmVmFlags_VisibleOverride;
        this->autoRotate = 0;
        this->pendingInterrupt = 0;
        this->posInterpEndTime = 0;
        this->currentTimeInScript.Initialize();
    }

    AnmVm()
    {
        this->activeSpriteIndex = -1;
    }

    ZunBool IsVisible()
    {
        return this->flags.isVisible;
    }

    void SetInvisible()
    {
        this->flags.isVisible = false;
    }

    ZunBool IsStopped()
    {
        return this->flags.isStopped;
    }

    void SetVisible()
    {
        this->flags.isVisible = true;
    }

    ZunBool IsVisibleOverride()
    {
        return this->flags.isVisibleOverride;
    }

    void SetVisibleOverride(ZunBool visible)
    {
        this->flags.isVisibleOverride = visible;
    }

    u32 GetBlendMode()
    {
        return this->flags.blendMode;
    }

    void SetBlendMode(u32 blendMode)
    {
        this->flags.blendMode = blendMode;
    }

    u32 GetColorOp()
    {
        return this->flags.colorOp;
    }

    void SetColorOp(u32 colorOp)
    {
        this->flags.colorOp = colorOp;
    }

    ZunBool UsesPosOffset()
    {
        return this->flags.usePosOffset;
    }

    void SetUsePosOffset(ZunBool usePosOffset)
    {
        this->flags.usePosOffset = usePosOffset;
    }

    u32 GetFlip()
    {
        return this->flags.flip;
    }

    void SetFlip(u32 flip)
    {
        this->flags.flip = flip;
    }

    u32 GetAnchor()
    {
        return this->flags.anchor;
    }

    void SetAnchor(u32 anchor)
    {
        this->flags.anchor = anchor;
    }

    u32 GetMoveInterpMode()
    {
        return this->flags.moveInterpMode;
    }

    void SetMoveInterpMode(u32 moveInterpMode)
    {
        this->flags.moveInterpMode = moveInterpMode;
    }

    ZunBool IsZWriteDisabled()
    {
        return this->flags.zWriteDisable;
    }

    void SetZWriteDisable(ZunBool zWriteDisable)
    {
        this->flags.zWriteDisable = zWriteDisable;
    }

    void SetStopped(ZunBool stopped)
    {
        this->flags.isStopped = stopped;
    }

    D3DXVECTOR3 pos;
    f32 scaleInterpInitialY;
    f32 scaleInterpInitialX;
    ZunTimer scaleInterpTime;
    i16 activeSpriteIndex;
    i16 baseSpriteIndex;
    i16 anmFileIndex;
    alignment_padding(0x2);
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
    alignment_padding(0x2);
};
ZUN_ASSERT_TYPE(AnmVm, 0x110, 4);
} // namespace th06
