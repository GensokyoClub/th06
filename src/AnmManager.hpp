#pragma once
#include <d3d8.h>
#include <d3dx8math.h>

#include "AnmIdx.hpp"
#include "AnmVm.hpp"
#include "Supervisor.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"

namespace th06
{
// structure of a vertex with SetVertexShade FVF set to D3DFVF_DIFFUSE | D3DFVF_XYZRHW
struct VertexDiffuseXyzrwh
{
    D3DXVECTOR3 position;
    float position_w;
    D3DCOLOR diffuse;
};
ZUN_ASSERT_TYPE(VertexDiffuseXyzrwh, 0x14, 4);

// Structure of a vertex with SetVertexShade FVF set to D3DFVF_TEX1 | D3DFVF_XYZRHW
struct VertexTex1Xyzrwh
{
    D3DXVECTOR3 position;
    float position_w;
    float texture_u;
    float texture_v;
};
ZUN_ASSERT_TYPE(VertexTex1Xyzrwh, 0x18, 4);

// Structure of a vertex with SetVertexShade FVF set to D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW
struct VertexTex1DiffuseXyzrwh
{
    D3DXVECTOR3 position;
    float position_w;
    D3DCOLOR diffuse;
    float texture_u;
    float texture_v;
};
ZUN_ASSERT_TYPE(VertexTex1DiffuseXyzrwh, 0x1c, 4);

// Structure of a vertex with SetVertexShade FVF set to D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZ
struct VertexTex1DiffuseXyz
{
    D3DXVECTOR3 position;
    D3DCOLOR diffuse;
    float texture_u;
    float texture_v;
};
ZUN_ASSERT_TYPE(VertexTex1DiffuseXyz, 0x18, 4);

struct RenderVertexInfo
{
    D3DXVECTOR3 position;
    float texture_u;
    float texture_v;
};
ZUN_ASSERT_TYPE(RenderVertexInfo, 0x14, 4);

struct AnmRawSprite
{
    u32 id;
    ZunVec2 offset;
    ZunVec2 size;
};
ZUN_ASSERT_TYPE(AnmRawSprite, 0x14, 4);

struct AnmRawScript
{
    u32 id;
    u32 firstInstructionOffset;
};
ZUN_ASSERT_TYPE(AnmRawScript, 8, 4);

struct AnmRawEntry
{
    i32 numSprites;
    i32 numScripts;
    u32 textureIdx;
    i32 width;
    i32 height;
    u32 format;
    u32 colorKey;
    u32 nameOffset;
    u32 spriteIdxOffset;
    u32 mipmapNameOffset;
    u32 version;
    unreferenced_fields(0x4);
    u32 textureOffset;
    u32 hasData;
    u32 nextOffset;
    unreferenced_fields(0x4);
    u32 spriteOffsets[];
};
ZUN_ASSERT_TYPE(AnmRawEntry, 0x40, 4);

#define MAX_ANM_SCRIPTS 2048
#define MAX_ANM_SPRITES 2048
#if BUILT_SINCE(100)
#define MAX_ANM_FILES 128
#else
#define MAX_ANM_FILES 64
#endif

struct AnmManager
{
  public:
    AnmManager();
    ~AnmManager();

    void ReleaseVertexBuffer();
    void SetupVertexBuffer();

    ZunResult CreateEmptyTexture(i32 textureIdx, u32 width, u32 height, i32 textureFormat);
    ZunResult LoadTexture(i32 textureIdx, const char *textureName, i32 textureFormat, D3DCOLOR colorKey);
    ZunResult LoadTextureAlphaChannel(i32 textureIdx, const char *textureName, i32 textureFormat, D3DCOLOR colorKey);
    void ReleaseTexture(i32 textureIdx);

    void ReleaseSurfaces(void)
    {
        for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->surfaces); i++)
        {
            SAFE_RELEASE(this->surfaces[i]);
        }
    }

    void TakeScreenshotIfRequested()
    {
        if (this->screenshotTextureId >= 0)
        {
            this->TakeScreenshot(this->screenshotTextureId, this->screenshotLeft, this->screenshotTop,
                                 this->screenshotWidth, this->screenshotHeight);
            this->screenshotTextureId = -1;
        }
    }

    void TakeScreenshot(i32 textureId, i32 left, i32 top, i32 width, i32 height);

    void InitializeAndSetSprite(AnmVm *vm, i32 spriteIdx)
    {
        vm->Initialize();
        this->SetActiveSprite(vm, spriteIdx);
    }

    void SetAndExecuteScript(AnmVm *vm, AnmRawInstr *beginingOfScript);
    void SetAndExecuteScriptIdx(AnmVm *vm, i32 anmFileIdx)
    {
        vm->anmFileIndex = anmFileIdx;
        this->SetAndExecuteScript(vm, this->scripts[anmFileIdx]);
    }

    void ClearScriptRange(i32 base, i32 range)
    {
        for (i32 i = 0; i < range; i++)
        {
            this->scripts[i + base] = NULL;
        }
    }

    ZunBool ShouldDraw(AnmVm *vm)
    {
        if (vm->sprite == NULL)
        {
            return false;
        }
        if (vm->sprite->sourceFileIndex < 0)
        {
            return false;
        }
        return this->textures[vm->sprite->sourceFileIndex] != NULL;
    }

    void ClearBlendMode()
    {
        this->currentBlendMode = AnmBlendMode_NotSet;
    }
    void ClearColorOp()
    {
        this->currentColorOp = AnmColorOp_NotSet;
    }
    void ClearVertexShader()
    {
        this->currentVertexShader = AnmVertexShader_NotSet;
    }
    void ClearTexture()
    {
        this->currentTexture = NULL;
    }
    void ClearZWrite()
    {
        this->currentZWriteDisable = AnmZWriteState_NotSet;
    }
    void ClearSprite()
    {
        this->currentSprite = NULL;
    }

    i32 ExecuteScript(AnmVm *vm);
    ZunResult Draw(AnmVm *vm);
    void DrawTextToSprite(u32 spriteDstIndex, i32 xPos, i32 yPos, i32 spriteWidth, i32 spriteHeight, i32 fontWidth,
                          i32 fontHeight, ZunColor textColor, ZunColor shadowColor, const char *strToPrint);
    void DrawStringFormat(AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt, ...);
    void DrawStringFormat2(AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt, ...);
    void DrawVmTextFmt(AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt, ...);
    ZunResult DrawNoRotation(AnmVm *vm);

