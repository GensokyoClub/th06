#include "stdafx.h"

#include "Global.hpp"
#include <ddraw.h>

namespace th06
{
namespace Controller
{
static u32 SetButtonFromControllerInputs(u16 *outButtons, ControllerButton controllerButtonToTest,
                                         TouhouButton touhouButton, u32 inputButtons);
static u32 SetButtonFromDirectInputJoystate(u16 *outButtons, ControllerButton controllerButtonToTest,
                                            TouhouButton touhouButton, u8 *inputButtons);
} // namespace Controller

ControllerMapping g_ControllerMapping = {
#if BUILT_SINCE(102g)
    0,  1,  2,  4, -1,
    -1, -1, -1, 3
#elif BUILT_SINCE(102)
    0,  1,  0,  -1, -1,
    -1, -1, -1, -1
#else
    1, 2, 1, 0, 0,
    0, 0, 0
#endif
};

Chain g_FuncChain;

// Controller
#if BUILT_SINCE(102g) || BUILT_BEFORE(101)
JOYCAPS g_JoystickDevCaps;
#endif
u16 g_FocusButtonConflictState;
u16 g_PreviousFrameKeys;
u16 g_IsEigthFrameInput;
u16 g_CountFramesInputsWereHeld;
u16 g_CurFrameInput;

// CMyFont
CMyFont g_CMyFonts;

Chain::~Chain()
{
}

ChainElem::ChainElem()
{
    prev = NULL;
    next = NULL;
    callback = NULL;
    unkPtr = this;
    addedCallback = NULL;
    deletedCallback = NULL;
    priority = 0;

    isHeapAllocated = false;
}

ChainElem::~ChainElem()
{
    if (deletedCallback != NULL)
    {
        this->deletedCallback(this->arg);
    }

    prev = NULL;
    next = NULL;
    callback = NULL;
    addedCallback = NULL;
    deletedCallback = NULL;
}

Chain::Chain()
{
    midiOutputDeviceCount = midiOutGetNumDevs();
    unk = 0;
}

ZunResult Chain::AddToCalcChain(ChainElem *elem, int priority)
{
    ChainElem *cur = &this->calcChain;
#if BUILT_SINCE(102g)
    DebugPrint("add calc chain (pri = %d)\n", priority);
#endif
    elem->priority = priority;

    while (cur->next != NULL)
    {
        if (cur->priority > priority)
        {
            break;
        }

        cur = cur->next;
    }

    if (cur->priority > priority)
    {
        elem->next = cur;
        elem->prev = cur->prev;

        if (elem->prev != NULL)
        {
            elem->prev->next = elem;
        }

        cur->prev = elem;
    }
    else
    {
        elem->next = NULL;
        elem->prev = cur;
        cur->next = elem;
    }

    if (elem->addedCallback != NULL)
    {
        ZunResult res = elem->addedCallback(elem->arg);
        elem->addedCallback = NULL;

        return res;
    }
    else
    {
        return ZUN_SUCCESS;
    }
}

ZunResult Chain::AddToDrawChain(ChainElem *elem, int priority)
{
    ChainElem *cur = &this->drawChain;
#if BUILT_SINCE(102g)
    DebugPrint("add draw chain (pri = %d)\n", priority);
#endif
    elem->priority = priority;

    while (cur->next != NULL)
    {
        if (cur->priority > priority)
        {
            break;
        }

        cur = cur->next;
    }

    if (cur->priority > priority)
    {
        elem->next = cur;
        elem->prev = cur->prev;

        if (elem->prev != NULL)
        {
            elem->prev->next = elem;
        }

        cur->prev = elem;
    }
    else
    {
        elem->next = NULL;
        elem->prev = cur;
        cur->next = elem;
    }

    if (elem->addedCallback != NULL)
    {
        return elem->addedCallback(elem->arg);
    }
    else
    {
        return ZUN_SUCCESS;
    }
}

int Chain::RunCalcChain(void)
{
    ChainElem *tmp1;
    ChainElem *current;
    int updatedCount;

restart_from_first_job:
    updatedCount = 0;
    current = &this->calcChain;

    while (current != NULL)
    {
        if (current->callback != NULL)
        {
        execute_again:
            switch (current->callback(current->arg))
            {
            case CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB:
                tmp1 = current;
                current = current->next;
                Cut(tmp1);

                updatedCount++;
                continue;

            case CHAIN_CALLBACK_RESULT_EXECUTE_AGAIN:
                goto execute_again;

            case CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS:
                return 0;

            case CHAIN_CALLBACK_RESULT_BREAK:
                return 1;

            case CHAIN_CALLBACK_RESULT_EXIT_GAME_ERROR:
                return -1;

            case CHAIN_CALLBACK_RESULT_RESTART_FROM_FIRST_JOB:
                goto restart_from_first_job;

            default:
                break;
            }

            updatedCount++;
        }

        current = current->next;
    }

    return updatedCount;
}

int Chain::RunDrawChain(void)
{
    ChainElem *tmp1;

    int updatedCount = 0;
    ChainElem *current = &this->drawChain;

    while (current != NULL)
    {
        if (current->callback != NULL)
        {
        execute_again:
            switch (current->callback(current->arg))
            {
            case CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB:
                tmp1 = current;
                current = current->next;
                Cut(tmp1);

                updatedCount++;
                continue;

            case CHAIN_CALLBACK_RESULT_EXECUTE_AGAIN:
                goto execute_again;

            case CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS:
                return 0;

            case CHAIN_CALLBACK_RESULT_BREAK:
                return 1;

            case CHAIN_CALLBACK_RESULT_EXIT_GAME_ERROR:
                return -1;

            default:
                break;
            }

            updatedCount++;
        }

        current = current->next;
    }

    return updatedCount;
}

void Chain::ReleaseSingleChain(ChainElem *root)
{
    // NOTE: Those names are like this to get perfect stack frame matching
    // TODO: Give meaningfull names that still match.
    ChainElem a0;
    ChainElem *current;
    ChainElem *tmp;
    ChainElem *wasNext;

    tmp = ZUN_NEW(ChainElem);
    a0.next = tmp;

    current = root;
    while (current != NULL)
    {
        tmp->unkPtr = current;
        tmp->next = ZUN_NEW(ChainElem);
        tmp = tmp->next;
        current = current->next;
    }

    current = &a0;
    while (current != NULL)
    {
        Cut(current->unkPtr);
        current = current->next;
    }

    tmp = a0.next;

    while (tmp != NULL)
    {
        wasNext = tmp->next;

        ZUN_DELETE(tmp);

        tmp = wasNext;
    }
}

void Chain::Release(void)
{
    ReleaseSingleChain(&this->calcChain);
    ReleaseSingleChain(&this->drawChain);
}

ChainElem *Chain::CreateElem(ChainCallback callback)
{
    ChainElem *elem = ZUN_NEW(ChainElem);

    elem->SetCallback(callback);
    elem->isHeapAllocated = true;

    return elem;
}

void Chain::Cut(ChainElem *to_remove)
{
#if BUILT_SINCE(013a) || BUILD_VERSION == BUILD_VERSION_013
    ZunBool isDrawChain = false;
#endif

    if (to_remove == NULL)
    {
        return;
    }

    ChainElem *tmp = &this->calcChain;

    while (tmp != NULL)
    {
        if (tmp == to_remove)
        {
            goto destroy_elem;
        }

        tmp = tmp->next;
    }

    {
#if BUILT_SINCE(013a) || BUILD_VERSION == BUILD_VERSION_013
        isDrawChain = true;
#endif

        tmp = &this->drawChain;
        while (tmp != NULL)
        {
            if (tmp == to_remove)
            {
                goto destroy_elem;
            }

            tmp = tmp->next;
        }
    }

    return;

destroy_elem:
#if BUILT_SINCE(013a) || BUILD_VERSION == BUILD_VERSION_013
    if (!isDrawChain)
    {
        DebugPrint("calc cut Chain (Pri = %d)\n", to_remove->priority);
    }
    else
    {
        DebugPrint("draw cut Chain (Pri = %d)\n", to_remove->priority);
    }
#endif

    if (to_remove->prev != NULL)
    {
        to_remove->callback = NULL;
        to_remove->prev->next = to_remove->next;

        if (to_remove->next != NULL)
        {
            to_remove->next->prev = to_remove->prev;
        }

        to_remove->prev = NULL;
        to_remove->next = NULL;

        if (to_remove->isHeapAllocated)
        {
            delete to_remove;
            to_remove = NULL;
        }
        else
        {
            if (to_remove->deletedCallback != NULL)
            {
                ChainDeletedCallback callback = to_remove->deletedCallback;
                to_remove->deletedCallback = NULL;
                callback(to_remove->arg);
            }
        }
    }
}

// ZunMemory
ZunMemory g_ZunMemoryMgr;

// FileSystem
long g_ReadFileSize;

u16 Controller::GetJoystickCaps(void)
{
    // NOTE: This isn't stubbed in trial 0.08.
    // Current trial build is based on version 0.13.
    // Why was winmm joystick API removed for that????
    // 1.00 and 1.02g+ probe the joystick; 1.01 to 1.02f return early.
#if BUILT_SINCE(102g) || BUILT_BEFORE(101)
    JOYINFOEX pji;

    pji.dwSize = sizeof(JOYINFOEX);
    pji.dwFlags = JOY_RETURNALL;

    if (joyGetPosEx(JOYSTICKID1, &pji) != JOYERR_NOERROR)
    {
        g_GameErrorContext.Log(TH_ERR_NO_PAD_FOUND);
        return 1;
    }

    joyGetDevCaps(JOYSTICKID1, &g_JoystickDevCaps, sizeof(g_JoystickDevCaps));
#endif
    return 0;
}

#define JOYSTICK_MIDPOINT(min, max) ((min + max) / 2)
#define JOYSTICK_BUTTON_PRESSED(button, x, y) (x > y ? button : 0)
#define JOYSTICK_BUTTON_PRESSED_INVERT(button, x, y) (x < y ? button : 0)
#define KEYBOARD_KEY_PRESSED(button, x) keyboardState[x] & 0x80 ? button : 0
#define JOYSTICK_MASK_PRESSED(button, mask, held) ((held) & (mask) ? (button) : 0)

// The trial reads the shared mapping; release reads the loaded configuration.
#if BUILT_SINCE(102g)
#define CONTROLLER_MAPPING g_Supervisor.cfg.controllerMapping
#else
#define CONTROLLER_MAPPING g_ControllerMapping
#endif

u16 Controller::GetControllerInput(u16 buttons)
{
    // NOTE: Those names are like this to get perfect stack frame matching
    // TODO: Give meaningfull names that still match.
#if BUILT_SINCE(102g) || BUILT_BEFORE(102)
    JOYINFOEX aa;
#endif
#if BUILT_SINCE(102g) || BUILT_BEFORE(101)
    u32 ab;
#endif
#if BUILT_SINCE(102g)
    u32 ac;
#endif
    DIJOYSTATE2 a0;
    u32 a2;
    HRESULT aaa;

#if BUILT_BEFORE(102)
    memset(&aa, 0, sizeof(aa));
#endif
    if (g_Supervisor.controller == NULL)
    {
#if BUILT_SINCE(102g)
        memset(&aa, 0, sizeof(aa));
        aa.dwSize = sizeof(JOYINFOEX);
        aa.dwFlags = JOY_RETURNALL;

        if (joyGetPosEx(JOYSTICKID1, &aa) != JOYERR_NOERROR)
        {
            return buttons;
        }

        ac = SetButtonFromControllerInputs(&buttons, g_ControllerMapping.shootButton, TH_BUTTON_SHOOT, aa.dwButtons);

        if (g_ControllerMapping.shootButton != g_ControllerMapping.focusButton)
        {
            SetButtonFromControllerInputs(&buttons, g_ControllerMapping.focusButton, TH_BUTTON_FOCUS, aa.dwButtons);
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

        SetButtonFromControllerInputs(&buttons, CONTROLLER_MAPPING.bombButton, TH_BUTTON_BOMB, aa.dwButtons);
        SetButtonFromControllerInputs(&buttons, CONTROLLER_MAPPING.menuButton, TH_BUTTON_MENU, aa.dwButtons);
        SetButtonFromControllerInputs(&buttons, CONTROLLER_MAPPING.upButton, TH_BUTTON_UP, aa.dwButtons);
        SetButtonFromControllerInputs(&buttons, CONTROLLER_MAPPING.downButton, TH_BUTTON_DOWN, aa.dwButtons);
        SetButtonFromControllerInputs(&buttons, CONTROLLER_MAPPING.leftButton, TH_BUTTON_LEFT, aa.dwButtons);
        SetButtonFromControllerInputs(&buttons, CONTROLLER_MAPPING.rightButton, TH_BUTTON_RIGHT, aa.dwButtons);
        SetButtonFromControllerInputs(&buttons, CONTROLLER_MAPPING.skipButton, TH_BUTTON_SKIP, aa.dwButtons);

        ab = ((g_JoystickDevCaps.wXmax - g_JoystickDevCaps.wXmin) / 2 / 2);

        buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_RIGHT, aa.dwXpos,
                                           JOYSTICK_MIDPOINT(g_JoystickDevCaps.wXmin, g_JoystickDevCaps.wXmax) + ab);
        buttons |= JOYSTICK_BUTTON_PRESSED(
            TH_BUTTON_LEFT, JOYSTICK_MIDPOINT(g_JoystickDevCaps.wXmin, g_JoystickDevCaps.wXmax) - ab, aa.dwXpos);

        ab = ((g_JoystickDevCaps.wYmax - g_JoystickDevCaps.wYmin) / 2 / 2);
        buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_DOWN, aa.dwYpos,
                                           JOYSTICK_MIDPOINT(g_JoystickDevCaps.wYmin, g_JoystickDevCaps.wYmax) + ab);
        buttons |= JOYSTICK_BUTTON_PRESSED(
            TH_BUTTON_UP, JOYSTICK_MIDPOINT(g_JoystickDevCaps.wYmin, g_JoystickDevCaps.wYmax) - ab, aa.dwYpos);

#elif BUILT_BEFORE(101)
        // 1.00's mapping holds joystick button masks; the axes are checked after both paths.
        aa.dwSize = sizeof(JOYINFOEX);
        aa.dwFlags = JOY_RETURNALL;

        if (joyGetPosEx(JOYSTICKID1, &aa) != JOYERR_NOERROR)
        {
            return buttons;
        }

        buttons |= JOYSTICK_MASK_PRESSED(TH_BUTTON_SHOOT, g_ControllerMapping.shootButton, aa.dwButtons);
        buttons |= JOYSTICK_MASK_PRESSED(TH_BUTTON_BOMB, g_ControllerMapping.bombButton, aa.dwButtons);
        if (g_ControllerMapping.shootButton != g_ControllerMapping.focusButton)
        {
            buttons |= JOYSTICK_MASK_PRESSED(TH_BUTTON_FOCUS, g_ControllerMapping.focusButton, aa.dwButtons);
        }
        else if (aa.dwButtons & g_ControllerMapping.focusButton)
        {
            g_FocusButtonConflictState++;
            if (g_FocusButtonConflictState >= 8)
            {
                buttons |= TH_BUTTON_FOCUS;
            }
        }
        else
        {
            g_FocusButtonConflictState = 0;
        }
        buttons |= JOYSTICK_MASK_PRESSED(TH_BUTTON_MENU, g_ControllerMapping.menuButton, aa.dwButtons);
        buttons |= JOYSTICK_MASK_PRESSED(TH_BUTTON_UP, g_ControllerMapping.upButton, aa.dwButtons);
        buttons |= JOYSTICK_MASK_PRESSED(TH_BUTTON_DOWN, g_ControllerMapping.downButton, aa.dwButtons);
        buttons |= JOYSTICK_MASK_PRESSED(TH_BUTTON_LEFT, g_ControllerMapping.leftButton, aa.dwButtons);
        buttons |= JOYSTICK_MASK_PRESSED(TH_BUTTON_RIGHT, g_ControllerMapping.rightButton, aa.dwButtons);
#endif
#if BUILT_SINCE(101)
        return buttons;
#endif
    }
    else
    {
        aaa = g_Supervisor.controller->Poll();
        if (FAILED(aaa))
        {
            i32 retryCount = 0;

            DebugPrint("error : DIERR_INPUTLOST\n");
#if BUILT_SINCE(102)
            aaa = g_Supervisor.controller->Acquire();

            while (aaa == DIERR_INPUTLOST)
            {
                aaa = g_Supervisor.controller->Acquire();
#else
            while (g_Supervisor.controller->Acquire() == DIERR_INPUTLOST)
            {
#endif
                DebugPrint("error : DIERR_INPUTLOST %d\n", retryCount);

                retryCount++;

                if (retryCount >= 400)
                {
                    return buttons;
                }
            }

#if BUILT_SINCE(102f)
            return buttons;
#endif
        }
        // Until 1.02d a successful re-acquire falls through and reads the device state in the same frame, and the
        // state isn't cleared first.
#if BUILT_SINCE(102f)
        else
#endif
        {
#if BUILT_SINCE(102f)
            memset(&a0, 0, sizeof(a0));
#endif

#if BUILT_SINCE(102c)
            aaa = g_Supervisor.controller->GetDeviceState(sizeof(a0), &a0);
#else
            // Before 1.02c the result is dropped and the check below still tests Poll's.
            g_Supervisor.controller->GetDeviceState(sizeof(a0), &a0);
#endif

            if (FAILED(aaa))
            {
                return buttons;
            }

            a2 = SetButtonFromDirectInputJoystate(&buttons, CONTROLLER_MAPPING.shootButton, TH_BUTTON_SHOOT,
                                                  a0.rgbButtons);
#if BUILT_BEFORE(102g)
            SetButtonFromDirectInputJoystate(&buttons, CONTROLLER_MAPPING.bombButton, TH_BUTTON_BOMB, a0.rgbButtons);
#endif

            if (g_ControllerMapping.shootButton != g_ControllerMapping.focusButton)
            {
                SetButtonFromDirectInputJoystate(&buttons, CONTROLLER_MAPPING.focusButton, TH_BUTTON_FOCUS,
                                                 a0.rgbButtons);
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

#if BUILT_SINCE(102g)
            SetButtonFromDirectInputJoystate(&buttons, CONTROLLER_MAPPING.bombButton, TH_BUTTON_BOMB, a0.rgbButtons);
#endif
            SetButtonFromDirectInputJoystate(&buttons, CONTROLLER_MAPPING.menuButton, TH_BUTTON_MENU, a0.rgbButtons);
            SetButtonFromDirectInputJoystate(&buttons, CONTROLLER_MAPPING.upButton, TH_BUTTON_UP, a0.rgbButtons);
            SetButtonFromDirectInputJoystate(&buttons, CONTROLLER_MAPPING.downButton, TH_BUTTON_DOWN, a0.rgbButtons);
            SetButtonFromDirectInputJoystate(&buttons, CONTROLLER_MAPPING.leftButton, TH_BUTTON_LEFT, a0.rgbButtons);
            SetButtonFromDirectInputJoystate(&buttons, CONTROLLER_MAPPING.rightButton, TH_BUTTON_RIGHT, a0.rgbButtons);
#if BUILT_SINCE(102)
            SetButtonFromDirectInputJoystate(&buttons, CONTROLLER_MAPPING.skipButton, TH_BUTTON_SKIP, a0.rgbButtons);

            buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_RIGHT, a0.lX, g_Supervisor.cfg.padXAxis);
            buttons |= JOYSTICK_BUTTON_PRESSED_INVERT(TH_BUTTON_LEFT, a0.lX, -g_Supervisor.cfg.padXAxis);
            buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_DOWN, a0.lY, g_Supervisor.cfg.padYAxis);
            buttons |= JOYSTICK_BUTTON_PRESSED_INVERT(TH_BUTTON_UP, a0.lY, -g_Supervisor.cfg.padYAxis);
#elif BUILT_SINCE(101)
            buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_RIGHT, a0.lX, 900);
            buttons |= JOYSTICK_BUTTON_PRESSED_INVERT(TH_BUTTON_LEFT, a0.lX, -900);
            buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_DOWN, a0.lY, 900);
            buttons |= JOYSTICK_BUTTON_PRESSED_INVERT(TH_BUTTON_UP, a0.lY, -900);
#endif
#if BUILT_BEFORE(102)
            // 1.00 checks these after both paths; 1.01 keeps the copy without using it.
            aa.dwXpos = a0.lX;
            aa.dwYpos = a0.lY;
#endif
        }
    }

