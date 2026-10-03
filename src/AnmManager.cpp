#include "AnmManager.hpp"
#include "Global.hpp"
#include "Supervisor.hpp"
#include "TextHelper.hpp"
#include "ZunMath.hpp"
#include "ZunTimer.hpp"
#include "decomp.hpp"
#include "i18n.hpp"

#include <stdio.h>

namespace th06
{
DIFFABLE_STATIC_SORTED(T3, VertexTex1Xyzrwh, g_PrimitivesToDrawVertexBuf[4]);
DIFFABLE_STATIC_SORTED(T2, VertexTex1DiffuseXyzrwh, g_PrimitivesToDrawNoVertexBuf[4]);
DIFFABLE_STATIC_SORTED(T4, VertexTex1DiffuseXyz, g_PrimitivesToDrawUnknown[4]);
DIFFABLE_STATIC_SORTED(T1, AnmManager *, g_AnmManager);

DIFFABLE_STATIC_ARRAY_ASSIGN(const D3DFORMAT, 6, g_TextureFormatD3D8Mapping) = {
    D3DFMT_UNKNOWN, D3DFMT_A8R8G8B8, D3DFMT_A1R5G5B5, D3DFMT_R5G6B5, D3DFMT_R8G8B8, D3DFMT_A4R4G4B4,
};

#define TEX_FMT_UNKNOWN 0
#define TEX_FMT_A8R8G8B8 1
#define TEX_FMT_A1R5G5B5 2
#define TEX_FMT_R5G6B5 3
#define TEX_FMT_R8G8B8 4
#define TEX_FMT_A4R4G4B4 5

AnmManager::AnmManager()
{
    this->maybeLoadedSpriteCount = 0;

    memset(this, 0, sizeof(AnmManager));

    for (i32 spriteIndex = 0; spriteIndex < MAX_ANM_SPRITES; spriteIndex++)
    {
        this->sprites[spriteIndex].sourceFileIndex = -1;
    }

    g_PrimitivesToDrawVertexBuf[0].position.w = g_PrimitivesToDrawVertexBuf[1].position.w =
        g_PrimitivesToDrawVertexBuf[2].position.w = g_PrimitivesToDrawVertexBuf[3].position.w = 1.0f;
    g_PrimitivesToDrawVertexBuf[0].textureUV.x = 0.0f;
    g_PrimitivesToDrawVertexBuf[0].textureUV.y = 0.0f;
    g_PrimitivesToDrawVertexBuf[1].textureUV.x = 1.0f;
    g_PrimitivesToDrawVertexBuf[1].textureUV.y = 0.0f;
    g_PrimitivesToDrawVertexBuf[2].textureUV.x = 0.0f;
    g_PrimitivesToDrawVertexBuf[2].textureUV.y = 1.0f;
    g_PrimitivesToDrawVertexBuf[3].textureUV.x = 1.0f;
    g_PrimitivesToDrawVertexBuf[3].textureUV.y = 1.0f;

    g_PrimitivesToDrawNoVertexBuf[0].position.w = g_PrimitivesToDrawNoVertexBuf[1].position.w =
        g_PrimitivesToDrawNoVertexBuf[2].position.w = g_PrimitivesToDrawNoVertexBuf[3].position.w = 1.0f;
    g_PrimitivesToDrawNoVertexBuf[0].textureUV.x = 0.0f;
    g_PrimitivesToDrawNoVertexBuf[0].textureUV.y = 0.0f;
    g_PrimitivesToDrawNoVertexBuf[1].textureUV.x = 1.0f;
    g_PrimitivesToDrawNoVertexBuf[1].textureUV.y = 0.0f;
    g_PrimitivesToDrawNoVertexBuf[2].textureUV.x = 0.0f;
    g_PrimitivesToDrawNoVertexBuf[2].textureUV.y = 1.0f;
    g_PrimitivesToDrawNoVertexBuf[3].textureUV.x = 1.0f;
    g_PrimitivesToDrawNoVertexBuf[3].textureUV.y = 1.0f;

    this->vertexBuffer = NULL;
    this->SetCurrentTexture(NULL);
    this->SetCurrentBlendMode(AnmBlendMode_Normal);
    this->SetCurrentColorOp(AnmColorOp_Modulate);
    this->currentTextureFactor = 1;
    this->SetCurrentVertexShader(AnmVertexShader_0);
    this->SetCurrentZWriteDisable(false);
    this->screenshotTextureId = -1;
}

void AnmManager::SetupVertexBuffer()
{
    this->vertexBufferContents[0].position.x = this->vertexBufferContents[2].position.x = -128.0f;
    this->vertexBufferContents[1].position.x = this->vertexBufferContents[3].position.x = 128.0f;

    this->vertexBufferContents[0].position.y = this->vertexBufferContents[1].position.y = -128.0f;
    this->vertexBufferContents[2].position.y = this->vertexBufferContents[3].position.y = 128.0f;

    this->vertexBufferContents[2].position.z = this->vertexBufferContents[3].position.z = 0.0f;
    this->vertexBufferContents[0].position.z = this->vertexBufferContents[1].position.z = 0.0f;

    this->vertexBufferContents[0].textureUV.x = this->vertexBufferContents[2].textureUV.x = 0.0f;
    this->vertexBufferContents[1].textureUV.x = this->vertexBufferContents[3].textureUV.x = 1.0f;
    this->vertexBufferContents[0].textureUV.y = this->vertexBufferContents[1].textureUV.y = 0.0f;
    this->vertexBufferContents[2].textureUV.y = this->vertexBufferContents[3].textureUV.y = 1.0f;

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

    BYTE *buffer;

    if (!g_Supervisor.IsVertexBufferDisabled())
    {
        g_Supervisor.d3dDevice->CreateVertexBuffer(sizeof(this->vertexBufferContents), 0, D3DFVF_TEX1 | D3DFVF_XYZ,
                                                   D3DPOOL_MANAGED, &this->vertexBuffer);

        this->vertexBuffer->Lock(0, 0, &buffer, 0);
        memcpy(buffer, this->vertexBufferContents, sizeof(this->vertexBufferContents));
        this->vertexBuffer->Unlock();

        g_Supervisor.d3dDevice->SetStreamSource(0, g_AnmManager->vertexBuffer, sizeof(RenderVertexInfo));
    }
}

ZunResult AnmManager::LoadTexture(i32 textureIdx, const char *textureName, i32 textureFormat, D3DCOLOR colorKey)
{
    ReleaseTexture(textureIdx);
    this->imageDataArray[textureIdx] = FileSystem::OpenPath(textureName);

    if (this->imageDataArray[textureIdx] == NULL)
    {
        return ZUN_ERROR;
    }

    if (g_Supervisor.Is16bitColorMode())
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

#pragma var_order(surfaceDesc, data, lockedRectDst, lockedRectSrc, textureSrc)
ZunResult AnmManager::LoadTextureAlphaChannel(i32 textureIdx, const char *textureName, i32 textureFormat,
                                              D3DCOLOR colorKey)
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

    D3DSURFACE_DESC surfaceDesc;
    D3DLOCKED_RECT lockedRectDst;
    D3DLOCKED_RECT lockedRectSrc;

    LPDIRECT3DTEXTURE8 textureSrc = NULL;
    u8 *data = FileSystem::OpenPath(textureName);

    if (data == NULL)
    {
        return ZUN_ERROR;
    }

    this->textures[textureIdx]->GetLevelDesc(0, &surfaceDesc);

    if (surfaceDesc.Format != D3DFMT_A8R8G8B8 && surfaceDesc.Format != D3DFMT_A4R4G4B4 &&
        surfaceDesc.Format != D3DFMT_A1R5G5B5)
    {
        g_GameErrorContext.Fatal(TH_ERR_ANMMANAGER_UNK_TEX_FORMAT);
        goto err;
    }

    if (D3DXCreateTextureFromFileInMemoryEx(g_Supervisor.d3dDevice, data, g_LastFileSize, 0, 0, 0, 0,
                                            surfaceDesc.Format, D3DPOOL_SYSTEMMEM, D3DX_FILTER_NONE | D3DX_FILTER_POINT,
                                            D3DX_DEFAULT, colorKey, NULL, NULL, &textureSrc) != D3D_OK)
    {
        goto err;
    }

    if (this->textures[textureIdx]->LockRect(0, &lockedRectDst, NULL, 0) != D3D_OK)
        goto err;

    if (textureSrc->LockRect(0, &lockedRectSrc, NULL, D3DLOCK_NO_DIRTY_UPDATE) != D3D_OK)
        goto err;

    // Copy over the alpha channel from the source to the destination, taking
    // into account the texture format.
    switch (surfaceDesc.Format)
    {
#pragma var_order(dstData, srcData, y, x)
    case D3DFMT_A8R8G8B8: {
        i32 x, y;
        u8 *dstData, *srcData;
        for (y = 0; y < surfaceDesc.Height; y++)
        {
            dstData = (u8 *)lockedRectDst.pBits + y * lockedRectDst.Pitch;
            srcData = (u8 *)lockedRectSrc.pBits + y * lockedRectSrc.Pitch;

            for (x = 0; x < surfaceDesc.Width; x++, srcData += 4, dstData += 4)
            {
                dstData[3] = srcData[0];
            }
        }
        break;
    }
#pragma var_order(dstData, srcData, y, x)
    case D3DFMT_A1R5G5B5: {
        i32 x, y;
        Argb1555Pixel *dstData, *srcData;
        for (y = 0; y < surfaceDesc.Height; y++)
        {
            dstData = (Argb1555Pixel *)((u8 *)lockedRectDst.pBits + y * lockedRectDst.Pitch);
            srcData = (Argb1555Pixel *)((u8 *)lockedRectSrc.pBits + y * lockedRectSrc.Pitch);

            for (x = 0; x < surfaceDesc.Width; x++, srcData++, dstData++)
            {
                dstData->a = srcData->b >> 4;
            }
        }
        break;
    }
#pragma var_order(dstData, srcData, y, x)
    case D3DFMT_A4R4G4B4: {
        i32 x, y;
        Argb4444Pixel *dstData, *srcData;
        for (y = 0; y < surfaceDesc.Height; y++)
        {
            dstData = (Argb4444Pixel *)((u8 *)lockedRectDst.pBits + y * lockedRectDst.Pitch);
            srcData = (Argb4444Pixel *)((u8 *)lockedRectSrc.pBits + y * lockedRectSrc.Pitch);

            for (x = 0; x < surfaceDesc.Width; x++, srcData++, dstData++)
            {
                dstData->a = srcData->b;
            }
        }
        break;
    }
    }

    textureSrc->UnlockRect(0);
    this->textures[textureIdx]->UnlockRect(0);

    SAFE_RELEASE(textureSrc);

    ZUN_FREE(data);
    return ZUN_SUCCESS;

err:
    SAFE_RELEASE(textureSrc);

    ZUN_FREE(data);
    return ZUN_ERROR;
}

ZunResult AnmManager::CreateEmptyTexture(i32 textureIdx, u32 width, u32 height, i32 textureFormat)
{
    D3DXCreateTexture(g_Supervisor.d3dDevice, width, height, 1, 0, g_TextureFormatD3D8Mapping[textureFormat],
                      D3DPOOL_MANAGED, this->textures + textureIdx);

    return ZUN_SUCCESS;
}

#pragma var_order(anm, anmName, rawSprite, index, curOffset)
ZunResult AnmManager::LoadAnm(i32 anmIdx, const char *path, i32 spriteIdxOffset)
{
    this->ReleaseAnm(anmIdx);
    this->anmFiles[anmIdx] = (AnmRawEntry *)FileSystem::OpenPath(path);

    AnmRawEntry *anm = this->anmFiles[anmIdx];

    if (anm == NULL)
    {
        g_GameErrorContext.Fatal(TH_ERR_ANMMANAGER_SPRITE_CORRUPTED, path);
        return ZUN_ERROR;
    }

    anm->textureIdx = anmIdx;

    const char *anmName = (char *)anm + anm->nameOffset;

    if (*anmName == '@')
    {
        this->CreateEmptyTexture(anm->textureIdx, anm->width, anm->height, anm->format);
    }
    else if (this->LoadTexture(anm->textureIdx, anmName, anm->format, anm->colorKey) != ZUN_SUCCESS)
    {
        g_GameErrorContext.Fatal(TH_ERR_ANMMANAGER_TEXTURE_CORRUPTED, anmName);
        return ZUN_ERROR;
    }

    if (anm->mipmapNameOffset != 0)
    {
        anmName = (char *)anm + anm->mipmapNameOffset;
        if (this->LoadTextureAlphaChannel(anm->textureIdx, anmName, anm->format, anm->colorKey) != ZUN_SUCCESS)
        {
            g_GameErrorContext.Fatal(TH_ERR_ANMMANAGER_TEXTURE_CORRUPTED, anmName);
            return ZUN_ERROR;
        }
    }

    anm->spriteIdxOffset = spriteIdxOffset;

    u32 *curOffset = anm->spriteOffsets;

    i32 index;
    AnmRawSprite *rawSprite;

    for (index = 0; index < this->anmFiles[anmIdx]->numSprites; index++, curOffset++)
    {
        rawSprite = (AnmRawSprite *)((u8 *)anm + *curOffset);

        AnmLoadedSprite loadedSprite;
        loadedSprite.sourceFileIndex = this->anmFiles[anmIdx]->textureIdx;
        loadedSprite.startPixelInclusive.x = rawSprite->offset.x;
        loadedSprite.startPixelInclusive.y = rawSprite->offset.y;
        loadedSprite.endPixelInclusive.x = rawSprite->offset.x + rawSprite->size.x;
        loadedSprite.endPixelInclusive.y = rawSprite->offset.y + rawSprite->size.y;
        loadedSprite.textureWidth = anm->width;
        loadedSprite.textureHeight = anm->height;
        this->LoadSprite(rawSprite->id + spriteIdxOffset, &loadedSprite);
    }

    for (index = 0; index < anm->numScripts; index++, curOffset += 2)
    {
        this->scripts[curOffset[0] + spriteIdxOffset] = (AnmRawInstr *)((u8 *)anm + curOffset[1]);
        this->spriteIndices[curOffset[0] + spriteIdxOffset] = spriteIdxOffset;
    }

    this->anmFilesSpriteIndexOffsets[anmIdx] = spriteIdxOffset;

    return ZUN_SUCCESS;
}

#pragma var_order(entry, spriteIdx, spriteIdxOffset, i, byteOffset)
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
            memset(&this->sprites[*spriteIdx + spriteIdxOffset], 0, sizeof(AnmLoadedSprite));
            this->sprites[*spriteIdx + spriteIdxOffset].sourceFileIndex = -1;
        }

        for (i = 0; i < this->anmFiles[anmIdx]->numScripts; i++, byteOffset += 2)
        {
            this->scripts[*byteOffset + spriteIdxOffset] = NULL;
            this->spriteIndices[*byteOffset + spriteIdxOffset] = 0;
        }
        this->anmFilesSpriteIndexOffsets[anmIdx] = 0;
        AnmRawEntry *entry = this->anmFiles[anmIdx];
        this->ReleaseTexture(entry->textureIdx);
        ZUN_FREE(this->anmFiles[anmIdx]);
        this->anmFiles[anmIdx] = NULL;
        this->SetCurrentBlendMode(AnmBlendMode_NotSet);
        this->SetCurrentColorOp(AnmColorOp_NotSet);
        this->SetCurrentVertexShader(AnmVertexShader_NotSet);
        this->SetCurrentTexture(NULL);
    }
}

