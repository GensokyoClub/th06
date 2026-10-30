#pragma once
#include "ZunColor.hpp"
#include "decomp.hpp"
#include "i18n.hpp"

#include <d3d8.h>
#include <string>

namespace th06
{
struct TextFont
{
    HFONT font;
    i32 width;
    i32 height;
    std::string name;
    i32 weight;

    TextFont();

    ~TextFont();

    void SetName(const char *value);

    void Release();

    void SetFontSize(i32 h, i32 w)
    {
        height = h;
        width = w;
    }
    HFONT GetFont()
    {
        return font;
    }
    HFONT Create();
};

void TextHelper_CreateTextBuffer();
void TextHelper_ReleaseTextBuffer();
void TextHelper_RenderTextToTexture(i32 xPos, i32 yPos, i32 spriteWidth, i32 spriteHeight, i32 fontHeight,
                                    i32 fontWidth, ZunColor textColor, ZunColor shadowColor, const char *string,
                                    LPDIRECT3DTEXTURE8 outTexture);

} // namespace th06
