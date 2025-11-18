#pragma once

#include <SDL2/SDL_video.h>

#include "GLFunc.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"

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

    static void CreateGameWindow();
    static i32 InitD3dRendering();
    static void InitD3dDevice();

    SDL_Window *window;
    SDL_GLContext glContext;
    i32 isAppClosing;
    i32 lastActiveAppValue;
    i32 isAppActive;
    u8 curFrame;
    i32 screenSaveActive;
    i32 lowPowerActive;
    i32 powerOffActive;
    u32 renderBackendIndex;
};

DIFFABLE_EXTERN(GameWindow, g_GameWindow)
DIFFABLE_EXTERN(i32, g_TickCountToEffectiveFramerate)
DIFFABLE_EXTERN(double, g_LastFrameTime)
}; // namespace th06
