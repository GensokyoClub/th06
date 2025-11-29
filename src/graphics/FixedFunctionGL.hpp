#pragma once

#include "GfxInterface.hpp"

namespace th06
{
struct FixedFunctionGL : GfxInterface 
{
    static GfxInterface *Init();
    static void SetContextFlags();

    virtual void SetFogRange(f32 nearPlane, f32 farPlane);
    virtual void SetFogColor(ZunColor color);
    virtual void ToggleVertexAttribute(u8 attr, bool enable);
    virtual void SetAttributePointer(VertexAttributeArrays attr, std::size_t stride, void *ptr);
    virtual void SetColorOp(TextureOpComponent component, ColorOp op);
    virtual void SetTextureFactor(ZunColor factor);
    virtual void SetTransformMatrix(TransformMatrix type, ZunMatrix &matrix);
    virtual void Draw();
};
}; // namespace th06

