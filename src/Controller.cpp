#include "Controller.hpp"
#include "Connection.hpp"

#include "GameErrorContext.hpp"
#include "GameManager.hpp"
#include "Rng.hpp"
#include "Supervisor.hpp"
#include "i18n.hpp"
#include "utils.hpp"
#include <map>
std::map<int, Bits<16>> g_ctrl_bits_self;
std::map<int, Bits<16>> g_ctrl_bits_rcved;
std::map<int, int> g_ctrl_rng_rcved;
std::map<int, int> g_ctrl_rng_self;
std::map<int, InGameCtrlType> g_ctrl_rcved;
std::map<int, InGameCtrlType> g_ctrl_self;

extern Host g_host;
extern Guest g_guest;
extern int g_delay;
extern bool g_is_host;
extern bool g_is_connected;
bool g_is_sync = true;
extern bool g_istry_to_reconnect;
extern bool g_is_single_mode;

extern bool g_resync_trigger;
extern int g_resync_stage_frame;

struct CurKeyStates
{
    bool K_F2; // life
    bool K_F3; // bomb
    bool K_F4; // power
    bool K_R;
    bool K_Q;

    bool K_F6; // insane mode

    bool K_N; // add delay
    bool K_M; // dec delay
} g_cur_ctrl_key_state;

