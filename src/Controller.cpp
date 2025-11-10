#include "Controller.hpp"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_scancode.h>

#include "GameErrorContext.hpp"
#include "Supervisor.hpp"
#include "i18n.hpp"
#include "utils.hpp"

namespace th06
{
// DIFFABLE_STATIC(JOYCAPSA, g_JoystickCaps)
u16 g_FocusButtonConflictState;
u8 *keyboardState;

u16 Controller::GetJoystickCaps(void)
{
    //    JOYINFOEX pji;

    //    pji.dwSize = sizeof(JOYINFOEX);
    //    pji.dwFlags = JOY_RETURNALL;

    //    if (joyGetPosEx(0, &pji) != MMSYSERR_NOERROR)
    //    {
    //        GameErrorContext::Log(&g_GameErrorContext, TH_ERR_NO_PAD_FOUND);
    //        return 1;
    //    }
    //
    //    joyGetDevCapsA(0, &g_JoystickCaps, sizeof(g_JoystickCaps));
    return 0;
}

#define JOYSTICK_MIDPOINT(min, max) ((min + max) / 2)
#define JOYSTICK_BUTTON_PRESSED(button, x, y) (x > y ? button : 0)
#define JOYSTICK_BUTTON_PRESSED_INVERT(button, x, y) (x < y ? button : 0)
#define KEYBOARD_KEY_PRESSED(button, x) keyboardState[x] ? button : 0

u16 Controller::GetControllerInput(u16 buttons)
{
    // NOTE: Those names are like this to get perfect stack frame matching
    // TODO: Give meaningfull names that still match.
    //    JOYINFOEX aa;
    //    u32 ab;
    u32 shootPressed;
    //    DIJOYSTATE2 a0;
    //    u32 a2;
    //    HRESULT aaa;
    i16 stickX;
    i16 stickY;

    //
    if (g_Supervisor.gameController != NULL)
    {
        //        memset(&aa, 0, sizeof(aa));
        //        aa.dwSize = sizeof(JOYINFOEX);
        //        aa.dwFlags = JOY_RETURNALL;
        //
        //        if (joyGetPosEx(0, &aa) != MMSYSERR_NOERROR)
        //        {
        //            return buttons;
        //        }
        //
        shootPressed = SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.shootButton,
                                                     TH_BUTTON_SHOOT, g_Supervisor.gameController);

        if (g_ControllerMapping.shootButton != g_ControllerMapping.focusButton)
        {
            SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.focusButton, TH_BUTTON_FOCUS,
                                          g_Supervisor.gameController);
        }
        else
        {
            if (shootPressed != 0)
            {
                if (g_FocusButtonConflictState < 16)
                {
                    g_FocusButtonConflictState++;
                }

                if (g_FocusButtonConflictState >= 8)
                {
                    buttons |= TH_BUTTON_FOCUS;
                }
            }
            else
            {
                if (g_FocusButtonConflictState > 8)
                {
                    g_FocusButtonConflictState -= 8;
                }
                else
                {
                    g_FocusButtonConflictState = 0;
                }
            }
        }

        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.bombButton, TH_BUTTON_BOMB,
                                      g_Supervisor.gameController);
        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.menuButton, TH_BUTTON_MENU,
                                      g_Supervisor.gameController);
        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.upButton, TH_BUTTON_UP,
                                      g_Supervisor.gameController);
        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.downButton, TH_BUTTON_DOWN,
                                      g_Supervisor.gameController);
        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.leftButton, TH_BUTTON_LEFT,
                                      g_Supervisor.gameController);
        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.rightButton, TH_BUTTON_RIGHT,
                                      g_Supervisor.gameController);
        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.skipButton, TH_BUTTON_SKIP,
                                      g_Supervisor.gameController);

        if (SDL_GameControllerHasAxis(g_Supervisor.gameController, SDL_CONTROLLER_AXIS_LEFTX) &&
            SDL_GameControllerHasAxis(g_Supervisor.gameController, SDL_CONTROLLER_AXIS_LEFTY))
        {
            stickX = SDL_GameControllerGetAxis(g_Supervisor.gameController, SDL_CONTROLLER_AXIS_LEFTX);
            stickY = SDL_GameControllerGetAxis(g_Supervisor.gameController, SDL_CONTROLLER_AXIS_LEFTY);
        }
        else if (SDL_GameControllerHasAxis(g_Supervisor.gameController, SDL_CONTROLLER_AXIS_RIGHTX) &&
                 SDL_GameControllerHasAxis(g_Supervisor.gameController, SDL_CONTROLLER_AXIS_RIGHTY))
        {
            stickX = SDL_GameControllerGetAxis(g_Supervisor.gameController, SDL_CONTROLLER_AXIS_RIGHTX);
            stickY = SDL_GameControllerGetAxis(g_Supervisor.gameController, SDL_CONTROLLER_AXIS_RIGHTY);
        }
        else
        {
            return buttons;
        }

        // SDL sticks run from -32768 to 32767, with the minimum being up / left and the max being down / right
        //
        //        ab = ((g_JoystickCaps.wXmax - g_JoystickCaps.wXmin) / 2 / 2);
        //

        buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_RIGHT, stickX, JOYSTICK_MIDPOINT(0, INT16_MAX));
        buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_LEFT, -stickX, JOYSTICK_MIDPOINT(0, INT16_MAX));

        buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_DOWN, stickY, JOYSTICK_MIDPOINT(0, INT16_MAX));
        buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_UP, -stickY, JOYSTICK_MIDPOINT(0, INT16_MAX));
        //
        //        ab = ((g_JoystickCaps.wYmax - g_JoystickCaps.wYmin) / 2 / 2);
        //        buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_DOWN, aa.dwYpos,
        //                                           JOYSTICK_MIDPOINT(g_JoystickCaps.wYmin, g_JoystickCaps.wYmax) +
        //                                           ab);
        //        buttons |= JOYSTICK_BUTTON_PRESSED(
        //            TH_BUTTON_UP, JOYSTICK_MIDPOINT(g_JoystickCaps.wYmin, g_JoystickCaps.wYmax) - ab, aa.dwYpos);
        //
    }
    //    else
    //    {
    //        // FIXME: Next if not matching.
    //        aaa = g_Supervisor.controller->Poll();
    //        if (FAILED(aaa))
    //        {
    //            i32 retryCount = 0;
    //
    //            utils::DebugPrint2("error : DIERR_INPUTLOST\n");
    //            aaa = g_Supervisor.controller->Acquire();
    //
    //            while (aaa == DIERR_INPUTLOST)
    //            {
    //                aaa = g_Supervisor.controller->Acquire();
    //                utils::DebugPrint2("error : DIERR_INPUTLOST %d\n", retryCount);
    //
    //                retryCount++;
    //
    //                if (retryCount >= 400)
    //                {
    //                    return buttons;
    //                }
    //            }
    //
    //            return buttons;
    //        }
    //        else
    //        {
    //            memset(&a0, 0, sizeof(a0));
    //
    //            aaa = g_Supervisor.controller->GetDeviceState(sizeof(a0), &a0);
    //
    //            if (FAILED(aaa))
    //            {
    //                return buttons;
    //            }
    //
    //            a2 = SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.shootButton,
    //                                                  TH_BUTTON_SHOOT, a0.rgbButtons);
    //
    //            if (g_Supervisor.cfg.controllerMapping.shootButton != g_Supervisor.cfg.controllerMapping.focusButton)
    //            {
    //                SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.focusButton,
    //                                                 TH_BUTTON_FOCUS, a0.rgbButtons);
    //            }
    //            else
    //            {
    //                if (a2 != 0)
    //                {
    //                    if (g_FocusButtonConflictState < 16)
    //                    {
    //                        g_FocusButtonConflictState++;
    //                    }
    //
    //                    if (g_FocusButtonConflictState >= 8)
    //                    {
    //                        buttons |= TH_BUTTON_FOCUS;
    //                    }
    //                }
    //                else
    //                {
    //                    if (g_FocusButtonConflictState > 8)
    //                    {
    //                        g_FocusButtonConflictState -= 8;
    //                    }
    //                    else
    //                    {
    //                        g_FocusButtonConflictState = 0;
    //                    }
    //                }
    //            }
    //
    //            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.bombButton,
    //            TH_BUTTON_BOMB,
    //                                             a0.rgbButtons);
    //            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.menuButton,
    //            TH_BUTTON_MENU,
    //                                             a0.rgbButtons);
    //            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.upButton, TH_BUTTON_UP,
    //                                             a0.rgbButtons);
    //            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.downButton,
    //            TH_BUTTON_DOWN,
    //                                             a0.rgbButtons);
    //            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.leftButton,
    //            TH_BUTTON_LEFT,
    //                                             a0.rgbButtons);
    //            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.rightButton,
    //            TH_BUTTON_RIGHT,
    //                                             a0.rgbButtons);
    //            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.skipButton,
    //            TH_BUTTON_SKIP,
    //                                             a0.rgbButtons);
    //
    //            buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_RIGHT, a0.lX, g_Supervisor.cfg.padXAxis);
    //            buttons |= JOYSTICK_BUTTON_PRESSED_INVERT(TH_BUTTON_LEFT, a0.lX, -g_Supervisor.cfg.padXAxis);
    //            buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_DOWN, a0.lY, g_Supervisor.cfg.padYAxis);
    //            buttons |= JOYSTICK_BUTTON_PRESSED_INVERT(TH_BUTTON_UP, a0.lY, -g_Supervisor.cfg.padYAxis);
    //        }
    //    }
    //
    return buttons;
}

