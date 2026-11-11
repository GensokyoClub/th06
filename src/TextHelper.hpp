#pragma once
#include "ZunColor.hpp"
#include "decomp.hpp"
#include "i18n.hpp"

#include <d3d8.h>
#include <string>

namespace th06
{

void TextHelper_CreateTextBuffer();
void TextHelper_ReleaseTextBuffer();
void TextHelper_RenderTextToTexture(i32 xPos, i32 yPos, i32 spriteWidth, i32 spriteHeight, i32 fontHeight,
                                    i32 fontWidth, ZunColor textColor, ZunColor shadowColor, const char *string,
                                    LPDIRECT3DTEXTURE8 outTexture);

} // namespace th06