namespace th06
{
DIFFABLE_STATIC(JOYCAPSA, g_JoystickCaps)
DIFFABLE_STATIC(u16, g_FocusButtonConflictState)

u16 Controller::GetJoystickCaps(void)
{
    JOYINFOEX pji;

    pji.dwSize = sizeof(JOYINFOEX);
    pji.dwFlags = JOY_RETURNALL;

    if (joyGetPosEx(0, &pji) != MMSYSERR_NOERROR)
    {
        g_GameErrorContext.Log(TH_ERR_NO_PAD_FOUND);
        return 1;
    }

    joyGetDevCapsA(0, &g_JoystickCaps, sizeof(g_JoystickCaps));
    return 0;
}

#define JOYSTICK_MIDPOINT(min, max) ((min + max) / 2)
#define JOYSTICK_BUTTON_PRESSED(button, x, y) (x > y ? button : 0)
#define JOYSTICK_BUTTON_PRESSED_INVERT(button, x, y) (x < y ? button : 0)
#define KEYBOARD_KEY_PRESSED(button, x) keyboardState[x] & 0x80 ? button : 0

u16 Controller::GetControllerInput(u16 buttons)
{
    // NOTE: Those names are like this to get perfect stack frame matching
    // TODO: Give meaningfull names that still match.
    JOYINFOEX aa;
    u32 ab;
    u32 ac;
    DIJOYSTATE2 a0;
    u32 a2;
    HRESULT aaa;

    if (g_Supervisor.controller == NULL)
    {
        memset(&aa, 0, sizeof(aa));
        aa.dwSize = sizeof(JOYINFOEX);
        aa.dwFlags = JOY_RETURNALL;

        if (joyGetPosEx(0, &aa) != MMSYSERR_NOERROR)
        {
            return buttons;
        }

        ac = SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.shootButton, TH_BUTTON_SHOOT,
                                           aa.dwButtons);

        if (g_ControllerMapping.shootButton != g_ControllerMapping.focusButton)
        {
            SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.focusButton, TH_BUTTON_FOCUS,
                                          aa.dwButtons);
        }
        else
        {
            if (ac != 0)
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
                                      aa.dwButtons);
        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.menuButton, TH_BUTTON_MENU,
                                      aa.dwButtons);
        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.upButton, TH_BUTTON_UP,
                                      aa.dwButtons);
        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.downButton, TH_BUTTON_DOWN,
                                      aa.dwButtons);
        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.leftButton, TH_BUTTON_LEFT,
                                      aa.dwButtons);
        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.rightButton, TH_BUTTON_RIGHT,
                                      aa.dwButtons);
        SetButtonFromControllerInputs(&buttons, g_Supervisor.cfg.controllerMapping.skipButton, TH_BUTTON_SKIP,
                                      aa.dwButtons);

        ab = ((g_JoystickCaps.wXmax - g_JoystickCaps.wXmin) / 2 / 2);

        buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_RIGHT, aa.dwXpos,
                                           JOYSTICK_MIDPOINT(g_JoystickCaps.wXmin, g_JoystickCaps.wXmax) + ab);
        buttons |= JOYSTICK_BUTTON_PRESSED(
            TH_BUTTON_LEFT, JOYSTICK_MIDPOINT(g_JoystickCaps.wXmin, g_JoystickCaps.wXmax) - ab, aa.dwXpos);

        ab = ((g_JoystickCaps.wYmax - g_JoystickCaps.wYmin) / 2 / 2);
        buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_DOWN, aa.dwYpos,
                                           JOYSTICK_MIDPOINT(g_JoystickCaps.wYmin, g_JoystickCaps.wYmax) + ab);
        buttons |= JOYSTICK_BUTTON_PRESSED(
            TH_BUTTON_UP, JOYSTICK_MIDPOINT(g_JoystickCaps.wYmin, g_JoystickCaps.wYmax) - ab, aa.dwYpos);

        return buttons;
    }
    else
    {
        // FIXME: Next if not matching.
        aaa = g_Supervisor.controller->Poll();
        if (FAILED(aaa))
        {
            i32 retryCount = 0;

            utils::DebugPrint2("error : DIERR_INPUTLOST\n");
            aaa = g_Supervisor.controller->Acquire();

            while (aaa == DIERR_INPUTLOST)
            {
                aaa = g_Supervisor.controller->Acquire();
                utils::DebugPrint2("error : DIERR_INPUTLOST %d\n", retryCount);

                retryCount++;

                if (retryCount >= 400)
                {
                    return buttons;
                }
            }

            return buttons;
        }
        else
        {
            memset(&a0, 0, sizeof(a0));

            aaa = g_Supervisor.controller->GetDeviceState(sizeof(a0), &a0);

            if (FAILED(aaa))
            {
                return buttons;
            }

            a2 = SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.shootButton,
                                                  TH_BUTTON_SHOOT, a0.rgbButtons);

            if (g_Supervisor.cfg.controllerMapping.shootButton != g_Supervisor.cfg.controllerMapping.focusButton)
            {
                SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.focusButton,
                                                 TH_BUTTON_FOCUS, a0.rgbButtons);
            }
            else
            {
                if (a2 != 0)
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

            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.bombButton, TH_BUTTON_BOMB,
                                             a0.rgbButtons);
            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.menuButton, TH_BUTTON_MENU,
                                             a0.rgbButtons);
            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.upButton, TH_BUTTON_UP,
                                             a0.rgbButtons);
            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.downButton, TH_BUTTON_DOWN,
                                             a0.rgbButtons);
            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.leftButton, TH_BUTTON_LEFT,
                                             a0.rgbButtons);
            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.rightButton, TH_BUTTON_RIGHT,
                                             a0.rgbButtons);
            SetButtonFromDirectInputJoystate(&buttons, g_Supervisor.cfg.controllerMapping.skipButton, TH_BUTTON_SKIP,
                                             a0.rgbButtons);

            buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_RIGHT, a0.lX, g_Supervisor.cfg.padXAxis);
            buttons |= JOYSTICK_BUTTON_PRESSED_INVERT(TH_BUTTON_LEFT, a0.lX, -g_Supervisor.cfg.padXAxis);
            buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_DOWN, a0.lY, g_Supervisor.cfg.padYAxis);
            buttons |= JOYSTICK_BUTTON_PRESSED_INVERT(TH_BUTTON_UP, a0.lY, -g_Supervisor.cfg.padYAxis);
        }
    }

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
                                              enum TouhouButton touhouButton, u32 inputButtons)
{
    DWORD mask;

    if (controllerButtonToTest < 0)
    {
        return 0;
    }

    mask = 1 << controllerButtonToTest;

    *outButtons |= (inputButtons & mask ? touhouButton & 0xFFFF : 0);

    return inputButtons & mask ? touhouButton & 0xFFFF : 0;
}

DIFFABLE_STATIC_ARRAY(u8, (32 * 4), g_ControllerData)