void AnmManager::ReleaseTexture(i32 textureIdx)
{
    SAFE_RELEASE(this->textures[textureIdx]);
    ZUN_FREE(this->imageDataArray[textureIdx]);
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

ZunResult AnmManager::SetActiveSprite(AnmVm *vm, i32 sprite_index)
{
    if (this->sprites[sprite_index].sourceFileIndex < 0)
    {
        return ZUN_ERROR;
    }

    vm->activeSpriteIndex = sprite_index;
    vm->sprite = this->sprites + sprite_index;
    D3DXMatrixIdentity(&vm->matrix);
    vm->matrix.m[0][0] = vm->sprite->widthPx / vm->sprite->textureWidth;
    vm->matrix.m[1][1] = vm->sprite->heightPx / vm->sprite->textureHeight;

    return ZUN_SUCCESS;
}

void AnmManager::SetAndExecuteScript(AnmVm *vm, AnmRawInstr *beginingOfScript)
{
    vm->flags.flip = AnmVmMirror_None;
    vm->Initialize();
    vm->beginingOfScript = beginingOfScript;
    vm->currentInstruction = vm->beginingOfScript;

    vm->currentTimeInScript = 0;

    vm->flags.isVisible = false;
    if (beginingOfScript != NULL)
    {
        this->ExecuteScript(vm);
    }
}

void AnmManager::SetRenderStateForVm(AnmVm *vm)
{
    if (this->currentBlendMode != vm->flags.blendMode)
    {
        this->currentBlendMode = vm->flags.blendMode;
        if (this->currentBlendMode == AnmBlendMode_Normal)
        {
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        }
        else // AnmBlendMode_Additive
        {
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        }
    }
    if (!g_Supervisor.IsHardwareBlendingDisabled() && !g_Supervisor.IsColorCompositingDisabled() &&
        this->currentColorOp != vm->flags.colorOp)
    {
        this->currentColorOp = vm->flags.colorOp;
        if (this->currentColorOp == AnmColorOp_Modulate)
        {
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        }
        else // AnmColorOp_Add
        {
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
        }
    }
    if (!g_Supervisor.IsVertexBufferDisabled())
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
    if (!g_Supervisor.IsDepthTestDisabled() && this->currentZWriteDisable != vm->flags.zWriteDisable)
    {
        this->currentZWriteDisable = vm->flags.zWriteDisable;
        if (!this->currentZWriteDisable)
        {
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        }
        else
        {
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        }
    }
}

ZunResult AnmManager::DrawInner(AnmVm *vm, ZunBool roundVertices)
{
    static const f32 g_ZeroPointFive = 0.5f;
    if (roundVertices)
    {
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
    if (this->currentVertexShader != AnmVertexShader_2)
    {
        if (!g_Supervisor.IsVertexBufferDisabled())
        {
            g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_XYZRHW);
        }
        else
        {
            g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
        }
        this->currentVertexShader = AnmVertexShader_2;
    }
    this->SetRenderStateForVm(vm);
    if (!g_Supervisor.IsVertexBufferDisabled())
    {
        g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_PrimitivesToDrawVertexBuf,
                                                sizeof(VertexTex1Xyzrwh));
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
        g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_PrimitivesToDrawNoVertexBuf,
                                                sizeof(VertexTex1DiffuseXyzrwh));
    }
    return ZUN_SUCCESS;
}

