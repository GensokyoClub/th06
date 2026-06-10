#include "Software.hpp"
#include "GameWindow.hpp"
#include "Supervisor.hpp"
#include "i18n.hpp"
#include "utils.hpp"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>

constexpr u8 alphaThreshold = 4;

GfxInterface *Software::Init()
{
    SDL_Init(SDL_INIT_VIDEO);

    u32 flags = 0;
    i32 height = GAME_WINDOW_HEIGHT_REAL;
    i32 width = GAME_WINDOW_WIDTH_REAL;
    i32 x = SDL_WINDOWPOS_UNDEFINED;
    i32 y = SDL_WINDOWPOS_UNDEFINED;

    if (g_Supervisor.cfg.windowed == 0)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    Software *self = new Software;

    SDL_Window *window = SDL_CreateWindow(TH_WINDOW_TITLE, x, y, width, height, flags);
    self->window = window;
    if (window == NULL)
    {
        delete self;
        return NULL;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    self->renderer = renderer;
    if (renderer == NULL)
    {
        delete self;
        return NULL;
    }

    self->model.Identity();
    self->view.Identity();
    self->projection.Identity();
    self->textureMatrix.Identity();

    SDL_Texture *framebufferTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                                        GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT);
    self->framebufferTexture = framebufferTexture;
    if (framebufferTexture == NULL)
    {
        delete self;
        return NULL;
    }
    u32 *framebuffer = new u32[GAME_WINDOW_WIDTH * GAME_WINDOW_HEIGHT];
    self->framebuffer = framebuffer;

    f32 *depthBuffer = new f32[GAME_WINDOW_WIDTH * GAME_WINDOW_HEIGHT];
    self->depthBuffer = depthBuffer;
    self->noVertexBuffer = g_Supervisor.cfg.opts & (1 << GCOS_DONT_USE_VERTEX_BUF);
    self->noFog = g_Supervisor.cfg.opts & (1 << GCOS_DONT_USE_FOG);

    utils::DebugPrint2(
        "WARNING: Using software rasterizer, which can be slow. If performance is bad, make sure you're compiling with "
        "optimizations (building as release), or go with another graphics backend if possible.");

    return self;
}

void Software::Exit()
{
    if (this->renderer)
    {
        SDL_DestroyRenderer(this->renderer);
        this->renderer = NULL;
    }
    if (this->window)
    {
        SDL_DestroyWindow(this->window);
        this->window = NULL;
    }
    if (this->framebufferTexture)
    {
        SDL_DestroyTexture(this->framebufferTexture);
        this->framebufferTexture = NULL;
    }
    if (this->framebuffer)
    {
        delete[] this->framebuffer;
        this->framebuffer = NULL;
    }
    if (this->depthBuffer)
    {
        delete[] this->depthBuffer;
        this->depthBuffer = NULL;
    }
}

void Software::SetFogRange(f32 nearPlane, f32 farPlane)
{
    fogNear = nearPlane;
    fogFar = farPlane;
}

void Software::SetFogColor(ZunColor color)
{
    fogColor = color;
}

void Software::ToggleVertexAttribute(u8 attr, bool enable)
{
    if (attr & VERTEX_ATTR_TEX_COORD)
    {
        useTexCoord = enable;
    }
    if (attr & VERTEX_ATTR_DIFFUSE)
    {
        useDiffuse = enable;
    }
}

void Software::SetAttributePointer(VertexAttributeArrays attr, std::size_t stride, void *ptr)
{
    switch (attr)
    {
    case VERTEX_ARRAY_POSITION:
        this->vertexData = ptr;
        this->vertexStride = stride;
        break;
    case VERTEX_ARRAY_TEX_COORD:
        this->texCoordData = ptr;
        this->texCoordStride = stride;
        break;
    case VERTEX_ARRAY_DIFFUSE:
        this->diffuseData = ptr;
        this->diffuseStride = stride;
        break;
    }
}

void Software::SetColorOp(TextureOpComponent component, ColorOp op)
{

    if (component == COMPONENT_ALPHA)
    {
        return;
    }
    colorOp = op;
}

