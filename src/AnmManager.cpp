#include "AnmManager.hpp"
#include "AnmVm.hpp"
#include "FileSystem.hpp"
#include "GameErrorContext.hpp"
#include "Rng.hpp"
#include "Supervisor.hpp"
#include "TextHelper.hpp"
#include "ZunMath.hpp"
#include "graphics/GLFunc.hpp"
#include "i18n.hpp"
#include "utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_surface.h>

template <typename T>
constexpr int countr_zero(T x) noexcept {
    static_assert(std::is_unsigned_v<T>, "countr_zero requires an unsigned type!");

#if defined(__clang__) || defined(__GNUC__)
    if constexpr (sizeof(T) <= sizeof(unsigned int))
        return __builtin_ctz(static_cast<unsigned int>(x));
    else if constexpr (sizeof(T) <= sizeof(unsigned long))
        return __builtin_ctzl(static_cast<unsigned long>(x));
    else
        return __builtin_ctzll(static_cast<unsigned long long>(x));

#elif defined(_MSC_VER)
    unsigned long index;
    if constexpr (sizeof(T) <= 4) {
        _BitScanForward(&index, static_cast<unsigned long>(x));
    } else {
        _BitScanForward64(&index, static_cast<unsigned long long>(x));
    }
    return static_cast<int>(index);

#else
    constexpr int debruijn32[32] = {
        0, 1, 28, 2, 29, 14, 24, 3,
        30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7,
        26, 12, 18, 6, 11, 5, 10, 9
    };
    // :lesanae:
    return debruijn32[((x & -x) * 0x077CB531u) >> 27];
#endif
}

// replaces X & ~(1 << countr_zero(X))
template <typename T>
T turn_off_rightmost_one(T val) {
    return val & (val - 1);
}
// replaces X & 1 << countr_zero(X)
template <typename T>
T isolate_rightmost_one(T val) {
    return val & -val;
}

template <typename T>
constexpr T bit_ceil(T x) noexcept {
    static_assert(std::is_unsigned_v<T>, "bit_ceil requires an unsigned integer type");

    if (x <= 1)
        return T(1);

    --x;
    for (size_t shift = 1; shift < std::numeric_limits<T>::digits; shift <<= 1)
        x |= x >> shift;

    return x + 1;
}

VertexTex1Xyzrhw g_PrimitivesToDrawVertexBuf[4];
VertexTex1DiffuseXyzrhw g_PrimitivesToDrawNoVertexBuf[4];
VertexTex1DiffuseXyz g_PrimitivesToDrawUnknown[4];
AnmManager *g_AnmManager;

SDL_PixelFormatEnum g_TextureFormatSDLMapping[6] = {
    SDL_PIXELFORMAT_UNKNOWN, SDL_PIXELFORMAT_RGBA32, SDL_PIXELFORMAT_RGBA5551,
    SDL_PIXELFORMAT_RGB565, SDL_PIXELFORMAT_RGB24, SDL_PIXELFORMAT_RGBA4444
};

GLenum g_TextureFormatGLFormatMapping[6] = {
    0, GL_RGBA, GL_RGBA,
    GL_RGB, GL_RGB, GL_RGBA
};

GLenum g_TextureFormatGLTypeMapping[6] = {
    0,
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT_5_5_5_1,
    GL_UNSIGNED_SHORT_5_6_5,
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT_4_4_4_4
};

u8 g_TextureFormatBytesPerPixel[6] = { 0, 4, 2, 2, 3, 2 };