ZunResult AnmManager::DrawNoRotation(AnmVm *vm)
{
    if (!vm->flags.isVisible)
    {
        return ZUN_ERROR;
    }
    if (!vm->flags.isVisibleOverride)
    {
        return ZUN_ERROR;
    }
    if (vm->color == COLOR_NONE)
    {
        return ZUN_ERROR;
    }
    float fVar2 = vm->sprite->widthPx * vm->scaleX / 2.0f;
    float fVar3 = vm->sprite->heightPx * vm->scaleY / 2.0f;
    if (!(vm->flags.anchor & AnmVmAnchor_Left))
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
    if (!(vm->flags.anchor & AnmVmAnchor_Top))
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
    return this->DrawInner(vm, RENDER_VERTICES_ROUND_INPUTS);
}

void AnmManager::TranslateRotation(VertexTex1Xyzrwh *param_1, f32 x, f32 y, f32 sine, f32 cosine, f32 xOffset,
                                   f32 yOffset)
{
    param_1->position.x = x * cosine + y * sine + xOffset;
    param_1->position.y = -x * sine + y * cosine + yOffset;
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
    if (!vm->flags.isVisible)
    {
        return ZUN_ERROR;
    }
    if (!vm->flags.isVisibleOverride)
    {
        return ZUN_ERROR;
    }
    if (vm->color == COLOR_NONE)
    {
        return ZUN_ERROR;
    }
    z = vm->rotation.z;
    sincos(z, zSine, zCosine);
    xOffset = rintf(vm->pos.x);
    yOffset = rintf(vm->pos.y);
    spriteXCenter = rintf(vm->sprite->widthPx * vm->scaleX / 2.0f);
    spriteYCenter = rintf(vm->sprite->heightPx * vm->scaleY / 2.0f);
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
    if (vm->flags.anchor & AnmVmAnchor_Left)
    {
        g_PrimitivesToDrawVertexBuf[0].position.x += spriteXCenter;
        g_PrimitivesToDrawVertexBuf[1].position.x += spriteXCenter;
        g_PrimitivesToDrawVertexBuf[2].position.x += spriteXCenter;
        g_PrimitivesToDrawVertexBuf[3].position.x += spriteXCenter;
    }
    if (vm->flags.anchor & AnmVmAnchor_Top)
    {
        g_PrimitivesToDrawVertexBuf[0].position.y += spriteYCenter;
        g_PrimitivesToDrawVertexBuf[1].position.y += spriteYCenter;
        g_PrimitivesToDrawVertexBuf[2].position.y += spriteYCenter;
        g_PrimitivesToDrawVertexBuf[3].position.y += spriteYCenter;
    }
    return this->DrawInner(vm, RENDER_VERTICES_DEFAULT);
}

