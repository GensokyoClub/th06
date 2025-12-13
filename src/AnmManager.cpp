#include "AnmManager.hpp"
#include "FileSystem.hpp"
#include "GameErrorContext.hpp"
#include "Rng.hpp"
#include "Supervisor.hpp"
#include "TextHelper.hpp"
#include "ZunMath.hpp"
#include "i18n.hpp"
#include "utils.hpp"

#include <stdio.h>

namespace th06
{
DIFFABLE_STATIC(VertexTex1Xyzrwh, g_PrimitivesToDrawVertexBuf[4]);
DIFFABLE_STATIC(VertexTex1DiffuseXyzrwh, g_PrimitivesToDrawNoVertexBuf[4]);
DIFFABLE_STATIC(VertexTex1DiffuseXyz, g_PrimitivesToDrawUnknown[4]);
DIFFABLE_STATIC(AnmManager *, g_AnmManager)

#ifndef DIFFBUILD
D3DFORMAT g_TextureFormatD3D8Mapping[6] = {
    D3DFMT_UNKNOWN, D3DFMT_A8R8G8B8, D3DFMT_A1R5G5B5, D3DFMT_R5G6B5, D3DFMT_R8G8B8, D3DFMT_A4R4G4B4,
};
#endif

#define TEX_FMT_UNKNOWN 0
#define TEX_FMT_A8R8G8B8 1
#define TEX_FMT_A1R5G5B5 2
#define TEX_FMT_R5G6B5 3
#define TEX_FMT_R8G8B8 4
#define TEX_FMT_A4R4G4B4 5

void AnmManager::ReleaseSurfaces(void)
{
    for (i32 idx = 0; idx < ARRAY_SIZE_SIGNED(this->surfaces); idx++)
    {
        if (this->surfaces[idx] != NULL)
        {
            this->surfaces[idx]->Release();
            this->surfaces[idx] = NULL;
        }
    }
}

void AnmManager::TakeScreenshotIfRequested()
{
    if (this->screenshotTextureId >= 0)
    {
        this->TakeScreenshot(this->screenshotTextureId, this->screenshotLeft, this->screenshotTop,
                             this->screenshotWidth, this->screenshotHeight);
        this->screenshotTextureId = -1;
    }
    return;
}

AnmManager::AnmManager()
{
    this->maybeLoadedSpriteCount = 0;

    memset(this, 0, sizeof(AnmManager));

    for (i32 spriteIndex = 0; spriteIndex < ARRAY_SIZE_SIGNED(this->sprites); spriteIndex++)
    {
        this->sprites[spriteIndex].sourceFileIndex = -1;
    }

    g_PrimitivesToDrawVertexBuf[3].position.w = 1.0;
    g_PrimitivesToDrawVertexBuf[2].position.w = g_PrimitivesToDrawVertexBuf[3].position.w;
    g_PrimitivesToDrawVertexBuf[1].position.w = g_PrimitivesToDrawVertexBuf[2].position.w;
    g_PrimitivesToDrawVertexBuf[0].position.w = g_PrimitivesToDrawVertexBuf[1].position.w;
    g_PrimitivesToDrawVertexBuf[0].textureUV.x = 0.0;
    g_PrimitivesToDrawVertexBuf[0].textureUV.y = 0.0;
    g_PrimitivesToDrawVertexBuf[1].textureUV.x = 1.0;
    g_PrimitivesToDrawVertexBuf[1].textureUV.y = 0.0;
    g_PrimitivesToDrawVertexBuf[2].textureUV.x = 0.0;
    g_PrimitivesToDrawVertexBuf[2].textureUV.y = 1.0;
    g_PrimitivesToDrawVertexBuf[3].textureUV.x = 1.0;
    g_PrimitivesToDrawVertexBuf[3].textureUV.y = 1.0;

    g_PrimitivesToDrawNoVertexBuf[3].position.w = 1.0;
    g_PrimitivesToDrawNoVertexBuf[2].position.w = g_PrimitivesToDrawNoVertexBuf[3].position.w;
    g_PrimitivesToDrawNoVertexBuf[1].position.w = g_PrimitivesToDrawNoVertexBuf[2].position.w;
    g_PrimitivesToDrawNoVertexBuf[0].position.w = g_PrimitivesToDrawNoVertexBuf[1].position.w;
    g_PrimitivesToDrawNoVertexBuf[0].textureUV.x = 0.0;
    g_PrimitivesToDrawNoVertexBuf[0].textureUV.y = 0.0;
    g_PrimitivesToDrawNoVertexBuf[1].textureUV.x = 1.0;
    g_PrimitivesToDrawNoVertexBuf[1].textureUV.y = 0.0;
    g_PrimitivesToDrawNoVertexBuf[2].textureUV.x = 0.0;
    g_PrimitivesToDrawNoVertexBuf[2].textureUV.y = 1.0;
    g_PrimitivesToDrawNoVertexBuf[3].textureUV.x = 1.0;
    g_PrimitivesToDrawNoVertexBuf[3].textureUV.y = 1.0;

    this->vertexBuffer = NULL;
    this->currentTexture = NULL;
    this->currentBlendMode = 0;
    this->currentColorOp = 0;
    this->currentTextureFactor = 1;
    this->currentVertexShader = 0;
    this->currentZWriteDisable = 0;
    this->screenshotTextureId = -1;
}

void AnmManager::SetupVertexBuffer()
{
    this->vertexBufferContents[2].position.x = -128;
    this->vertexBufferContents[0].position.x = -128;
    this->vertexBufferContents[3].position.x = 128;
    this->vertexBufferContents[1].position.x = 128;

    this->vertexBufferContents[1].position.y = -128;
    this->vertexBufferContents[0].position.y = -128;
    this->vertexBufferContents[3].position.y = 128;
    this->vertexBufferContents[2].position.y = 128;

    this->vertexBufferContents[3].position.z = 0;
    this->vertexBufferContents[2].position.z = 0;
    this->vertexBufferContents[1].position.z = 0;
    this->vertexBufferContents[0].position.z = 0;

    this->vertexBufferContents[2].textureUV.x = 0;
    this->vertexBufferContents[0].textureUV.x = 0;
    this->vertexBufferContents[3].textureUV.x = 1;
    this->vertexBufferContents[1].textureUV.x = 1;
    this->vertexBufferContents[1].textureUV.y = 0;
    this->vertexBufferContents[0].textureUV.y = 0;
    this->vertexBufferContents[3].textureUV.y = 1;
    this->vertexBufferContents[2].textureUV.y = 1;

    g_PrimitivesToDrawUnknown[0].position = this->vertexBufferContents[0].position;
    g_PrimitivesToDrawUnknown[1].position = this->vertexBufferContents[1].position;
    g_PrimitivesToDrawUnknown[2].position = this->vertexBufferContents[2].position;
    g_PrimitivesToDrawUnknown[3].position = this->vertexBufferContents[3].position;

    g_PrimitivesToDrawUnknown[0].textureUV.x = this->vertexBufferContents[0].textureUV.x;
    g_PrimitivesToDrawUnknown[0].textureUV.y = this->vertexBufferContents[0].textureUV.y;
    g_PrimitivesToDrawUnknown[1].textureUV.x = this->vertexBufferContents[1].textureUV.x;
    g_PrimitivesToDrawUnknown[1].textureUV.y = this->vertexBufferContents[1].textureUV.y;
    g_PrimitivesToDrawUnknown[2].textureUV.x = this->vertexBufferContents[2].textureUV.x;
    g_PrimitivesToDrawUnknown[2].textureUV.y = this->vertexBufferContents[2].textureUV.y;
    g_PrimitivesToDrawUnknown[3].textureUV.x = this->vertexBufferContents[3].textureUV.x;
    g_PrimitivesToDrawUnknown[3].textureUV.y = this->vertexBufferContents[3].textureUV.y;

    RenderVertexInfo *buffer;

    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0)
    {
        g_Supervisor.d3dDevice->CreateVertexBuffer(sizeof(this->vertexBufferContents), 0, D3DFVF_TEX1 | D3DFVF_XYZ,
                                                   D3DPOOL_MANAGED, &this->vertexBuffer);

        this->vertexBuffer->Lock(0, 0, (BYTE **)&buffer, 0);
        memcpy(buffer, this->vertexBufferContents, sizeof(this->vertexBufferContents));
        this->vertexBuffer->Unlock();

        g_Supervisor.d3dDevice->SetStreamSource(0, g_AnmManager->vertexBuffer, sizeof(RenderVertexInfo));
    }
}