#if BUILT_BEFORE(101)
    ab = (g_JoystickDevCaps.wXmax - g_JoystickDevCaps.wXmin) / 2 / 2;
    buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_RIGHT, aa.dwXpos,
                                       JOYSTICK_MIDPOINT(g_JoystickDevCaps.wXmin, g_JoystickDevCaps.wXmax) + ab);
    buttons |= JOYSTICK_BUTTON_PRESSED(
        TH_BUTTON_LEFT, JOYSTICK_MIDPOINT(g_JoystickDevCaps.wXmin, g_JoystickDevCaps.wXmax) - ab, aa.dwXpos);
    ab = (g_JoystickDevCaps.wYmax - g_JoystickDevCaps.wYmin) / 2 / 2;
    buttons |= JOYSTICK_BUTTON_PRESSED(TH_BUTTON_DOWN, aa.dwYpos,
                                       JOYSTICK_MIDPOINT(g_JoystickDevCaps.wYmin, g_JoystickDevCaps.wYmax) + ab);
    buttons |= JOYSTICK_BUTTON_PRESSED(
        TH_BUTTON_UP, JOYSTICK_MIDPOINT(g_JoystickDevCaps.wYmin, g_JoystickDevCaps.wYmax) - ab, aa.dwYpos);
#endif
    return buttons;
}

