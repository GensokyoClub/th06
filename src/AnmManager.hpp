#pragma once

// #include <d3d8.h>
// #include <d3dx8math.h>

#include <SDL2/SDL_video.h>

#include "AnmIdx.hpp"
#include "AnmVm.hpp"
#include "GLFunc.hpp"
#include "GameManager.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"

#define TEX_FMT_UNKNOWN 0
#define TEX_FMT_A8R8G8B8 1
#define TEX_FMT_A1R5G5B5 2
#define TEX_FMT_R5G6B5 3
#define TEX_FMT_R8G8B8 4
#define TEX_FMT_A4R4G4B4 5

namespace th06
{
struct TextureData
{
    GLuint handle;
    void *fileData;

    // Fields needed to compensate for inability to read back texture for alpha loading
    u8 *textureData;
    u32 width;
    u32 height;
    i32 format;
};

// Endian-neutral version of ZunColor, for use with OpenGL
struct ColorData
{
    GLubyte r;
    GLubyte g;
    GLubyte b;
    GLubyte a;

    ColorData()
    {
    }

    ColorData(ZunColor color)
    {
        a = (color >> 24);
        r = (color >> 16) & 0xFF;
        g = (color >> 8) & 0xFF;
        b = color & 0xFF;
    };
};
static_assert(sizeof(ColorData) == 0x04, "ColorData has additional padding between struct members");

// NOTE: Every usage of a position with RHW in EoSD simply sets RHW to 1.0f
// D3D8 interprets vertices with D3DFVF_XYZRHW as having already been transformed, so Zun
// uses RHW simply to draw polygons in an orthographic manner
// This has to be worked around, since OpenGL does transform vertices with vec4 positions
// With the workaround done, all uses of XYZRHW vertices should be replaceable with XYZ vertices

// structure of a vertex with SetVertexShade FVF set to D3DFVF_DIFFUSE | D3DFVF_XYZRHW
struct VertexDiffuseXyzrhw
{
    ZunVec4 position;
    ColorData diffuse;
};

// Structure of a vertex with SetVertexShade FVF set to D3DFVF_TEX1 | D3DFVF_XYZRHW
struct VertexTex1Xyzrhw
{
    ZunVec4 position;
    ZunVec2 textureUV;
};

// Structure of a vertex with SetVertexShade FVF set to D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW
struct VertexTex1DiffuseXyzrhw
{
    ZunVec4 position;
    ColorData diffuse;
    ZunVec2 textureUV;
};

// Structure of a vertex with SetVertexShade FVF set to D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZ
struct VertexTex1DiffuseXyz
{
    ZunVec3 position;
    ColorData diffuse;
    ZunVec2 textureUV;
};

struct VertexTex1Xy
{
    ZunVec2 position;
    ZunVec2 textureUV;
};

struct AnmRawSprite
{
    u32 id;
    ZunVec2 offset;
    ZunVec2 size;
};

struct AnmRawScript
{
    u32 id;
    AnmRawInstr *firstInstruction;
};

// WARNING: scripts seems unused, but if it were to be used,
//   this would be dangerous for compatibility since AnmRawScript contains a pointer

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
    u32 alphaNameOffset;
    u32 version;
    u32 unk1;
    u32 textureOffset;
    u32 hasData;
    u32 nextOffset;
    u32 unk2;
    u32 spriteOffsets[10];
    AnmRawScript scripts[10];
};
ZUN_ASSERT_SIZE(AnmRawEntry, 0xb8);

struct RenderVertexInfo
{
    ZunVec3 position;
    ZunVec2 textureUV;
};
ZUN_ASSERT_SIZE(RenderVertexInfo, 0x14);

struct AnmManager
{
    AnmManager();
    ~AnmManager();

    //    void ReleaseVertexBuffer();
    void SetupVertexBuffer();

    bool CreateEmptyTexture(i32 textureIdx, u32 width, u32 height, i32 textureFormat);
    bool LoadTexture(i32 textureIdx, char *textureName, i32 textureFormat, ZunColor colorKey);
    bool LoadTextureAlphaChannel(i32 textureIdx, char *textureName, i32 textureFormat, ZunColor colorKey);

    void ReleaseTexture(i32 textureIdx);
    void TakeScreenshotIfRequested();
    void TakeScreenshot(i32 textureId, i32 left, i32 top, i32 width, i32 height);

    void SetAndExecuteScript(AnmVm *vm, AnmRawInstr *beginingOfScript);
    void SetAndExecuteScriptIdx(AnmVm *vm, i32 anmFileIdx)
    {
        vm->anmFileIndex = anmFileIdx;
        this->SetAndExecuteScript(vm, this->scripts[anmFileIdx]);
    }

    void InitializeAndSetSprite(AnmVm *vm, i32 spriteIdx)
    {
        vm->Initialize();
        this->SetActiveSprite(vm, spriteIdx);
    }

