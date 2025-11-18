#pragma once

// #include <d3d8.h>
// #include <d3dx8math.h>

#include <SDL2/SDL_video.h>

#include "AnmIdx.hpp"
#include "AnmVm.hpp"
#include "GameManager.hpp"
#include "graphics/GfxInterface.hpp"
#include "graphics/GLFunc.hpp"
#include "ZunTimer.hpp"
#include "diffbuild.hpp"
#include "graphics/GLFunc.hpp"
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

enum ProjectionMode
{
    PROJECTION_MODE_PERSPECTIVE,
    PROJECTION_MODE_ORTHOGRAPHIC
};

struct VertexAttribArrayState
{
    void *ptr;
    std::size_t stride;
};

enum DirtyRenderStateBitShifts
{
    DIRTY_FOG = 0,
    DIRTY_DEPTH_CONFIG = 1,
    DIRTY_VERTEX_ATTRIBUTE_ENABLE = 2,
    DIRTY_VERTEX_ATTRIBUTE_ARRAY = 3,
    DIRTY_COLOR_OP = 4,
    DIRTY_TEXTURE_FACTOR = 5,
    DIRTY_MODEL_MATRIX = 6,
    DIRTY_VIEW_MATRIX = 7,
    DIRTY_PROJECTION_MATRIX = 8,
    DIRTY_TEXTURE_MATRIX = 9,
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
    // bool LoadTextureAlphaChannel(i32 textureIdx, char *textureName, i32 textureFormat, ZunColor colorKey);

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