ZunResult AnmManager::DrawFacingCamera(AnmVm *vm)
{
    if (!vm->flags.isVisible)
    {
        return ZUN_ERROR;
    }
    if (!vm->flags.isVisibleOverride)
    {
        return ZUN_ERROR;
    }
    if (vm->color == COLOR_NONE)
    {
        return ZUN_ERROR;
    }

    f32 centerX = vm->sprite->widthPx * vm->scaleX / 2.0f;
    f32 centerY = vm->sprite->heightPx * vm->scaleY / 2.0f;
    if (!(vm->flags.anchor & AnmVmAnchor_Left))
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
    if (!(vm->flags.anchor & AnmVmAnchor_Top))
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
    return this->DrawInner(vm, RENDER_VERTICES_DEFAULT);
}

#pragma var_order(textureMatrix, rotationMatrix, worldTransformMatrix)
ZunResult AnmManager::Draw3(AnmVm *vm)
{
    D3DXMATRIX worldTransformMatrix;
    D3DXMATRIX rotationMatrix;
    D3DXMATRIX textureMatrix;

    if (!vm->flags.isVisible)
    {
        return ZUN_ERROR;
    }
    if (!vm->flags.isVisibleOverride)
    {
        return ZUN_ERROR;
    }
    if (vm->color == COLOR_NONE)
    {
        return ZUN_ERROR;
    }

    worldTransformMatrix = vm->matrix;
    worldTransformMatrix.m[0][0] *= vm->scaleX;
    worldTransformMatrix.m[1][1] *= -vm->scaleY;

    // NOTE: These comparisons being doubles is not a typo
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

    if (!(vm->flags.anchor & AnmVmAnchor_Left))
    {
        worldTransformMatrix.m[3][0] = vm->pos.x;
    }
    else
    {
        worldTransformMatrix.m[3][0] = fabsf(vm->sprite->widthPx * vm->scaleX / 2.0f) + vm->pos.x;
    }

    if (!(vm->flags.anchor & AnmVmAnchor_Top))
    {
        worldTransformMatrix.m[3][1] = -vm->pos.y;
    }
    else
    {
        worldTransformMatrix.m[3][1] = -vm->pos.y - fabsf(vm->sprite->heightPx * vm->scaleY / 2.0f);
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
    if (this->currentVertexShader != AnmVertexShader_3)
    {
        if (!g_Supervisor.IsVertexBufferDisabled())
        {
            g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_XYZ);
            g_Supervisor.d3dDevice->SetStreamSource(0, this->vertexBuffer, sizeof(RenderVertexInfo));
        }
        else
        {
            g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZ);
        }
        this->currentVertexShader = AnmVertexShader_3;
    }

    // Reset the render state based on the settings fo the given VM.
    this->SetRenderStateForVm(vm);

    // Draw the VM.
    if (!g_Supervisor.IsVertexBufferDisabled())
    {
        g_Supervisor.d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    }
    else
    {
        g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_PrimitivesToDrawUnknown,
                                                sizeof(VertexTex1DiffuseXyz));
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
    if (!vm->flags.isVisibleOverride)
    {
        return ZUN_ERROR;
    }

    if (vm->rotation.x != 0.0f || vm->rotation.y != 0.0f || vm->rotation.z != 0.0f)
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
    if (vm->flags.anchor & AnmVmAnchor_Left)
    {
        worldTransformMatrix.m[3][0] += (vm->sprite->widthPx * vm->scaleX) / 2.0f;
    }
    if (vm->flags.anchor & AnmVmAnchor_Top)
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
        if (this->currentVertexShader != AnmVertexShader_3)
        {
            if (!g_Supervisor.IsVertexBufferDisabled())
            {
                g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_XYZ);
                g_Supervisor.d3dDevice->SetStreamSource(0, this->vertexBuffer, sizeof(RenderVertexInfo));
            }
            else
            {
                g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZ);
            }
            this->currentVertexShader = AnmVertexShader_3;
        }
    }
    this->SetRenderStateForVm(vm);
    if (!g_Supervisor.IsVertexBufferDisabled())
    {
        g_Supervisor.d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    }
    else
    {
        g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_PrimitivesToDrawUnknown,
                                                sizeof(VertexTex1DiffuseXyz));
    }
    return ZUN_SUCCESS;
}