ZunResult AnmManager::LoadTexture(i32 textureIdx, char *textureName, i32 textureFormat, D3DCOLOR colorKey)
{
    ReleaseTexture(textureIdx);
    this->imageDataArray[textureIdx] = FileSystem::OpenPath(textureName, 0);

    if (this->imageDataArray[textureIdx] == NULL)
    {
        return ZUN_ERROR;
    }

    if (((g_Supervisor.cfg.opts >> GCOS_FORCE_16BIT_COLOR_MODE) & 1) != 0)
    {
        if (g_TextureFormatD3D8Mapping[textureFormat] == D3DFMT_A8R8G8B8 ||
            g_TextureFormatD3D8Mapping[textureFormat] == D3DFMT_UNKNOWN)
        {
            textureFormat = TEX_FMT_A4R4G4B4;
        }
        else if (g_TextureFormatD3D8Mapping[textureFormat] == D3DFMT_R8G8B8)
        {
            textureFormat = TEX_FMT_R5G6B5;
        }
    }

    if (D3DXCreateTextureFromFileInMemoryEx(g_Supervisor.d3dDevice, this->imageDataArray[textureIdx], g_LastFileSize, 0,
                                            0, 0, 0, g_TextureFormatD3D8Mapping[textureFormat], D3DPOOL_MANAGED,
                                            D3DX_FILTER_NONE | D3DX_FILTER_POINT, D3DX_DEFAULT, colorKey, NULL, NULL,
                                            &this->textures[textureIdx]) != D3D_OK)
    {
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

#pragma var_order(surfaceDesc, data, lockedRectDst, lockedRectSrc, textureSrc, dstData0, srcData0, y0, x0, dstData1,   \
                  srcData1, y1, x1, dstData2, srcData2, y2, x2)
ZunResult AnmManager::LoadTextureAlphaChannel(i32 textureIdx, char *textureName, i32 textureFormat, D3DCOLOR colorKey)
{
    struct Argb1555Pixel
    {
        u16 b : 5;
        u16 g : 5;
        u16 r : 5;
        u16 a : 1;
    };

    struct Argb4444Pixel
    {
        u16 b : 4;
        u16 g : 4;
        u16 r : 4;
        u16 a : 4;
    };

    IDirect3DTexture8 *textureSrc;
    D3DSURFACE_DESC surfaceDesc;
    D3DLOCKED_RECT lockedRectDst;
    D3DLOCKED_RECT lockedRectSrc;
    u8 *data;

    u8 *dstData0;
    u8 *srcData0;
    i32 x0;
    i32 y0;
    Argb1555Pixel *dstData1;
    Argb1555Pixel *srcData1;
    i32 y1;
    i32 x1;
    Argb4444Pixel *dstData2;
    Argb4444Pixel *srcData2;
    i32 y2;
    i32 x2;

    textureSrc = NULL;
    data = FileSystem::OpenPath(textureName, 0);

    if (data == NULL)
    {
        return ZUN_ERROR;
    }

    this->textures[textureIdx]->GetLevelDesc(0, &surfaceDesc);

    if (surfaceDesc.Format != D3DFMT_A8R8G8B8 && surfaceDesc.Format != D3DFMT_A4R4G4B4 &&
        surfaceDesc.Format != D3DFMT_A1R5G5B5)
    {
        GameErrorContext::Fatal(&g_GameErrorContext, TH_ERR_ANMMANAGER_UNK_TEX_FORMAT);
        goto err;
    }

    if (D3DXCreateTextureFromFileInMemoryEx(g_Supervisor.d3dDevice, data, g_LastFileSize, 0, 0, 0, 0,
                                            surfaceDesc.Format, D3DPOOL_SYSTEMMEM, D3DX_FILTER_NONE | D3DX_FILTER_POINT,
                                            D3DX_DEFAULT, colorKey, NULL, NULL, &textureSrc) != D3D_OK)
    {
        goto err;
    }

    if (this->textures[textureIdx]->LockRect(0, &lockedRectDst, NULL, 0) != 0)
        goto err;

    if (textureSrc->LockRect(0, &lockedRectSrc, NULL, D3DLOCK_NO_DIRTY_UPDATE) != 0)
        goto err;

    // Copy over the alpha channel from the source to the destination, taking
    // into account the texture format.
    switch (surfaceDesc.Format)
    {
    case D3DFMT_A8R8G8B8:
        for (y0 = 0; y0 < surfaceDesc.Height; y0++)
        {
            dstData0 = (u8 *)lockedRectDst.pBits + y0 * lockedRectDst.Pitch;
            srcData0 = (u8 *)lockedRectSrc.pBits + y0 * lockedRectSrc.Pitch;

            for (x0 = 0; x0 < surfaceDesc.Width; x0++, srcData0 += 4, dstData0 += 4)
            {
                dstData0[3] = srcData0[0];
            }
        }
        break;

    case D3DFMT_A1R5G5B5:
        for (y1 = 0; y1 < surfaceDesc.Height; y1++)
        {

            dstData1 = (Argb1555Pixel *)((u8 *)lockedRectDst.pBits + y1 * lockedRectDst.Pitch);
            srcData1 = (Argb1555Pixel *)((u8 *)lockedRectSrc.pBits + y1 * lockedRectSrc.Pitch);

            for (x1 = 0; x1 < surfaceDesc.Width; x1++, srcData1++, dstData1++)
            {
                dstData1->a = srcData1->b >> 4;
            }
        }
        break;

    case D3DFMT_A4R4G4B4:
        for (y2 = 0; y2 < surfaceDesc.Height; y2++)
        {
            dstData2 = (Argb4444Pixel *)((u8 *)lockedRectDst.pBits + y2 * lockedRectDst.Pitch);
            srcData2 = (Argb4444Pixel *)((u8 *)lockedRectSrc.pBits + y2 * lockedRectSrc.Pitch);

            for (x2 = 0; x2 < surfaceDesc.Width; x2++, srcData2++, dstData2++)
            {
                dstData2->a = srcData2->b;
            }
        }
        break;
    }

    textureSrc->UnlockRect(0);
    this->textures[textureIdx]->UnlockRect(0);

    if (textureSrc != NULL)
    {
        textureSrc->Release();
        textureSrc = NULL;
    }

    free(data);
    return ZUN_SUCCESS;

err:
    if (textureSrc != NULL)
    {
        textureSrc->Release();
        textureSrc = NULL;
    }

    free(data);
    return ZUN_ERROR;
}

ZunResult AnmManager::CreateEmptyTexture(i32 textureIdx, u32 width, u32 height, i32 textureFormat)
{
    D3DXCreateTexture(g_Supervisor.d3dDevice, width, height, 1, 0, g_TextureFormatD3D8Mapping[textureFormat],
                      D3DPOOL_MANAGED, this->textures + textureIdx);

    return ZUN_SUCCESS;
}

#pragma var_order(anm, anmName, rawSprite, index, curSpriteOffset, loadedSprite)
ZunResult AnmManager::LoadAnm(i32 anmIdx, char *path, i32 spriteIdxOffset)
{
    this->ReleaseAnm(anmIdx);
    this->anmFiles[anmIdx] = (AnmRawEntry *)FileSystem::OpenPath(path, 0);

    AnmRawEntry *anm = this->anmFiles[anmIdx];

    if (anm == NULL)
    {
        GameErrorContext::Fatal(&g_GameErrorContext, TH_ERR_ANMMANAGER_SPRITE_CORRUPTED, path);
        return ZUN_ERROR;
    }

    anm->textureIdx = anmIdx;

    char *anmName = (char *)((u8 *)anm + anm->nameOffset);

    if (*anmName == '@')
    {
        this->CreateEmptyTexture(anm->textureIdx, anm->width, anm->height, anm->format);
    }
    else if (this->LoadTexture(anm->textureIdx, anmName, anm->format, anm->colorKey) != ZUN_SUCCESS)
    {
        GameErrorContext::Fatal(&g_GameErrorContext, TH_ERR_ANMMANAGER_TEXTURE_CORRUPTED, anmName);
        return ZUN_ERROR;
    }

    if (anm->mipmapNameOffset != 0)
    {
        anmName = (char *)((u8 *)anm + anm->mipmapNameOffset);
        if (this->LoadTextureAlphaChannel(anm->textureIdx, anmName, anm->format, anm->colorKey) != ZUN_SUCCESS)
        {
            GameErrorContext::Fatal(&g_GameErrorContext, TH_ERR_ANMMANAGER_TEXTURE_CORRUPTED, anmName);
            return ZUN_ERROR;
        }
    }

    anm->spriteIdxOffset = spriteIdxOffset;

    u32 *curSpriteOffset = anm->spriteOffsets;

    i32 index;
    AnmRawSprite *rawSprite;

    for (index = 0; index < this->anmFiles[anmIdx]->numSprites; index++, curSpriteOffset++)
    {
        rawSprite = (AnmRawSprite *)((u8 *)anm + *curSpriteOffset);

        AnmLoadedSprite loadedSprite;
        loadedSprite.sourceFileIndex = this->anmFiles[anmIdx]->textureIdx;
        loadedSprite.startPixelInclusive.x = rawSprite->offset.x;
        loadedSprite.startPixelInclusive.y = rawSprite->offset.y;
        loadedSprite.endPixelInclusive.x = rawSprite->offset.x + rawSprite->size.x;
        loadedSprite.endPixelInclusive.y = rawSprite->offset.y + rawSprite->size.y;
        loadedSprite.textureWidth = (float)anm->width;
        loadedSprite.textureHeight = (float)anm->height;
        this->LoadSprite(rawSprite->id + spriteIdxOffset, &loadedSprite);
    }

    for (index = 0; index < anm->numScripts; index++, curSpriteOffset += 2)
    {
        this->scripts[curSpriteOffset[0] + spriteIdxOffset] = (AnmRawInstr *)((u8 *)anm + curSpriteOffset[1]);
        this->spriteIndices[curSpriteOffset[0] + spriteIdxOffset] = spriteIdxOffset;
    }

    this->anmFilesSpriteIndexOffsets[anmIdx] = spriteIdxOffset;

    return ZUN_SUCCESS;
}

#pragma var_order(entry, spriteIdx, spriteIdxOffset, i, byteOffset, anmFilePtr, anmIdx, )
void AnmManager::ReleaseAnm(i32 anmIdx)
{
    if (this->anmFiles[anmIdx] != NULL)
    {
        i32 *spriteIdx;
        i32 i;
        i32 spriteIdxOffset = this->anmFilesSpriteIndexOffsets[anmIdx];
        u32 *byteOffset = this->anmFiles[anmIdx]->spriteOffsets;
        for (i = 0; i < this->anmFiles[anmIdx]->numSprites; i++, byteOffset++)
        {
            spriteIdx = (i32 *)((u8 *)this->anmFiles[anmIdx] + *byteOffset);
            memset(&this->sprites[*spriteIdx + spriteIdxOffset], 0,
                   sizeof(this->sprites[*spriteIdx + spriteIdxOffset]));
            this->sprites[*spriteIdx + spriteIdxOffset].sourceFileIndex = -1;
        }

        for (i = 0; i < this->anmFiles[anmIdx]->numScripts; i++, byteOffset += 2)
        {
            this->scripts[*byteOffset + spriteIdxOffset] = NULL;
            this->spriteIndices[*byteOffset + spriteIdxOffset] = NULL;
        }
        this->anmFilesSpriteIndexOffsets[anmIdx] = NULL;
        AnmRawEntry *entry = this->anmFiles[anmIdx];
        this->ReleaseTexture(entry->textureIdx);
        AnmRawEntry *anmFilePtr = this->anmFiles[anmIdx];
        free(anmFilePtr);
        this->anmFiles[anmIdx] = 0;
        this->currentBlendMode = 0xff;
        this->currentColorOp = 0xff;
        this->currentVertexShader = 0xff;
        this->currentTexture = NULL;
    }
}

void AnmManager::ReleaseTexture(i32 textureIdx)
{
    if (this->textures[textureIdx] != NULL)
    {
        this->textures[textureIdx]->Release();
        this->textures[textureIdx] = NULL;
    }

    void *imageDataArray = this->imageDataArray[textureIdx];
    free(imageDataArray);

    this->imageDataArray[textureIdx] = NULL;
}

void AnmManager::LoadSprite(u32 spriteIdx, AnmLoadedSprite *sprite)
{
    this->sprites[spriteIdx] = *sprite;
    this->sprites[spriteIdx].spriteId = this->maybeLoadedSpriteCount++;

    this->sprites[spriteIdx].uvStart.x =
        this->sprites[spriteIdx].startPixelInclusive.x / (this->sprites[spriteIdx].textureWidth);
    this->sprites[spriteIdx].uvEnd.x =
        this->sprites[spriteIdx].endPixelInclusive.x / (this->sprites[spriteIdx].textureWidth);
    this->sprites[spriteIdx].uvStart.y =
        this->sprites[spriteIdx].startPixelInclusive.y / (this->sprites[spriteIdx].textureHeight);
    this->sprites[spriteIdx].uvEnd.y =
        this->sprites[spriteIdx].endPixelInclusive.y / (this->sprites[spriteIdx].textureHeight);

    this->sprites[spriteIdx].widthPx =
        this->sprites[spriteIdx].endPixelInclusive.x - this->sprites[spriteIdx].startPixelInclusive.x;
    this->sprites[spriteIdx].heightPx =
        this->sprites[spriteIdx].endPixelInclusive.y - this->sprites[spriteIdx].startPixelInclusive.y;
}

ZunResult AnmManager::SetActiveSprite(AnmVm *vm, u32 sprite_index)
{
    if (this->sprites[sprite_index].sourceFileIndex < 0)
    {
        return ZUN_ERROR;
    }

    vm->activeSpriteIndex = (i16)sprite_index;
    vm->sprite = this->sprites + sprite_index;
    D3DXMatrixIdentity(&vm->matrix);
    vm->matrix.m[0][0] = vm->sprite->widthPx / vm->sprite->textureWidth;
    vm->matrix.m[1][1] = vm->sprite->heightPx / vm->sprite->textureHeight;

    return ZUN_SUCCESS;
}

void AnmManager::SetAndExecuteScript(AnmVm *vm, AnmRawInstr *beginingOfScript)
{
    ZunTimer *timer;

    vm->flags.flip = 0;
    vm->Initialize();
    vm->beginingOfScript = beginingOfScript;
    vm->currentInstruction = vm->beginingOfScript;

    timer = &(vm->currentTimeInScript);
    timer->current = 0;
    timer->subFrame = 0.0;
    timer->previous = -999;

    vm->flags.isVisible = 0;
    if (beginingOfScript)
    {
        this->ExecuteScript(vm);
    }
}

void AnmManager::SetRenderStateForVm(AnmVm *vm)
{
    if (this->currentBlendMode != vm->flags.blendMode)
    {
        this->currentBlendMode = vm->flags.blendMode;
        if (this->currentBlendMode == AnmVmBlendMode_InvSrcAlpha)
        {
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        }
        else
        {
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        }
    }
    if ((((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0) &&
        (((g_Supervisor.cfg.opts >> GCOS_NO_COLOR_COMP) & 1) == 0) && (this->currentColorOp != vm->flags.colorOp))
    {
        this->currentColorOp = vm->flags.colorOp;
        if (this->currentColorOp == AnmVmColorOp_Modulate)
        {
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        }
        else
        {
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
        }
    }
    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0)
    {
        if (this->currentTextureFactor != vm->color)
        {
            this->currentTextureFactor = vm->color;
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, this->currentTextureFactor);
        }
    }
    else
    {
        g_PrimitivesToDrawNoVertexBuf[0].diffuse = vm->color;
        g_PrimitivesToDrawNoVertexBuf[1].diffuse = vm->color;
        g_PrimitivesToDrawNoVertexBuf[2].diffuse = vm->color;
        g_PrimitivesToDrawNoVertexBuf[3].diffuse = vm->color;
        g_PrimitivesToDrawUnknown[0].diffuse = vm->color;
        g_PrimitivesToDrawUnknown[1].diffuse = vm->color;
        g_PrimitivesToDrawUnknown[2].diffuse = vm->color;
        g_PrimitivesToDrawUnknown[3].diffuse = vm->color;
    }
    if ((((g_Supervisor.cfg.opts >> GCOS_TURN_OFF_DEPTH_TEST) & 1) == 0) &&
        (this->currentZWriteDisable != vm->flags.zWriteDisable))
    {
        this->currentZWriteDisable = vm->flags.zWriteDisable;
        if (this->currentZWriteDisable == 0)
        {
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, 1);
        }
        else
        {
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, 0);
        }
    }
    return;
}