#pragma var_order(joyinfoex, joyButtonBit, joyButtonIndex, dires, dijoystate2, diRetryCount)
// This is for rebinding keys
u8 *th06::Controller::GetControllerState()
{
    JOYINFOEX joyinfoex;
    u32 joyButtonBit;
    u32 joyButtonIndex;

    i32 dires;
    DIJOYSTATE2 dijoystate2;
    i32 diRetryCount;

    memset(&g_ControllerData, 0, sizeof(g_ControllerData));
    if (g_Supervisor.controller == NULL)
    {
        memset(&joyinfoex, 0, sizeof(JOYINFOEX));
        joyinfoex.dwSize = sizeof(JOYINFOEX);
        joyinfoex.dwFlags = JOY_RETURNALL;
        if (joyGetPosEx(0, &joyinfoex) != JOYERR_NOERROR)
        {
            return g_ControllerData;
        }
        for (joyButtonBit = joyinfoex.dwButtons, joyButtonIndex = 0; joyButtonIndex < 32;
             joyButtonIndex += 1, joyButtonBit >>= 1)
        {
            if ((joyButtonBit & 1) != 0)
            {
                g_ControllerData[joyButtonIndex] = 0x80;
            }
        }
        return g_ControllerData;
    }
    else
    {
        dires = g_Supervisor.controller->Poll();
        if (FAILED(dires))
        {
            diRetryCount = 0;
            utils::DebugPrint2("error : DIERR_INPUTLOST\n");
            dires = g_Supervisor.controller->Acquire();
            while (dires == DIERR_INPUTLOST)
            {
                dires = g_Supervisor.controller->Acquire();
                utils::DebugPrint2("error : DIERR_INPUTLOST %d\n", diRetryCount);
                diRetryCount++;
                if (diRetryCount >= 400)
                {
                    return g_ControllerData;
                }
            }
            return g_ControllerData;
        }
        /* dires = */ g_Supervisor.controller->GetDeviceState(sizeof(DIJOYSTATE2), &dijoystate2);
        // TODO: seems ZUN forgot "dires =" above
        if (FAILED(dires))
        {
            return g_ControllerData;
        }
        memcpy(&g_ControllerData, dijoystate2.rgbButtons, sizeof(dijoystate2.rgbButtons));
        return g_ControllerData;
    }
}