    void SetCurrentVertexShader(u8 vertexShader)
    {
        this->currentVertexShader = vertexShader;
    }
    void SetCurrentColorOp(u8 colorOp)
    {
        this->currentColorOp = colorOp;
    }
    void SetCurrentBlendMode(u8 blendMode)
    {
        this->currentBlendMode = blendMode;
    }
    void SetCurrentZWriteDisable(u8 zwriteDisable)
    {
        this->currentZWriteDisable = zwriteDisable;
    }
    void SetCurrentTexture(GLuint textureHandle)
    {
        if (this->currentTextureHandle != textureHandle)
        {
            this->currentTextureHandle = textureHandle;
            g_glFuncTable.glBindTexture(GL_TEXTURE_2D, textureHandle);
        }
    }
    void SetCurrentSprite(AnmLoadedSprite *sprite)
    {
        this->currentSprite = sprite;
    }

    i32 ExecuteScript(AnmVm *vm);
    bool Draw(AnmVm *vm);
    void DrawTextToSprite(u32 spriteDstIndex, i32 xPos, i32 yPos, i32 spriteWidth, i32 spriteHeight, i32 fontWidth,
                          i32 fontHeight, ZunColor textColor, ZunColor shadowColor, const char *strToPrint);
    static void DrawStringFormat(AnmManager *mgr, AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt, ...);
    static void DrawStringFormat2(AnmManager *mgr, AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt, ...);
    static void DrawVmTextFmt(AnmManager *anm_mgr, AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt, ...);
    bool DrawNoRotation(AnmVm *vm);
    bool DrawOrthographic(AnmVm *vm, bool roundToPixel);
    bool DrawFacingCamera(AnmVm *vm);
    bool Draw2(AnmVm *vm);
    bool Draw3(AnmVm *vm);

    void LoadSprite(u32 spriteIdx, AnmLoadedSprite *sprite);
    bool SetActiveSprite(AnmVm *vm, u32 spriteIdx);

    void ReleaseSurfaces(void);
    bool LoadSurface(i32 surfaceIdx, const char *path);
    void ReleaseSurface(i32 surfaceIdx);
    void CopySurfaceToBackBuffer(i32 surfaceIdx, i32 left, i32 top, i32 x, i32 y);
    void CopySurfaceRectToBackBuffer(i32 surfaceIdx, i32 rectX, i32 rectY, i32 rectLeft, i32 rectTop, i32 width,
                                     i32 height);

    void TranslateRotation(VertexTex1Xyzrhw *param_1, float x, float y, float sine, float cosine, float xOffset,
                           float yOffset);

    void ReleaseAnm(i32 anmIdx);
    bool LoadAnm(i32 anmIdx, const char *path, i32 spriteIdxOffset);
    void ExecuteAnmIdx(AnmVm *vm, i32 anmFileIdx)
    {
        vm->anmFileIndex = anmFileIdx;
        vm->pos = ZunVec3(0, 0, 0);
        vm->posOffset = ZunVec3(0, 0, 0);

        vm->fontHeight = 15;
        vm->fontWidth = 15;

        this->SetAndExecuteScript(vm, this->scripts[anmFileIdx]);
    }

    void SetRenderStateForVm(AnmVm *vm);

    void RequestScreenshot()
    {
        this->screenshotTextureId = 3;
        this->screenshotLeft = GAME_REGION_LEFT;
        this->screenshotTop = GAME_REGION_TOP;
        this->screenshotWidth = GAME_REGION_WIDTH;
        this->screenshotHeight = GAME_REGION_HEIGHT;
    }

    static SDL_Surface *LoadToSurfaceWithFormat(const char *filename, SDL_PixelFormatEnum format, u8 **fileData);
    static u8 *ExtractSurfacePixels(SDL_Surface *src, u8 pixelDepth);
    static void FlipSurface(SDL_Surface *surface);
    void ApplySurfaceToColorBuffer(SDL_Surface *src, const SDL_Rect &srcRect, const SDL_Rect &dstRect);
    // Creates, binds, and set parameters for a new texture
    void CreateTextureObject();

    AnmLoadedSprite sprites[2048];
    AnmVm virtualMachine;
    //    GLuint textures[264];
    //    void *imageDataArray[256];
    TextureData textures[264];
    i32 maybeLoadedSpriteCount;
    AnmRawInstr *scripts[2048];
    i32 spriteIndices[2048];
    AnmRawEntry *anmFiles[128];
    u32 anmFilesSpriteIndexOffsets[128];
    SDL_Surface *surfaces[32];
    //    SDL_Surface *surfacesBis[32];
    //    D3DXIMAGE_INFO surfaceSourceInfo[32];
    ZunColor currentTextureFactor;
    GLuint currentTextureHandle;
    GLuint dummyTextureHandle;
    u8 currentBlendMode;
    u8 currentColorOp;
    u8 currentVertexShader;
    u8 currentZWriteDisable;
    AnmLoadedSprite *currentSprite;
    //    IDirect3DVertexBuffer8 *vertexBuffer;
    RenderVertexInfo vertexBufferContents[4];
    i32 screenshotTextureId;
    i32 screenshotLeft;
    i32 screenshotTop;
    i32 screenshotWidth;
    i32 screenshotHeight;
};
ZUN_ASSERT_SIZE(AnmManager, 0x2112c);

extern AnmManager *g_AnmManager;
}; // namespace th06
