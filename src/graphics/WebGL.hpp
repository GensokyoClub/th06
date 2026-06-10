#pragma once

#include "GLFunc.hpp"
#include "GfxInterface.hpp"
#include <SDL2/SDL.h>
#include <vector>

enum GlShaderUniform {
    UNIFORM_MODELVIEW,
    UNIFORM_PROJECTION,
    UNIFORM_TEXTURE_MATRIX,
    UNIFORM_ENV_DIFFUSE,
    UNIFORM_TEX_COORD_FLAG,
    UNIFORM_DIFFUSE_FLAG,
    UNIFORM_TEXTURE_SAMPLER,
    UNIFORM_FOG_NEAR,
    UNIFORM_FOG_FAR,
    UNIFORM_FOG_COLOR,
    UNIFORM_COLOR_OP,
    UNIFORMS_COUNT
};

struct WebGL : GfxInterface {
    static void SetContextFlags();
    static GfxInterface *Create();

    bool Init();
    virtual void Exit();
    ~WebGL() override { Exit(); };

    virtual void SetFogRange(f32 nearPlane, f32 farPlane) override;
    virtual void SetFogColor(ZunColor color) override;
    virtual void ToggleVertexAttribute(u8 attr, bool enable) override;
    virtual void SetAttributePointer(VertexAttributeArrays attr, std::size_t stride, void *ptr) override;
    virtual void SetColorOp(TextureOpComponent component, ColorOp op) override;
    virtual void SetTextureFactor(ZunColor factor) override;
    virtual void SetTransformMatrix(TransformMatrix type, const ZunMatrix &matrix) override;

    virtual void SetTextureFilter() override;

    virtual void GetViewport(u32 *viewport) override;
    virtual void GetDepthRange(f32 *depthRange) override;
    virtual void SetViewport(i32 x, i32 y, i32 width, i32 height) override;
    virtual void SetDepthRange(f32 nearPlane, f32 farPlane) override;

    virtual void Enable(Capabilities cap) override;
    virtual bool HasError() override;
    virtual void SetBlendMode(BlendMode mode) override;
    virtual void SetDepthMask(bool enable) override;
    virtual void SetDepthFunc(DepthFunc func) override;

    virtual void SetClearDepth(f32 depth) override;
    virtual void SetClearColor(f32 r, f32 g, f32 b, f32 a) override;
    virtual void Clear(u32 clearBits) override;

    virtual GfxTextureHandle CreateTexture() override;
    virtual void BindTexture(GfxTextureHandle handle) override;
    virtual void DeleteTexture(GfxTextureHandle handle) override;
    virtual void SetTextureImage(u32 width, u32 height, PixelFormat fmt, PixelDataType type, const void *data) override;
    virtual void SetTextureSubImage(i32 xoffset, i32 yoffset, i32 width, i32 height, const void *data) override;

    virtual void ReadPixels(i32 x, i32 y, i32 width, i32 height, const void *pixels) override;

    virtual void Draw(PrimitiveType type, i32 start, i32 count) override;
    virtual void SwapBuffers() override;

  private:
    SDL_Window *window;
    SDL_GLContext glContext;

    GLuint fragmentShaderHandle;
    GLuint vertexShaderHandle;
    GLuint programHandle;

    GLint uniforms[UNIFORMS_COUNT];
};
