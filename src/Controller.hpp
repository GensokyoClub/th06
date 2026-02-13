#pragma once

#include "inttypes.hpp"

namespace th06
{
enum TouhouButton
{
    TH_BUTTON_SHOOT = 1 << 0,
    TH_BUTTON_BOMB = 1 << 1,
    TH_BUTTON_FOCUS = 1 << 2,
    TH_BUTTON_MENU = 1 << 3,
    TH_BUTTON_UP = 1 << 4,
    TH_BUTTON_DOWN = 1 << 5,
    TH_BUTTON_LEFT = 1 << 6,
    TH_BUTTON_RIGHT = 1 << 7,
    TH_BUTTON_SKIP = 1 << 8,

    // Player 2
    TH_BUTTON_SHOOT2 = 1 << 9,
    TH_BUTTON_BOMB2 = 1 << 10,
    TH_BUTTON_FOCUS2 = 1 << 11,
    TH_BUTTON_UP2 = 1 << 12,
    TH_BUTTON_DOWN2 = 1 << 13,
    TH_BUTTON_LEFT2 = 1 << 14,
    TH_BUTTON_RIGHT2 = 1 << 15,

    TH_BUTTON_UP_LEFT = TH_BUTTON_UP | TH_BUTTON_LEFT,
    TH_BUTTON_UP_RIGHT = TH_BUTTON_UP | TH_BUTTON_RIGHT,
    TH_BUTTON_DOWN_LEFT = TH_BUTTON_DOWN | TH_BUTTON_LEFT,
    TH_BUTTON_DOWN_RIGHT = TH_BUTTON_DOWN | TH_BUTTON_RIGHT,
    TH_BUTTON_DIRECTION = TH_BUTTON_DOWN | TH_BUTTON_RIGHT | TH_BUTTON_UP | TH_BUTTON_LEFT,

    // Player 2
    TH_BUTTON_UP_LEFT2 = TH_BUTTON_UP2 | TH_BUTTON_LEFT2,
    TH_BUTTON_UP_RIGHT2 = TH_BUTTON_UP2 | TH_BUTTON_RIGHT2,
    TH_BUTTON_DOWN_LEFT2 = TH_BUTTON_DOWN2 | TH_BUTTON_LEFT2,
    TH_BUTTON_DOWN_RIGHT2 = TH_BUTTON_DOWN2 | TH_BUTTON_RIGHT2,
    TH_BUTTON_DIRECTION2 = TH_BUTTON_DOWN2 | TH_BUTTON_RIGHT2 | TH_BUTTON_UP2 | TH_BUTTON_LEFT2,

    TH_BUTTON_SELECTMENU = TH_BUTTON_SHOOT,
    TH_BUTTON_RETURNMENU = TH_BUTTON_MENU | TH_BUTTON_BOMB,
    TH_BUTTON_WRONG_CHEATCODE = TH_BUTTON_SHOOT | TH_BUTTON_BOMB | TH_BUTTON_MENU,
    TH_BUTTON_ANY = 0xFFFF,
};

namespace Controller
{
u16 GetJoystickCaps(void);
u32 SetButtonFromControllerInputs(u16 *outButtons, i16 controllerButtonToTest, enum TouhouButton touhouButton,
                                  u32 inputButtons);

unsigned int SetButtonFromDirectInputJoystate(u16 *outButtons, i16 controllerButtonToTest,
                                              enum TouhouButton touhouButton, u8 *inputButtons);

u16 GetControllerInput(u16 buttons);
u8 *GetControllerState();
u16 GetInput(void);
void ResetKeyboard(void);
}; // namespace Controller
}; // namespace th06