void AnmManager::CreateTextureObject() {
    g_glFuncTable.glGenTextures(1, &this->currentTextureHandle);
    g_glFuncTable.glBindTexture(GL_TEXTURE_2D, this->currentTextureHandle);
    g_glFuncTable.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

SDL_Surface *AnmManager::LoadToSurfaceWithFormat(const char *filename, SDL_PixelFormatEnum format, u8 **fileData) {
    u8 *data = FileSystem::OpenPath(filename);

    if (data == NULL) {
        return NULL;
    }

    SDL_RWops *rwData = SDL_RWFromConstMem(data, g_LastFileSize);

    if (rwData == NULL) {
        std::free(data);
        return NULL;
    }

    SDL_Surface *imageSrcSurface = IMG_Load_RW(rwData, 1);

    if (imageSrcSurface == NULL) {
        utils::DebugPrint2("%s", SDL_GetError());
        std::free(data);
        return NULL;
    }

    SDL_Surface *imageTargetSurface = SDL_ConvertSurfaceFormat(imageSrcSurface, format, 0);

    SDL_FreeSurface(imageSrcSurface);

    if (imageTargetSurface != NULL && fileData != NULL) {
        *fileData = data;
    } else {
        std::free(data);
    }

    return imageTargetSurface;
}

u8 *AnmManager::ExtractSurfacePixels(SDL_Surface *src, u8 pixelDepth) {
    SDL_LockSurface(src);

    const i32 dstPitch = src->w * pixelDepth;
    const i32 srcPitch = src->pitch;

    u8 *pixelData = new u8[dstPitch * src->h];
    u8 *dstPtr = pixelData;
    u8 *srcPtr = (u8 *)src->pixels;

    for (int i = 0; i < src->h; i++) {
        std::memcpy(dstPtr, srcPtr, dstPitch);
        dstPtr += dstPitch;
        srcPtr += srcPitch;
    }

    SDL_UnlockSurface(src);

    return pixelData;
}

void AnmManager::FlipSurface(SDL_Surface *surface) {
    if (surface->h < 2) {
        return;
    }

    SDL_LockSurface(surface);

    u8 *copyBuf = new u8[surface->h / 2 * surface->pitch];

    u32 lowIndex = 0;
    u8 *highPtr = ((u8 *)surface->pixels) + (surface->h - 1) * surface->pitch;

    std::memcpy(copyBuf, surface->pixels, surface->h / 2 * surface->pitch);

    for (int i = 0; i < surface->h / 2; i++) {
        std::memcpy(((u8 *)surface->pixels) + lowIndex, highPtr, surface->pitch);
        std::memcpy(highPtr, copyBuf + lowIndex, surface->pitch);

        lowIndex += surface->pitch;
        highPtr -= surface->pitch;
    }

    SDL_UnlockSurface(surface);

    delete[] copyBuf;
}

void AnmManager::ReleaseSurfaces(void) {
    for (i32 idx = 0; idx < ARRAY_SIZE_SIGNED(this->surfaces); idx++) {
        if (this->surfaces[idx] != NULL) {
            SDL_FreeSurface(this->surfaces[idx]);
            this->surfaces[idx] = NULL;
        }
    }
}

void AnmManager::TakeScreenshotIfRequested() {
    if (this->screenshotTextureId >= 0) {
        this->TakeScreenshot(this->screenshotTextureId, this->screenshotLeft, this->screenshotTop, this->screenshotWidth, this->screenshotHeight);
        this->screenshotTextureId = -1;
    }
    return;
}

AnmManager::~AnmManager() {
    if (this->dummyTextureHandle != 0) {
        g_glFuncTable.glDeleteTextures(1, &this->dummyTextureHandle);
        this->dummyTextureHandle = 0;
    }

    IMG_Quit();
}

// void AnmManager::ReleaseVertexBuffer()
// {
//     if (this->vertexBuffer != NULL)
//     {
//         this->vertexBuffer->Release();
//         this->vertexBuffer = NULL;
//     }
// }

AnmManager::AnmManager() {
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    this->maybeLoadedSpriteCount = 0;

    std::memset(this, 0, sizeof(AnmManager));

    for (i32 spriteIndex = 0; spriteIndex < ARRAY_SIZE_SIGNED(this->sprites); spriteIndex++) {
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

    // OpenGL considers textures to be incomplete if the bound texture has no
    // image defined Incomplete textures result in texturing being turned off,
    // but EoSD has places where it uses the texturing engine to color fragments
    // without using the texture itself. The dummy texture is necessary to
    // ensure the texture can't be considered incomplete in these cases.
    this->CreateTextureObject();
    this->dummyTextureHandle = this->currentTextureHandle;
    g_glFuncTable.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    this->currentBlendMode = 0;
    this->screenshotTextureId = -1;
    this->projectionMode = PROJECTION_MODE_PERSPECTIVE;

    this->dirtyFlags = 0;

    for (u32 i = 0; i < ARRAY_SIZE_SIGNED(this->transformMatrices); i++) {
        transformMatrices[i].Identity();
        dirtyTransformMatrices[i].Identity();
    }
}

void AnmManager::SetupVertexBuffer() {
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

    //    RenderVertexInfo *buffer;

    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0) {
        this->SetAttributePointer(VERTEX_ARRAY_POSITION, sizeof(*vertexBufferContents), &this->vertexBufferContents[0].position);
        this->SetAttributePointer(VERTEX_ARRAY_TEX_COORD, sizeof(*vertexBufferContents), &this->vertexBufferContents[0].textureUV);
    }
}

bool AnmManager::LoadTexture(i32 textureIdx, char *textureName, i32 textureFormat, ZunColor colorKey) {
    u8 *rawTextureData;
    SDL_Surface *textureSurface;

    ReleaseTexture(textureIdx);

    if (((g_Supervisor.cfg.opts >> GCOS_FORCE_16BIT_COLOR_MODE) & 1) != 0) {
        if (g_TextureFormatSDLMapping[textureFormat] == SDL_PIXELFORMAT_RGBA32 || g_TextureFormatSDLMapping[textureFormat] == SDL_PIXELFORMAT_UNKNOWN) {
            textureFormat = TEX_FMT_A4R4G4B4;
        } else if (g_TextureFormatSDLMapping[textureFormat] == SDL_PIXELFORMAT_RGB24) {
            textureFormat = TEX_FMT_R5G6B5;
        }
    }

    textureSurface = LoadToSurfaceWithFormat(textureName, g_TextureFormatSDLMapping[textureFormat], (u8 **)&this->textures[textureIdx].fileData);

    // Hideous hack to account for ANM entries that report a different texture
    // size than the actual size
    AnmRawEntry *entry = this->anmFiles[textureIdx];
    if (textureSurface->w != entry->width || textureSurface->h != entry->height) {
        SDL_Surface *textureSurface2 = SDL_CreateRGBSurfaceWithFormat(
            0, entry->width, entry->height,
            g_TextureFormatBytesPerPixel[textureFormat] * 8,
            g_TextureFormatSDLMapping[textureFormat]
        );
        SDL_Rect srcRect = { 0, 0, textureSurface->w, textureSurface->h };
        SDL_Rect dstRect = { 0, 0, entry->width, entry->height };
        SDL_BlitScaled(textureSurface, &srcRect, textureSurface2, &dstRect);
        SDL_FreeSurface(textureSurface);
        textureSurface = textureSurface2;
    }

    if (textureSurface == NULL) {
        return false;
    }

    CreateTextureObject();

    // Clear any errors that might be pending
    while (g_glFuncTable.glGetError() != GL_NO_ERROR) {

    }

    rawTextureData = ExtractSurfacePixels(textureSurface, g_TextureFormatBytesPerPixel[textureFormat]);

    this->textures[textureIdx].handle = this->currentTextureHandle;
    this->textures[textureIdx].textureData = rawTextureData;
    this->textures[textureIdx].width = textureSurface->w;
    this->textures[textureIdx].height = textureSurface->h;
    this->textures[textureIdx].format = textureFormat;

    // Note that the original D3DX call here used D3DX_FILTER_NONE |
    // D3DX_FILTER_POINT for the filter args, which is illegal, and I'm not sure
    // what filtering mode that ends up using in practice. MIP filtering used
    // D3DX_FILTER_BOX Both of those should be globally disabled for the texture
    // unit anyway This also drops colorKey (an equivalent doesn't exist in
    // OpenGL). But I'm not sure its use ever matters anyway

    g_glFuncTable.glTexImage2D(
        GL_TEXTURE_2D, 0, g_TextureFormatGLFormatMapping[textureFormat],
        textureSurface->w, textureSurface->h, 0,
        g_TextureFormatGLFormatMapping[textureFormat],
        g_TextureFormatGLTypeMapping[textureFormat], rawTextureData
    );

    SDL_FreeSurface(textureSurface);

    if (g_glFuncTable.glGetError() != GL_NO_ERROR) {
        ReleaseTexture(textureIdx);

        return false;
    }

    return true;
}

// bool AnmManager::LoadTextureAlphaChannel(i32 textureIdx, char *textureName,
// i32 textureFormat, ZunColor colorKey)
// {
//     SDL_Surface *alphaSurface;
//     TextureData *textureDesc;

//     u8 *dstData;
//     u8 *srcData;
//     u8 *dstData8;
//     u8 *srcData8;
//     u16 *dstData16;
//     u16 *srcData16;
//     u32 x;
//     u32 y;

//     textureDesc = this->textures + textureIdx;

//     if (textureDesc->format != TEX_FMT_A8R8G8B8 && textureDesc->format !=
//     TEX_FMT_A4R4G4B4 &&
//         textureDesc->format != TEX_FMT_A1R5G5B5)
//     {
//         GameErrorContext::Fatal(&g_GameErrorContext,
//         TH_ERR_ANMMANAGER_UNK_TEX_FORMAT); return false;
//     }

//     alphaSurface = LoadToSurfaceWithFormat(textureName,
//     g_TextureFormatSDLMapping[textureFormat], NULL);

//     if (alphaSurface == NULL)
//     {
//         return false;
//     }

//     SDL_LockSurface(alphaSurface);

//     dstData = (u8 *)textureDesc->textureData;
//     srcData = (u8 *)alphaSurface->pixels;

//     // Copy over the alpha channel from the source to the destination, taking
//     // into account the texture format.
//     switch (textureDesc->format)
//     {
//     case TEX_FMT_A8R8G8B8:
//         dstData8 = dstData;
//         for (y = 0; y < textureDesc->height; y++)
//         {
//             srcData8 = srcData + alphaSurface->pitch * y;

//             for (x = 0; x < textureDesc->width; x++, srcData8 += 4, dstData8
//             += 4)
//             {
//                 dstData8[3] = srcData8[0];
//             }
//         }
//         break;

//         // The dereferences here make the assumption that rows are 16-bit
//         aligned. With SDL, this is guaranteed

//     case TEX_FMT_A1R5G5B5:
//         dstData16 = (u16 *)dstData;
//         for (y = 0; y < textureDesc->height; y++)
//         {
//             srcData16 = (u16 *)(srcData + alphaSurface->pitch * y);

//             for (x = 0; x < textureDesc->width; x++, srcData16++,
//             dstData16++)
//             {
//                 *dstData16 &= 0xfffe;
//                 *dstData16 |= (*srcData16 & 0x8000) >> 15;
//             }
//         }
//         break;

//     case TEX_FMT_A4R4G4B4:
//         dstData16 = (u16 *)dstData;
//         for (y = 0; y < textureDesc->height; y++)
//         {
//             srcData16 = (u16 *)(srcData + alphaSurface->pitch * y);

//             for (x = 0; x < textureDesc->width; x++, srcData16++,
//             dstData16++)
//             {
//                 *dstData16 &= 0xfff0;
//                 *dstData16 |= (*srcData16 & 0xf000) >> 12;
//             }
//         }
//         break;
//     }

//     SDL_UnlockSurface(alphaSurface);
//     SDL_FreeSurface(alphaSurface);

//     this->SetCurrentTexture(this->textures[textureIdx].handle);
//     g_glFuncTable.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureDesc->width,
//     textureDesc->height, 0, GL_RGBA,
//                                g_TextureFormatGLTypeMapping[textureFormat],
//                                textureDesc->textureData);

//     return true;
// }

bool AnmManager::CreateEmptyTexture(i32 textureIdx, u32 width, u32 height, i32 textureFormat) {
    CreateTextureObject();

    this->textures[textureIdx].handle = this->currentTextureHandle;
    this->textures[textureIdx].width = bit_ceil(width);
    this->textures[textureIdx].height = bit_ceil(height);
    this->textures[textureIdx].format = textureFormat;

    g_glFuncTable.glTexImage2D(
        GL_TEXTURE_2D, 0, g_TextureFormatGLFormatMapping[textureFormat],
        textures[textureIdx].width, textures[textureIdx].height, 0,
        g_TextureFormatGLFormatMapping[textureFormat],
        g_TextureFormatGLTypeMapping[textureFormat], NULL
    );

    return true;
}

bool AnmManager::LoadAnm(i32 anmIdx, const char *path, i32 spriteIdxOffset) {
    this->ReleaseAnm(anmIdx);
    this->anmFiles[anmIdx] = (AnmRawEntry *)FileSystem::OpenPath(path);

    AnmRawEntry *anm = this->anmFiles[anmIdx];

    if (anm == NULL) {
        GameErrorContext::Fatal(&g_GameErrorContext, TH_ERR_ANMMANAGER_SPRITE_CORRUPTED, path);
        return false;
    }

    anm->textureIdx = anmIdx;

    char *anmName = (char *)((u8 *)anm + anm->nameOffset);

    // D3D seems to treat unknown texture format as a wildcard, but SDL treats
    // it as an error
    //   This is a hack to avoid that for now
    if (anm->format == TEX_FMT_UNKNOWN) {
        anm->format = TEX_FMT_A8R8G8B8;
    }

    if (*anmName == '@') {
        this->CreateEmptyTexture(anm->textureIdx, anm->width, anm->height, anm->format);
    } else if (!this->LoadTexture(anm->textureIdx, anmName, anm->format, anm->colorKey)) {
        GameErrorContext::Fatal(&g_GameErrorContext, TH_ERR_ANMMANAGER_TEXTURE_CORRUPTED, anmName);
        return false;
    }

    if (anm->alphaNameOffset != 0) {
        anmName = (char *)((u8 *)anm + anm->alphaNameOffset);
        // if (!this->LoadTextureAlphaChannel(anm->textureIdx, anmName,
        // anm->format, anm->colorKey))
        // {
        //     GameErrorContext::Fatal(&g_GameErrorContext,
        //     TH_ERR_ANMMANAGER_TEXTURE_CORRUPTED, anmName); return false;
        // }
    }

    anm->spriteIdxOffset = spriteIdxOffset;

    u32 *curSpriteOffset = anm->spriteOffsets;

    i32 index;
    AnmRawSprite *rawSprite;

    for (index = 0; index < this->anmFiles[anmIdx]->numSprites; index++, curSpriteOffset++) {
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

    for (index = 0; index < anm->numScripts; index++, curSpriteOffset += 2) {
        this->scripts[curSpriteOffset[0] + spriteIdxOffset] = (AnmRawInstr *)((u8 *)anm + curSpriteOffset[1]);
        this->spriteIndices[curSpriteOffset[0] + spriteIdxOffset] = spriteIdxOffset;
    }

    this->anmFilesSpriteIndexOffsets[anmIdx] = spriteIdxOffset;

    return true;
}

void AnmManager::ReleaseAnm(i32 anmIdx) {
    if (this->anmFiles[anmIdx] != NULL) {
        i32 *spriteIdx;
        i32 spriteIdxOffset = this->anmFilesSpriteIndexOffsets[anmIdx];
        u32 *byteOffset = this->anmFiles[anmIdx]->spriteOffsets;

        i32 i;
        for (i = 0; i < this->anmFiles[anmIdx]->numSprites; i++, byteOffset++) {
            spriteIdx = (i32 *)((u8 *)this->anmFiles[anmIdx] + *byteOffset);
            memset(&this->sprites[*spriteIdx + spriteIdxOffset], 0, sizeof(this->sprites[*spriteIdx + spriteIdxOffset]));
            this->sprites[*spriteIdx + spriteIdxOffset].sourceFileIndex = -1;
        }
        for (i = 0; i < this->anmFiles[anmIdx]->numScripts;
             i++, byteOffset += 2) {
            this->scripts[*byteOffset + spriteIdxOffset] = NULL;
            this->spriteIndices[*byteOffset + spriteIdxOffset] = 0;
        }

        this->anmFilesSpriteIndexOffsets[anmIdx] = 0;
        AnmRawEntry *entry = this->anmFiles[anmIdx];
        this->ReleaseTexture(entry->textureIdx);
        AnmRawEntry *anmFilePtr = this->anmFiles[anmIdx];
        free(anmFilePtr);
        this->anmFiles[anmIdx] = 0;
        this->currentBlendMode = 0xff;
        this->currentTextureHandle = 0;
    }
}

void AnmManager::ReleaseTexture(i32 textureIdx) {
    if (this->textures[textureIdx].handle != 0) {
        if (this->currentTextureHandle == this->textures[textureIdx].handle) {
            this->currentTextureHandle = 0;
        }

        g_glFuncTable.glDeleteTextures(1, &this->textures[textureIdx].handle);

        this->textures[textureIdx].handle = 0;
    }

    free(this->textures[textureIdx].fileData);
    this->textures[textureIdx].fileData = NULL;

    delete[] this->textures[textureIdx].textureData;
    this->textures[textureIdx].textureData = NULL;
}

void AnmManager::LoadSprite(u32 spriteIdx, AnmLoadedSprite *sprite) {
    this->sprites[spriteIdx] = *sprite;
    this->sprites[spriteIdx].spriteId = this->maybeLoadedSpriteCount++;

    // OpenGL texel centers are located at half coordinates and EoSD uses whole
    // number-aligned UVs
    //   With linear filtering, a whole-numbered UV will pull equally from from
    //   the intended texel and the next one, which can cause very obvious
    //   visual artifacts and gaps on the edges of sprites. This prevents that,
    //   at the cost of flattening the sprite a tiny amount. If the option were
    //   added to use nearest filtering for textures, this offsetting would be
    //   able to be disabled.
    // Note that this is a quick and dirty fix and I have no idea if it breaks
    // some assumption somewhere
    //   else in the ANM code, but it causes orthographic draws to look visually
    //   correct, so ¯\_(ツ)_/¯
    this->sprites[spriteIdx].uvStart.x = (this->sprites[spriteIdx].startPixelInclusive.x + 0.5f) / this->sprites[spriteIdx].textureWidth;
    this->sprites[spriteIdx].uvEnd.x = (this->sprites[spriteIdx].endPixelInclusive.x - 0.5f) / this->sprites[spriteIdx].textureWidth;
    this->sprites[spriteIdx].uvStart.y = (this->sprites[spriteIdx].startPixelInclusive.y + 0.5f) / this->sprites[spriteIdx].textureHeight;
    this->sprites[spriteIdx].uvEnd.y = (this->sprites[spriteIdx].endPixelInclusive.y - 0.5f) / this->sprites[spriteIdx].textureHeight;

    this->sprites[spriteIdx].widthPx = this->sprites[spriteIdx].endPixelInclusive.x - this->sprites[spriteIdx].startPixelInclusive.x;
    this->sprites[spriteIdx].heightPx = this->sprites[spriteIdx].endPixelInclusive.y - this->sprites[spriteIdx].startPixelInclusive.y;
}

bool AnmManager::SetActiveSprite(AnmVm *vm, u32 sprite_index) {
    if (this->sprites[sprite_index].sourceFileIndex < 0) {
        return false;
    }

    vm->activeSpriteIndex = (i16)sprite_index;
    vm->sprite = this->sprites + sprite_index;
    vm->matrix.Identity();
    vm->matrix.m[0][0] = vm->sprite->widthPx / vm->sprite->textureWidth;
    vm->matrix.m[1][1] = vm->sprite->heightPx / vm->sprite->textureHeight;

    return true;
}

void AnmManager::SetAndExecuteScript(AnmVm *vm, AnmRawInstr *beginingOfScript) {
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
    if (beginingOfScript) {
        this->ExecuteScript(vm);
    }
}

void AnmManager::SetRenderStateForVm(AnmVm *vm) {
    if (this->currentBlendMode != vm->flags.blendMode) {
        this->currentBlendMode = vm->flags.blendMode;
        if (this->currentBlendMode == AnmVmBlendMode_InvSrcAlpha) {
            g_glFuncTable.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            g_glFuncTable.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        }
    }

    if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0) {
        this->SetColorOp(COMPONENT_RGB, (ColorOp)vm->flags.colorOp);
    }

    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0) {
        this->SetTextureFactor(vm->color);
    } else {
        g_PrimitivesToDrawNoVertexBuf[0].diffuse = vm->color;
        g_PrimitivesToDrawNoVertexBuf[1].diffuse = vm->color;
        g_PrimitivesToDrawNoVertexBuf[2].diffuse = vm->color;
        g_PrimitivesToDrawNoVertexBuf[3].diffuse = vm->color;
        g_PrimitivesToDrawUnknown[0].diffuse = vm->color;
        g_PrimitivesToDrawUnknown[1].diffuse = vm->color;
        g_PrimitivesToDrawUnknown[2].diffuse = vm->color;
        g_PrimitivesToDrawUnknown[3].diffuse = vm->color;
    }

    this->SetDepthMask(!vm->flags.zWriteDisable);

    return;
}