#undef CONTROLLER_MAPPING

u32 Controller::SetButtonFromDirectInputJoystate(u16 *outButtons, ControllerButton controllerButtonToTest,
                                                 TouhouButton touhouButton, u8 *inputButtons)
{
#if BUILT_SINCE(102)
    if (controllerButtonToTest < 0)
    {
        return 0;
    }

    *outButtons |= (inputButtons[controllerButtonToTest] & 0x80 ? touhouButton & 0xFFFF : 0);

    return inputButtons[controllerButtonToTest] & 0x80 ? touhouButton & 0xFFFF : 0;
#else
    if (controllerButtonToTest == 0)
    {
        return 0;
    }
    i32 buttonIndex;
    for (buttonIndex = 0; buttonIndex < 32; buttonIndex++, controllerButtonToTest >>= 1)
    {
        if (controllerButtonToTest & 1)
        {
            break;
        }
    }
    // 1.00 has no bounds check and reads inputButtons[32] when no bit is set.
#if BUILT_SINCE(101)
    if (buttonIndex >= 32)
    {
        return 0;
    }
#endif
    *outButtons |= (inputButtons[buttonIndex] & 0x80 ? touhouButton & 0xFFFF : 0);
    return inputButtons[buttonIndex] & 0x80 ? touhouButton & 0xFFFF : 0;
#endif
}