static f32 g_ZeroPointFive = 0.5;

ZunResult AnmManager::DrawInner(AnmVm *vm, i32 param_3)
{
    if (param_3 != 0)
    {
        // TODO: It'd be nice to find a way to match this without inline assembly.
        __asm {
            fld g_PrimitivesToDrawVertexBuf[0 * TYPE g_PrimitivesToDrawVertexBuf].position.x
            frndint
            fsub g_ZeroPointFive
            fld g_PrimitivesToDrawVertexBuf[1 * TYPE g_PrimitivesToDrawVertexBuf].position.x
            frndint
            fsub g_ZeroPointFive
            fld g_PrimitivesToDrawVertexBuf[0 * TYPE g_PrimitivesToDrawVertexBuf].position.y
            frndint
            fsub g_ZeroPointFive
            fld g_PrimitivesToDrawVertexBuf[2 * TYPE g_PrimitivesToDrawVertexBuf].position.y
            frndint
            fsub g_ZeroPointFive
            fst g_PrimitivesToDrawVertexBuf[2 * TYPE g_PrimitivesToDrawVertexBuf].position.y
            fstp g_PrimitivesToDrawVertexBuf[3 * TYPE g_PrimitivesToDrawVertexBuf].position.y
            fst g_PrimitivesToDrawVertexBuf[0 * TYPE g_PrimitivesToDrawVertexBuf].position.y
            fstp g_PrimitivesToDrawVertexBuf[1 * TYPE g_PrimitivesToDrawVertexBuf].position.y
            fst g_PrimitivesToDrawVertexBuf[1 * TYPE g_PrimitivesToDrawVertexBuf].position.x
            fstp g_PrimitivesToDrawVertexBuf[3 * TYPE g_PrimitivesToDrawVertexBuf].position.x
            fst g_PrimitivesToDrawVertexBuf[0 * TYPE g_PrimitivesToDrawVertexBuf].position.x
            fstp g_PrimitivesToDrawVertexBuf[2 * TYPE g_PrimitivesToDrawVertexBuf].position.x
        }
    }
    g_PrimitivesToDrawVertexBuf[0].position.z = g_PrimitivesToDrawVertexBuf[1].position.z =
        g_PrimitivesToDrawVertexBuf[2].position.z = g_PrimitivesToDrawVertexBuf[3].position.z = vm->pos.z;
    if (this->currentSprite != vm->sprite)
    {
        this->currentSprite = vm->sprite;
        g_PrimitivesToDrawVertexBuf[0].textureUV.x = g_PrimitivesToDrawVertexBuf[2].textureUV.x =
            vm->sprite->uvStart.x + vm->uvScrollPos.x;
        g_PrimitivesToDrawVertexBuf[1].textureUV.x = g_PrimitivesToDrawVertexBuf[3].textureUV.x =
            vm->sprite->uvEnd.x + vm->uvScrollPos.x;
        g_PrimitivesToDrawVertexBuf[0].textureUV.y = g_PrimitivesToDrawVertexBuf[1].textureUV.y =
            vm->sprite->uvStart.y + vm->uvScrollPos.y;
        g_PrimitivesToDrawVertexBuf[2].textureUV.y = g_PrimitivesToDrawVertexBuf[3].textureUV.y =
            vm->sprite->uvEnd.y + vm->uvScrollPos.y;
        if (this->currentTexture != this->textures[vm->sprite->sourceFileIndex])
        {
            this->currentTexture = this->textures[vm->sprite->sourceFileIndex];
            g_Supervisor.d3dDevice->SetTexture(0, this->currentTexture);
        }
    }
    if (this->currentVertexShader != 2)
    {
        if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0)
        {
            g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_XYZRHW);
        }
        else
        {
            g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
        }
        this->currentVertexShader = 2;
    }
    this->SetRenderStateForVm(vm);
    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0)
    {
        g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_PrimitivesToDrawVertexBuf, 0x18);
    }
    else
    {
        g_PrimitivesToDrawNoVertexBuf[0].position.x = g_PrimitivesToDrawVertexBuf[0].position.x;
        g_PrimitivesToDrawNoVertexBuf[0].position.y = g_PrimitivesToDrawVertexBuf[0].position.y;
        g_PrimitivesToDrawNoVertexBuf[0].position.z = g_PrimitivesToDrawVertexBuf[0].position.z;
        g_PrimitivesToDrawNoVertexBuf[1].position.x = g_PrimitivesToDrawVertexBuf[1].position.x;
        g_PrimitivesToDrawNoVertexBuf[1].position.y = g_PrimitivesToDrawVertexBuf[1].position.y;
        g_PrimitivesToDrawNoVertexBuf[1].position.z = g_PrimitivesToDrawVertexBuf[1].position.z;
        g_PrimitivesToDrawNoVertexBuf[2].position.x = g_PrimitivesToDrawVertexBuf[2].position.x;
        g_PrimitivesToDrawNoVertexBuf[2].position.y = g_PrimitivesToDrawVertexBuf[2].position.y;
        g_PrimitivesToDrawNoVertexBuf[2].position.z = g_PrimitivesToDrawVertexBuf[2].position.z;
        g_PrimitivesToDrawNoVertexBuf[3].position.x = g_PrimitivesToDrawVertexBuf[3].position.x;
        g_PrimitivesToDrawNoVertexBuf[3].position.y = g_PrimitivesToDrawVertexBuf[3].position.y;
        g_PrimitivesToDrawNoVertexBuf[3].position.z = g_PrimitivesToDrawVertexBuf[3].position.z;
        g_PrimitivesToDrawNoVertexBuf[0].textureUV.x = g_PrimitivesToDrawNoVertexBuf[2].textureUV.x =
            vm->sprite->uvStart.x + vm->uvScrollPos.x;
        g_PrimitivesToDrawNoVertexBuf[1].textureUV.x = g_PrimitivesToDrawNoVertexBuf[3].textureUV.x =
            vm->sprite->uvEnd.x + vm->uvScrollPos.x;
        g_PrimitivesToDrawNoVertexBuf[0].textureUV.y = g_PrimitivesToDrawNoVertexBuf[1].textureUV.y =
            vm->sprite->uvStart.y + vm->uvScrollPos.y;
        g_PrimitivesToDrawNoVertexBuf[2].textureUV.y = g_PrimitivesToDrawNoVertexBuf[3].textureUV.y =
            vm->sprite->uvEnd.y + vm->uvScrollPos.y;
        g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_PrimitivesToDrawNoVertexBuf, 0x1c);
    }
    return ZUN_SUCCESS;
}