void AnmManager::UpdateDirtyStates() {
    while (this->dirtyFlags != 0) {
        u32 curFlagIndex = countr_zero(this->dirtyFlags);
        this->dirtyFlags &= ~(1 << curFlagIndex);

        // This would all be nicer if the enum was flag values rather than
        // indices,
        //   but compilers just aren't able to deal with that in the switch
        //   statement :/
        switch (curFlagIndex) {
            case DIRTY_FOG:
                if (this->dirtyFogNear != this->fogNear ||
                    this->dirtyFogFar != this->fogFar) {
                    this->fogNear = this->dirtyFogNear;
                    this->fogFar = this->dirtyFogFar;
                    gfxBackend->SetFogRange(this->fogNear, this->fogFar);
                }

                if (this->dirtyFogColor != this->fogColor) {
                    this->fogColor = this->dirtyFogColor;
                    gfxBackend->SetFogColor(this->fogColor);
                }

                break;
            case DIRTY_DEPTH_CONFIG:
                if (this->dirtyDepthMask != this->depthMask) {
                    this->depthMask = this->dirtyDepthMask;
                    g_glFuncTable.glDepthMask(this->depthMask);
                }

                if (this->dirtyDepthFunc != this->depthFunc) {
                    this->depthFunc = this->dirtyDepthFunc;

                    // This'll end up less awkward once there's a render backend
                    // abstraction layer I swear
                    if (this->depthFunc == DEPTH_FUNC_ALWAYS) {
                        g_glFuncTable.glDepthFunc(GL_ALWAYS);
                    } else {
                        g_glFuncTable.glDepthFunc(GL_LEQUAL);
                    }
                }

                break;
            case DIRTY_VERTEX_ATTRIBUTE_ENABLE: {
                u8 changedAttributes = this->dirtyEnabledVertexAttributes ^ this->enabledVertexAttributes;
                this->enabledVertexAttributes = this->dirtyEnabledVertexAttributes;

                while (changedAttributes != 0) {
                    u8 isolated = isolate_rightmost_one(changedAttributes);
                    gfxBackend->ToggleVertexAttribute(isolated, this->enabledVertexAttributes & isolated);
                    changedAttributes = turn_off_rightmost_one(changedAttributes);
                }

                break;
            }
            case DIRTY_VERTEX_ATTRIBUTE_ARRAY:
                for (u32 i = 0; i < 3; i++) {
                    if (!std::memcmp(&this->attribArrays[i], &this->dirtyAttribArrays[i], sizeof(*this->attribArrays))) {
                        continue;
                    }

                    this->attribArrays[i] = this->dirtyAttribArrays[i];

                    gfxBackend->SetAttributePointer((VertexAttributeArrays)i, this->attribArrays[i].stride, this->attribArrays[i].ptr);
                }

                break;
            case DIRTY_COLOR_OP:
                for (u32 i = 0; i < 2; i++) {
                    if (this->colorOps[i] == this->dirtyColorOps[i]) {
                        continue;
                    }

                    this->colorOps[i] = this->dirtyColorOps[i];
                    gfxBackend->SetColorOp((TextureOpComponent)i, this->colorOps[i]);
                }

                break;
            case DIRTY_TEXTURE_FACTOR:
                this->textureFactor = this->dirtytTextureFactor;
                gfxBackend->SetTextureFactor(this->textureFactor);
                break;
            case DIRTY_MODEL_MATRIX:
            case DIRTY_VIEW_MATRIX:
            case DIRTY_PROJECTION_MATRIX:
            case DIRTY_TEXTURE_MATRIX:
                std::memcpy(
                    &this->transformMatrices[curFlagIndex - DIRTY_MODEL_MATRIX],
                    &this->dirtyTransformMatrices[curFlagIndex - DIRTY_MODEL_MATRIX],
                    sizeof(*this->transformMatrices)
                );
                gfxBackend->SetTransformMatrix(
                    (TransformMatrix)(curFlagIndex - DIRTY_MODEL_MATRIX),
                    this->transformMatrices[curFlagIndex - DIRTY_MODEL_MATRIX]
                );
        }
    }
}

