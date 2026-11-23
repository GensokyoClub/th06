#pragma once
#include <d3dx8math.h>

#include "AnmManager.hpp"
#include "AnmVm.hpp"
#include "Chain.hpp"
#include "Supervisor.hpp"
#include "ZunResult.hpp"
#include "ZunTimer.hpp"
#include "decomp.hpp"

namespace th06
{
class StageMenu
{
  public:
    i32 OnUpdateGameMenu();
    i32 OnUpdateRetryMenu();

    void OnDrawGameMenu();
    void OnDrawRetryMenu();

  private:
    // Current state of this menu.
    u32 curState;
    // Number of frames since last state change. Used to delay certain actions
    // until an animation is finished.
    i32 numFrames;
    AnmVm menuSprites[6];
    AnmVm menuBackground;
};
ZUN_ASSERT_TYPE(StageMenu, 0x778, 4);

#define TEXT_RIGHT_ARROW 0x7f

#define ASCII_SCORE_POPUPS_COUNT 512
#define ASCII_STAR_POPUPS_COUNT 3
#define ASCII_TOTAL_POPUPS_COUNT (ASCII_SCORE_POPUPS_COUNT + ASCII_STAR_POPUPS_COUNT)

#define ASCII_SCORE_POPUPS_START 0
#define ASCII_STAR_POPUPS_START ASCII_SCORE_POPUPS_COUNT

#define ASCII_STRING_COUNT 256

struct AsciiManagerString
{
    char text[64];
    D3DXVECTOR3 position;
    D3DCOLOR color;
    Float2 scale;
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

        this->popupVm.flags.anchor = AnmVmAnchor_TopLeft;

        g_AnmManager->SetNewSprite(&this->popupVm, 0);
        // BUG: That's not how your sprite numbers work ZUN
        g_AnmManager->SetNewSprite(&this->fontVm, ' ');

        this->popupVm.pos.z = 0.1f;
        this->isSelected = false;
    }

    void DrawStrings();
    void DrawPopupsWithHwVertexProcessing();
    void DrawPopupsWithoutHwVertexProcessing();

    void AddString(D3DXVECTOR3 *position, const char *text);
    void AddFormatText(D3DXVECTOR3 *position, const char *fmt, ...);
    void CreateScorePopup(D3DXVECTOR3 *position, i32 score, D3DCOLOR color);
    void CreateStarPopup(D3DXVECTOR3 *position, i32 score, D3DCOLOR color);

    // Popups use 10 as a "digit" to offset the texture UV onto the correct
    // part of the texture. Since 10 is a valid score any negative args
    // passed to CreateScorePopup generate the powerup text instead.
    // ZUN probably should've made a dedicated function for this.

#define POWER_UP_DIGIT_HACK 10
#define CreatePowerUpPopup(position, color) CreateScorePopup(position, -1, color)

    void ResetStrings()
    {
        this->numStrings = 0;
    }

    void SetColor(D3DCOLOR color)
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
    AnmVm popupVm;
    AsciiManagerString strings[ASCII_STRING_COUNT];
    i32 numStrings;
    D3DCOLOR color;
    Float2 scale;
    // If true, we are drawing an element of the Gui class.
    ZunBool isGui;
    // If true, we are drawing the currently selected element of the MainMenu
    // class.
    ZunBool isSelected;
    i32 nextScorePopup;
    i32 nextStarPopup;
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