u32 Controller::SetButtonFromDirectInputJoystate(u16 *outButtons, i16 controllerButtonToTest,
                                                 enum TouhouButton touhouButton, u8 *inputButtons)
{
    if (controllerButtonToTest < 0)
    {
        return 0;
    }

    *outButtons |= (inputButtons[controllerButtonToTest] & 0x80 ? touhouButton & 0xFFFF : 0);

    return inputButtons[controllerButtonToTest] & 0x80 ? touhouButton & 0xFFFF : 0;
}

u32 Controller::SetButtonFromControllerInputs(u16 *outButtons, i16 controllerButtonToTest,
                                              enum TouhouButton touhouButton, SDL_GameController *controller)
{
    u8 pressed;

    if (controllerButtonToTest < 0)
    {
        return 0;
    }

    pressed = SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)controllerButtonToTest);

    *outButtons |= pressed ? touhouButton & 0xFFFF : 0;

    return pressed ? touhouButton & 0xFFFF : 0;
}

DIFFABLE_STATIC_ARRAY(u8, SDL_CONTROLLER_BUTTON_MAX, g_ControllerData);

// This is for rebinding keys
u8 *th06::Controller::GetControllerState()
{
    //    JOYINFOEX joyinfoex;
    //    u32 joyButtonBit;
    //    u32 joyButtonIndex;
    //
    //    i32 dires;
    //    DIJOYSTATE2 dijoystate2;
    //    i32 diRetryCount;

    if (g_Supervisor.gameController != NULL)
    {
        memset(&g_ControllerData, 0, sizeof(g_ControllerData));

        SDL_Joystick *joystick = SDL_GameControllerGetJoystick(g_Supervisor.gameController);

        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
        {
            if (SDL_GameControllerGetButton(g_Supervisor.gameController, (SDL_GameControllerButton)i))
            {
                g_ControllerData[i] = 0x80;
            }
        }
    }

    //
    //    if (g_Supervisor.controller == NULL)
    //    {
    //        memset(&joyinfoex, 0, sizeof(JOYINFOEX));
    //        joyinfoex.dwSize = sizeof(JOYINFOEX);
    //        joyinfoex.dwFlags = JOY_RETURNALL;
    //        if (joyGetPosEx(0, &joyinfoex) != JOYERR_NOERROR)
    //        {
    //            return g_ControllerData;
    //        }
    //        for (joyButtonBit = joyinfoex.dwButtons, joyButtonIndex = 0; joyButtonIndex < 32;
    //             joyButtonIndex += 1, joyButtonBit >>= 1)
    //        {
    //            if ((joyButtonBit & 1) != 0)
    //            {
    //                g_ControllerData[joyButtonIndex] = 0x80;
    //            }
    //        }
    //        return g_ControllerData;
    //    }
    //    else
    //    {
    //        dires = g_Supervisor.controller->Poll();
    //        if (FAILED(dires))
    //        {
    //            diRetryCount = 0;
    //            utils::DebugPrint2("error : DIERR_INPUTLOST\n");
    //            dires = g_Supervisor.controller->Acquire();
    //            while (dires == DIERR_INPUTLOST)
    //            {
    //                dires = g_Supervisor.controller->Acquire();
    //                utils::DebugPrint2("error : DIERR_INPUTLOST %d\n", diRetryCount);
    //                diRetryCount++;
    //                if (diRetryCount >= 400)
    //                {
    //                    return g_ControllerData;
    //                }
    //            }
    //            return g_ControllerData;
    //        }
    //        /* dires = */ g_Supervisor.controller->GetDeviceState(sizeof(DIJOYSTATE2), &dijoystate2);
    //        // TODO: seems ZUN forgot "dires =" above
    //        if (FAILED(dires))
    //        {
    //            return g_ControllerData;
    //        }
    //        memcpy(&g_ControllerData, dijoystate2.rgbButtons, sizeof(dijoystate2.rgbButtons));
    return g_ControllerData;
    //    }
}