bool AnmManager::DrawOrthographic(AnmVm *vm, bool roundToPixel) {
    if (roundToPixel) {
        // In the original D3D code, 0.5 was subtracted from the final position
        // here to center on D3D
        //   pixels. This has been changed to round to OpenGL pixels. See
        //   comment in inverseViewportMatrix() for a more detailed explanation
        //   and porting notes.

        g_PrimitivesToDrawVertexBuf[0].position.x = rintf(g_PrimitivesToDrawVertexBuf[0].position.x);
        g_PrimitivesToDrawVertexBuf[2].position.x = g_PrimitivesToDrawVertexBuf[0].position.x;
        g_PrimitivesToDrawVertexBuf[1].position.x = rintf(g_PrimitivesToDrawVertexBuf[1].position.x);
        g_PrimitivesToDrawVertexBuf[3].position.x = g_PrimitivesToDrawVertexBuf[1].position.x;
        g_PrimitivesToDrawVertexBuf[0].position.y = rintf(g_PrimitivesToDrawVertexBuf[0].position.y);
        g_PrimitivesToDrawVertexBuf[1].position.y = g_PrimitivesToDrawVertexBuf[0].position.y;
        g_PrimitivesToDrawVertexBuf[2].position.y = rintf(g_PrimitivesToDrawVertexBuf[2].position.y);
        g_PrimitivesToDrawVertexBuf[3].position.y = g_PrimitivesToDrawVertexBuf[2].position.y;
    }
    g_PrimitivesToDrawVertexBuf[0].position.z = g_PrimitivesToDrawVertexBuf[1].position.z = g_PrimitivesToDrawVertexBuf[2].position.z = g_PrimitivesToDrawVertexBuf[3].position.z = vm->pos.z;
    if (this->currentSprite != vm->sprite) {
        this->currentSprite = vm->sprite;
        g_PrimitivesToDrawVertexBuf[0].textureUV.x = g_PrimitivesToDrawVertexBuf[2].textureUV.x = vm->sprite->uvStart.x + vm->uvScrollPos.x;
        g_PrimitivesToDrawVertexBuf[1].textureUV.x = g_PrimitivesToDrawVertexBuf[3].textureUV.x = vm->sprite->uvEnd.x + vm->uvScrollPos.x;
        g_PrimitivesToDrawVertexBuf[0].textureUV.y = g_PrimitivesToDrawVertexBuf[1].textureUV.y = vm->sprite->uvStart.y + vm->uvScrollPos.y;
        g_PrimitivesToDrawVertexBuf[2].textureUV.y = g_PrimitivesToDrawVertexBuf[3].textureUV.y = vm->sprite->uvEnd.y + vm->uvScrollPos.y;

        this->SetCurrentTexture(this->textures[vm->sprite->sourceFileIndex].handle);
    }

    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0) {
        this->SetVertexAttributes(VERTEX_ATTR_TEX_COORD);
    } else {
        this->SetVertexAttributes(VERTEX_ATTR_TEX_COORD | VERTEX_ATTR_DIFFUSE);
    }

    this->SetRenderStateForVm(vm);
    this->SetProjectionMode(PROJECTION_MODE_ORTHOGRAPHIC);

    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0) {
        this->SetAttributePointer(VERTEX_ARRAY_POSITION, sizeof(*g_PrimitivesToDrawVertexBuf), &g_PrimitivesToDrawVertexBuf[0].position);
        this->SetAttributePointer(VERTEX_ARRAY_TEX_COORD, sizeof(*g_PrimitivesToDrawVertexBuf), &g_PrimitivesToDrawVertexBuf[0].textureUV);
    } else {
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
        g_PrimitivesToDrawNoVertexBuf[0].textureUV.x = g_PrimitivesToDrawNoVertexBuf[2].textureUV.x = vm->sprite->uvStart.x + vm->uvScrollPos.x;
        g_PrimitivesToDrawNoVertexBuf[1].textureUV.x = g_PrimitivesToDrawNoVertexBuf[3].textureUV.x = vm->sprite->uvEnd.x + vm->uvScrollPos.x;
        g_PrimitivesToDrawNoVertexBuf[0].textureUV.y = g_PrimitivesToDrawNoVertexBuf[1].textureUV.y = vm->sprite->uvStart.y + vm->uvScrollPos.y;
        g_PrimitivesToDrawNoVertexBuf[2].textureUV.y = g_PrimitivesToDrawNoVertexBuf[3].textureUV.y = vm->sprite->uvEnd.y + vm->uvScrollPos.y;

        this->SetAttributePointer(VERTEX_ARRAY_POSITION, sizeof(*g_PrimitivesToDrawNoVertexBuf), &g_PrimitivesToDrawNoVertexBuf[0].position);
        this->SetAttributePointer(VERTEX_ARRAY_TEX_COORD, sizeof(*g_PrimitivesToDrawNoVertexBuf), &g_PrimitivesToDrawNoVertexBuf[0].textureUV);
        this->SetAttributePointer(VERTEX_ARRAY_DIFFUSE, sizeof(*g_PrimitivesToDrawNoVertexBuf), &g_PrimitivesToDrawNoVertexBuf[0].diffuse);
    }

    this->BackendDrawCall();

    return true;
}

bool AnmManager::DrawNoRotation(AnmVm *vm) {
    float fVar2;
    float fVar3;

    if (vm->flags.isVisible == 0) {
        return false;
    }
    if (vm->flags.flag1 == 0) {
        return false;
    }
    if (vm->color == 0) {
        return false;
    }
    fVar2 = (vm->sprite->widthPx * vm->scaleX) / 2.0f;
    fVar3 = (vm->sprite->heightPx * vm->scaleY) / 2.0f;
    if ((vm->flags.anchor & AnmVmAnchor_Left) == 0) {
        g_PrimitivesToDrawVertexBuf[0].position.x = g_PrimitivesToDrawVertexBuf[2].position.x = vm->pos.x - fVar2;
        g_PrimitivesToDrawVertexBuf[1].position.x = g_PrimitivesToDrawVertexBuf[3].position.x = fVar2 + vm->pos.x;
    } else {
        g_PrimitivesToDrawVertexBuf[0].position.x = g_PrimitivesToDrawVertexBuf[2].position.x = vm->pos.x;
        g_PrimitivesToDrawVertexBuf[1].position.x = g_PrimitivesToDrawVertexBuf[3].position.x = fVar2 + vm->pos.x + fVar2;
    }
    if ((vm->flags.anchor & AnmVmAnchor_Top) == 0) {
        g_PrimitivesToDrawVertexBuf[0].position.y = g_PrimitivesToDrawVertexBuf[1].position.y = vm->pos.y - fVar3;
        g_PrimitivesToDrawVertexBuf[2].position.y = g_PrimitivesToDrawVertexBuf[3].position.y = fVar3 + vm->pos.y;
    } else {
        g_PrimitivesToDrawVertexBuf[0].position.y = g_PrimitivesToDrawVertexBuf[1].position.y = vm->pos.y;
        g_PrimitivesToDrawVertexBuf[2].position.y = g_PrimitivesToDrawVertexBuf[3].position.y = fVar3 + vm->pos.y + fVar3;
    }
    return this->DrawOrthographic(vm, true);
}

void AnmManager::TranslateRotation(VertexTex1Xyzrhw *param_1, f32 x, f32 y, f32 sine, f32 cosine, f32 xOffset, f32 yOffset) {
    param_1->position.x = x * cosine + y * sine + xOffset;
    param_1->position.y = -x * sine + y * cosine + yOffset;
    return;
}

