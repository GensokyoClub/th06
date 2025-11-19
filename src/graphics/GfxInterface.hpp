#pragma once

#include <cstddef>
#include "inttypes.hpp"
#include "ZunColor.hpp"
#include "ZunMath.hpp"

namespace th06
{
enum DepthFunc
{
    DEPTH_FUNC_LEQUAL,
    DEPTH_FUNC_ALWAYS
};

// Position is implied, since everything uses is anyway
enum VertexAttributeFlags
{
    VERTEX_ATTR_TEX_COORD = (1 << 0),
    VERTEX_ATTR_DIFFUSE = (1 << 1),
};

enum VertexAttributeArrays
{
    VERTEX_ARRAY_POSITION,
    VERTEX_ARRAY_TEX_COORD,
    VERTEX_ARRAY_DIFFUSE
};

enum ColorOp
{
    COLOR_OP_MODULATE,
    COLOR_OP_ADD,
    COLOR_OP_REPLACE
};

enum TextureOpComponent
{
    COMPONENT_RGB,
    COMPONENT_ALPHA
};

enum TransformMatrix
{
    MATRIX_MODEL,
    MATRIX_VIEW,
    MATRIX_PROJECTION,
    MATRIX_TEXTURE
};

struct GfxInterface 
{
    virtual void SetFogRange(f32 nearPlane, f32 farPlane) = 0;
    virtual void SetFogColor(ZunColor color) = 0;
    virtual void ToggleVertexAttribute(u8 attr, bool enable) = 0;
    virtual void SetAttributePointer(VertexAttributeArrays attr, std::size_t stride, void *ptr) = 0;
    virtual void SetColorOp(TextureOpComponent component, ColorOp op) = 0;
    virtual void SetTextureFactor(ZunColor factor) = 0;
    virtual void SetTransformMatrix(TransformMatrix type, ZunMatrix &matrix) = 0;
    virtual void Draw() = 0;
};
}; // namespace th06