ZunResult AnmManager::DrawNoRotation(AnmVm *vm)
{
    float fVar2;
    float fVar3;

    if (vm->flags.isVisible == 0)
    {
        return ZUN_ERROR;
    }
    if (vm->flags.flag1 == 0)
    {
        return ZUN_ERROR;
    }
    if (vm->color == 0)
    {
        return ZUN_ERROR;
    }
    fVar2 = (vm->sprite->widthPx * vm->scaleX) / 2.0f;
    fVar3 = (vm->sprite->heightPx * vm->scaleY) / 2.0f;
    if ((vm->flags.anchor & AnmVmAnchor_Left) == 0)
    {
        g_PrimitivesToDrawVertexBuf[0].position.x = g_PrimitivesToDrawVertexBuf[2].position.x = vm->pos.x - fVar2;
        g_PrimitivesToDrawVertexBuf[1].position.x = g_PrimitivesToDrawVertexBuf[3].position.x = fVar2 + vm->pos.x;
    }
    else
    {
        g_PrimitivesToDrawVertexBuf[0].position.x = g_PrimitivesToDrawVertexBuf[2].position.x = vm->pos.x;
        g_PrimitivesToDrawVertexBuf[1].position.x = g_PrimitivesToDrawVertexBuf[3].position.x =
            fVar2 + vm->pos.x + fVar2;
    }
    if ((vm->flags.anchor & AnmVmAnchor_Top) == 0)
    {
        g_PrimitivesToDrawVertexBuf[0].position.y = g_PrimitivesToDrawVertexBuf[1].position.y = vm->pos.y - fVar3;
        g_PrimitivesToDrawVertexBuf[2].position.y = g_PrimitivesToDrawVertexBuf[3].position.y = fVar3 + vm->pos.y;
    }
    else
    {
        g_PrimitivesToDrawVertexBuf[0].position.y = g_PrimitivesToDrawVertexBuf[1].position.y = vm->pos.y;
        g_PrimitivesToDrawVertexBuf[2].position.y = g_PrimitivesToDrawVertexBuf[3].position.y =
            fVar3 + vm->pos.y + fVar3;
    }
    return this->DrawInner(vm, 1);
}

void AnmManager::TranslateRotation(VertexTex1Xyzrwh *param_1, f32 x, f32 y, f32 sine, f32 cosine, f32 xOffset,
                                   f32 yOffset)
{
    param_1->position.x = x * cosine + y * sine + xOffset;
    param_1->position.y = -x * sine + y * cosine + yOffset;
    return;
}

#pragma var_order(spriteXCenter, spriteYCenter, yOffset, xOffset, zSine, z, zCosine)
ZunResult AnmManager::Draw(AnmVm *vm)
{
    f32 zSine;
    f32 zCosine;
    f32 spriteXCenter;
    f32 spriteYCenter;
    f32 xOffset;
    f32 yOffset;
    f32 z;

    if (vm->rotation.z == 0.0f)
    {
        return this->DrawNoRotation(vm);
    }
    if (vm->flags.isVisible == 0)
    {
        return ZUN_ERROR;
    }
    if (vm->flags.flag1 == 0)
    {
        return ZUN_ERROR;
    }
    if (vm->color == 0)
    {
        return ZUN_ERROR;
    }
    z = vm->rotation.z;
    sincos(z, zSine, zCosine);
    xOffset = rintf(vm->pos.x);
    yOffset = rintf(vm->pos.y);
    spriteXCenter = rintf((vm->sprite->widthPx * vm->scaleX) / 2.0f);
    spriteYCenter = rintf((vm->sprite->heightPx * vm->scaleY) / 2.0f);
    this->TranslateRotation(&g_PrimitivesToDrawVertexBuf[0], -spriteXCenter - 0.5f, -spriteYCenter - 0.5f, zSine,
                            zCosine, xOffset, yOffset);
    this->TranslateRotation(&g_PrimitivesToDrawVertexBuf[1], spriteXCenter - 0.5f, -spriteYCenter - 0.5f, zSine,
                            zCosine, xOffset, yOffset);
    this->TranslateRotation(&g_PrimitivesToDrawVertexBuf[2], -spriteXCenter - 0.5f, spriteYCenter - 0.5f, zSine,
                            zCosine, xOffset, yOffset);
    this->TranslateRotation(&g_PrimitivesToDrawVertexBuf[3], spriteXCenter - 0.5f, spriteYCenter - 0.5f, zSine, zCosine,
                            xOffset, yOffset);
    g_PrimitivesToDrawVertexBuf[0].position.z = g_PrimitivesToDrawVertexBuf[1].position.z =
        g_PrimitivesToDrawVertexBuf[2].position.z = g_PrimitivesToDrawVertexBuf[3].position.z = vm->pos.z;
    if ((vm->flags.anchor & AnmVmAnchor_Left) != 0)
    {
        g_PrimitivesToDrawVertexBuf[0].position.x += spriteXCenter;
        g_PrimitivesToDrawVertexBuf[1].position.x += spriteXCenter;
        g_PrimitivesToDrawVertexBuf[2].position.x += spriteXCenter;
        g_PrimitivesToDrawVertexBuf[3].position.x += spriteXCenter;
    }
    if ((vm->flags.anchor & AnmVmAnchor_Top) != 0)
    {
        g_PrimitivesToDrawVertexBuf[0].position.y += spriteYCenter;
        g_PrimitivesToDrawVertexBuf[1].position.y += spriteYCenter;
        g_PrimitivesToDrawVertexBuf[2].position.y += spriteYCenter;
        g_PrimitivesToDrawVertexBuf[3].position.y += spriteYCenter;
    }
    return this->DrawInner(vm, 0);
}