#define GET_ARG(type, num) ((type *)curInstr->args)[num]
#define GET_INT_ARG(num) GET_ARG(i32, num)
#define GET_FLOAT_ARG(num) GET_ARG(float, num)

i32 AnmManager::ExecuteScript(AnmVm *vm)
{
    if (vm->currentInstruction == NULL)
    {
        return 1;
    }

    if (vm->pendingInterrupt != 0)
    {
        goto run_interrupt;
    }

    AnmRawInstr *curInstr;
    while (curInstr = vm->currentInstruction, curInstr->time <= vm->currentTimeInScript)
    {
        switch (curInstr->opcode)
        {
        case ANM_OPCODE_ANM_DELETE:
            vm->flags.isVisible = false;
            // fallthrough
        case ANM_OPCODE_ANM_STATIC:
            vm->currentInstruction = NULL;
            return 1;
        case ANM_OPCODE_SET_SPRITE:
            vm->flags.isVisible = true;
            this->SetActiveSprite(vm, GET_INT_ARG(0) + this->spriteIndices[vm->anmFileIndex]);
            vm->timeOfLastSpriteSet = vm->currentTimeInScript;
            break;
        case ANM_OPCODE_SPRITE_SET_RAND: {
            vm->flags.isVisible = true;
            u32 *args = (u32 *)curInstr->args;
            this->SetActiveSprite(vm,
                                  args[0] + g_Rng.GetRandomU16InRange(args[1]) + this->spriteIndices[vm->anmFileIndex]);
            vm->timeOfLastSpriteSet = vm->currentTimeInScript;
            break;
        }
        case ANM_OPCODE_SCALE:
            vm->scaleX = GET_FLOAT_ARG(0);
            vm->scaleY = GET_FLOAT_ARG(1);
            break;
        case ANM_OPCODE_ALPHA:
            COLOR_SET_COMPONENT(vm->color, COLOR_ALPHA_BYTE_IDX, GET_INT_ARG(0) & 0xff);
            break;
        case ANM_OPCODE_COLOR:
            vm->color = COLOR_COMBINE_ALPHA(GET_INT_ARG(0), vm->color);
            break;
        case ANM_OPCODE_JUMP:
            vm->currentInstruction = (AnmRawInstr *)((u32)vm->beginingOfScript + GET_INT_ARG(0));
            vm->currentTimeInScript.current = vm->currentInstruction->time;
            continue;
        case ANM_OPCODE_SCALE_FLIP_X:
            vm->flags.flip ^= AnmVmMirror_X;
            vm->scaleX *= -1.0f;
            break;
        case ANM_OPCODE_POSITION_MODE:
            vm->flags.usePosOffset = GET_INT_ARG(0);
            break;
        case ANM_OPCODE_SCALE_FLIP_Y:
            vm->flags.flip ^= AnmVmMirror_Y;
            vm->scaleY *= -1.0f;
            break;
        case ANM_OPCODE_ROTATION: {
            f32 *rotationVals = (f32 *)curInstr->args;
            vm->rotation.x = *rotationVals++;
            vm->rotation.y = *rotationVals++;
            vm->rotation.z = *rotationVals;
            break;
        }
        case ANM_OPCODE_ROTATION_SPEED: {
            f32 *angleVelVals = (f32 *)curInstr->args;
            vm->angleVel.x = *angleVelVals++;
            vm->angleVel.y = *angleVelVals++;
            vm->angleVel.z = *angleVelVals;
            break;
        }
        case ANM_OPCODE_SCALE_SPEED: {
            f32 *scaleInterpVals = (f32 *)curInstr->args;
            vm->scaleInterpFinalX = *scaleInterpVals++;
            vm->scaleInterpFinalY = *scaleInterpVals;
            vm->scaleInterpEndTime = 0;
            break;
        }
        case ANM_OPCODE_SCALE_INTERP_LINEAR: {
            f32 *scaleInterpVals = (f32 *)curInstr->args;
            vm->scaleInterpFinalX = *scaleInterpVals++;
            vm->scaleInterpFinalY = *scaleInterpVals++;
            vm->scaleInterpEndTime = *(u16 *)scaleInterpVals;
            vm->scaleInterpTime = 0;
            vm->scaleInterpInitialX = vm->scaleX;
            vm->scaleInterpInitialY = vm->scaleY;
            break;
        }
        case ANM_OPCODE_ALPHA_INTERP_LINEAR: {
            u32 *alphaInterpVals = (u32 *)curInstr->args;
            vm->alphaInterpInitial = vm->color;
            vm->alphaInterpFinal = COLOR_SET_ALPHA2(vm->color, alphaInterpVals[0]);
            vm->alphaInterpEndTime = alphaInterpVals[1];
            vm->alphaInterpTime = 0;
            break;
        }
        case ANM_OPCODE_BLEND_MODE_ADDITIVE:
            vm->flags.blendMode = AnmBlendMode_Additive;
            break;
        case ANM_OPCODE_BLEND_MODE_NORMAL:
            vm->flags.blendMode = AnmBlendMode_Normal;
            break;
        case ANM_OPCODE_MOVE_POSITION:
            if (!vm->flags.usePosOffset)
            {
                vm->pos = D3DXVECTOR3(GET_FLOAT_ARG(0), GET_FLOAT_ARG(1), GET_FLOAT_ARG(2));
            }
            else
            {
                vm->posOffset = D3DXVECTOR3(GET_FLOAT_ARG(0), GET_FLOAT_ARG(1), GET_FLOAT_ARG(2));
            }
            break;
        case ANM_OPCODE_MOVE_POSITION_INTERP_ACCELERATE_SLOW:
            vm->flags.moveInterpMode = AnmVmInterp_AccelerateSlow;
            goto move_position_interp_common;
        case ANM_OPCODE_MOVE_POSITION_INTERP_DECELERATE_SLOW:
            vm->flags.moveInterpMode = AnmVmInterp_DecelerateSlow;
            goto move_position_interp_common;
        case ANM_OPCODE_MOVE_POSITION_INTERP_LINEAR:
            vm->flags.moveInterpMode = AnmVmInterp_Linear;
        move_position_interp_common:
            if (!vm->flags.usePosOffset)
            {
                vm->posInterpInitial = vm->pos;
            }
            else
            {
                vm->posInterpInitial = vm->posOffset;
            }
            vm->posInterpFinal = D3DXVECTOR3(GET_FLOAT_ARG(0), GET_FLOAT_ARG(1), GET_FLOAT_ARG(2));
            vm->posInterpEndTime = GET_INT_ARG(3);
            vm->posInterpTime = 0;
            break;
        case ANM_OPCODE_ANM_HALT_INVISIBLE:
            vm->flags.isVisible = false;
        case ANM_OPCODE_ANM_HALT: {
            if (vm->pendingInterrupt == 0)
            {
                vm->flags.isStopped = true;
                vm->currentTimeInScript--;
                goto break_parser;
            }
        run_interrupt:
            AnmRawInstr *nextInstr = NULL;
            curInstr = vm->beginingOfScript;
            while ((curInstr->opcode != ANM_OPCODE_INTERRUPT_LABEL || vm->pendingInterrupt != GET_INT_ARG(0)) &&
                   curInstr->opcode != ANM_OPCODE_ANM_DELETE && curInstr->opcode != ANM_OPCODE_ANM_STATIC)
            {
                if (curInstr->opcode == ANM_OPCODE_INTERRUPT_LABEL && GET_INT_ARG(0) == -1)
                {
                    nextInstr = curInstr;
                }
                curInstr = (AnmRawInstr *)((u32)(curInstr + 1) + curInstr->argsSize);
            }

            vm->pendingInterrupt = 0;
            vm->flags.isStopped = false;
            if (curInstr->opcode != ANM_OPCODE_INTERRUPT_LABEL)
            {
                if (nextInstr == NULL)
                {
                    vm->currentTimeInScript--;
                    goto break_parser;
                }
                curInstr = nextInstr;
            }

            curInstr = (AnmRawInstr *)((u32)(curInstr + 1) + curInstr->argsSize);
            vm->currentInstruction = curInstr;
            vm->currentTimeInScript = vm->currentInstruction->time;
            vm->flags.isVisible = true;
            continue;
        }
        case ANM_OPCODE_ANM_FLAG_VISIBLE:
            vm->flags.isVisible = GET_INT_ARG(0);
            break;
        case ANM_OPCODE_ANCHOR_TOP_LEFT:
            vm->flags.anchor = AnmVmAnchor_TopLeft;
            break;
        case ANM_OPCODE_SET_AUTO_ROTATE:
            vm->autoRotate = GET_INT_ARG(0);
            break;
        case ANM_OPCODE_SCROLL_SET_X:
            vm->uvScrollPos.x += GET_FLOAT_ARG(0);
            if (vm->uvScrollPos.x >= 1.0f)
            {
                vm->uvScrollPos.x -= 1.0f;
            }
            else if (vm->uvScrollPos.x < 0.0f)
            {
                vm->uvScrollPos.x += 1.0f;
            }
            break;
        case ANM_OPCODE_SCROLL_SET_Y:
            vm->uvScrollPos.y += GET_FLOAT_ARG(0);
            if (vm->uvScrollPos.y >= 1.0f)
            {
                vm->uvScrollPos.y -= 1.0f;
            }
            else if (vm->uvScrollPos.y < 0.0f)
            {
                vm->uvScrollPos.y += 1.0f;
            }
            break;
        case ANM_OPCODE_FLAG_DISABLE_Z_WRITE:
            vm->flags.zWriteDisable = GET_INT_ARG(0);
            break;
        case ANM_OPCODE_NOP:
        case ANM_OPCODE_INTERRUPT_LABEL:
        default:
            break;
        }
        vm->currentInstruction = (AnmRawInstr *)((u32)(curInstr + 1) + curInstr->argsSize);
    }

break_parser:
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
        vm->scaleInterpTime++;
        if ((i32)vm->scaleInterpTime >= vm->scaleInterpEndTime)
        {
            vm->scaleY = vm->scaleInterpFinalY;
            vm->scaleX = vm->scaleInterpFinalX;
            vm->scaleInterpEndTime = 0;
            vm->scaleInterpFinalY = 0.0f;
            vm->scaleInterpFinalX = 0.0f;
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
        if (vm->flags.flip & AnmVmMirror_X)
        {
            vm->scaleX *= -1.0f;
        }
        if (vm->flags.flip & AnmVmMirror_Y)
        {
            vm->scaleY *= -1.0f;
        }
    }
    else
    {
        vm->scaleY = g_Supervisor.effectiveFramerateMultiplier * vm->scaleInterpFinalY + vm->scaleY;
        vm->scaleX = g_Supervisor.effectiveFramerateMultiplier * vm->scaleInterpFinalX + vm->scaleX;
    }

#pragma var_order(colors, alphaInterpVal, colorInterp, colorIdx)
    if (vm->alphaInterpEndTime > 0)
    {
        vm->alphaInterpTime++;
        D3DCOLOR colors[2];
        colors[0] = vm->alphaInterpInitial;
        colors[1] = vm->alphaInterpFinal;
        float alphaInterpVal = vm->alphaInterpTime.AsFramesFloat() / (f32)vm->alphaInterpEndTime;
        if (alphaInterpVal >= 1.0f)
        {
            alphaInterpVal = 1.0f;
        }
        i32 colorIdx;
        i32 colorInterp;
        for (colorIdx = 0; colorIdx < 4; colorIdx++)
        {
            colorInterp =
                ((f32)COLOR_GET_COMPONENT(colors[1], colorIdx) - (f32)COLOR_GET_COMPONENT(colors[0], colorIdx)) *
                    alphaInterpVal +
                COLOR_GET_COMPONENT(colors[0], colorIdx);
            if (colorInterp < 0)
            {
                colorInterp = 0;
            }
            COLOR_SET_COMPONENT(colors[0], colorIdx, colorInterp >= 256 ? 255 : colorInterp);
        }
        vm->color = colors[0];
        if ((i32)vm->alphaInterpTime >= vm->alphaInterpEndTime)
        {
            vm->alphaInterpEndTime = 0;
        }
    }

    if (vm->posInterpEndTime != 0)
    {
        float interpVal = vm->posInterpTime.AsFramesFloat() / (f32)vm->posInterpEndTime;
        if (interpVal >= 1.0f)
        {
            interpVal = 1.0f;
        }
        switch (vm->flags.moveInterpMode)
        {
        case AnmVmInterp_DecelerateSlow:
            interpVal = 1.0f - interpVal;
            interpVal *= interpVal;
            interpVal = 1.0f - interpVal;
            break;
        case AnmVmInterp_AccelerateSlow:
            interpVal = 1.0f - interpVal;
            interpVal = interpVal * interpVal * interpVal * interpVal;
            interpVal = 1.0f - interpVal;
            break;
        }
        if (!vm->flags.usePosOffset)
        {
            vm->pos.x = interpVal * vm->posInterpFinal.x + (1.0f - interpVal) * vm->posInterpInitial.x;
            vm->pos.y = interpVal * vm->posInterpFinal.y + (1.0f - interpVal) * vm->posInterpInitial.y;
            vm->pos.z = interpVal * vm->posInterpFinal.z + (1.0f - interpVal) * vm->posInterpInitial.z;
        }
        else
        {
            vm->posOffset.x = interpVal * vm->posInterpFinal.x + (1.0f - interpVal) * vm->posInterpInitial.x;
            vm->posOffset.y = interpVal * vm->posInterpFinal.y + (1.0f - interpVal) * vm->posInterpInitial.y;
            vm->posOffset.z = interpVal * vm->posInterpFinal.z + (1.0f - interpVal) * vm->posInterpInitial.z;
        }

        if ((i32)vm->posInterpTime >= vm->posInterpEndTime)
        {
            vm->posInterpEndTime = 0;
        }
        vm->posInterpTime++;
    }
    vm->currentTimeInScript++;
    return 0;
}