#define RENDER_VERTICES_DEFAULT false
#define RENDER_VERTICES_ROUND_INPUTS true
    ZunResult DrawInner(AnmVm *vm, ZunBool roundVertices);
    ZunResult DrawFacingCamera(AnmVm *vm);
    ZunResult Draw2(AnmVm *vm);
    ZunResult Draw3(AnmVm *vm);

    void LoadSprite(u32 spriteIdx, AnmLoadedSprite *sprite);
    ZunResult SetActiveSprite(AnmVm *vm, i32 spriteIdx);

    ZunResult LoadSurface(i32 surfaceIdx, const char *path);
    void ReleaseSurface(i32 surfaceIdx);
    void CopySurfaceToBackBuffer(i32 surfaceIdx, i32 left, i32 top, i32 x, i32 y);
    void DrawEndingRect(i32 surfaceIdx, i32 rectX, i32 rectY, i32 rectLeft, i32 rectTop, i32 width, i32 height);

    void TranslateRotation(VertexTex1Xyzrwh *param_1, float x, float y, float sine, float cosine, float xOffset,
                           float yOffset);

    void ReleaseAnm(i32 anmIdx);
    ZunResult LoadAnm(i32 anmIdx, const char *path, i32 spriteIdxOffset);
    void ExecuteAnmIdx(AnmVm *vm, i32 anmFileIdx)
    {
        vm->anmFileIndex = anmFileIdx;
        vm->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->fontWidth = vm->fontHeight = DEFAULT_ANM_FONT_SIZE;

        this->SetAndExecuteScript(vm, this->scripts[anmFileIdx]);
    }

    void SetRenderStateForVm(AnmVm *vm);

    void RequestScreenshot(i32 textureId = 3, i32 left = GAME_REGION_POS_X, i32 top = GAME_REGION_POS_Y,
                           i32 width = GAME_REGION_WIDTH, i32 height = GAME_REGION_HEIGHT)
    {
        this->screenshotTextureId = textureId;
        this->screenshotLeft = left;
        this->screenshotTop = top;
        this->screenshotWidth = width;
        this->screenshotHeight = height;
    }

    AnmLoadedSprite *GetSprite(i32 index)
    {
        return &this->sprites[index];
    }

  private:
    AnmLoadedSprite sprites[MAX_ANM_SPRITES];
    AnmVm virtualMachine;
    LPDIRECT3DTEXTURE8 textures[264];
    void *imageDataArray[256];
    i32 maybeLoadedSpriteCount;
    AnmRawInstr *scripts[MAX_ANM_SCRIPTS];
    i32 spriteIndices[MAX_ANM_SPRITES];
    AnmRawEntry *anmFiles[MAX_ANM_FILES];
    u32 anmFilesSpriteIndexOffsets[MAX_ANM_FILES];
    LPDIRECT3DSURFACE8 surfaces[32];
    LPDIRECT3DSURFACE8 surfacesBis[32];
    D3DXIMAGE_INFO surfaceSourceInfo[32];
    D3DCOLOR currentTextureFactor;
    LPDIRECT3DTEXTURE8 currentTexture;
    u8 currentBlendMode;
    u8 currentColorOp;
    u8 currentVertexShader;
    u8 currentZWriteDisable;
    AnmLoadedSprite *currentSprite;
    LPDIRECT3DVERTEXBUFFER8 vertexBuffer;
    RenderVertexInfo vertexBufferContents[4];
    i32 screenshotTextureId;
    i32 screenshotLeft;
    i32 screenshotTop;
    i32 screenshotWidth;
    i32 screenshotHeight;
};
#if BUILT_SINCE(100)
ZUN_ASSERT_TYPE(AnmManager, 0x2112c, 4);
#else
ZUN_ASSERT_TYPE(AnmManager, 0x20f2c, 4);
#endif

extern AnmManager *g_AnmManager;
} // namespace th06