ZunResult AnmManager::DrawFacingCamera(AnmVm *vm)
{
    f32 centerX;
    f32 centerY;

    if (!vm->flags.isVisible)
    {
        return ZUN_ERROR;
    }
    if (!vm->flags.flag1)
    {
        return ZUN_ERROR;
    }
    if (vm->color == 0)
    {
        return ZUN_ERROR;
    }

    centerX = vm->sprite->widthPx * vm->scaleX / 2.0f;
    centerY = vm->sprite->heightPx * vm->scaleY / 2.0f;
    if ((vm->flags.anchor & AnmVmAnchor_Left) == 0)
    {
        g_PrimitivesToDrawVertexBuf[0].position.x = g_PrimitivesToDrawVertexBuf[2].position.x = vm->pos.x - centerX;
        g_PrimitivesToDrawVertexBuf[1].position.x = g_PrimitivesToDrawVertexBuf[3].position.x = vm->pos.x + centerX;
    }
    else
    {
        g_PrimitivesToDrawVertexBuf[0].position.x = g_PrimitivesToDrawVertexBuf[2].position.x = vm->pos.x;
        g_PrimitivesToDrawVertexBuf[1].position.x = g_PrimitivesToDrawVertexBuf[3].position.x =
            vm->pos.x + centerX + centerX;
    }
    if ((vm->flags.anchor & AnmVmAnchor_Top) == 0)
    {
        g_PrimitivesToDrawVertexBuf[0].position.y = g_PrimitivesToDrawVertexBuf[1].position.y = vm->pos.y - centerY;
        g_PrimitivesToDrawVertexBuf[2].position.y = g_PrimitivesToDrawVertexBuf[3].position.y = vm->pos.y + centerY;
    }
    else
    {
        g_PrimitivesToDrawVertexBuf[0].position.y = g_PrimitivesToDrawVertexBuf[1].position.y = vm->pos.y;
        g_PrimitivesToDrawVertexBuf[2].position.y = g_PrimitivesToDrawVertexBuf[3].position.y =
            vm->pos.y + centerY + centerY;
    }
    return this->DrawInner(vm, 0);
}

#pragma var_order(textureMatrix, rotationMatrix, worldTransformMatrix, scaledXCenter, scaledYCenter)
ZunResult AnmManager::Draw3(AnmVm *vm)
{
    D3DXMATRIX worldTransformMatrix;
    D3DXMATRIX rotationMatrix;
    D3DXMATRIX textureMatrix;
    f32 scaledXCenter;
    f32 scaledYCenter;

    if (!vm->flags.isVisible)
    {
        return ZUN_ERROR;
    }
    if (!vm->flags.flag1)
    {
        return ZUN_ERROR;
    }
    if (vm->color == 0)
    {
        return ZUN_ERROR;
    }

    worldTransformMatrix = vm->matrix;
    worldTransformMatrix.m[0][0] *= vm->scaleX;
    worldTransformMatrix.m[1][1] *= -vm->scaleY;

    if (vm->rotation.x != 0.0)
    {
        D3DXMatrixRotationX(&rotationMatrix, vm->rotation.x);
        D3DXMatrixMultiply(&worldTransformMatrix, &worldTransformMatrix, &rotationMatrix);
    }

    if (vm->rotation.y != 0.0)
    {
        D3DXMatrixRotationY(&rotationMatrix, vm->rotation.y);
        D3DXMatrixMultiply(&worldTransformMatrix, &worldTransformMatrix, &rotationMatrix);
    }

    if (vm->rotation.z != 0.0)
    {
        D3DXMatrixRotationZ(&rotationMatrix, vm->rotation.z);
        D3DXMatrixMultiply(&worldTransformMatrix, &worldTransformMatrix, &rotationMatrix);
    }

    if ((vm->flags.anchor & AnmVmAnchor_Left) == 0)
    {
        worldTransformMatrix.m[3][0] = vm->pos.x;
    }
    else
    {
        scaledXCenter = vm->sprite->widthPx * vm->scaleX / 2.0f;
        worldTransformMatrix.m[3][0] = fabsf(scaledXCenter) + vm->pos.x;
    }

    if ((vm->flags.anchor & AnmVmAnchor_Top) == 0)
    {
        worldTransformMatrix.m[3][1] = -vm->pos.y;
    }
    else
    {
        scaledYCenter = vm->sprite->heightPx * vm->scaleY / 2.0f;
        worldTransformMatrix.m[3][1] = -vm->pos.y - fabsf(scaledYCenter);
    }

    worldTransformMatrix.m[3][2] = vm->pos.z;

    // Now, set transform matrix.
    g_Supervisor.d3dDevice->SetTransform(D3DTS_WORLD, &worldTransformMatrix);

    // Load sprite if vm->sprite is not the same as current sprite.
    if (this->currentSprite != vm->sprite)
    {
        this->currentSprite = vm->sprite;
        textureMatrix = vm->matrix;
        textureMatrix.m[2][0] = vm->sprite->uvStart.x + vm->uvScrollPos.x;
        textureMatrix.m[2][1] = vm->sprite->uvStart.y + vm->uvScrollPos.y;
        g_Supervisor.d3dDevice->SetTransform(D3DTS_TEXTURE0, &textureMatrix);
        if (this->currentTexture != this->textures[vm->sprite->sourceFileIndex])
        {
            this->currentTexture = this->textures[vm->sprite->sourceFileIndex];
            g_Supervisor.d3dDevice->SetTexture(0, this->currentTexture);
        }
    }

    // Set vertex shader to TEX1 | XYZ
    if (this->currentVertexShader != 3)
    {
        if ((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF & 1) == 0)
        {
            g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_XYZ);
            g_Supervisor.d3dDevice->SetStreamSource(0, this->vertexBuffer, 0x14);
        }
        else
        {
            g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZ);
        }
        this->currentVertexShader = 3;
    }

    // Reset the render state based on the settings fo the given VM.
    this->SetRenderStateForVm(vm);

    // Draw the VM.
    if ((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF & 1) == 0)
    {
        g_Supervisor.d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    }
    else
    {
        g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_PrimitivesToDrawUnknown, 0x18);
    }
    return ZUN_SUCCESS;
}

#pragma var_order(textureMatrix, unusedMatrix, worldTransformMatrix)
ZunResult AnmManager::Draw2(AnmVm *vm)
{
    D3DXMATRIX worldTransformMatrix;
    D3DXMATRIX unusedMatrix;
    D3DXMATRIX textureMatrix;

    if (!vm->flags.isVisible)
    {
        return ZUN_ERROR;
    }
    if (!vm->flags.flag1)
    {
        return ZUN_ERROR;
    }

    if (vm->rotation.x != 0 || vm->rotation.y != 0 || vm->rotation.z != 0)
    {
        return this->Draw3(vm);
    }

    if (vm->color == 0)
    {
        return ZUN_ERROR;
    }

    worldTransformMatrix = vm->matrix;
    worldTransformMatrix.m[3][0] = rintf(vm->pos.x) - 0.5f;
    worldTransformMatrix.m[3][1] = -rintf(vm->pos.y) + 0.5f;
    if ((vm->flags.anchor & AnmVmAnchor_Left) != 0)
    {
        worldTransformMatrix.m[3][0] += (vm->sprite->widthPx * vm->scaleX) / 2.0f;
    }
    if ((vm->flags.anchor & AnmVmAnchor_Top) != 0)
    {
        worldTransformMatrix.m[3][1] -= (vm->sprite->heightPx * vm->scaleY) / 2.0f;
    }
    worldTransformMatrix.m[3][2] = vm->pos.z;
    worldTransformMatrix.m[0][0] *= vm->scaleX;
    worldTransformMatrix.m[1][1] *= -vm->scaleY;
    g_Supervisor.d3dDevice->SetTransform(D3DTS_WORLD, &worldTransformMatrix);

    if (this->currentSprite != vm->sprite)
    {
        this->currentSprite = vm->sprite;
        textureMatrix = vm->matrix;
        textureMatrix.m[2][0] = vm->sprite->uvStart.x + vm->uvScrollPos.x;
        textureMatrix.m[2][1] = vm->sprite->uvStart.y + vm->uvScrollPos.y;
        g_Supervisor.d3dDevice->SetTransform(D3DTS_TEXTURE0, &textureMatrix);
        if (this->currentTexture != this->textures[vm->sprite->sourceFileIndex])
        {
            this->currentTexture = this->textures[vm->sprite->sourceFileIndex];
            g_Supervisor.d3dDevice->SetTexture(0, this->currentTexture);
        }
        if (this->currentVertexShader != 3)
        {
            if ((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF & 1) == 0)
            {
                g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_XYZ);
                g_Supervisor.d3dDevice->SetStreamSource(0, this->vertexBuffer, 0x14);
            }
            else
            {
                g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZ);
            }
            this->currentVertexShader = 3;
        }
    }
    this->SetRenderStateForVm(vm);
    if ((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF & 1) == 0)
    {
        g_Supervisor.d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    }
    else
    {
        g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_PrimitivesToDrawUnknown, 0x18);
    }
    return ZUN_SUCCESS;
}