void Software::SetTextureFactor(ZunColor factor)
{
    textureFactor = factor;
}

void Software::SetTransformMatrix(TransformMatrix type, const ZunMatrix &matrix)
{
    switch (type)
    {
    case MATRIX_MODEL:
        model = matrix;
        break;
    case MATRIX_VIEW:
        view = matrix;
        break;
    case MATRIX_PROJECTION:
        projection = matrix;
        break;
    case MATRIX_TEXTURE:
        textureMatrix = matrix;
        break;
    }
}

void Software::Enable(Capabilities cap)
{
    if (cap == CAPS_DEPTH_TEST)
    {
        useDepthTest = true;
    }
}

void Software::SetBlendMode(BlendMode mode)
{
    blendMode = mode;
}

void Software::SetViewport(i32 x, i32 y, i32 width, i32 height)
{
    viewport[0] = x;
    viewport[1] = y;
    viewport[2] = width;
    viewport[3] = height;
}

void Software::GetViewport(u32 *viewport)
{
    for (int i = 0; i < 4; i++)
    {
        viewport[i] = this->viewport[i];
    }
}

void Software::GetDepthRange(f32 *depthRange)
{
    depthRange[0] = this->depthNear;
    depthRange[1] = this->depthFar;
}

inline ZunColor RGBAToZunColor(u8 r, u8 g, u8 b, u8 a)
{
    return ((ZunColor)a << 24) | ((ZunColor)r << 16) | ((ZunColor)g << 8) | (ZunColor)b;
}

inline ZunColor ColorDataToZunColor(ColorData colorData)
{
    return RGBAToZunColor(colorData.r, colorData.g, colorData.b, colorData.a);
}

void Software::SetClearColor(f32 r, f32 g, f32 b, f32 a)
{
    clearColor = RGBAToZunColor((u8)(r * 255), (u8)(g * 255), (u8)(b * 255), (u8)(a * 255));
}