u32 Controller::SetButtonFromControllerInputs(u16 *outButtons, ControllerButton controllerButtonToTest,
                                              TouhouButton touhouButton, u32 inputButtons)
{
    if (controllerButtonToTest < 0)
    {
        return 0;
    }

    u32 mask = 1 << controllerButtonToTest;

    *outButtons |= (inputButtons & mask ? touhouButton & 0xFFFF : 0);

    return inputButtons & mask ? touhouButton & 0xFFFF : 0;
}

// This is for rebinding keys
u8 *th06::Controller::GetControllerState()
{
    static u8 g_ControllerData[128];

    memset(&g_ControllerData, 0, sizeof(g_ControllerData));

    if (g_Supervisor.controller == NULL)
#pragma var_order(joyinfoex, joyButtonBit, joyButtonIndex)
    {
#if BUILT_SINCE(102g)
        JOYINFOEX joyinfoex;
        u32 joyButtonBit;
        u32 joyButtonIndex;

        memset(&joyinfoex, 0, sizeof(JOYINFOEX));
        joyinfoex.dwSize = sizeof(JOYINFOEX);
        joyinfoex.dwFlags = JOY_RETURNALL;
        if (joyGetPosEx(JOYSTICKID1, &joyinfoex) != JOYERR_NOERROR)
        {
            return g_ControllerData;
        }
        for (joyButtonBit = joyinfoex.dwButtons, joyButtonIndex = 0; joyButtonIndex < 32;
             joyButtonIndex++, joyButtonBit >>= 1)
        {
            if (joyButtonBit & 1)
            {
                g_ControllerData[joyButtonIndex] = 0x80;
            }
        }
#endif
        return g_ControllerData;
    }
    else
#pragma var_order(dires, dijoystate2)
    {
        DIJOYSTATE2 dijoystate2;

        HRESULT dires = g_Supervisor.controller->Poll();
        if (FAILED(dires))
        {
            i32 diRetryCount = 0;
            DebugPrint("error : DIERR_INPUTLOST\n");
            dires = g_Supervisor.controller->Acquire();
            while (dires == DIERR_INPUTLOST)
            {
                dires = g_Supervisor.controller->Acquire();
                DebugPrint("error : DIERR_INPUTLOST %d\n", diRetryCount);
                diRetryCount++;
                if (diRetryCount >= 400)
                {
                    return g_ControllerData;
                }
            }
#if BUILT_SINCE(102f)
            return g_ControllerData;
#endif
        }

#if BUILT_SINCE(102g) || (BUILT_SINCE(102) && BUILT_BEFORE(102c))
        // These builds check the earlier Poll result instead of the GetDeviceState result.
        // Yes this really did get broken in 1.02, fixed in 1.02d, and unfixed in 1.02g.
        g_Supervisor.controller->GetDeviceState(sizeof(DIJOYSTATE2), &dijoystate2);
#else
        dires = g_Supervisor.controller->GetDeviceState(sizeof(DIJOYSTATE2), &dijoystate2);
#endif
        if (FAILED(dires))
        {
            return g_ControllerData;
        }
        memcpy(g_ControllerData, dijoystate2.rgbButtons, sizeof(dijoystate2.rgbButtons));
        return g_ControllerData;
    }
}