#pragma var_order(curInstr, local_c, local_10, local_14, local_18, local_1c, local_20, nextInstr, local_28, local_2c,  \
                  local_30, local_34, local_38, local_3c)
i32 AnmManager::ExecuteScript(AnmVm *vm)
{
    AnmRawInstr *curInstr;
    u32 *local_c;
    f32 *local_10;
    f32 *local_14;
    f32 *local_18;
    f32 *local_1c;
    u32 *local_20;
    AnmRawInstr *nextInstr;
    ZunColor local_28;
    ZunColor local_2c;
    f32 local_30;
    i32 local_34;
    i32 local_38;
    f32 local_3c;

    if (vm->currentInstruction == NULL)
    {
        return 1;
    }

    if (vm->pendingInterrupt != 0)
    {
        goto yolo;
    }

    while (curInstr = vm->currentInstruction, curInstr->time <= vm->currentTimeInScript.AsFrames())
    {
        switch (curInstr->opcode)
        {
        case AnmOpcode_Exit:
            vm->flags.isVisible = 0;
        case AnmOpcode_ExitHide:
            vm->currentInstruction = NULL;
            return 1;
        case AnmOpcode_SetActiveSprite:
            vm->flags.isVisible = 1;
            this->SetActiveSprite(vm, curInstr->args[0] + this->spriteIndices[vm->anmFileIndex]);
            vm->timeOfLastSpriteSet = vm->currentTimeInScript.AsFrames();
            break;
        case AnmOpcode_SetRandomSprite:
            vm->flags.isVisible = 1;
            local_c = &curInstr->args[0];
            this->SetActiveSprite(vm, local_c[0] + g_Rng.GetRandomU16InRange(local_c[1]) +
                                          this->spriteIndices[vm->anmFileIndex]);
            vm->timeOfLastSpriteSet = vm->currentTimeInScript.AsFrames();
            break;
        case AnmOpcode_SetScale:
            vm->scaleX = *(f32 *)&curInstr->args[0];
            vm->scaleY = *(f32 *)&curInstr->args[1];
            break;
        case AnmOpcode_SetAlpha:
            COLOR_SET_COMPONENT(vm->color, COLOR_ALPHA_BYTE_IDX, curInstr->args[0] & 0xff);
            break;
        case AnmOpcode_SetColor:
            vm->color = COLOR_COMBINE_ALPHA(curInstr->args[0], vm->color);
            break;
        case AnmOpcode_Jump:
            vm->currentInstruction = (AnmRawInstr *)((i32)vm->beginingOfScript->args + curInstr->args[0] - 4);
            vm->currentTimeInScript.current = vm->currentInstruction->time;
            continue;
        case AnmOpcode_FlipX:
            vm->flags.flip ^= 1;
            vm->scaleX *= -1.f;
            break;
        case AnmOpcode_UsePosOffset:
            vm->flags.usePosOffset = curInstr->args[0];
            break;
        case AnmOpcode_FlipY:
            vm->flags.flip ^= 2;
            vm->scaleY *= -1.f;
            break;
        case AnmOpcode_SetRotation:
            local_10 = (f32 *)&curInstr->args[0];
            vm->rotation.x = *local_10++;
            vm->rotation.y = *local_10++;
            vm->rotation.z = *local_10;
            break;
        case AnmOpcode_SetAngleVel:
            local_14 = (f32 *)&curInstr->args[0];
            vm->angleVel.x = *local_14++;
            vm->angleVel.y = *local_14++;
            vm->angleVel.z = *local_14;
            break;
        case AnmOpcode_SetScaleSpeed:
            local_18 = (f32 *)&curInstr->args[0];
            vm->scaleInterpFinalX = *local_18++;
            vm->scaleInterpFinalY = *local_18;
            vm->scaleInterpEndTime = 0;
            break;
        case AnmOpcode_ScaleTime:
            local_1c = (f32 *)&curInstr->args[0];
            vm->scaleInterpFinalX = *local_1c++;
            vm->scaleInterpFinalY = *local_1c++;
            vm->scaleInterpEndTime = *(u16 *)local_1c;
            vm->scaleInterpTime.InitializeForPopup();
            vm->scaleInterpInitialX = vm->scaleX;
            vm->scaleInterpInitialY = vm->scaleY;
            break;
        case AnmOpcode_Fade:
            local_20 = (u32 *)&curInstr->args[0];
            vm->alphaInterpInitial = vm->color;
            vm->alphaInterpFinal = COLOR_SET_ALPHA2(vm->color, local_20[0]);
            vm->alphaInterpEndTime = local_20[1];
            vm->alphaInterpTime.InitializeForPopup();
            break;
        case AnmOpcode_SetBlendAdditive:
            vm->flags.blendMode = AnmVmBlendMode_One;
            break;
        case AnmOpcode_SetBlendDefault:
            vm->flags.blendMode = AnmVmBlendMode_InvSrcAlpha;
            break;
        case AnmOpcode_SetPosition:
            if (vm->flags.usePosOffset == 0)
            {
                vm->pos =
                    D3DXVECTOR3(*(f32 *)&curInstr->args[0], *(f32 *)&curInstr->args[1], *(f32 *)&curInstr->args[2]);
            }
            else
            {
                vm->posOffset =
                    D3DXVECTOR3(*(f32 *)&curInstr->args[0], *(f32 *)&curInstr->args[1], *(f32 *)&curInstr->args[2]);
            }
            break;
        case AnmOpcode_PosTimeAccel:
            vm->flags.posTime = 2;
            goto PosTimeDoStuff;
        case AnmOpcode_PosTimeDecel:
            vm->flags.posTime = 1;
            goto PosTimeDoStuff;
        case AnmOpcode_PosTimeLinear:
            vm->flags.posTime = 0;
        PosTimeDoStuff:
            if (vm->flags.usePosOffset == 0)
            {
                memcpy(vm->posInterpInitial, vm->pos, sizeof(D3DXVECTOR3));
            }
            else
            {
                memcpy(vm->posInterpInitial, vm->posOffset, sizeof(D3DXVECTOR3));
            }
            vm->posInterpFinal =
                D3DXVECTOR3(*(f32 *)&curInstr->args[0], *(f32 *)&curInstr->args[1], *(f32 *)&curInstr->args[2]);
            vm->posInterpEndTime = curInstr->args[3];
            vm->posInterpTime.InitializeForPopup();
            break;
        case AnmOpcode_StopHide:
            vm->flags.isVisible = 0;
        case AnmOpcode_Stop:
            if (vm->pendingInterrupt == 0)
            {
                vm->flags.isStopped = 1;
                vm->currentTimeInScript.Decrement(1);
                goto stop;
            }
        yolo:
            nextInstr = NULL;
            curInstr = vm->beginingOfScript;
            while ((curInstr->opcode != AnmOpcode_InterruptLabel || vm->pendingInterrupt != curInstr->args[0]) &&
                   curInstr->opcode != AnmOpcode_Exit && curInstr->opcode != AnmOpcode_ExitHide)
            {
                if (curInstr->opcode == AnmOpcode_InterruptLabel && curInstr->args[0] == 0xffffffff)
                {
                    nextInstr = curInstr;
                }
                curInstr = (AnmRawInstr *)((i32)curInstr->args + curInstr->argsCount);
            }

            vm->pendingInterrupt = 0;
            vm->flags.isStopped = 0;
            if (curInstr->opcode != AnmOpcode_InterruptLabel)
            {
                if (nextInstr == NULL)
                {
                    vm->currentTimeInScript.Decrement(1);
                    goto stop;
                }
                curInstr = nextInstr;
            }

            curInstr = (AnmRawInstr *)((i32)curInstr->args + curInstr->argsCount);
            vm->currentInstruction = curInstr;
            vm->currentTimeInScript.SetCurrent(vm->currentInstruction->time);
            vm->flags.isVisible = 1;
            continue;
        case AnmOpcode_SetVisibility:
            vm->flags.isVisible = curInstr->args[0];
            break;
        case AnmOpcode_AnchorTopLeft:
            vm->flags.anchor = AnmVmAnchor_TopLeft;
            break;
        case AnmOpcode_SetAutoRotate:
            vm->autoRotate = curInstr->args[0];
            break;
        case AnmOpcode_UVScrollX:
            vm->uvScrollPos.x += *(f32 *)&curInstr->args[0];
            if (vm->uvScrollPos.x >= 1.0f)
            {
                vm->uvScrollPos.x -= 1.0f;
            }
            else if (vm->uvScrollPos.x < 0.0f)
            {
                vm->uvScrollPos.x += 1.0f;
            }
            break;
        case AnmOpcode_UVScrollY:
            vm->uvScrollPos.y += *(f32 *)&curInstr->args[0];
            if (vm->uvScrollPos.y >= 1.0f)
            {
                vm->uvScrollPos.y -= 1.0f;
            }
            else if (vm->uvScrollPos.y < 0.0f)
            {
                vm->uvScrollPos.y += 1.0f;
            }
            break;
        case AnmOpcode_SetZWriteDisable:
            vm->flags.zWriteDisable = curInstr->args[0];
            break;
        case AnmOpcode_Nop:
        case AnmOpcode_InterruptLabel:
        default:
            break;
        }
        vm->currentInstruction = (AnmRawInstr *)((u32)curInstr->args + curInstr->argsCount);
    }

stop:
    if (vm->angleVel.x != 0.0f)
    {
        vm->rotation.x =
            utils::AddNormalizeAngle(vm->rotation.x, g_Supervisor.effectiveFramerateMultiplier * vm->angleVel.x);
    }
    if (vm->angleVel.y != 0.0f)
    {
        vm->rotation.y =
            utils::AddNormalizeAngle(vm->rotation.y, g_Supervisor.effectiveFramerateMultiplier * vm->angleVel.y);
    }
    if (vm->angleVel.z != 0.0f)
    {
        vm->rotation.z =
            utils::AddNormalizeAngle(vm->rotation.z, g_Supervisor.effectiveFramerateMultiplier * vm->angleVel.z);
    }
    if (vm->scaleInterpEndTime > 0)
    {
        vm->scaleInterpTime.Tick();
        if (vm->scaleInterpTime.AsFrames() >= vm->scaleInterpEndTime)
        {
            vm->scaleY = vm->scaleInterpFinalY;
            vm->scaleX = vm->scaleInterpFinalX;
            vm->scaleInterpEndTime = 0;
            vm->scaleInterpFinalY = 0.0;
            vm->scaleInterpFinalX = 0.0;
        }
        else
        {
            vm->scaleX = (vm->scaleInterpFinalX - vm->scaleInterpInitialX) * vm->scaleInterpTime.AsFramesFloat() /
                             vm->scaleInterpEndTime +
                         vm->scaleInterpInitialX;
            vm->scaleY = (vm->scaleInterpFinalY - vm->scaleInterpInitialY) * vm->scaleInterpTime.AsFramesFloat() /
                             vm->scaleInterpEndTime +
                         vm->scaleInterpInitialY;
        }
        if ((vm->flags.flip & 1) != 0)
        {
            vm->scaleX = vm->scaleX * -1.f;
        }
        if ((vm->flags.flip & 2) != 0)
        {
            vm->scaleY = vm->scaleY * -1.f;
        }
    }
    else
    {
        vm->scaleY = g_Supervisor.effectiveFramerateMultiplier * vm->scaleInterpFinalY + vm->scaleY;
        vm->scaleX = g_Supervisor.effectiveFramerateMultiplier * vm->scaleInterpFinalX + vm->scaleX;
    }
    if (0 < vm->alphaInterpEndTime)
    {
        vm->alphaInterpTime.Tick();
        local_2c = vm->alphaInterpInitial;
        local_28 = vm->alphaInterpFinal;
        local_30 = vm->alphaInterpTime.AsFramesFloat() / (f32)vm->alphaInterpEndTime;
        if (local_30 >= 1.0f)
        {
            local_30 = 1.0;
        }
        for (local_38 = 0; local_38 < 4; local_38++)
        {
            local_34 = ((f32)COLOR_GET_COMPONENT(local_28, local_38) - (f32)COLOR_GET_COMPONENT(local_2c, local_38)) *
                           local_30 +
                       COLOR_GET_COMPONENT(local_2c, local_38);
            if (local_34 < 0)
            {
                local_34 = 0;
            }
            COLOR_SET_COMPONENT(local_2c, local_38, local_34 >= 256 ? 255 : local_34);
        }
        vm->color = local_2c;
        if (vm->alphaInterpTime.AsFrames() >= vm->alphaInterpEndTime)
        {
            vm->alphaInterpEndTime = 0;
        }
    }
    if (vm->posInterpEndTime != 0)
    {
        local_3c = vm->posInterpTime.AsFramesFloat() / (f32)vm->posInterpEndTime;
        if (local_3c >= 1.0f)
        {
            local_3c = 1.0;
        }
        switch (vm->flags.posTime)
        {
        case 1:
            local_3c = 1.0f - local_3c;
            local_3c *= local_3c;
            local_3c = 1.0f - local_3c;
            break;
        case 2:
            local_3c = 1.0f - local_3c;
            local_3c = local_3c * local_3c * local_3c * local_3c;
            local_3c = 1.0f - local_3c;
            break;
        }
        if (vm->flags.usePosOffset == 0)
        {
            vm->pos.x = local_3c * vm->posInterpFinal.x + (1.0f - local_3c) * vm->posInterpInitial.x;
            vm->pos.y = local_3c * vm->posInterpFinal.y + (1.0f - local_3c) * vm->posInterpInitial.y;
            vm->pos.z = local_3c * vm->posInterpFinal.z + (1.0f - local_3c) * vm->posInterpInitial.z;
        }
        else
        {
            vm->posOffset.x = local_3c * vm->posInterpFinal.x + (1.0f - local_3c) * vm->posInterpInitial.x;
            vm->posOffset.y = local_3c * vm->posInterpFinal.y + (1.0f - local_3c) * vm->posInterpInitial.y;
            vm->posOffset.z = local_3c * vm->posInterpFinal.z + (1.0f - local_3c) * vm->posInterpInitial.z;
        }

        if (vm->posInterpTime.AsFrames() >= vm->posInterpEndTime)
        {
            vm->posInterpEndTime = 0;
        }
        vm->posInterpTime.Tick();
    }
    vm->currentTimeInScript.Tick();
    return 0;
}

