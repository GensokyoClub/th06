
#define _WIN32_WINNT 0x0500
#include "Connection.hpp"
#include "ConnectionUI.hpp"
#include <windows.h>

#include <D3DX8.h>
#include <stdio.h>

#include "AnmManager.hpp"
#include "Chain.hpp"
#include "FileSystem.hpp"
#include "GameErrorContext.hpp"
#include "GameWindow.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "Supervisor.hpp"
#include "ZunResult.hpp"
#include "i18n.hpp"
#include "utils.hpp"

using namespace th06;

Host g_host;
Guest g_guest;
int g_delay = 1;
bool g_is_host = false;
bool g_is_connected = false;
bool g_is_single_mode = false;

#pragma var_order(renderResult, testCoopLevelRes, msg, testResetRes, waste1, waste2, waste3, waste4, waste5, waste6)
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    i32 renderResult = 0;
    i32 testCoopLevelRes;
    i32 testResetRes;
    MSG msg;
    i32 waste1, waste2, waste3, waste4, waste5, waste6;

    ConnectionUI ui(g_host,g_guest);
    ui.Show();
    g_delay = ui.GetDelay();
    g_is_host = ui.IsHost();
    if(!ui.IsGameStarted())
        return 1;
    if(!ui.IsConnected())
    {
        g_is_connected=false;
        g_is_single_mode = true;
    }else{
        g_is_connected = true;
        g_is_single_mode = false;
    }
    //g_is_connected = false;

    //if (utils::CheckForRunningGameInstance())
    //{
    //    g_GameErrorContext.Flush();
    //    return 1;
    //}

    g_Supervisor.hInstance = hInstance;

    if (g_Supervisor.LoadConfig(TH_CONFIG_FILE) != ZUN_SUCCESS)
    {
        g_GameErrorContext.Flush();
        return -1;
    }

    if (GameWindow::InitD3dInterface())
    {
        g_GameErrorContext.Flush();
        return 1;
    }

    SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &g_GameWindow.screenSaveActive, 0);
    SystemParametersInfo(SPI_GETLOWPOWERACTIVE, 0, &g_GameWindow.lowPowerActive, 0);
    SystemParametersInfo(SPI_GETPOWEROFFACTIVE, 0, &g_GameWindow.powerOffActive, 0);
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0, NULL, SPIF_SENDCHANGE);
    SystemParametersInfo(SPI_SETLOWPOWERACTIVE, 0, NULL, SPIF_SENDCHANGE);
    SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0, NULL, SPIF_SENDCHANGE);

restart:
    GameWindow::CreateGameWindow(hInstance);

    if (GameWindow::InitD3dRendering())
    {
        g_GameErrorContext.Flush();
        return 1;
    }
    g_SoundPlayer.InitializeDSound(g_GameWindow.window);
    Controller::GetJoystickCaps();
    Controller::ResetKeyboard();

    g_AnmManager = new AnmManager();

    if (Supervisor::RegisterChain() != ZUN_SUCCESS)
    {
        goto stop;
    }
    if (!g_Supervisor.cfg.windowed)
    {
        ShowCursor(FALSE);
    }

    g_GameWindow.curFrame = 0;

    while (!g_GameWindow.isAppClosing)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            testCoopLevelRes = g_Supervisor.d3dDevice->TestCooperativeLevel();
            if (testCoopLevelRes == D3D_OK)
            {
                renderResult = g_GameWindow.Render();
                if (renderResult != 0)
                {
                    goto stop;
                }
            }
            else if (testCoopLevelRes == D3DERR_DEVICENOTRESET)
            {
                g_AnmManager->ReleaseSurfaces();
                testResetRes = g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);
                if (testResetRes != 0)
                {
                    goto stop;
                }
                GameWindow::InitD3dDevice();
                g_Supervisor.unk198 = 3;
            }
        }
    }

stop:
    g_Chain.Release();
    g_SoundPlayer.Release();

    delete g_AnmManager;
    g_AnmManager = NULL;
    if (g_Supervisor.d3dDevice != NULL)
    {
        g_Supervisor.d3dDevice->Release();
        g_Supervisor.d3dDevice = NULL;
    }

    ShowWindow(g_GameWindow.window, 0);
    MoveWindow(g_GameWindow.window, 0, 0, 0, 0, 0);
    DestroyWindow(g_GameWindow.window);

    if (renderResult == 2)
    {
        g_GameErrorContext.ResetContext();

        g_GameErrorContext.Log(TH_ERR_OPTION_CHANGED_RESTART);

        if (!g_Supervisor.cfg.windowed)
        {
            ShowCursor(TRUE);
        }
        goto restart;
    }

    FileSystem::WriteDataToFile(TH_CONFIG_FILE, &g_Supervisor.cfg, sizeof(g_Supervisor.cfg));
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, g_GameWindow.screenSaveActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfo(SPI_SETLOWPOWERACTIVE, g_GameWindow.lowPowerActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfo(SPI_SETPOWEROFFACTIVE, g_GameWindow.powerOffActive, NULL, SPIF_SENDCHANGE);

    if (g_Supervisor.d3dIface != NULL)
    {
        g_Supervisor.d3dIface->Release();
        g_Supervisor.d3dIface = NULL;
    }

    ShowCursor(TRUE);
    g_GameErrorContext.Flush();
    return 0;
}