u16 Controller::GetInput(void)
{
    u8 keyboardState[256];
    u16 buttons = 0;

    if (g_Supervisor.keyboard == NULL)
    {
        GetKeyboardState(keyboardState);

        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP, VK_UP);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN, VK_DOWN);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_LEFT, VK_LEFT);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_RIGHT, VK_RIGHT);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP, VK_NUMPAD8);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN, VK_NUMPAD2);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_LEFT, VK_NUMPAD4);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_RIGHT, VK_NUMPAD6);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP_LEFT, VK_NUMPAD7);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP_RIGHT, VK_NUMPAD9);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN_LEFT, VK_NUMPAD1);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN_RIGHT, VK_NUMPAD3);
#if BUILT_SINCE(102g)
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_HOME, VK_HOME);
#else
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_HOME, VK_F12);
#endif
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SHOOT, 'Z');
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_BOMB, 'X');
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_FOCUS, VK_SHIFT);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_MENU, VK_ESCAPE);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SKIP, VK_CONTROL);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_Q, 'Q');
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_S, 'S');
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_ENTER, VK_RETURN);
    }
    else
    {
        HRESULT res = g_Supervisor.keyboard->GetDeviceState(sizeof(keyboardState), keyboardState);

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
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP, DIK_NUMPAD8);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN, DIK_NUMPAD2);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_LEFT, DIK_NUMPAD4);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_RIGHT, DIK_NUMPAD6);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP_LEFT, DIK_NUMPAD7);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_UP_RIGHT, DIK_NUMPAD9);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN_LEFT, DIK_NUMPAD1);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_DOWN_RIGHT, DIK_NUMPAD3);
#if BUILT_SINCE(102g)
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_HOME, DIK_HOME);
#else
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_HOME, DIK_F12);
#endif
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SHOOT, DIK_Z);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_BOMB, DIK_X);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_FOCUS, DIK_LSHIFT);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_FOCUS, DIK_RSHIFT);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_MENU, DIK_ESCAPE);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SKIP, DIK_LCONTROL);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_SKIP, DIK_RCONTROL);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_Q, DIK_Q);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_S, DIK_S);
        buttons |= KEYBOARD_KEY_PRESSED(TH_BUTTON_ENTER, DIK_RETURN);
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