void AnmManager::DrawTextToSprite(u32 textureDstIdx, i32 xPos, i32 yPos, i32 spriteWidth, i32 spriteHeight,
                                  i32 fontWidth, i32 fontHeight, ZunColor textColor, ZunColor shadowColor,
                                  char *strToPrint)
{
    if (fontWidth <= 0)
    {
        fontWidth = 15;
    }
    if (fontHeight <= 0)
    {
        fontHeight = 15;
    }
    TextHelper::RenderTextToTexture(xPos, yPos, spriteWidth, spriteHeight, fontWidth, fontHeight, textColor,
                                    shadowColor, strToPrint, this->textures[textureDstIdx]);
    return;
}

#pragma var_order(argptr, buffer, fontWidth)
void AnmManager::DrawVmTextFmt(AnmManager *anmMgr, AnmVm *vm, ZunColor textColor, ZunColor shadowColor, char *fmt, ...)
{
    u32 fontWidth;
    char buffer[64];
    va_list argptr;

    fontWidth = vm->fontWidth;
    va_start(argptr, fmt);
    vsprintf(buffer, fmt, argptr);
    va_end(argptr);
    anmMgr->DrawTextToSprite(vm->sprite->sourceFileIndex, vm->sprite->startPixelInclusive.x,
                             vm->sprite->startPixelInclusive.y, vm->sprite->textureWidth, vm->sprite->textureHeight,
                             fontWidth, vm->fontHeight, textColor, shadowColor, buffer);
    vm->flags.isVisible = true;
    return;
}

#pragma var_order(args, secondPartStartX, buf, fontWidth)
void AnmManager::DrawStringFormat(AnmManager *mgr, AnmVm *vm, ZunColor textColor, ZunColor shadowColor, char *fmt, ...)
{
    char buf[64];
    va_list args;
    i32 fontWidth;
    i32 secondPartStartX;

    fontWidth = vm->fontWidth <= 0 ? 15 : vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    mgr->DrawTextToSprite(vm->sprite->sourceFileIndex, vm->sprite->startPixelInclusive.x,
                          vm->sprite->startPixelInclusive.y, vm->sprite->textureWidth, vm->sprite->textureHeight,
                          fontWidth, vm->fontHeight, textColor, shadowColor, " ");
    secondPartStartX =
        vm->sprite->startPixelInclusive.x + vm->sprite->textureWidth - ((f32)strlen(buf) * (f32)(fontWidth + 1) / 2.0f);
    mgr->DrawTextToSprite(vm->sprite->sourceFileIndex, secondPartStartX, vm->sprite->startPixelInclusive.y,
                          vm->sprite->textureWidth, vm->sprite->textureHeight, fontWidth, vm->fontHeight, textColor,
                          shadowColor, buf);
    vm->flags.isVisible = true;
    return;
}