void AnmManager::DrawTextToSprite(u32 textureDstIdx, i32 xPos, i32 yPos, i32 spriteWidth, i32 spriteHeight,
                                  i32 fontWidth, i32 fontHeight, ZunColor textColor, ZunColor shadowColor,
                                  const char *strToPrint)
{
    if (fontWidth <= 0)
    {
        fontWidth = DEFAULT_ANM_FONT_SIZE;
    }
    if (fontHeight <= 0)
    {
        fontHeight = DEFAULT_ANM_FONT_SIZE;
    }
    TextHelper::RenderTextToTexture(xPos, yPos, spriteWidth, spriteHeight, fontWidth, fontHeight, textColor,
                                    shadowColor, strToPrint, this->textures[textureDstIdx]);
}

#pragma var_order(args, buffer, fontWidth)
void AnmManager::DrawVmTextFmt(AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt, ...)
{
    char buffer[64];
    va_list args;

    i32 fontWidth = vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
    this->DrawTextToSprite(vm->sprite->sourceFileIndex, vm->sprite->startPixelInclusive.x,
                           vm->sprite->startPixelInclusive.y, vm->sprite->textureWidth, vm->sprite->textureHeight,
                           fontWidth, vm->fontHeight, textColor, shadowColor, buffer);
    vm->flags.isVisible = true;
}

