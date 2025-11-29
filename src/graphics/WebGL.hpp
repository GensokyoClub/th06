#pragma once

#include "GfxInterface.hpp"
#include <SDL2/SDL_opengl.h>

namespace th06
{
enum GlShaderUniform
{
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

struct WebGL : GfxInterface 
{
    static void SetContextFlags();
    static GfxInterface *Create();
    
    bool Init();

    virtual void SetFogRange(f32 nearPlane, f32 farPlane);
    virtual void SetFogColor(ZunColor color);
    virtual void ToggleVertexAttribute(u8 attr, bool enable);
    virtual void SetAttributePointer(VertexAttributeArrays attr, std::size_t stride, void *ptr);
    virtual void SetColorOp(TextureOpComponent component, ColorOp op);
    virtual void SetTextureFactor(ZunColor factor);
    virtual void SetTransformMatrix(TransformMatrix type, ZunMatrix &matrix);
    virtual void Draw();

private:
    GLuint fragmentShaderHandle;
    GLuint vertexShaderHandle;
    GLuint programHandle;

    GLint uniforms[UNIFORMS_COUNT];
};
}; // namespace th06

