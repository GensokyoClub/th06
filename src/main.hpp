#pragma once

#include "ZunBool.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include <windows.h>

#define GAME_WINDOW_WIDTH 640
#define GAME_WINDOW_HEIGHT 480

namespace th06
{
enum RenderResult
{
    RENDER_RESULT_KEEP_RUNNING,
    RENDER_RESULT_EXIT_SUCCESS,
    RENDER_RESULT_EXIT_ERROR,
};

struct GameWindow
{
    RenderResult Render();
    static void Present();

    static i32 InitD3dInterface();
    static void CreateGameWindow(HINSTANCE hInstance);
    static i32 InitD3dRendering();
    static void InitD3dDevice();
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND window;
    ZunBool isAppClosing;
    ZunBool isAppActive;
    ZunBool showCursor;
    u8 curFrame;
    BOOL screenSaveActive;
    BOOL lowPowerActive;
    BOOL powerOffActive;
};

DIFFABLE_EXTERN(GameWindow, g_GameWindow)
DIFFABLE_EXTERN(i32, g_TickCountToEffectiveFramerate)
DIFFABLE_EXTERN(double, g_LastFrameTime)
}; // namespace th06