// ----------------------------------------------------------------------------
//
// font.cpp - フォント描画部分
//
// Copyright (c) 2001 if (if@edokko.com)
// All Rights Reserved.
// https://t-pot.com/program/12_font/index.html
//
// ----------------------------------------------------------------------------
#define RELEASE(o)                                                                                                     \
    if (o)                                                                                                             \
    {                                                                                                                  \
        o->Release();                                                                                                  \
        o = NULL;                                                                                                      \
    }

void CMyFont::Init(LPDIRECT3DDEVICE8 lpD3DDEV, int w, int h)
{
    HDC hTextDC = NULL;
    HFONT hFont = NULL, hOldFont = NULL;

    hTextDC = CreateCompatibleDC(NULL);
    hFont = CreateFont(h, w, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TH_FONT_NAME);
    if (!hFont)
        return;
    hOldFont = (HFONT)SelectObject(hTextDC, hFont);

    if (FAILED(D3DXCreateFont(lpD3DDEV, hFont, &m_lpFont)))
    {
        MessageBox(NULL, "D3DXCreateFontIndirect FALSE", "ok", MB_OK);
        return;
    }
    SelectObject(hTextDC, hOldFont);
    DeleteObject(hFont);
}

// ----------------------------------------------------------------------------
void CMyFont::Print(char *str, int x, int y, D3DCOLOR color)
{
    RECT rect;
    rect.left = x;
    rect.right = GAME_WINDOW_WIDTH;
    rect.top = y;
    rect.bottom = GAME_WINDOW_HEIGHT;

    m_lpFont->DrawText(str, -1, &rect, DT_LEFT | DT_EXPANDTABS, color);
}
// ----------------------------------------------------------------------------
void CMyFont::Clean()
{
    RELEASE(m_lpFont);
}

Pbg3Archive **g_Pbg3ArcList;