#pragma var_order(args, secondPartStartX, buf, fontWidth)
void AnmManager::DrawStringFormat2(AnmManager *mgr, AnmVm *vm, ZunColor textColor, ZunColor shadowColor, char *fmt, ...)
{
    char buf[64];
    va_list args;
    i32 fontWidth;
    i32 secondPartStartX;

    fontWidth = vm->fontWidth <= 0 ? 15 : vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    mgr->DrawTextToSprite(vm->sprite->sourceFileIndex, vm->sprite->startPixelInclusive.x,
                          vm->sprite->startPixelInclusive.y, vm->sprite->textureWidth, vm->sprite->textureHeight,
                          fontWidth, vm->fontHeight, textColor, shadowColor, " ");
    secondPartStartX = vm->sprite->startPixelInclusive.x + vm->sprite->textureWidth / 2.0f -
                       ((f32)strlen(buf) * (f32)(fontWidth + 1) / 4.0f);
    mgr->DrawTextToSprite(vm->sprite->sourceFileIndex, secondPartStartX, vm->sprite->startPixelInclusive.y,
                          vm->sprite->textureWidth, vm->sprite->textureHeight, fontWidth, vm->fontHeight, textColor,
                          shadowColor, buf);
    vm->flags.isVisible = true;
    return;
}

ZunResult AnmManager::LoadSurface(i32 surfaceIdx, char *path)
{
    if (this->surfaces[surfaceIdx] != NULL)
    {
        this->ReleaseSurface(surfaceIdx);
    }
    u8 *data = FileSystem::OpenPath(path, 0);
    if (data == NULL)
    {
        GameErrorContext::Fatal(&g_GameErrorContext, TH_ERR_CANNOT_BE_LOADED, path);
        return ZUN_ERROR;
    }

    LPDIRECT3DSURFACE8 surface;
    if (g_Supervisor.d3dDevice->CreateImageSurface(0x280, 0x400, g_Supervisor.presentParameters.BackBufferFormat,
                                                   &surface) != D3D_OK)
    {
        return ZUN_ERROR;
    }

    if (D3DXLoadSurfaceFromFileInMemory(surface, NULL, NULL, data, g_LastFileSize, NULL, D3DX_FILTER_NONE, 0,
                                        &this->surfaceSourceInfo[surfaceIdx]) != D3D_OK)
    {
        goto fail;
    }
    if (g_Supervisor.d3dDevice->CreateRenderTarget(this->surfaceSourceInfo[surfaceIdx].Width,
                                                   this->surfaceSourceInfo[surfaceIdx].Height,
                                                   g_Supervisor.presentParameters.BackBufferFormat, D3DMULTISAMPLE_NONE,
                                                   TRUE, &this->surfaces[surfaceIdx]) != D3D_OK &&
        g_Supervisor.d3dDevice->CreateImageSurface(
            this->surfaceSourceInfo[surfaceIdx].Width, this->surfaceSourceInfo[surfaceIdx].Height,
            g_Supervisor.presentParameters.BackBufferFormat, &this->surfaces[surfaceIdx]) != D3D_OK)
    {
        goto fail;
    }
    if (g_Supervisor.d3dDevice->CreateImageSurface(
            this->surfaceSourceInfo[surfaceIdx].Width, this->surfaceSourceInfo[surfaceIdx].Height,
            g_Supervisor.presentParameters.BackBufferFormat, &this->surfacesBis[surfaceIdx]) != D3D_OK)
    {
        goto fail;
    }

    if (D3DXLoadSurfaceFromSurface(this->surfaces[surfaceIdx], NULL, NULL, surface, NULL, NULL, D3DX_FILTER_NONE, 0) !=
        D3D_OK)
    {
        goto fail;
    }

    if (D3DXLoadSurfaceFromSurface(this->surfacesBis[surfaceIdx], NULL, NULL, surface, NULL, NULL, D3DX_FILTER_NONE,
                                   0) != D3D_OK)
    {
        goto fail;
    }

    if (surface != NULL)
    {
        surface->Release();
        surface = NULL;
    }
    free(data);
    return ZUN_SUCCESS;

fail:
    if (surface != NULL)
    {
        surface->Release();
        surface = NULL;
    }
    free(data);
    return ZUN_ERROR;
}

void AnmManager::ReleaseSurface(i32 surfaceIdx)
{
    if (this->surfaces[surfaceIdx] != NULL)
    {
        this->surfaces[surfaceIdx]->Release();
        this->surfaces[surfaceIdx] = NULL;
    }
    if (this->surfacesBis[surfaceIdx] != NULL)
    {
        this->surfacesBis[surfaceIdx]->Release();
        this->surfacesBis[surfaceIdx] = NULL;
    }
}

void AnmManager::CopySurfaceToBackBuffer(i32 surfaceIdx, i32 left, i32 top, i32 x, i32 y)
{
    if (this->surfacesBis[surfaceIdx] == NULL)
    {
        return;
    }

    IDirect3DSurface8 *destSurface;
    if (g_Supervisor.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &destSurface) != D3D_OK)
    {
        return;
    }
    if (this->surfaces[surfaceIdx] == NULL)
    {
        if (g_Supervisor.d3dDevice->CreateRenderTarget(
                this->surfaceSourceInfo[surfaceIdx].Width, this->surfaceSourceInfo[surfaceIdx].Height,
                g_Supervisor.presentParameters.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                &this->surfaces[surfaceIdx]) != D3D_OK)
        {
            if (g_Supervisor.d3dDevice->CreateImageSurface(
                    this->surfaceSourceInfo[surfaceIdx].Width, this->surfaceSourceInfo[surfaceIdx].Height,
                    g_Supervisor.presentParameters.BackBufferFormat, &this->surfaces[surfaceIdx]) != D3D_OK)
            {
                destSurface->Release();
                return;
            }
        }
        if (D3DXLoadSurfaceFromSurface(this->surfaces[surfaceIdx], NULL, NULL, this->surfacesBis[surfaceIdx], NULL,
                                       NULL, D3DX_FILTER_NONE, 0) != D3D_OK)
        {
            destSurface->Release();
            return;
        }
    }

    RECT sourceRect;
    POINT destPoint;
    sourceRect.left = left;
    sourceRect.top = top;
    sourceRect.right = this->surfaceSourceInfo[surfaceIdx].Width;
    sourceRect.bottom = this->surfaceSourceInfo[surfaceIdx].Height;
    destPoint.x = x;
    destPoint.y = y;
    g_Supervisor.d3dDevice->CopyRects(this->surfaces[surfaceIdx], &sourceRect, 1, destSurface, &destPoint);
    destSurface->Release();
}

void AnmManager::DrawEndingRect(i32 surfaceIdx, i32 rectX, i32 rectY, i32 rectLeft, i32 rectTop, i32 width, i32 height)
{
    if (this->surfacesBis[surfaceIdx] == NULL)
    {
        return;
    }

    IDirect3DSurface8 *D3D_Surface;
    if (g_Supervisor.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &D3D_Surface) != D3D_OK)
    {
        return;
    }

    if (this->surfaces[surfaceIdx] == NULL)
    {
        if (g_Supervisor.d3dDevice->CreateRenderTarget(
                this->surfaceSourceInfo[surfaceIdx].Width, this->surfaceSourceInfo[surfaceIdx].Height,
                g_Supervisor.presentParameters.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                &this->surfaces[surfaceIdx]) != D3D_OK)
        {
            if (g_Supervisor.d3dDevice->CreateImageSurface(
                    this->surfaceSourceInfo[surfaceIdx].Width, this->surfaceSourceInfo[surfaceIdx].Height,
                    g_Supervisor.presentParameters.BackBufferFormat, &this->surfaces[surfaceIdx]) != D3D_OK)
            {
                D3D_Surface->Release();
                return;
            }
        }
        if (D3DXLoadSurfaceFromSurface(this->surfaces[surfaceIdx], NULL, NULL, this->surfacesBis[surfaceIdx], NULL,
                                       NULL, D3DX_FILTER_NONE, 0) != D3D_OK)
        {
            D3D_Surface->Release();
            return;
        }
    }

    RECT rect;
    POINT point;
    rect.left = rectLeft;
    rect.top = rectTop;
    rect.right = rectLeft + width;
    rect.bottom = rectTop + height;
    point.x = rectX;
    point.y = rectY;
    g_Supervisor.d3dDevice->CopyRects(this->surfaces[surfaceIdx], &rect, 1, D3D_Surface, &point);
    D3D_Surface->Release();
}

#pragma var_order(rect, destSurface, sourceSurface)
void AnmManager::TakeScreenshot(i32 textureId, i32 left, i32 top, i32 width, i32 height)
{
    LPDIRECT3DSURFACE8 sourceSurface;
    LPDIRECT3DSURFACE8 destSurface;
    RECT rect;

    if (this->textures[textureId] == NULL)
    {
        return;
    }
    if (g_Supervisor.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &sourceSurface) != D3D_OK)
    {
        return;
    }
    if (this->textures[textureId]->GetSurfaceLevel(0, &destSurface) != D3D_OK)
    {
        sourceSurface->Release();
        return;
    }

    rect.left = left;
    rect.top = top;
    rect.right = left + width;
    rect.bottom = top + height;
    if (D3DXLoadSurfaceFromSurface(destSurface, NULL, NULL, sourceSurface, NULL, &rect, D3DX_DEFAULT, 0) != D3D_OK)
    {
        destSurface->Release();
        sourceSurface->Release();
        return;
    }
    destSurface->Release();
    sourceSurface->Release();
    return;
}
}; // namespace th06
