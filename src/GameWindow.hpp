#pragma once

#include <SDL2/SDL_video.h>

#include "GLFunc.hpp"
#include "inttypes.hpp"

// The internal resolution EoSD uses. 640x480. I can't think of any reason anyone sane
//   would want to change this
#define GAME_WINDOW_WIDTH (640)
#define GAME_WINDOW_HEIGHT (480)

// The actual resolution used for the output window and viewport scaling
//   These values are computed at runtime from the created window size.
extern i32 g_GameWindowWidthReal;
extern i32 g_GameWindowHeightReal;
extern i32 g_ViewportWidth;
extern i32 g_ViewportHeight;
extern i32 g_ViewportOffX;
extern i32 g_ViewportOffY;
extern f32 g_WidthResolutionScale;
extern f32 g_HeightResolutionScale;

#define GAME_WINDOW_WIDTH_REAL (g_GameWindowWidthReal)
#define GAME_WINDOW_HEIGHT_REAL (g_GameWindowHeightReal)
#define VIEWPORT_WIDTH (g_ViewportWidth)
#define VIEWPORT_OFF_X (g_ViewportOffX)
#define VIEWPORT_HEIGHT (g_ViewportHeight)
#define VIEWPORT_OFF_Y (g_ViewportOffY)
#define WIDTH_RESOLUTION_SCALE (g_WidthResolutionScale)
#define HEIGHT_RESOLUTION_SCALE (g_HeightResolutionScale)

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

extern GameWindow g_GameWindow;
extern i32 g_TickCountToEffectiveFramerate;
extern double g_LastFrameTime;
