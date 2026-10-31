#pragma once
#include <d3dx8math.h>

#include "AnmManager.hpp"
#include "Chain.hpp"
#include "StageMenu.hpp"
#include "Supervisor.hpp"
#include "ZunResult.hpp"
#include "ZunTimer.hpp"
#include "decomp.hpp"

namespace th06
{
#define TEXT_RIGHT_ARROW 0x7f

#define ASCII_SCORE_POPUPS_COUNT 512
#define ASCII_PLAYER_POPUPS_COUNT 3
#define ASCII_TOTAL_POPUPS_COUNT (ASCII_SCORE_POPUPS_COUNT + ASCII_PLAYER_POPUPS_COUNT)

#define ASCII_SCORE_POPUPS_START 0
#define ASCII_PLAYER_POPUPS_START ASCII_SCORE_POPUPS_COUNT

#define ASCII_STRING_COUNT 256

struct AsciiManagerString
{
    char text[64];
    D3DXVECTOR3 position;
    D3DCOLOR color;
    ZunVec2 scale;
    // If true, we are drawing the currently selected element of the MainMenu
    // class.
    ZunBool isSelected;
    // If true, we are drawing an element of the Gui class.
    ZunBool isGui;
};
ZUN_ASSERT_TYPE(AsciiManagerString, 0x60, 4);

struct AsciiManagerPopup
{
    char digits[8];
    D3DXVECTOR3 position;
    D3DCOLOR color;
    ZunTimer timer;
    u8 inUse;
    u8 characterCount;
    alignment_padding(0x2);
};
ZUN_ASSERT_TYPE(AsciiManagerPopup, 0x28, 4);

// The AsciiManager is responsible for drawing various textual elements on the
// screen:
//
// - The FPS counter
// - The in-game menus
// - Various text elements such as the "Stage clear" prompt.
struct AsciiManager
{
    void InitializeVms()
    {
        memset(this, 0, sizeof(AsciiManager));

        this->color = COLOR_WHITE;
        this->scale.x = 1.0f;
        this->scale.y = 1.0f;

        this->scorePopupVm.flags.anchor = AnmVmAnchor_TopLeft;

        g_AnmManager->InitializeAndSetSprite(&this->scorePopupVm, 0);
        // BUG: That's not how your sprite numbers work ZUN
        g_AnmManager->InitializeAndSetSprite(&this->fontVm, ' ');

        this->scorePopupVm.pos.z = 0.1f;
        this->isSelected = false;
    }

    void DrawStrings();
    void DrawPopupsWithHwVertexProcessing();
    void DrawPopupsWithoutHwVertexProcessing();

    void AddString(D3DXVECTOR3 *position, const char *text);
    void AddFormatText(D3DXVECTOR3 *position, const char *fmt, ...);
    void CreatePopup1(D3DXVECTOR3 *position, i32 value, D3DCOLOR color);
    void CreatePopup2(D3DXVECTOR3 *position, i32 value, D3DCOLOR color);

    void SetColor(ZunColor color)
    {
        this->color = color;
    }

    void SetScale(f32 x, f32 y)
    {
        this->scale.x = x;
        this->scale.y = y;
    }

    void SetIsGuiMode(ZunBool isGui)
    {
        this->isGui = isGui;
    }

    void SetIsSelected(ZunBool isSelected)
    {
        this->isSelected = isSelected;
    }

    AnmVm fontVm;
    AnmVm scorePopupVm;
    AsciiManagerString strings[ASCII_STRING_COUNT];
    i32 numStrings;
    D3DCOLOR color;
    ZunVec2 scale;
    // If true, we are drawing an element of the Gui class.
    ZunBool isGui;
    // If true, we are drawing the currently selected element of the MainMenu
    // class.
    ZunBool isSelected;
    i32 nextPopupIndex1;
    i32 nextPopupIndex2;
    unreferenced_fields(0x4);
    // Menu that shows up when the player presses the menu button while in-game.
    StageMenu gameMenu;
    // Menu that shows up when the player dies after losing their last life.
    StageMenu retryMenu;
    AsciiManagerPopup popups[ASCII_TOTAL_POPUPS_COUNT];
};
ZUN_ASSERT_TYPE(AsciiManager, 0xc1ac, 4);

ZunResult AsciiManager_RegisterChain();
void AsciiManager_CutChain();

extern AsciiManager g_AsciiManager;
} // namespace th06