#pragma var_order(args, secondPartStartX, buffer, fontWidth)
void AnmManager::DrawStringFormat(AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt, ...)
{
    char buffer[64];
    va_list args;

    i32 fontWidth = vm->fontWidth <= 0 ? DEFAULT_ANM_FONT_SIZE : vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
    this->DrawTextToSprite(vm->sprite->sourceFileIndex, vm->sprite->startPixelInclusive.x,
                           vm->sprite->startPixelInclusive.y, vm->sprite->textureWidth, vm->sprite->textureHeight,
                           fontWidth, vm->fontHeight, textColor, shadowColor, " ");
    i32 secondPartStartX = vm->sprite->startPixelInclusive.x + vm->sprite->textureWidth -
                           ((f32)strlen(buffer) * (f32)(fontWidth + 1) / 2.0f);
    this->DrawTextToSprite(vm->sprite->sourceFileIndex, secondPartStartX, vm->sprite->startPixelInclusive.y,
                           vm->sprite->textureWidth, vm->sprite->textureHeight, fontWidth, vm->fontHeight, textColor,
                           shadowColor, buffer);
    vm->flags.isVisible = true;
}

#pragma var_order(args, secondPartStartX, buffer, fontWidth)
void AnmManager::DrawStringFormat2(AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt, ...)
{
    char buffer[64];
    va_list args;

    i32 fontWidth = vm->fontWidth <= 0 ? DEFAULT_ANM_FONT_SIZE : vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
    this->DrawTextToSprite(vm->sprite->sourceFileIndex, vm->sprite->startPixelInclusive.x,
                           vm->sprite->startPixelInclusive.y, vm->sprite->textureWidth, vm->sprite->textureHeight,
                           fontWidth, vm->fontHeight, textColor, shadowColor, " ");
    i32 secondPartStartX = vm->sprite->startPixelInclusive.x + vm->sprite->textureWidth / 2.0f -
                           ((f32)strlen(buffer) * (f32)(fontWidth + 1) / 4.0f);
    this->DrawTextToSprite(vm->sprite->sourceFileIndex, secondPartStartX, vm->sprite->startPixelInclusive.y,
                           vm->sprite->textureWidth, vm->sprite->textureHeight, fontWidth, vm->fontHeight, textColor,
                           shadowColor, buffer);
    vm->flags.isVisible = true;
}