void Software::SetTextureFilter()
{
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

void Software::SetClearDepth(f32 depth)
{
    clearDepth = depth;
}

void Software::Clear(u32 clearBits)
{
    if (clearBits & CLEAR_COLOR_BUFFER)
    {
        std::fill(framebuffer, framebuffer + GAME_WINDOW_WIDTH * GAME_WINDOW_HEIGHT, clearColor);
    }
    if (clearBits & CLEAR_DEPTH_BUFFER)
    {
        std::fill(depthBuffer, depthBuffer + GAME_WINDOW_WIDTH * GAME_WINDOW_HEIGHT, clearDepth);
    }
}

void Software::SetDepthRange(f32 nearPlane, f32 farPlane)
{
    depthNear = nearPlane;
    depthFar = farPlane;
}

void Software::SetDepthMask(bool enable)
{
    depthMask = enable;
}

void Software::SetDepthFunc(DepthFunc func)
{
    depthFunc = func;
}

GfxTextureHandle Software::CreateTexture()
{
    std::unique_ptr<Texture> texture = std::unique_ptr<Texture>(new Texture());

    u32 id;
    if (!freeTextures.empty())
    {
        id = freeTextures.back();
        freeTextures.pop_back();
        textures[id] = std::move(texture);
    }
    else
    {
        id = textures.size();
        textures.push_back(std::move(texture));
    }

    return {id};
}

void Software::BindTexture(GfxTextureHandle handle)
{
    if (handle >= textures.size())
        return;
    if (!textures[handle.id])
        return;
    boundTexture = textures[handle.id].get();
}

void Software::DeleteTexture(GfxTextureHandle handle)
{
    if (handle.id >= textures.size())
        return;
    if (!textures[handle.id])
        return;
    textures[handle.id].reset();
    freeTextures.push_back(handle.id);
}

inline SDL_PixelFormatEnum GetSDLPixelFormat(PixelFormat fmt, PixelDataType type)
{
    switch (type)
    {
    case PIXEL_UNSIGNED_BYTE:
        if (fmt == PIXEL_RGB)
            return SDL_PIXELFORMAT_RGB24;
        else
            return SDL_PIXELFORMAT_RGBA32;
    case PIXEL_UNSIGNED_SHORT_4_4_4_4:
        return SDL_PIXELFORMAT_RGBA4444;
    case PIXEL_UNSIGNED_SHORT_5_5_5_1:
        return SDL_PIXELFORMAT_RGBA5551;
    case PIXEL_UNSIGNED_SHORT_5_6_5:
        return SDL_PIXELFORMAT_RGB565;
    }
}

void Software::SetTextureImage(u32 width, u32 height, PixelFormat fmt, PixelDataType type, const void *data)
{
    if (boundTexture)
    {
        u32 bpp = 2;
        if (type == PIXEL_UNSIGNED_BYTE)
        {
            if (fmt == PIXEL_RGB)
                bpp = 3;
            else
                bpp = 4;
        }
        boundTexture->texels.resize(width * height);
        if (data)
            SDL_ConvertPixels(width, height, GetSDLPixelFormat(fmt, type), data, width * bpp, SDL_PIXELFORMAT_ARGB8888,
                              boundTexture->texels.data(), width * sizeof(u32));
        boundTexture->width = width;
        boundTexture->height = height;
        boundTexture->format = fmt;
        boundTexture->type = type;
    }
}

void Software::SetTextureSubImage(i32 xoffset, i32 yoffset, i32 width, i32 height, const void *data)
{
    if (boundTexture)
    {
        SDL_ConvertPixels(width, height, SDL_PIXELFORMAT_RGB24, data, width * 3, SDL_PIXELFORMAT_ARGB8888,
                          boundTexture->texels.data() + (yoffset * boundTexture->width) + xoffset,
                          boundTexture->width * sizeof(u32));
    }
}

void Software::ReadPixels(i32 x, i32 y, i32 width, i32 height, const void *pixels)
{
    u8 *dst = (u8 *)pixels;
    i32 pitch = width * 4;
    for (i32 row = 0; row < height; row++)
    {
        const u8 *src = (u8 *)framebuffer + ((GAME_WINDOW_HEIGHT - 1 - (y + row)) * GAME_WINDOW_WIDTH + x) * 4;
        memcpy(dst + row * pitch, src, pitch);
    }
}

inline ZunVec3 Software::ProjectToNDC(ZunVec3 vertex, ZunMatrix mv, ZunMatrix p, f32 &viewZ, f32 &W)
{
    ZunVec4 clip = mv * ZunVec4(vertex, 1.0f);
    viewZ = clip.z;
    clip = p * clip;
    ZunVec3 ndc = {clip.x, clip.y, clip.z};
    if (clip.w != 0)
    {
        ndc /= clip.w;
        W = 1.0f / clip.w;
    }

    return ndc;
}

inline ZunVec2 Software::ProjectTexCoordToNDC(ZunVec2 texCoord, ZunMatrix textureMatrix)
{
    ZunVec4 clip = textureMatrix * ZunVec4(ZunVec3(texCoord.x, texCoord.y, 1.0f), 1.0f);
    ZunVec2 ndc = {clip.x, clip.y};
    return ndc;
}

inline ZunVec3 Software::NDCToScreen(ZunVec3 vertex)
{
    ZunVec3 screen;
    screen.x = (vertex.x + 1) / 2.0f * viewport[2] + viewport[0];
    screen.y = (1 - (vertex.y + 1) / 2.0f) * viewport[3] + viewport[1];
    screen.z = vertex.z;
    return screen;
}

inline f32 EdgeFunction(ZunVec3 v0, ZunVec3 v1, ZunVec3 v2)
{
    return (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
}

inline u8 AlphaBlendU8(u8 src, u8 dst, u8 a, u8 ia)
{
    return (u8)ZUN_MIN((((u32)src * a + (u32)dst * ia + 128) >> 8), 255);
}

inline u8 LerpU8(u32 a, u32 b, u32 t)
{
    return (u8)((a * (255 - t) + b * t) >> 8);
}

inline u32 InterpZunColor(ZunColor src, ZunColor dst, u32 t)
{
    return RGBAToZunColor(LerpU8(ZunR(src), ZunR(dst), t), LerpU8(ZunG(src), ZunG(dst), t),
                          LerpU8(ZunB(src), ZunB(dst), t), ZunA(src));
}

inline ZunColor ZunColorMul(u32 a, u32 b)
{
    u32 R = (ZunR(a) * ZunR(b)) >> 8;
    u32 G = (ZunG(a) * ZunG(b)) >> 8;
    u32 B = (ZunB(a) * ZunB(b)) >> 8;
    u32 A = (ZunA(a) * ZunA(b)) >> 8;

    return RGBAToZunColor(R, G, B, A);
}

void Software::Draw(PrimitiveType type, i32 start, i32 count)
{
    if (count == 0)
        return;
    u32 increment = type == PRIM_TRIANGLE_STRIP ? 1 : 3;
    u32 index = start;
    u32 last_index = start + count;
    if (type == PRIM_TRIANGLE_STRIP)
        last_index -= 2;
    ZunMatrix modelview = view * model;
    while (index < last_index)
    {
        f32 invw0, invw1, invw2;
        f32 viewZ0, viewZ1, viewZ2;
        f32 ndcZ0, ndcZ1, ndcZ2;
        ZunVec3 v0 =
            ProjectToNDC(*(ZunVec3 *)((u8 *)vertexData + vertexStride * index), modelview, projection, viewZ0, invw0);
        ZunVec3 v1 = ProjectToNDC(*(ZunVec3 *)((u8 *)vertexData + vertexStride * (index + 1)), modelview, projection,
                                  viewZ1, invw1);
        ZunVec3 v2 = ProjectToNDC(*(ZunVec3 *)((u8 *)vertexData + vertexStride * (index + 2)), modelview, projection,
                                  viewZ2, invw2);

        ZunVec2 tc0, tc1, tc2;
        Diffuse diffuse0, diffuse1, diffuse2;
        u32 *texels;
        i32 texW, texH;
        if (useTexCoord)
        {
            const ZunVec2 texDim = {boundTexture ? boundTexture->width : 0, boundTexture ? boundTexture->height : 0};
            tc0 =
                ProjectTexCoordToNDC(*(ZunVec2 *)((u8 *)texCoordData + texCoordStride * index), textureMatrix) * texDim;
            tc1 = ProjectTexCoordToNDC(*(ZunVec2 *)((u8 *)texCoordData + texCoordStride * (index + 1)), textureMatrix) *
                  texDim;
            tc2 = ProjectTexCoordToNDC(*(ZunVec2 *)((u8 *)texCoordData + texCoordStride * (index + 2)), textureMatrix) *
                  texDim;
            if (boundTexture)
            {
                texels = boundTexture->texels.data();
                texW = boundTexture->width;
                texH = boundTexture->height;
            }
        }

        if (useDiffuse)
        {
            diffuse0 = Diffuse(*(ColorData *)((u8 *)diffuseData + diffuseStride * index));
            diffuse1 = Diffuse(*(ColorData *)((u8 *)diffuseData + diffuseStride * (index + 1)));
            diffuse2 = Diffuse(*(ColorData *)((u8 *)diffuseData + diffuseStride * (index + 2)));
        }

        if (type == PRIM_TRIANGLE_STRIP && ((index - start) & 1))
        {
            std::swap(v0, v1);
            std::swap(tc0, tc1);
            std::swap(invw0, invw1);
            std::swap(viewZ0, viewZ1);
            std::swap(diffuse0, diffuse1);
        }

        if (EdgeFunction(v0, v1, v2) < 0)
        {
            std::swap(v1, v2);
            std::swap(tc1, tc2);
            std::swap(invw1, invw2);
            std::swap(viewZ1, viewZ2);
            std::swap(diffuse1, diffuse2);
        }
        ndcZ0 = v0.z;
        ndcZ1 = v1.z;
        ndcZ2 = v2.z;
        v0 = NDCToScreen(v0);
        v1 = NDCToScreen(v1);
        v2 = NDCToScreen(v2);

        i32 xmin = std::max(viewport[0], (i32)std::floor(std::min({v0.x, v1.x, v2.x})));
        i32 xmax = std::min(viewport[0] + viewport[2] - 1, (i32)std::ceil(std::max({v0.x, v1.x, v2.x})));
        i32 ymin = std::max(viewport[1], (i32)std::floor(std::min({v0.y, v1.y, v2.y})));
        i32 ymax = std::min(viewport[1] + viewport[3] - 1, (i32)std::ceil(std::max({v0.y, v1.y, v2.y})));

        const ZunVec3 vP = ZunVec3(xmin + 0.5f, ymin + 0.5f, 0);
        ZunVec3 edges = {EdgeFunction(v1, v2, vP), EdgeFunction(v2, v0, vP), EdgeFunction(v0, v1, vP)};
        f32 area = EdgeFunction(v0, v1, v2);

        ZunVec3 e_dx = {v1.y - v2.y, v2.y - v0.y, v0.y - v1.y};
        ZunVec3 e_dy = {v2.x - v1.x, v0.x - v2.x, v1.x - v0.x};

        float invarea = 1.0f / area;
        ZunVec3 w0 = edges * invarea;
        const ZunVec3 w_dx = e_dx * invarea;
        const ZunVec3 w_dy = e_dy * invarea;

        ZunVec2 uv_row = (tc0 * invw0) * w0.x + (tc1 * invw1) * w0.y + (tc2 * invw2) * w0.z;
        const ZunVec2 uv_dx = ((tc0 * invw0) * w_dx.x + (tc1 * invw1) * w_dx.y + (tc2 * invw2) * w_dx.z);
        const ZunVec2 uv_dy = ((tc0 * invw0) * w_dy.x + (tc1 * invw1) * w_dy.y + (tc2 * invw2) * w_dy.z);

        Diffuse dif0;
        Diffuse dif_dx, dif_dy;
        if (useDiffuse)
        {
            dif0 = (diffuse0 * invw0) * w0.x + (diffuse1 * invw1) * w0.y + (diffuse2 * invw2) * w0.z;
            dif_dx = (diffuse0 * invw0) * w_dx.x + (diffuse1 * invw1) * w_dx.y + (diffuse2 * invw2) * w_dx.z;
            dif_dy = (diffuse0 * invw0) * w_dy.x + (diffuse1 * invw1) * w_dy.y + (diffuse2 * invw2) * w_dy.z;
        }

        f32 invw_row = invw0 * w0.x + invw1 * w0.y + invw2 * w0.z;
        const f32 invw_dx = invw0 * w_dx.x + invw1 * w_dx.y + invw2 * w_dx.z;
        const f32 invw_dy = invw0 * w_dy.x + invw1 * w_dy.y + invw2 * w_dy.z;

        const f32 depthDif = depthFar - depthNear;

        f32 ndcZ_row = (w0.x * ndcZ0 * invw0 + w0.y * ndcZ1 * invw1 + w0.z * ndcZ2 * invw2);
        f32 ndcZ_dx = (w_dx.x * ndcZ0 * invw0 + w_dx.y * ndcZ1 * invw1 + w_dx.z * ndcZ2 * invw2);
        f32 ndcZ_dy = (w_dy.x * ndcZ0 * invw0 + w_dy.y * ndcZ1 * invw1 + w_dy.z * ndcZ2 * invw2);

        f32 fogZ_row = viewZ0 * invw0 * w0.x + viewZ1 * invw1 * w0.y + viewZ2 * invw2 * w0.z;
        const f32 fogZ_dx = viewZ0 * invw0 * w_dx.x + viewZ1 * invw1 * w_dx.y + viewZ2 * invw2 * w_dx.z;
        const f32 fogZ_dy = viewZ0 * invw0 * w_dy.x + viewZ1 * invw1 * w_dy.y + viewZ2 * invw2 * w_dy.z;

        ZunColor precompTextureFactor = textureFactor;
        const f32 precompInvFogDif = 1.0f / (fogFar - fogNear);

        for (int y = ymin; y <= ymax; y++, w0 += w_dy, uv_row += uv_dy, invw_row += invw_dy, ndcZ_row += ndcZ_dy,
                 fogZ_row += fogZ_dy, dif0 += dif_dy)
        {
            ZunVec3 w = w0;
            ZunVec2 uv1 = uv_row;
            f32 invw = invw_row;
            f32 ndcZ = ndcZ_row;
            f32 fogZ = fogZ_row;
            Diffuse dif = dif0;
            for (int x = xmin; x <= xmax;
                 x++, w += w_dx, uv1 += uv_dx, invw += invw_dx, ndcZ += ndcZ_dx, fogZ += fogZ_dx, dif += dif_dx)
            {
                if (w.x >= 0 && w.y >= 0 && w.z >= 0)
                {
                    const i32 pixelCoord = y * GAME_WINDOW_WIDTH + x;
                    const f32 clipW = 1.0f / invw; // bad
                    i32 u = uv1.x * clipW;
                    i32 v = uv1.y * clipW;
                    f32 depth;
                    if (useDepthTest)
                    {
                        depth = ((ndcZ * clipW) * 0.5f + 0.5f) * depthDif + depthNear;
                        if (depthFunc == DEPTH_FUNC_LEQUAL && depth > depthBuffer[pixelCoord])
                        {
                            continue;
                        }
                    }
                    ZunColor diffuse = RGBAToZunColor((u8)(dif.r * clipW), (u8)(dif.g * clipW), (u8)(dif.b * clipW),
                                                      (u8)(dif.a * clipW));
                    ZunColor fragColor = COLOR_WHITE;
                    ZunColor fragArg1 = fragColor;
                    ZunColor fragArg2 = fragColor;
                    if (boundTexture && useTexCoord)
                    {
                        fragArg1 = texels[(v & (texH - 1)) * texW + (u & (texW - 1))];
                    }
                    else
                    {
                        fragArg1 = diffuse;
                    }

                    if (!noVertexBuffer)
                    {
                        fragArg2 = precompTextureFactor;
                    }
                    else
                    {
                        fragArg2 = diffuse;
                    }

                    switch (colorOp)
                    {
                    case COLOR_OP_MODULATE:
                        fragColor = ZunColorMul(fragArg1, fragArg2);
                        break;
                    case COLOR_OP_ADD:
                        fragColor =
                            RGBAToZunColor(ZunR(fragArg1) + ZunR(fragArg2), ZunG(fragArg1) + ZunG(fragArg2),
                                           ZunB(fragArg1) + ZunB(fragArg2), (ZunA(fragArg1) * ZunA(fragArg2)) >> 8);
                        break;
                    case COLOR_OP_REPLACE:
                        fragColor = fragArg1;
                    }

                    if (!noFog)
                    {
                        f32 fogDepth = fogZ * clipW;
                        f32 fogCoefficient = (fogFar - fogDepth) * precompInvFogDif;
                        fragColor = InterpZunColor(fragColor, fogColor,
                                                   255 - (u32)(ZUN_MIN(ZUN_MAX(fogCoefficient, 0.0f), 1.0f) * 255.0f));
                    }

                    ZunColor src = fragColor;
                    ZunColor dst = framebuffer[pixelCoord];
                    u8 srcFactor = ZunA(src);
                    u8 dstFactor = 255;
                    if (blendMode == BLEND_INV_SRC_ALPHA)
                    {
                        dstFactor -= srcFactor;
                    }
                    if (ZunA(fragColor) >= alphaThreshold)
                    {
                        if (useDepthTest && depthMask)
                            depthBuffer[pixelCoord] = depth;
                        framebuffer[pixelCoord] =
                            RGBAToZunColor(AlphaBlendU8(ZunR(src), ZunR(dst), srcFactor, dstFactor),
                                           AlphaBlendU8(ZunG(src), ZunG(dst), srcFactor, dstFactor),
                                           AlphaBlendU8(ZunB(src), ZunB(dst), srcFactor, dstFactor), ZunA(src));
                    }
                }
            }
        }
        index += increment;
    }
}

void Software::SwapBuffers()
{
    SDL_UpdateTexture(framebufferTexture, NULL, framebuffer, GAME_WINDOW_WIDTH * sizeof(u32));
    SDL_RenderCopy(renderer, framebufferTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
}