bool AnmManager::Draw(AnmVm *vm) {
    f32 zSine;
    f32 zCosine;
    f32 spriteXCenter;
    f32 spriteYCenter;
    f32 xOffset;
    f32 yOffset;
    f32 z;

    if (vm->rotation.z == 0.0f) {
        return this->DrawNoRotation(vm);
    }
    if (vm->flags.isVisible == 0) {
        return false;
    }
    if (vm->flags.flag1 == 0) {
        return false;
    }
    if (vm->color == 0) {
        return false;
    }
    z = vm->rotation.z;
    fsincos_wrapper(&zSine, &zCosine, z);
    xOffset = rintf(vm->pos.x);
    yOffset = rintf(vm->pos.y);
    spriteXCenter = rintf((vm->sprite->widthPx * vm->scaleX) / 2.0f);
    spriteYCenter = rintf((vm->sprite->heightPx * vm->scaleY) / 2.0f);
    this->TranslateRotation(&g_PrimitivesToDrawVertexBuf[0], -spriteXCenter - 0.5f, -spriteYCenter - 0.5f, zSine, zCosine, xOffset, yOffset);
    this->TranslateRotation(&g_PrimitivesToDrawVertexBuf[1], spriteXCenter - 0.5f, -spriteYCenter - 0.5f, zSine, zCosine, xOffset, yOffset);
    this->TranslateRotation(&g_PrimitivesToDrawVertexBuf[2], -spriteXCenter - 0.5f, spriteYCenter - 0.5f, zSine, zCosine, xOffset, yOffset);
    this->TranslateRotation(&g_PrimitivesToDrawVertexBuf[3], spriteXCenter - 0.5f, spriteYCenter - 0.5f, zSine, zCosine, xOffset, yOffset);
    g_PrimitivesToDrawVertexBuf[0].position.z = g_PrimitivesToDrawVertexBuf[1].position.z = g_PrimitivesToDrawVertexBuf[2].position.z = g_PrimitivesToDrawVertexBuf[3].position.z = vm->pos.z;
    if ((vm->flags.anchor & AnmVmAnchor_Left) != 0) {
        g_PrimitivesToDrawVertexBuf[0].position.x += spriteXCenter;
        g_PrimitivesToDrawVertexBuf[1].position.x += spriteXCenter;
        g_PrimitivesToDrawVertexBuf[2].position.x += spriteXCenter;
        g_PrimitivesToDrawVertexBuf[3].position.x += spriteXCenter;
    }
    if ((vm->flags.anchor & AnmVmAnchor_Top) != 0) {
        g_PrimitivesToDrawVertexBuf[0].position.y += spriteYCenter;
        g_PrimitivesToDrawVertexBuf[1].position.y += spriteYCenter;
        g_PrimitivesToDrawVertexBuf[2].position.y += spriteYCenter;
        g_PrimitivesToDrawVertexBuf[3].position.y += spriteYCenter;
    }
    return this->DrawOrthographic(vm, false);
}

bool AnmManager::DrawFacingCamera(AnmVm *vm) {
    f32 centerX;
    f32 centerY;

    if (!vm->flags.isVisible) {
        return false;
    }
    if (!vm->flags.flag1) {
        return false;
    }
    if (vm->color == 0) {
        return false;
    }

    centerX = vm->sprite->widthPx * vm->scaleX / 2.0f;
    centerY = vm->sprite->heightPx * vm->scaleY / 2.0f;
    if ((vm->flags.anchor & AnmVmAnchor_Left) == 0) {
        g_PrimitivesToDrawVertexBuf[0].position.x = g_PrimitivesToDrawVertexBuf[2].position.x = vm->pos.x - centerX;
        g_PrimitivesToDrawVertexBuf[1].position.x = g_PrimitivesToDrawVertexBuf[3].position.x = vm->pos.x + centerX;
    } else {
        g_PrimitivesToDrawVertexBuf[0].position.x = g_PrimitivesToDrawVertexBuf[2].position.x = vm->pos.x;
        g_PrimitivesToDrawVertexBuf[1].position.x = g_PrimitivesToDrawVertexBuf[3].position.x = vm->pos.x + centerX + centerX;
    }
    if ((vm->flags.anchor & AnmVmAnchor_Top) == 0) {
        g_PrimitivesToDrawVertexBuf[0].position.y = g_PrimitivesToDrawVertexBuf[1].position.y = vm->pos.y - centerY;
        g_PrimitivesToDrawVertexBuf[2].position.y = g_PrimitivesToDrawVertexBuf[3].position.y = vm->pos.y + centerY;
    } else {
        g_PrimitivesToDrawVertexBuf[0].position.y = g_PrimitivesToDrawVertexBuf[1].position.y = vm->pos.y;
        g_PrimitivesToDrawVertexBuf[2].position.y = g_PrimitivesToDrawVertexBuf[3].position.y = vm->pos.y + centerY + centerY;
    }
    return this->DrawOrthographic(vm, false);
}

bool AnmManager::Draw3(AnmVm *vm) {
    ZunMatrix worldTransformMatrix;
    ZunMatrix rotationMatrix;
    ZunMatrix textureMatrix;
    f32 scaledXCenter;
    f32 scaledYCenter;

    if (!vm->flags.isVisible) {
        return false;
    }
    if (!vm->flags.flag1) {
        return false;
    }
    if (vm->color == 0) {
        return false;
    }

    SetProjectionMode(PROJECTION_MODE_PERSPECTIVE);

    ZunMatrix originalView = this->dirtyTransformMatrices[MATRIX_VIEW];

    worldTransformMatrix = vm->matrix;
    worldTransformMatrix.m[0][0] *= vm->scaleX;
    worldTransformMatrix.m[1][1] *= -vm->scaleY;

    if (vm->rotation.x != 0.0) {
        worldTransformMatrix.Rotate(vm->rotation.x, 1.0f, 0.0f, 0.0f);
    }

    if (vm->rotation.y != 0.0) {
        worldTransformMatrix.Rotate(vm->rotation.y, 0.0f, 1.0f, 0.0f);
    }

    if (vm->rotation.z != 0.0) {
        worldTransformMatrix.Rotate(vm->rotation.z, 0.0f, 0.0f, 1.0f);
    }

    if ((vm->flags.anchor & AnmVmAnchor_Left) == 0) {
        worldTransformMatrix.m[3][0] = vm->pos.x;
    } else {
        scaledXCenter = vm->sprite->widthPx * vm->scaleX / 2.0f;
        worldTransformMatrix.m[3][0] = ZUN_FABSF(scaledXCenter) + vm->pos.x;
    }

    if ((vm->flags.anchor & AnmVmAnchor_Top) == 0) {
        worldTransformMatrix.m[3][1] = -vm->pos.y;
    } else {
        scaledYCenter = vm->sprite->heightPx * vm->scaleY / 2.0f;
        worldTransformMatrix.m[3][1] = -vm->pos.y - ZUN_FABSF(scaledYCenter);
    }

    worldTransformMatrix.m[3][2] = vm->pos.z;

    // Now, set transform matrix.
    ZunMatrix modelView = originalView * worldTransformMatrix;
    this->SetTransformMatrix(MATRIX_VIEW, modelView);

    // Load sprite if vm->sprite is not the same as current sprite.
    if (this->currentSprite != vm->sprite) {
        this->currentSprite = vm->sprite;
        textureMatrix = vm->matrix;
        textureMatrix.m[3][0] = vm->sprite->uvStart.x + vm->uvScrollPos.x;
        textureMatrix.m[3][1] = vm->sprite->uvStart.y + vm->uvScrollPos.y;

        this->SetTransformMatrix(MATRIX_TEXTURE, textureMatrix);

        SetCurrentTexture(this->textures[vm->sprite->sourceFileIndex].handle);
    }

    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0) {
        this->SetVertexAttributes(VERTEX_ATTR_TEX_COORD);
    } else {
        this->SetVertexAttributes(VERTEX_ATTR_TEX_COORD | VERTEX_ATTR_DIFFUSE);
    }

    // Reset the render state based on the settings fo the given VM.
    this->SetRenderStateForVm(vm);

    // Draw the VM.
    if ((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF & 1) == 0) {
        this->SetAttributePointer(VERTEX_ARRAY_POSITION, sizeof(*g_PrimitivesToDrawUnknown), &g_PrimitivesToDrawUnknown[0].position);
        this->SetAttributePointer(VERTEX_ARRAY_TEX_COORD, sizeof(*g_PrimitivesToDrawUnknown), &g_PrimitivesToDrawUnknown[0].textureUV);
    } else {
        this->SetAttributePointer(VERTEX_ARRAY_POSITION, sizeof(*g_PrimitivesToDrawUnknown), &g_PrimitivesToDrawUnknown[0].position);
        this->SetAttributePointer(VERTEX_ARRAY_TEX_COORD, sizeof(*g_PrimitivesToDrawUnknown), &g_PrimitivesToDrawUnknown[0].textureUV);
        this->SetAttributePointer(VERTEX_ARRAY_DIFFUSE, sizeof(*g_PrimitivesToDrawUnknown), &g_PrimitivesToDrawUnknown[0].diffuse);
    }

    this->BackendDrawCall();

    this->SetTransformMatrix(MATRIX_VIEW, originalView);
    return true;
}

