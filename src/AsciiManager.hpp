#pragma once

// #include <d3dx8math.h>

#include "AnmManager.hpp"
#include "Chain.hpp"
#include "StageMenu.hpp"
#include "ZunColor.hpp"
#include "ZunMath.hpp"
#include "ZunResult.hpp"
#include "ZunTimer.hpp"
#include "inttypes.hpp"

namespace th06
{
#define TEXT_RIGHT_ARROW 0x7f

struct AsciiManagerString
{
    char text[64];
    ZunVec3 position;
    ZunColor color;
    ZunVec2 scale;
    // If true, we are drawing the currently selected element of the MainMenu
    // class.
    u32 isSelected;
    // If true, we are drawing an element of the Gui class.
    u32 isGui;
};
ZUN_ASSERT_SIZE(AsciiManagerString, 0x60);

struct AsciiManagerPopup
{
    char digits[8];
    ZunVec3 position;
    ZunColor color;
    ZunTimer timer;
    u8 inUse;
    u8 characterCount;
};
ZUN_ASSERT_SIZE(AsciiManagerPopup, 0x28);

struct WeirdPadding
{
    u32 unk;
};

// The AsciiManager is responsible for drawing various textual elements on the
// screen:
//
// - The FPS counter
// - The in-game menus
// - Various text elements such as the "Stage clear" prompt.
struct AsciiManager
{
    AsciiManager();

    static ZunResult RegisterChain();
    static void CutChain();

    static ChainCallbackResult OnUpdate(AsciiManager *s);
    static ChainCallbackResult OnDrawMenus(AsciiManager *s);
    static ChainCallbackResult OnDrawPopups(AsciiManager *s);
    static ZunResult AddedCallback(AsciiManager *s);
    static ZunResult DeletedCallback(AsciiManager *s);

    void InitializeVms();

    void DrawStrings();
    void DrawPopupsWithHwVertexProcessing();
    void DrawPopupsWithoutHwVertexProcessing();

    void AddString(ZunVec3 *position, char *text);
    void AddFormatText(ZunVec3 *position, const char *fmt, ...);
    void CreatePopup1(ZunVec3 *position, i32 value, ZunColor color);
    void CreatePopup2(ZunVec3 *position, i32 value, ZunColor color);

    void SetColor(ZunColor color)
    {
        this->color = color;
    }

    AnmVm vm0;
    AnmVm vm1;
    AsciiManagerString strings[256];
    i32 numStrings;
    ZunColor color;
    ZunVec2 scale;
    // If true, we are drawing an element of the Gui class.
    u32 isGui;
    // If true, we are drawing the currently selected element of the MainMenu
    // class.
    bool isSelected;
    i32 nextPopupIndex1;
    i32 nextPopupIndex2;
    // Seems unused
    WeirdPadding unk3;
    // Menu that shows up when the player presses the menu button while in-game.
    StageMenu gameMenu;
    // Menu that shows up when the player dies after losing their last life.
    StageMenu retryMenu;
    AsciiManagerPopup popups[515];
};
ZUN_ASSERT_SIZE(AsciiManager, 0xc1ac);
DIFFABLE_EXTERN(AsciiManager, g_AsciiManager);
}; // namespace th06