#pragma var_order(pbg3Idx, entryname, entryIdx, fsize, data, file)
#if BUILT_SINCE(100)
u8 *FileSystem::OpenPath(const char *filepath, ZunBool isExternalResource)
#else
u8 *FileSystem::OpenPath(const char *filepath)
#endif
{
    u8 *data;
    FILE *file;
    long fsize;
    const char *entryname;
    i32 pbg3Idx;

    i32 entryIdx = -1;
#if BUILT_SINCE(100)
    if (!isExternalResource)
#endif
    {
        entryname = strrchr(filepath, '\\');
        if (entryname == NULL)
        {
            entryname = filepath;
        }
        else
        {
            entryname++;
        }
        entryname = strrchr(entryname, '/');
        if (entryname == NULL)
        {
            entryname = filepath;
        }
        else
        {
            entryname++;
        }
        if (g_Pbg3ArcList != NULL)
        {
            for (pbg3Idx = 0; pbg3Idx < 16; pbg3Idx++)
            {
                if (g_Pbg3ArcList[pbg3Idx] != NULL)
                {
                    entryIdx = g_Pbg3ArcList[pbg3Idx]->FindEntry(entryname);
                    if (entryIdx >= 0)
                    {
                        break;
                    }
                }
            }
        }
#if BUILT_SINCE(100)
        if (entryIdx < 0)
        {
#if BUILT_BEFORE(102g)
            DebugPrint("%s not find ... \n", entryname);
#endif
            return NULL;
        }
#endif
    }
    if (entryIdx >= 0)
    {
        DebugPrint("%s Decode ... \n", entryname);
        data = g_Pbg3ArcList[pbg3Idx]->ReadDecompressEntry(entryIdx, entryname);
        g_ReadFileSize = g_Pbg3ArcList[pbg3Idx]->GetEntrySize(entryIdx);
    }
    else
    {
        DebugPrint("%s Load ... \n", filepath);
        file = fopen(filepath, "rb");
        if (file == NULL)
        {
            DebugPrint("error : %s is not found.\n", filepath);
            return NULL;
        }
        else
        {
            fseek(file, 0, SEEK_END);
            fsize = ftell(file);
            g_ReadFileSize = fsize;
            fseek(file, 0, SEEK_SET);
            data = ZUN_ALLOC(fsize);
            fread(data, 1, fsize, file);
            fclose(file);
        }
    }
    return data;
}

int FileSystem::WriteDataToFile(const char *path, const void *data, size_t size)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL)
    {
        return -1;
    }
    else
    {
        if (fwrite(data, 1, size, f) != size)
        {
            fclose(f);
            return -2;
        }
        else
        {
            fclose(f);
            return 0;
        }
    }
}

// GameErrorContext
GameErrorContext g_GameErrorContext;

// Rng
Rng g_Rng;

const char *GameErrorContext::Log(const char *fmt, ...)
{
    char tmpBuffer[512];
    size_t tmpBufferSize;
    va_list args;

    va_start(args, fmt);
    vsprintf(tmpBuffer, fmt, args);

    tmpBufferSize = strlen(tmpBuffer);

    if (this->m_BufferEnd + tmpBufferSize < &this->m_Buffer[sizeof(this->m_Buffer) - 1])
    {
        strcpy(this->m_BufferEnd, tmpBuffer);

        this->m_BufferEnd += tmpBufferSize;
        *this->m_BufferEnd = '\0';
    }

    va_end(args);

    return fmt;
}

const char *GameErrorContext::Fatal(const char *fmt, ...)
{
    char tmpBuffer[512];
    size_t tmpBufferSize;
    va_list args;

    va_start(args, fmt);
    vsprintf(tmpBuffer, fmt, args);

    tmpBufferSize = strlen(tmpBuffer);

    if (this->m_BufferEnd + tmpBufferSize < &this->m_Buffer[sizeof(this->m_Buffer) - 1])
    {
        strcpy(this->m_BufferEnd, tmpBuffer);

        this->m_BufferEnd += tmpBufferSize;
        *this->m_BufferEnd = '\0';
    }

    va_end(args);

    this->m_ShowMessageBox = true;

    return fmt;
}

#if BUILT_BEFORE(102g)
const char *GameErrorContext::DetailedLog(const char *fmt, ...)
{
    char tmpBuffer[512];
    size_t tmpBufferSize;
    va_list args;

    va_start(args, fmt);
    vsprintf(tmpBuffer, fmt, args);

    tmpBufferSize = strlen(tmpBuffer);

    if (this->m_DetailedBufferEnd + tmpBufferSize < &this->m_DetailedBuffer[sizeof(this->m_DetailedBuffer) - 1])
    {
        strcpy(this->m_DetailedBufferEnd, tmpBuffer);

        this->m_DetailedBufferEnd += tmpBufferSize;
        *this->m_DetailedBufferEnd = '\0';
    }

    va_end(args);

    return fmt;
}
#endif

u16 Rng::GetRandomU16(void)
{
    u16 a = (this->seed ^ 0x9630) - 0x6553;

    this->seed = (((a & 0xc000) >> 14) + a * 4) & 0xFFFF;
    this->generationCount++;
    return this->seed;
}

u32 Rng::GetRandomU32(void)
{
    return GetRandomU16() << 16 | GetRandomU16();
}

f32 Rng::GetRandomF32(void)
{
    return (f32)GetRandomU32() / (f32)0xFFFFFFFF;
}