bool AnmManager::Draw2(AnmVm *vm) {
    ZunMatrix worldTransformMatrix;
    ZunMatrix unusedMatrix;
    ZunMatrix textureMatrix;

    if (!vm->flags.isVisible) {
        return false;
    }
    if (!vm->flags.flag1) {
        return false;
    }

    if (vm->rotation.x != 0 || vm->rotation.y != 0 || vm->rotation.z != 0) {
        return this->Draw3(vm);
    }

    if (vm->color == 0) {
        return false;
    }

    SetProjectionMode(PROJECTION_MODE_PERSPECTIVE);

    worldTransformMatrix = vm->matrix;
    worldTransformMatrix.m[3][0] = rintf(vm->pos.x) - 0.5f;
    worldTransformMatrix.m[3][1] = -rintf(vm->pos.y) + 0.5f;
    if ((vm->flags.anchor & AnmVmAnchor_Left) != 0) {
        worldTransformMatrix.m[3][0] += (vm->sprite->widthPx * vm->scaleX) / 2.0f;
    }
    if ((vm->flags.anchor & AnmVmAnchor_Top) != 0) {
        worldTransformMatrix.m[3][1] -= (vm->sprite->heightPx * vm->scaleY) / 2.0f;
    }
    worldTransformMatrix.m[3][2] = vm->pos.z;
    worldTransformMatrix.m[0][0] *= vm->scaleX;
    worldTransformMatrix.m[1][1] *= -vm->scaleY;

    ZunMatrix originalView = this->dirtyTransformMatrices[MATRIX_VIEW];
    ZunMatrix modelView = originalView * worldTransformMatrix;
    this->SetTransformMatrix(MATRIX_VIEW, modelView);

    if (this->currentSprite != vm->sprite) {
        this->currentSprite = vm->sprite;
        textureMatrix = vm->matrix;
        textureMatrix.m[3][0] = vm->sprite->uvStart.x + vm->uvScrollPos.x;
        textureMatrix.m[3][1] = vm->sprite->uvStart.y + vm->uvScrollPos.y;

        this->SetTransformMatrix(MATRIX_TEXTURE, textureMatrix);
        SetCurrentTexture(this->textures[vm->sprite->sourceFileIndex].handle);

        if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0) {
            this->SetVertexAttributes(VERTEX_ATTR_TEX_COORD);
        } else {
            this->SetVertexAttributes(VERTEX_ATTR_TEX_COORD | VERTEX_ATTR_DIFFUSE);
        }
    }

    this->SetRenderStateForVm(vm);

    if ((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF & 1) == 0) {
        this->SetAttributePointer(VERTEX_ARRAY_POSITION, sizeof(*g_PrimitivesToDrawUnknown), &g_PrimitivesToDrawUnknown[0].position);
        this->SetAttributePointer(VERTEX_ARRAY_TEX_COORD, sizeof(*g_PrimitivesToDrawUnknown), &g_PrimitivesToDrawUnknown[0].textureUV);
    } else {
        this->SetAttributePointer(VERTEX_ARRAY_POSITION, sizeof(*g_PrimitivesToDrawUnknown), &g_PrimitivesToDrawUnknown[0].position);
        this->SetAttributePointer(VERTEX_ARRAY_TEX_COORD, sizeof(*g_PrimitivesToDrawUnknown), &g_PrimitivesToDrawUnknown[0].textureUV);
        this->SetAttributePointer(VERTEX_ARRAY_DIFFUSE, sizeof(*g_PrimitivesToDrawUnknown), &g_PrimitivesToDrawUnknown[0].diffuse);
    }

    this->BackendDrawCall();

    this->SetTransformMatrix(MATRIX_VIEW, originalView);

    return true;
}

#define AnmF32Arg(index) (*(f32 *)&curInstr->args[index])
#define AnmI32Arg(index) (*(i32 *)&curInstr->args[index])
#define AnmU32Arg(index) (*(u32 *)&curInstr->args[index])
#define AnmI16Arg(index) (*(i16 *)&curInstr->args[index])