ZunResult AnmManager::LoadSurface(i32 surfaceIdx, const char *path)
{
    if (this->surfaces[surfaceIdx] != NULL)
    {
        this->ReleaseSurface(surfaceIdx);
    }
    u8 *data = FileSystem::OpenPath(path);
    if (data == NULL)
    {
        g_GameErrorContext.Fatal(TH_ERR_CANNOT_BE_LOADED, path);
        return ZUN_ERROR;
    }

    LPDIRECT3DSURFACE8 surface;
    if (g_Supervisor.d3dDevice->CreateImageSurface(640, 1024, g_Supervisor.presentParameters.BackBufferFormat,
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

    SAFE_RELEASE(surface);
    ZUN_FREE(data);
    return ZUN_SUCCESS;

fail:
    SAFE_RELEASE(surface);
    ZUN_FREE(data);
    return ZUN_ERROR;
}

void AnmManager::ReleaseSurface(i32 surfaceIdx)
{
    SAFE_RELEASE(this->surfaces[surfaceIdx]);
    SAFE_RELEASE(this->surfacesBis[surfaceIdx]);
}

void AnmManager::CopySurfaceToBackBuffer(i32 surfaceIdx, i32 left, i32 top, i32 x, i32 y)
{
    if (this->surfacesBis[surfaceIdx] == NULL)
    {
        return;
    }

    LPDIRECT3DSURFACE8 destSurface;
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

    LPDIRECT3DSURFACE8 D3D_Surface;
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
    if (this->textures[textureId] == NULL)
    {
        return;
    }

    LPDIRECT3DSURFACE8 sourceSurface;
    if (g_Supervisor.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &sourceSurface) != D3D_OK)
    {
        return;
    }

    LPDIRECT3DSURFACE8 destSurface;
    if (this->textures[textureId]->GetSurfaceLevel(0, &destSurface) != D3D_OK)
    {
        sourceSurface->Release();
        return;
    }

    RECT rect;
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
}
AnmManager::~AnmManager()
{
}

} // namespace th06