u16 Controller::GetInput(void)
{
    u8 keyboardState[256];
    memset(keyboardState, 0, sizeof(keyboardState));
    memset(&g_cur_ctrl_key_state, 0, sizeof(g_cur_ctrl_key_state));
    u16 buttons;

    buttons = 0;

    if (g_Supervisor.keyboard == NULL)
    {
        GetKeyboardState(keyboardState);

        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP, VK_UP);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN, VK_DOWN);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_LEFT, VK_LEFT);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_RIGHT, VK_RIGHT);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP, VK_NUMPAD8);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN, VK_NUMPAD2);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_LEFT, VK_NUMPAD4);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_RIGHT, VK_NUMPAD6);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP_LEFT, VK_NUMPAD7);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP_RIGHT, VK_NUMPAD9);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN_LEFT, VK_NUMPAD1);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN_RIGHT, VK_NUMPAD3);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SHOOT, 'Z');
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_BOMB, 'X');
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_FOCUS, VK_SHIFT);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_MENU, VK_ESCAPE);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SKIP, VK_CONTROL);

        // Player 2
        if (g_is_single_mode)
        {
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP2, 'I');
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN2, 'K');
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_LEFT2, 'J');
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_RIGHT2, 'L');
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SHOOT2, 'F');
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_BOMB2, 'G');
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_FOCUS2, 'D');
        }
        g_cur_ctrl_key_state.K_F2 = (keyboardState[VK_F2] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_F3 = (keyboardState[VK_F3] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_F4 = (keyboardState[VK_F4] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_F6 = (keyboardState[VK_F6] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_N = (keyboardState['N'] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_M = (keyboardState['M'] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_Q = (keyboardState['Q'] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_R = (keyboardState['R'] & 0x80) ? true : false;
    }
    else
    {
        HRESULT res = g_Supervisor.keyboard->GetDeviceState(sizeof(keyboardState), keyboardState);
        if (res != 0)
        {
            g_Supervisor.keyboard->Acquire();
            res = g_Supervisor.keyboard->GetDeviceState(sizeof(keyboardState), keyboardState);
            if (res != 0)
                return Controller::GetControllerInput(buttons);
        }
        buttons = 0;
        if (res == DIERR_INPUTLOST)
        {
            g_Supervisor.keyboard->Acquire();

            return Controller::GetControllerInput(buttons);
        }

        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP, DIK_UP);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN, DIK_DOWN);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_LEFT, DIK_LEFT);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_RIGHT, DIK_RIGHT);

        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP, DIK_NUMPAD8);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN, DIK_NUMPAD2);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_LEFT, DIK_NUMPAD4);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_RIGHT, DIK_NUMPAD6);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP_LEFT, DIK_NUMPAD7);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP_RIGHT, DIK_NUMPAD9);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN_LEFT, DIK_NUMPAD1);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN_RIGHT, DIK_NUMPAD3);

        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SHOOT, DIK_Z);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_BOMB, DIK_X);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_FOCUS, DIK_LSHIFT);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_FOCUS, DIK_RSHIFT);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_MENU, DIK_ESCAPE);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SKIP, DIK_LCONTROL);
        // buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SKIP, DIK_RCONTROL);

        if (g_is_single_mode)
        {
            // Player 2
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP2, DIK_I);
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN2, DIK_K);
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_LEFT2, DIK_J);
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_RIGHT2, DIK_L);
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SHOOT2, DIK_F);
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_BOMB2, DIK_G);
            buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_FOCUS2, DIK_D);
        }

        g_cur_ctrl_key_state.K_F2 = (keyboardState[DIK_F2] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_F3 = (keyboardState[DIK_F3] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_F4 = (keyboardState[DIK_F4] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_F6 = (keyboardState[DIK_F6] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_N = (keyboardState[DIK_N] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_M = (keyboardState[DIK_M] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_Q = (keyboardState[DIK_Q] & 0x80) ? true : false;
        g_cur_ctrl_key_state.K_R = (keyboardState[DIK_R] & 0x80) ? true : false;
    }
    return Controller::GetControllerInput(buttons);
}

void Controller::ResetKeyboard(void)
{
    u8 key_states[256];

    GetKeyboardState(key_states);
    for (i32 idx = 0; idx < 256; idx++)
    {
        *(key_states + idx) &= 0x7f;
    }
    SetKeyboardState(key_states);
}

bool Controller::RcvPacks()
{
    bool hasdata_all = false;
    bool hasdata;
    do
    {
        Pack pack;
        if (g_is_host)
        {
            g_host.PollReceive(pack, hasdata);
        }
        else
        {
            g_guest.PollReceive(pack, hasdata);
        }
        hasdata_all |= hasdata;
        if (!hasdata)
            return hasdata_all;
        if (pack.ctrl.ctrl_type == Ctrl_Key)
        {
            int frame = pack.ctrl.frame;
            for (int i = 0; i < KeyPackFrameNum; i++)
            {
                g_ctrl_bits_rcved[frame - i] = pack.ctrl.keys[i];
                g_ctrl_rng_rcved[frame - i] = pack.ctrl.rng_seed[i];
                g_ctrl_rcved[frame - i] = pack.ctrl.igc_type[i];
            }
        }
        else if (pack.ctrl.ctrl_type == Ctrl_Try_Resync)
        {
            if ((pack.ctrl.resync_setting.frame_to_re_sync > g_Supervisor.calcCount) &&
                (pack.ctrl.resync_setting.frame_to_re_sync <= g_Supervisor.calcCount + g_delay * 2 + 2))
            {
                g_resync_trigger = true;
                g_resync_stage_frame = pack.ctrl.resync_setting.frame_to_re_sync;
            }
        }
    } while (hasdata);
    return hasdata_all;
}

void Controller::SendKeys(int frame)
{
    Pack pack;
    pack.echoTick = 0;
    pack.sendTick = 0;
    pack.seq = 0;
    pack.type = 4;

    pack.ctrl.ctrl_type = Ctrl_Key;
    pack.ctrl.frame = frame;
    for (int i = 0; i < KeyPackFrameNum; i++)
    {
        std::map<int, Bits<16>>::iterator find_res = g_ctrl_bits_self.find(frame - i);
        if (find_res == g_ctrl_bits_self.end())
            ReadFromInt(pack.ctrl.keys[i], 0);
        else
            pack.ctrl.keys[i] = find_res->second;

        std::map<int, int>::iterator find_res2 = g_ctrl_rng_self.find(frame - i);
        if (find_res2 == g_ctrl_rng_self.end())
            pack.ctrl.rng_seed[i] = 0;
        else
            pack.ctrl.rng_seed[i] = find_res2->second;

        std::map<int, InGameCtrlType>::iterator find_res3 = g_ctrl_self.find(frame - i);
        if (find_res3 == g_ctrl_self.end())
            pack.ctrl.igc_type[i] = IGC_NONE;
        else
            pack.ctrl.igc_type[i] = find_res3->second;
    }
    if (g_is_host)
    {
        g_host.SendPack(pack);
    }
    else
    {
        g_guest.SendPack(pack);
    }
}

void HandleControlKeys(int frame)
{
    InGameCtrlType igctrl = IGC_NONE;
    if (g_cur_ctrl_key_state.K_F2)
        igctrl = Inf_Life;
    else if (g_cur_ctrl_key_state.K_F3)
        igctrl = Inf_Bomb;
    else if (g_cur_ctrl_key_state.K_F4)
        igctrl = Inf_Power;
    else if (g_cur_ctrl_key_state.K_Q)
        igctrl = Quick_Quit;
    else if (g_cur_ctrl_key_state.K_R)
        igctrl = Quick_Restart;
    else if (g_cur_ctrl_key_state.K_M)
        igctrl = Add_Delay;
    else if (g_cur_ctrl_key_state.K_N)
        igctrl = Dec_Delay;
    else if (g_cur_ctrl_key_state.K_F6)
        igctrl = Insane_Mode;
    g_ctrl_self[frame] = igctrl;
}

#define TH_ISDOWN(a, mask, b) ((a) & (mask) ? (b) : 0)

u16 GetKeys(int frame, bool is_in_UI, int &out_ctrl)
{
    InGameCtrlType self_ctrl = IGC_NONE;
    InGameCtrlType rcv_ctrl = IGC_NONE;

    out_ctrl = IGC_NONE;
    if (frame - g_delay < 0)
        return 0;

    u16 self_key = 0;
    std::map<int, Bits<16>>::iterator res = g_ctrl_bits_self.find(frame - g_delay);
    if (res != g_ctrl_bits_self.end())
        WriteToInt(res->second, self_key);

    std::map<int, InGameCtrlType>::iterator res2 = g_ctrl_self.find(frame - g_delay);
    if (res2 != g_ctrl_self.end())
        self_ctrl = res2->second;

    u16 rcv_key = 0;

    bool has_rcv_data = false;
    static bool inited = false;
    static LARGE_INTEGER freq;
    LARGE_INTEGER cur;
    LARGE_INTEGER ping_key_time;
    LARGE_INTEGER max_wait_to_time;
    if (!inited)
    {
        inited = true;
        QueryPerformanceFrequency(&freq);
    }
    QueryPerformanceCounter(&cur);
    max_wait_to_time.QuadPart = cur.QuadPart + freq.QuadPart * 5.0; // 5.0s
    ping_key_time.QuadPart = cur.QuadPart + freq.QuadPart * 0.1;    // 0.1s
    do
    {
        res = g_ctrl_bits_rcved.find(frame - g_delay);
        if (res != g_ctrl_bits_rcved.end())
        {
            WriteToInt(res->second, rcv_key);
            g_is_sync = (g_ctrl_rng_rcved[frame - g_delay] == g_ctrl_rng_self[frame - g_delay]);
            rcv_ctrl = g_ctrl_rcved[frame - g_delay];
            has_rcv_data = true;
            break;
        }
        else
        {
            int n_transfer = 1;
            while (cur.QuadPart < max_wait_to_time.QuadPart)
            {
                if (Controller::RcvPacks())
                {
                    Sleep(1);
                    break;
                }
                Sleep(1);
                QueryPerformanceCounter(&cur);
                // send key to another player to avoid lock
                if (cur.QuadPart > ping_key_time.QuadPart)
                {
                    ping_key_time.QuadPart = cur.QuadPart + freq.QuadPart * 0.1; // 0.1s
                    Controller::SendKeys(frame);
                }
            }
        }
    } while (cur.QuadPart < max_wait_to_time.QuadPart);
    if (!has_rcv_data)
    {
        rcv_key = 0;
        self_key = 0;
        self_ctrl = IGC_NONE;
        rcv_ctrl = IGC_NONE;
        g_is_connected = false;
        g_istry_to_reconnect = false;
    }

    if (self_ctrl != IGC_NONE && rcv_ctrl != IGC_NONE)
    {
        out_ctrl = g_is_host ? self_ctrl : rcv_ctrl;
    }
    else
    {
        out_ctrl = (self_ctrl == IGC_NONE) ? rcv_ctrl : self_ctrl;
    }

    if (is_in_UI)
        return self_key | rcv_key;
    u16 finres = 0;
    if (g_is_host)
    {
        finres = self_key;
        finres |= TH_ISDOWN(rcv_key, TH_BUTTON_LEFT, TH_BUTTON_LEFT2);
        finres |= TH_ISDOWN(rcv_key, TH_BUTTON_RIGHT, TH_BUTTON_RIGHT2);
        finres |= TH_ISDOWN(rcv_key, TH_BUTTON_UP, TH_BUTTON_UP2);
        finres |= TH_ISDOWN(rcv_key, TH_BUTTON_DOWN, TH_BUTTON_DOWN2);
        finres |= TH_ISDOWN(rcv_key, TH_BUTTON_SHOOT, TH_BUTTON_SHOOT2);
        finres |= TH_ISDOWN(rcv_key, TH_BUTTON_BOMB, TH_BUTTON_BOMB2);
        finres |= TH_ISDOWN(rcv_key, TH_BUTTON_FOCUS, TH_BUTTON_FOCUS2);

        finres |= TH_ISDOWN(rcv_key, TH_BUTTON_MENU, TH_BUTTON_MENU);
        finres |= TH_ISDOWN(rcv_key, TH_BUTTON_SKIP, TH_BUTTON_SKIP);
    }
    else
    {
        finres = rcv_key;
        finres |= TH_ISDOWN(self_key, TH_BUTTON_LEFT, TH_BUTTON_LEFT2);
        finres |= TH_ISDOWN(self_key, TH_BUTTON_RIGHT, TH_BUTTON_RIGHT2);
        finres |= TH_ISDOWN(self_key, TH_BUTTON_UP, TH_BUTTON_UP2);
        finres |= TH_ISDOWN(self_key, TH_BUTTON_DOWN, TH_BUTTON_DOWN2);
        finres |= TH_ISDOWN(self_key, TH_BUTTON_SHOOT, TH_BUTTON_SHOOT2);
        finres |= TH_ISDOWN(self_key, TH_BUTTON_BOMB, TH_BUTTON_BOMB2);
        finres |= TH_ISDOWN(self_key, TH_BUTTON_FOCUS, TH_BUTTON_FOCUS2);

        finres |= TH_ISDOWN(self_key, TH_BUTTON_MENU, TH_BUTTON_MENU);
        finres |= TH_ISDOWN(self_key, TH_BUTTON_SKIP, TH_BUTTON_SKIP);
    }
    return finres;
}

u16 Controller::GetInput_Single(int &cur_ctrl)
{
    u16 input = GetInput();
    HandleControlKeys(0);
    cur_ctrl = g_ctrl_self[0];
    return input;
}

u16 Controller::GetInput_Net(int frame, bool is_in_UI, int &cur_ctrl)
{
    if (!g_is_connected)
    {
        u16 input = GetInput();
        HandleControlKeys(frame);
        cur_ctrl = g_ctrl_self[frame];
        return input;
    }
    u16 btn = GetInput();
    Bits<16> cur_btn_bits;
    ReadFromInt(cur_btn_bits, btn);
    g_ctrl_bits_self[frame] = cur_btn_bits;
    g_ctrl_rng_self[frame] = g_Rng.seed;

    // remove frames
    int frame_rem = 80;
    std::map<int, Bits<16>>::iterator last_res = g_ctrl_bits_self.find(frame - frame_rem);
    if (last_res != g_ctrl_bits_self.end())
        g_ctrl_bits_self.erase(last_res);

    last_res = g_ctrl_bits_rcved.find(frame - frame_rem);
    if (last_res != g_ctrl_bits_rcved.end())
        g_ctrl_bits_rcved.erase(last_res);

    std::map<int, int>::iterator last_res_seed = g_ctrl_rng_rcved.find(frame - frame_rem);
    if (last_res_seed != g_ctrl_rng_rcved.end())
        g_ctrl_rng_rcved.erase(last_res_seed);
    last_res_seed = g_ctrl_rng_self.find(frame - frame_rem);
    if (last_res_seed != g_ctrl_rng_self.end())
        g_ctrl_rng_self.erase(last_res_seed);

    std::map<int, InGameCtrlType>::iterator last_res_ctrl = g_ctrl_rcved.find(frame - frame_rem);
    if (last_res_ctrl != g_ctrl_rcved.end())
        g_ctrl_rcved.erase(last_res_ctrl);
    last_res_ctrl = g_ctrl_self.find(frame - frame_rem);
    if (last_res_ctrl != g_ctrl_self.end())
        g_ctrl_self.erase(last_res_ctrl);

    HandleControlKeys(frame);
    Controller::SendKeys(frame);
    RcvPacks();

    u16 res = GetKeys(frame, is_in_UI, cur_ctrl);
    return res;
}

}; // namespace th06