namespace utils
{
f32 AddNormalizeAngle(f32 a, f32 b)
{
    i32 i = 0;
    a += b;
    while (a > ZUN_PI)
    {
        a -= ZUN_2PI;
        if (i++ > 16)
            break;
    }
    while (a < -ZUN_PI)
    {
        a += ZUN_2PI;
        if (i++ > 16)
            break;
    }
    return a;
}

#pragma var_order(sinOut, cosOut)
void Rotate(D3DXVECTOR3 *outVector, D3DXVECTOR3 *point, f32 angle)
{
    f32 sinOut = sinf(angle);
    f32 cosOut = cosf(angle);
    outVector->x = cosOut * point->x + sinOut * point->y;
    outVector->y = cosOut * point->y - sinOut * point->x;
}
} // namespace utils
} // namespace th06

typedef HRESULT(WINAPI *DIRECTDRAWCREATE)(GUID *, LPDIRECTDRAW *, IUnknown *);
typedef HRESULT(WINAPI *DIRECTDRAWCREATEEX)(GUID *, VOID **, REFIID, IUnknown *);
typedef HRESULT(WINAPI *DIRECTINPUTCREATE)(HINSTANCE, DWORD, LPDIRECTINPUT *, IUnknown *);

// Used by trial startup; unreferenced by the 1.02h startup path.
DWORD GetDXVersion()
{
    DIRECTDRAWCREATE DirectDrawCreate = NULL;
    DIRECTDRAWCREATEEX DirectDrawCreateEx = NULL;
    DIRECTINPUTCREATE DirectInputCreate = NULL;
    HINSTANCE hDDrawDLL = NULL;
    HINSTANCE hDInputDLL = NULL;
    HINSTANCE hD3D8DLL = NULL;
    LPDIRECTDRAW pDDraw = NULL;
    LPDIRECTDRAW2 pDDraw2 = NULL;
    LPDIRECTDRAWSURFACE pSurf = NULL;
    LPDIRECTDRAWSURFACE3 pSurf3 = NULL;
    LPDIRECTDRAWSURFACE4 pSurf4 = NULL;
    DWORD dwDXVersion = 0;
    HRESULT hr;

    // First see if DDRAW.DLL even exists.
    hDDrawDLL = LoadLibrary("DDRAW.DLL");
    if (hDDrawDLL == NULL)
    {
        dwDXVersion = 0;
        return dwDXVersion;
    }

    // See if we can create the DirectDraw object.
    DirectDrawCreate = (DIRECTDRAWCREATE)GetProcAddress(hDDrawDLL, "DirectDrawCreate");
    if (DirectDrawCreate == NULL)
    {
        dwDXVersion = 0;
        FreeLibrary(hDDrawDLL);
        OutputDebugString("Couldn't LoadLibrary DDraw\r\n");
        return dwDXVersion;
    }

    hr = DirectDrawCreate(NULL, &pDDraw, NULL);
    if (FAILED(hr))
    {
        dwDXVersion = 0;
        FreeLibrary(hDDrawDLL);
        OutputDebugString("Couldn't create DDraw\r\n");
        return dwDXVersion;
    }

    // So DirectDraw exists.  We are at least DX1.
    dwDXVersion = 0x100;

    // Let's see if IID_IDirectDraw2 exists.
    hr = pDDraw->QueryInterface(IID_IDirectDraw2, (VOID **)&pDDraw2);
    if (FAILED(hr))
    {
        // No IDirectDraw2 exists... must be DX1
        pDDraw->Release();
        FreeLibrary(hDDrawDLL);
        OutputDebugString("Couldn't QI DDraw2\r\n");
        return dwDXVersion;
    }

    // IDirectDraw2 exists. We must be at least DX2
    pDDraw2->Release();
    dwDXVersion = 0x200;

    //-------------------------------------------------------------------------
    // DirectX 7.0 Checks
    //-------------------------------------------------------------------------

    // Check for DirectX 7 by creating a DDraw7 object
    LPDIRECTDRAW7 pDD7;
    DirectDrawCreateEx = (DIRECTDRAWCREATEEX)GetProcAddress(hDDrawDLL, "DirectDrawCreateEx");
    if (NULL == DirectDrawCreateEx)
    {
        FreeLibrary(hDDrawDLL);
        return dwDXVersion;
    }

    if (FAILED(DirectDrawCreateEx(NULL, (VOID **)&pDD7, IID_IDirectDraw7, NULL)))
    {
        FreeLibrary(hDDrawDLL);
        return dwDXVersion;
    }

    // DDraw7 was created successfully. We must be at least DX7.0
    dwDXVersion = 0x700;
    pDD7->Release();

    //-------------------------------------------------------------------------
    // DirectX 8.0 Checks
    //-------------------------------------------------------------------------

    // Simply see if D3D8.dll exists.
    hD3D8DLL = LoadLibrary("D3D8.DLL");
    if (hD3D8DLL == NULL)
    {
        FreeLibrary(hDDrawDLL);
        return dwDXVersion;
    }

    // D3D8.dll exists. We must be at least DX8.0
    dwDXVersion = 0x800;

    //-------------------------------------------------------------------------
    // End of checking for versions of DirectX
    //-------------------------------------------------------------------------

    // Close open libraries and return
    FreeLibrary(hDDrawDLL);
    FreeLibrary(hD3D8DLL);

    return dwDXVersion;
}