i32 AnmManager::ExecuteScript(AnmVm *vm) {
    AnmRawInstr *curInstr;
    AnmRawInstr *nextInstr;
    ZunColor local_28;
    ZunColor local_2c;
    f32 local_30;
    i32 local_34;
    i32 local_38;
    f32 local_3c;

    if (vm->currentInstruction == NULL) {
        return 1;
    }

    if (vm->pendingInterrupt != 0) {
        goto yolo;
    }

    while (curInstr = vm->currentInstruction, curInstr->time <= vm->currentTimeInScript.AsFrames()) {
        switch (curInstr->opcode) {
            case AnmOpcode_Exit:
                vm->flags.isVisible = 0;
            case AnmOpcode_ExitHide:
                vm->currentInstruction = NULL;
                return 1;
            case AnmOpcode_SetActiveSprite:
                vm->flags.isVisible = 1;
                this->SetActiveSprite(vm, AnmI32Arg(0) + this->spriteIndices[vm->anmFileIndex]);
                vm->timeOfLastSpriteSet = vm->currentTimeInScript.AsFrames();
                break;
            case AnmOpcode_SetRandomSprite:
                vm->flags.isVisible = 1;
                this->SetActiveSprite(vm, AnmI32Arg(0) + g_Rng.GetRandomU16InRange(AnmI32Arg(1)) + this->spriteIndices[vm->anmFileIndex]);
                vm->timeOfLastSpriteSet = vm->currentTimeInScript.AsFrames();
                break;
            case AnmOpcode_SetScale:
                vm->scaleX = AnmF32Arg(0);
                vm->scaleY = AnmF32Arg(1);
                break;
            case AnmOpcode_SetAlpha:
                COLOR_SET_COMPONENT(vm->color, COLOR_ALPHA_BYTE_IDX, AnmI32Arg(0) & 0xff);
                break;
            case AnmOpcode_SetColor:
                vm->color = COLOR_COMBINE_ALPHA(AnmI32Arg(0), vm->color);
                break;
            case AnmOpcode_Jump:
                vm->currentInstruction = (AnmRawInstr *)(((u8 *)vm->beginingOfScript->args) + AnmI32Arg(0) - 4);
                vm->currentTimeInScript.current = vm->currentInstruction->time;
                continue;
            case AnmOpcode_FlipX:
                vm->flags.flip ^= 1;
                vm->scaleX *= -1.f;
                break;
            case AnmOpcode_UsePosOffset:
                vm->flags.usePosOffset = AnmI32Arg(0);
                break;
            case AnmOpcode_FlipY:
                vm->flags.flip ^= 2;
                vm->scaleY *= -1.f;
                break;
            case AnmOpcode_SetRotation:
                vm->rotation.x = AnmF32Arg(0);
                vm->rotation.y = AnmF32Arg(1);
                vm->rotation.z = AnmF32Arg(2);
                break;
            case AnmOpcode_SetAngleVel:
                vm->angleVel.x = AnmF32Arg(0);
                vm->angleVel.y = AnmF32Arg(1);
                vm->angleVel.z = AnmF32Arg(2);
                break;
            case AnmOpcode_SetScaleSpeed:
                vm->scaleInterpFinalX = AnmF32Arg(0);
                vm->scaleInterpFinalY = AnmF32Arg(1);
                vm->scaleInterpEndTime = 0;
                break;
            case AnmOpcode_ScaleTime:
                vm->scaleInterpFinalX = AnmF32Arg(0);
                vm->scaleInterpFinalY = AnmF32Arg(1);
                vm->scaleInterpEndTime = AnmI16Arg(2);
                vm->scaleInterpTime.InitializeForPopup();
                vm->scaleInterpInitialX = vm->scaleX;
                vm->scaleInterpInitialY = vm->scaleY;
                break;
            case AnmOpcode_Fade:
                vm->alphaInterpInitial = vm->color;
                vm->alphaInterpFinal = COLOR_SET_ALPHA2(vm->color, AnmU32Arg(0));
                vm->alphaInterpEndTime = AnmU32Arg(1);
                vm->alphaInterpTime.InitializeForPopup();
                break;
            case AnmOpcode_SetBlendAdditive:
                vm->flags.blendMode = AnmVmBlendMode_One;
                break;
            case AnmOpcode_SetBlendDefault:
                vm->flags.blendMode = AnmVmBlendMode_InvSrcAlpha;
                break;
            case AnmOpcode_SetPosition:
                if (vm->flags.usePosOffset == 0) {
                    vm->pos = ZunVec3(AnmF32Arg(0), AnmF32Arg(1), AnmF32Arg(2));
                } else {
                    vm->posOffset = ZunVec3(AnmF32Arg(0), AnmF32Arg(1), AnmF32Arg(2));
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
                if (vm->flags.usePosOffset == 0) {
                    // This was supposedly originally a memcpy, but any sane
                    // compiler should compile a struct assignment to a memcpy
                    vm->posInterpInitial = vm->pos;
                } else {
                    // This was supposedly originally a memcpy, but any sane
                    // compiler should compile a struct assignment to a memcpy
                    vm->posInterpInitial = vm->posOffset;
                }
                vm->posInterpFinal = ZunVec3(AnmF32Arg(0), AnmF32Arg(1), AnmF32Arg(2));
                vm->posInterpEndTime = AnmI32Arg(3);
                vm->posInterpTime.InitializeForPopup();
                break;
            case AnmOpcode_StopHide:
                vm->flags.isVisible = 0;
            case AnmOpcode_Stop:
                if (vm->pendingInterrupt == 0) {
                    vm->flags.isStopped = 1;
                    vm->currentTimeInScript.Decrement(1);
                    goto stop;
                }
            yolo:
                nextInstr = NULL;
                curInstr = vm->beginingOfScript;
                while ((curInstr->opcode != AnmOpcode_InterruptLabel || vm->pendingInterrupt != AnmI32Arg(0)) && curInstr->opcode != AnmOpcode_Exit && curInstr->opcode != AnmOpcode_ExitHide) {
                    if (curInstr->opcode == AnmOpcode_InterruptLabel &&
                        AnmI32Arg(0) == -1) {
                        nextInstr = curInstr;
                    }
                    curInstr = (AnmRawInstr *)(((u8 *)curInstr->args) + curInstr->argsCount);
                }

                vm->pendingInterrupt = 0;
                vm->flags.isStopped = 0;
                if (curInstr->opcode != AnmOpcode_InterruptLabel) {
                    if (nextInstr == NULL) {
                        vm->currentTimeInScript.Decrement(1);
                        goto stop;
                    }
                    curInstr = nextInstr;
                }

                curInstr = (AnmRawInstr *)(((u8 *)curInstr->args) + curInstr->argsCount);
                vm->currentInstruction = curInstr;
                vm->currentTimeInScript.SetCurrent(vm->currentInstruction->time);
                vm->flags.isVisible = 1;
                continue;
            case AnmOpcode_SetVisibility:
                vm->flags.isVisible = AnmI32Arg(0);
                break;
            case AnmOpcode_23:
                vm->flags.anchor = AnmVmAnchor_TopLeft;
                break;
            case AnmOpcode_SetAutoRotate:
                vm->autoRotate = AnmI32Arg(0);
                break;
            case AnmOpcode_UVScrollX:
                vm->uvScrollPos.x += AnmF32Arg(0);
                if (vm->uvScrollPos.x >= 1.0f) {
                    vm->uvScrollPos.x -= 1.0f;
                } else if (vm->uvScrollPos.x < 0.0f) {
                    vm->uvScrollPos.x += 1.0f;
                }
                break;
            case AnmOpcode_UVScrollY:
                vm->uvScrollPos.y += AnmF32Arg(0);
                if (vm->uvScrollPos.y >= 1.0f) {
                    vm->uvScrollPos.y -= 1.0f;
                } else if (vm->uvScrollPos.y < 0.0f) {
                    vm->uvScrollPos.y += 1.0f;
                }
                break;
            case AnmOpcode_SetZWriteDisable:
                vm->flags.zWriteDisable = AnmI32Arg(0);
                break;
            case AnmOpcode_Nop:
            case AnmOpcode_InterruptLabel:
            default:
                break;
        }
        vm->currentInstruction = (AnmRawInstr *)(((u8 *)curInstr->args) + curInstr->argsCount);
    }

stop:
    if (vm->angleVel.x != 0.0f) {
        vm->rotation.x = utils::AddNormalizeAngle(vm->rotation.x, g_Supervisor.effectiveFramerateMultiplier * vm->angleVel.x);
    }
    if (vm->angleVel.y != 0.0f) {
        vm->rotation.y = utils::AddNormalizeAngle(vm->rotation.y, g_Supervisor.effectiveFramerateMultiplier * vm->angleVel.y);
    }
    if (vm->angleVel.z != 0.0f) {
        vm->rotation.z = utils::AddNormalizeAngle(vm->rotation.z, g_Supervisor.effectiveFramerateMultiplier * vm->angleVel.z);
    }
    if (vm->scaleInterpEndTime > 0) {
        vm->scaleInterpTime.Tick();
        if (vm->scaleInterpTime.AsFrames() >= vm->scaleInterpEndTime) {
            vm->scaleY = vm->scaleInterpFinalY;
            vm->scaleX = vm->scaleInterpFinalX;
            vm->scaleInterpEndTime = 0;
            vm->scaleInterpFinalY = 0.0;
            vm->scaleInterpFinalX = 0.0;
        } else {
            vm->scaleX = (vm->scaleInterpFinalX - vm->scaleInterpInitialX) *
                             vm->scaleInterpTime.AsFramesFloat() /
                             vm->scaleInterpEndTime +
                         vm->scaleInterpInitialX;

            vm->scaleY = (vm->scaleInterpFinalY - vm->scaleInterpInitialY) *
                             vm->scaleInterpTime.AsFramesFloat() /
                             vm->scaleInterpEndTime +
                         vm->scaleInterpInitialY;
        }
        if ((vm->flags.flip & 1) != 0) {
            vm->scaleX = vm->scaleX * -1.f;
        }
        if ((vm->flags.flip & 2) != 0) {
            vm->scaleY = vm->scaleY * -1.f;
        }
    } else {
        vm->scaleY = g_Supervisor.effectiveFramerateMultiplier * vm->scaleInterpFinalY + vm->scaleY;
        vm->scaleX = g_Supervisor.effectiveFramerateMultiplier * vm->scaleInterpFinalX + vm->scaleX;
    }
    if (0 < vm->alphaInterpEndTime) {
        vm->alphaInterpTime.Tick();
        local_2c = vm->alphaInterpInitial;
        local_28 = vm->alphaInterpFinal;
        local_30 = vm->alphaInterpTime.AsFramesFloat() / (f32)vm->alphaInterpEndTime;
        if (local_30 >= 1.0f) {
            local_30 = 1.0;
        }
        for (local_38 = 0; local_38 < 4; local_38++) {
            local_34 = ((f32)COLOR_GET_COMPONENT(local_28, local_38) - (f32)COLOR_GET_COMPONENT(local_2c, local_38)) * local_30 + COLOR_GET_COMPONENT(local_2c, local_38);
            if (local_34 < 0) {
                local_34 = 0;
            }
            COLOR_SET_COMPONENT(local_2c, local_38, local_34 >= 256 ? 255 : local_34);
        }
        vm->color = local_2c;
        if (vm->alphaInterpTime.AsFrames() >= vm->alphaInterpEndTime) {
            vm->alphaInterpEndTime = 0;
        }
    }
    if (vm->posInterpEndTime != 0) {
        local_3c = vm->posInterpTime.AsFramesFloat() / (f32)vm->posInterpEndTime;
        if (local_3c >= 1.0f) {
            local_3c = 1.0;
        }
        switch (vm->flags.posTime) {
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
        if (vm->flags.usePosOffset == 0) {
            vm->pos.x = local_3c * vm->posInterpFinal.x + (1.0f - local_3c) * vm->posInterpInitial.x;
            vm->pos.y = local_3c * vm->posInterpFinal.y + (1.0f - local_3c) * vm->posInterpInitial.y;
            vm->pos.z = local_3c * vm->posInterpFinal.z + (1.0f - local_3c) * vm->posInterpInitial.z;
        } else {
            vm->posOffset.x = local_3c * vm->posInterpFinal.x + (1.0f - local_3c) * vm->posInterpInitial.x;
            vm->posOffset.y = local_3c * vm->posInterpFinal.y + (1.0f - local_3c) * vm->posInterpInitial.y;
            vm->posOffset.z = local_3c * vm->posInterpFinal.z + (1.0f - local_3c) * vm->posInterpInitial.z;
        }

        if (vm->posInterpTime.AsFrames() >= vm->posInterpEndTime) {
            vm->posInterpEndTime = 0;
        }
        vm->posInterpTime.Tick();
    }
    vm->currentTimeInScript.Tick();
    return 0;
}

#undef AnmI32Arg
#undef AnmF32Arg
#undef AnmU32Arg
#undef AnmI16Arg

void AnmManager::DrawTextToSprite(u32 textureDstIdx, i32 xPos, i32 yPos, i32 spriteWidth, i32 spriteHeight, i32 fontWidth, i32 fontHeight, ZunColor textColor, ZunColor shadowColor, const char *strToPrint) {
    if (fontWidth <= 0) {
        fontWidth = 15;
    }
    if (fontHeight <= 0) {
        fontHeight = 15;
    }

    TextHelper::RenderTextToTexture(
        xPos, yPos, spriteWidth, spriteHeight, fontWidth, fontHeight, textColor,
        shadowColor, strToPrint, &this->textures[textureDstIdx]);
    //
    //    this->SetCurrentTexture(this->textures[textureDstIdx].handle);
    //    g_glFuncTable.glTexImage2D(GL_TEXTURE_2D, 0,
    //    g_TextureFormatGLFormatMapping[]);

    return;
}

void AnmManager::DrawVmTextFmt(AnmManager *anmMgr, AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt, ...) {
    char buffer[64];
    va_list argptr;

    u32 fontWidth = vm->fontWidth;
    va_start(argptr, fmt);
    vsprintf(buffer, fmt, argptr);
    va_end(argptr);
    anmMgr->DrawTextToSprite(
        vm->sprite->sourceFileIndex, vm->sprite->startPixelInclusive.x,
        vm->sprite->startPixelInclusive.y, vm->sprite->textureWidth,
        vm->sprite->textureHeight, fontWidth, vm->fontHeight, textColor,
        shadowColor, buffer);
    vm->flags.isVisible = true;
    return;
}

void AnmManager::DrawStringFormat(AnmManager *mgr, AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt, ...) {
    char buf[64];
    va_list args;

    i32 fontWidth = vm->fontWidth <= 0 ? 15 : vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    mgr->DrawTextToSprite(
        vm->sprite->sourceFileIndex, vm->sprite->startPixelInclusive.x,
        vm->sprite->startPixelInclusive.y, vm->sprite->textureWidth,
        vm->sprite->textureHeight, fontWidth, vm->fontHeight, textColor,
        shadowColor, " ");
    i32 secondPartStartX = vm->sprite->startPixelInclusive.x + vm->sprite->textureWidth - ((f32)strlen(buf) * (f32)(fontWidth + 1) / 2.0f);
    mgr->DrawTextToSprite(
        vm->sprite->sourceFileIndex, secondPartStartX,
        vm->sprite->startPixelInclusive.y,
        vm->sprite->textureWidth, vm->sprite->textureHeight,
        fontWidth, vm->fontHeight, textColor, shadowColor,
        buf);
    vm->flags.isVisible = true;
    return;
}

void AnmManager::DrawStringFormat2(AnmManager *mgr, AnmVm *vm, ZunColor textColor, ZunColor shadowColor, const char *fmt, ...) {
    char buf[64];
    va_list args;
    i32 fontWidth;
    i32 secondPartStartX;

    fontWidth = vm->fontWidth <= 0 ? 15 : vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    mgr->DrawTextToSprite(
        vm->sprite->sourceFileIndex, vm->sprite->startPixelInclusive.x,
        vm->sprite->startPixelInclusive.y, vm->sprite->textureWidth,
        vm->sprite->textureHeight, fontWidth, vm->fontHeight, textColor,
        shadowColor, " ");
    secondPartStartX = vm->sprite->startPixelInclusive.x + vm->sprite->textureWidth / 2.0f - ((f32)strlen(buf) * (f32)(fontWidth + 1) / 4.0f);
    mgr->DrawTextToSprite(
        vm->sprite->sourceFileIndex, secondPartStartX,
        vm->sprite->startPixelInclusive.y,
        vm->sprite->textureWidth, vm->sprite->textureHeight,
        fontWidth, vm->fontHeight, textColor, shadowColor,
        buf);
    vm->flags.isVisible = true;
    return;
}

bool AnmManager::LoadSurface(i32 surfaceIdx, const char *path) {
    if (this->surfaces[surfaceIdx] != NULL) {
        this->ReleaseSurface(surfaceIdx);
    }

    this->surfaces[surfaceIdx] =
        LoadToSurfaceWithFormat(path, SDL_PIXELFORMAT_RGB24, NULL);

    if (this->surfaces[surfaceIdx] == NULL) {
        return false;
    }

    return true;
}

void AnmManager::ReleaseSurface(i32 surfaceIdx) {
    if (this->surfaces[surfaceIdx] != NULL) {
        SDL_FreeSurface(this->surfaces[surfaceIdx]);
        this->surfaces[surfaceIdx] = NULL;
    }
}

void AnmManager::CopySurfaceToBackBuffer(i32 surfaceIdx, i32 srcX, i32 srcY, i32 dstX, i32 dstY) {
    SDL_Surface *srcSurface = this->surfaces[surfaceIdx];

    if (srcSurface == NULL) {
        return;
    }

    CopySurfaceRectToBackBuffer(surfaceIdx, dstX, dstY, srcX, srcY, srcSurface->w - srcX, srcSurface->h - srcY);
}

void AnmManager::CopySurfaceRectToBackBuffer(i32 surfaceIdx, i32 dstX, i32 dstY, i32 rectLeft, i32 rectTop, i32 rectWidth, i32 rectHeight) {
    SDL_Surface *srcSurface = this->surfaces[surfaceIdx];

    if (srcSurface == NULL) {
        return;
    }

    ApplySurfaceToColorBuffer(srcSurface,
                              { .x = rectLeft, .y = rectTop, .w = rectWidth, .h = rectHeight },
                              { .x = dstX, .y = dstY, .w = rectWidth, .h = rectHeight });
}

void AnmManager::TakeScreenshot(i32 textureId, i32 left, i32 top, i32 width, i32 height) {
    u8 *dstFormatPixels = NULL;
    SDL_Surface *dstFormatSurface = NULL;
    SDL_Rect stretchDstRect;
    SDL_Rect stretchSrcRect;

    // OpenGL throws an error specifically for negative W / H and pixels are
    // undefined for 0 inputs.
    if (this->textures[textureId].handle == 0 || width <= 0 || height <= 0) {
        return;
    }

    this->SetCurrentTexture(this->textures[textureId].handle);

    u8 *backBufferPixels = new u8[((u32)(width * WIDTH_RESOLUTION_SCALE + 1)) * ((u32)(height * HEIGHT_RESOLUTION_SCALE + 1)) * 4];

    g_glFuncTable.glReadPixels(
        left * WIDTH_RESOLUTION_SCALE + VIEWPORT_OFF_X,
        GAME_WINDOW_HEIGHT_REAL - ((top + height) * HEIGHT_RESOLUTION_SCALE) -
            VIEWPORT_OFF_Y,
        width * WIDTH_RESOLUTION_SCALE, height * HEIGHT_RESOLUTION_SCALE,
        GL_RGBA, GL_UNSIGNED_BYTE, backBufferPixels);

    SDL_Surface *unstretchedSurface = SDL_CreateRGBSurfaceWithFormatFrom(
        backBufferPixels, width * WIDTH_RESOLUTION_SCALE,
        height * HEIGHT_RESOLUTION_SCALE, 32,
        width * WIDTH_RESOLUTION_SCALE * 4, SDL_PIXELFORMAT_RGBA32);

    SDL_Surface *stretchedSurface = SDL_CreateRGBSurfaceWithFormat(
        0, this->textures[textureId].width, this->textures[textureId].height,
        32, SDL_PIXELFORMAT_RGBA32);

    if (unstretchedSurface == NULL || stretchedSurface == NULL) {
        goto cleanup;
    }

    // OpenGL texture coordinates are upside down compared to the D3D
    // conventions. To account for this,
    //   we need to flip the texture
    FlipSurface(unstretchedSurface);

    stretchSrcRect.x = 0;
    stretchSrcRect.y = 0;
    stretchSrcRect.h = height * HEIGHT_RESOLUTION_SCALE;
    stretchSrcRect.w = width * WIDTH_RESOLUTION_SCALE;

    stretchDstRect.x = 0;
    stretchDstRect.y = 0;
    stretchDstRect.h = this->textures[textureId].height;
    stretchDstRect.w = this->textures[textureId].width;

    if (SDL_SoftStretchLinear(unstretchedSurface, &stretchSrcRect, stretchedSurface, &stretchDstRect) < 0) {
        goto cleanup;
    }

    dstFormatSurface = SDL_ConvertSurfaceFormat(
        stretchedSurface,
        g_TextureFormatSDLMapping[this->textures[textureId].format], 0);

    if (dstFormatSurface == NULL) {
        goto cleanup;
    }

    dstFormatPixels = ExtractSurfacePixels(
        dstFormatSurface,
        g_TextureFormatBytesPerPixel[this->textures[textureId].format]);

    g_glFuncTable.glTexImage2D(
        GL_TEXTURE_2D, 0,
        g_TextureFormatGLFormatMapping[this->textures[textureId].format],
        this->textures[textureId].width, this->textures[textureId].height, 0,
        g_TextureFormatGLFormatMapping[this->textures[textureId].format],
        g_TextureFormatGLTypeMapping[this->textures[textureId].format],
        dstFormatPixels);

cleanup:
    SDL_FreeSurface(unstretchedSurface);
    SDL_FreeSurface(stretchedSurface);
    SDL_FreeSurface(dstFormatSurface);
    delete[] backBufferPixels;
    delete[] dstFormatPixels;
}

// Utter mess that needs to be rewritten
void AnmManager::ApplySurfaceToColorBuffer(SDL_Surface *src, const SDL_Rect &srcRect, const SDL_Rect &dstRect) {
    ZunViewport originalViewport;
    ZunViewport fullscreenViewport;

    if (srcRect.w <= 0 || srcRect.h <= 0) {
        return;
    }

    originalViewport.Get();

    fullscreenViewport.x = 0;
    fullscreenViewport.y = 0;
    fullscreenViewport.height = GAME_WINDOW_HEIGHT;
    fullscreenViewport.width = GAME_WINDOW_WIDTH;
    fullscreenViewport.minZ = 0.0f;
    fullscreenViewport.maxZ = 1.0f;

    fullscreenViewport.Set();

    this->SetProjectionMode(PROJECTION_MODE_ORTHOGRAPHIC);

    CreateTextureObject();

    u32 textureWidth = bit_ceil((u32)src->w);
    u32 textureHeight = bit_ceil((u32)src->h);

    g_glFuncTable.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    u8 *surfaceData = ExtractSurfacePixels(src, 3);

    g_glFuncTable.glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, src->w, src->h, GL_RGB, GL_UNSIGNED_BYTE, surfaceData);

    delete[] surfaceData;

    VertexTex1DiffuseXyz verts[4];

    verts[0].position = ZunVec3(dstRect.x, dstRect.y, 0.0f);
    verts[1].position = ZunVec3(dstRect.x + dstRect.w, dstRect.y, 0.0f);
    verts[2].position = ZunVec3(dstRect.x, dstRect.y + dstRect.h, 0.0f);
    verts[3].position = ZunVec3(dstRect.x + dstRect.w, dstRect.y + dstRect.h, 0.0f);

    verts[0].textureUV = ZunVec2(0.0f, 0.0f);
    verts[1].textureUV = ZunVec2(((f32)src->w) / textureWidth, 0.0f);
    verts[2].textureUV = ZunVec2(0.0f, ((f32)src->h) / textureHeight);
    verts[3].textureUV = ZunVec2(((f32)src->w) / textureWidth, ((f32)src->h) / textureHeight);

    this->SetVertexAttributes(VERTEX_ATTR_TEX_COORD);

    this->SetAttributePointer(VERTEX_ARRAY_POSITION, sizeof(*verts), &verts[0].position);
    this->SetAttributePointer(VERTEX_ARRAY_TEX_COORD, sizeof(*verts), &verts[0].textureUV);

    this->SetColorOp(COMPONENT_ALPHA, COLOR_OP_REPLACE);
    this->SetColorOp(COMPONENT_RGB, COLOR_OP_REPLACE);

    this->SetDepthMask(false);
    this->SetDepthFunc(DEPTH_FUNC_ALWAYS);

    this->BackendDrawCall();

    this->SetColorOp(COMPONENT_ALPHA, COLOR_OP_MODULATE);
    this->SetColorOp(COMPONENT_RGB, COLOR_OP_MODULATE);

    g_glFuncTable.glDeleteTextures(1, &this->currentTextureHandle);

    this->SetCurrentSprite(NULL);
    this->SetCurrentTexture(0);
    this->SetCurrentBlendMode(0xff);

    originalViewport.Set();
}