u16 Controller::GetInput(void)
{
    u16 buttons = 0;

    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP, SDL_SCANCODE_UP);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN, SDL_SCANCODE_DOWN);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_LEFT, SDL_SCANCODE_LEFT);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_RIGHT, SDL_SCANCODE_RIGHT);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP, SDL_SCANCODE_KP_8);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN, SDL_SCANCODE_KP_2);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_LEFT, SDL_SCANCODE_KP_4);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_RIGHT, SDL_SCANCODE_KP_6);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP_LEFT, SDL_SCANCODE_KP_7);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP_RIGHT, SDL_SCANCODE_KP_9);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN_LEFT, SDL_SCANCODE_KP_1);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN_RIGHT, SDL_SCANCODE_KP_3);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_HOME, SDL_SCANCODE_HOME);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SHOOT, SDL_SCANCODE_Z);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_BOMB, SDL_SCANCODE_X);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_FOCUS, SDL_SCANCODE_LSHIFT);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_FOCUS, SDL_SCANCODE_RSHIFT);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_MENU, SDL_SCANCODE_ESCAPE);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SKIP, SDL_SCANCODE_LCTRL);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SKIP, SDL_SCANCODE_RCTRL);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_Q, SDL_SCANCODE_Q);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_S, SDL_SCANCODE_S);
    buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_ENTER, SDL_SCANCODE_RETURN);

    return Controller::GetControllerInput(buttons);
}

void Controller::ResetKeyboard(void)
{
    keyboardState = (u8 *)SDL_GetKeyboardState(NULL);
}
}; // namespace th06