    void BackendDrawCall()
    {
        if (this->dirtyFlags != 0)
        {
            this->UpdateDirtyStates();
        }

        g_glFuncTable.glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // We need to do checks in these because they're called nearly every ANM draw call and otherwise
    //   we'd be constantly setting the dirty flag. The other global state sets are done less often
    //   and so they don't need the checks until UpdateDirtyStates.

    void SetVertexAttributes(u8 vertexShader)
    {
        this->dirtyEnabledVertexAttributes = vertexShader;

        if (this->dirtyEnabledVertexAttributes == this->enabledVertexAttributes)
        {
            this->dirtyFlags &= ~(1 << DIRTY_VERTEX_ATTRIBUTE_ENABLE);
            return;
        }

        this->dirtyFlags |= (1 << DIRTY_VERTEX_ATTRIBUTE_ENABLE);
    }

    void SetColorOp(TextureOpComponent component, ColorOp op)
    {
        this->dirtyColorOps[component] = op;

        if ((g_Supervisor.cfg.opts >> GCOS_NO_COLOR_COMP) & 1 ||
            this->dirtyColorOps[component] == this->colorOps[component])
        {
            dirtyFlags &= ~(1 << DIRTY_COLOR_OP);
            return;
        }

        dirtyFlags |= (1 << DIRTY_COLOR_OP);
    }

    void SetCurrentBlendMode(u8 blendMode)
    {
        this->currentBlendMode = blendMode;
    }

    void SetDepthMask(bool depthEnable)
    {
        this->dirtyDepthMask = depthEnable;

        if ((g_Supervisor.cfg.opts >> GCOS_TURN_OFF_DEPTH_TEST) & 1 || this->dirtyDepthMask == this->depthMask)
        {
            return;
        }

        this->dirtyFlags |= (1 << DIRTY_DEPTH_CONFIG);
    }

    void SetDepthFunc(DepthFunc func)
    {
        if ((g_Supervisor.cfg.opts >> GCOS_TURN_OFF_DEPTH_TEST) & 1)
        {
            return;
        }

        this->dirtyDepthFunc = func;
        this->dirtyFlags |= (1 << DIRTY_DEPTH_CONFIG);
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

    void SetProjectionMode(ProjectionMode projectionMode)
    {
        if (this->projectionMode == projectionMode)
        {
            return;
        }

        this->projectionMode = projectionMode;

        if (projectionMode == PROJECTION_MODE_ORTHOGRAPHIC)
        {
            ZunMatrix identityMatrix;

            memcpy(this->perspectiveMatrixBackup, this->dirtyTransformMatrices, sizeof(this->perspectiveMatrixBackup));

            identityMatrix.Identity();

            this->SetTransformMatrix(MATRIX_VIEW, identityMatrix);
            this->SetTransformMatrix(MATRIX_TEXTURE, identityMatrix);

            ZunMatrix inverseMatrix = inverseViewportMatrix();

            this->SetTransformMatrix(MATRIX_PROJECTION, inverseMatrix);

            return;
        }

        g_Supervisor.viewport.Set();

        this->SetTransformMatrix(MATRIX_VIEW, this->perspectiveMatrixBackup[MATRIX_VIEW]);
        this->SetTransformMatrix(MATRIX_PROJECTION, this->perspectiveMatrixBackup[MATRIX_PROJECTION]);
    }

    void SetFogRange(f32 near, f32 far)
    {
        this->dirtyFogNear = near;
        this->dirtyFogFar = far;
        this->dirtyFlags |= (1 << DIRTY_FOG);
    }

    void SetFogColor(ZunColor color)
    {
        this->dirtyFogColor = color;
        this->dirtyFlags |= (1 << DIRTY_FOG);
    }

    void SetAttributePointer(VertexAttributeArrays attr, std::size_t stride, void *ptr)
    {
        this->dirtyAttribArrays[attr].ptr = ptr;
        this->dirtyAttribArrays[attr].stride = stride;

        if (!std::memcmp(&this->dirtyAttribArrays[attr], &this->attribArrays[attr], sizeof(*this->dirtyAttribArrays)))
        {
            return;
        }

        this->dirtyFlags |= (1 << DIRTY_VERTEX_ATTRIBUTE_ARRAY);
    }

    void SetTextureFactor(ZunColor factor)
    {
        this->dirtytTextureFactor = factor;

        if (this->dirtytTextureFactor == this->textureFactor)
        {
            this->dirtyFlags &= ~(1 << DIRTY_TEXTURE_FACTOR);
            return;
        }

        this->dirtyFlags |= 1 << DIRTY_TEXTURE_FACTOR;
    }

    void SetTransformMatrix(TransformMatrix type, ZunMatrix &matrix)
    {
        std::memcpy(&this->dirtyTransformMatrices[type], &matrix, sizeof(matrix));

        if (!std::memcmp(&this->transformMatrices[type], &matrix, sizeof(matrix)))
        {
            this->dirtyFlags &= ~(1 << (DIRTY_MODEL_MATRIX + (DirtyRenderStateBitShifts) type));
        }

        this->dirtyFlags |= 1 << (DIRTY_MODEL_MATRIX + (DirtyRenderStateBitShifts) type);
    }

    i32 ExecuteScript(AnmVm *vm);
    bool Draw(AnmVm *vm);
    void DrawTextToSprite(u32 spriteDstIndex, i32 xPos, i32 yPos, i32 spriteWidth, i32 spriteHeight, i32 fontWidth,
                          i32 fontHeight, ZunColor textColor, ZunColor shadowColor, const char *strToPrint);
    static void DrawStringFormat(AnmManager *mgr, AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt,
                                 ...);
    static void DrawStringFormat2(AnmManager *mgr, AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt,
                                  ...);
    static void DrawVmTextFmt(AnmManager *anm_mgr, AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt,
                              ...);
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
    void UpdateDirtyStates();

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
    GLuint currentTextureHandle;
    GLuint dummyTextureHandle;
    u8 currentBlendMode;
    ProjectionMode projectionMode;
    AnmLoadedSprite *currentSprite;
    //    IDirect3DVertexBuffer8 *vertexBuffer;
    RenderVertexInfo vertexBufferContents[4];
    i32 screenshotTextureId;
    i32 screenshotLeft;
    i32 screenshotTop;
    i32 screenshotWidth;
    i32 screenshotHeight;

    GfxInterface *gfxBackend;

  private:
    u32 dirtyFlags;
    f32 fogNear;
    f32 fogFar;
    ZunColor fogColor;
    bool depthMask;
    DepthFunc depthFunc;
    u8 enabledVertexAttributes;
    VertexAttribArrayState attribArrays[3];
    ColorOp colorOps[2];
    ZunColor textureFactor;
    ZunMatrix transformMatrices[4];

    f32 dirtyFogNear;
    f32 dirtyFogFar;
    ZunColor dirtyFogColor;
    bool dirtyDepthMask;
    DepthFunc dirtyDepthFunc;
    u8 dirtyEnabledVertexAttributes;
    VertexAttribArrayState dirtyAttribArrays[3];
    ColorOp dirtyColorOps[2];
    ZunColor dirtytTextureFactor;
    ZunMatrix dirtyTransformMatrices[4];


    ZunMatrix perspectiveMatrixBackup[4]; // Replaces matrix stack use for orthographic mode
};
ZUN_ASSERT_SIZE(AnmManager, 0x2112c);

extern AnmManager *g_AnmManager;
}; // namespace th06
