#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <cstdio>

#include "AnmManager.hpp"
#include "Chain.hpp"
#include "FileSystem.hpp"
#include "GameErrorContext.hpp"
#include "GameWindow.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "Supervisor.hpp"
#include "i18n.hpp"
#include "utils.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

using namespace th06;

// Forward declarations
static void main_loop();
static void cleanup();
static void restart_game();

static bool running = false;
static int renderResult = 0;

// ------------------------------------------------
// Initialization
// ------------------------------------------------

static bool initialize_game()
{
#ifdef _WIN32
    setlocale(LC_ALL, ".UTF8");
#endif

    if (!g_Supervisor.LoadConfig(TH_CONFIG_FILE))
    {
        g_GameErrorContext.Flush();
        return false;
    }

    GameWindow::CreateGameWindow();

    if (GameWindow::InitD3dRendering())
    {
        g_GameErrorContext.Flush();
        return false;
    }

    #ifdef __EMSCRIPTEN__
        emscripten_set_element_css_size("#canvas", 640, 480);
    #endif

    g_SoundPlayer.InitializeDSound();
    Controller::GetJoystickCaps();
    Controller::ResetKeyboard();

    g_AnmManager = new AnmManager();

    if (!Supervisor::RegisterChain())
    {
        return false;
    }

    if (!g_Supervisor.cfg.windowed)
        SDL_ShowCursor(SDL_DISABLE);

    g_GameWindow.curFrame = 0;
    running = true;

    return true;
}

// ------------------------------------------------
// Main loop (called each frame by Emscripten)
// ------------------------------------------------

static void main_loop()
{
    if (!running)
        return;

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
        {
            running = false;
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
            cleanup();
            return;
        }
    }

    renderResult = g_GameWindow.Render();
    if (renderResult != 0)
    {
        running = false;
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
        cleanup();

        if (renderResult == 2)
        {
            restart_game();
        }
    }
}

// ------------------------------------------------
// Cleanup & Restart
// ------------------------------------------------

static void cleanup()
{
    g_Chain.Release();
    g_SoundPlayer.Release();

    delete g_AnmManager;
    g_AnmManager = nullptr;

    if (g_GameWindow.window)
    {
        SDL_DestroyWindow(g_GameWindow.window);
        g_GameWindow.window = nullptr;
    }

    if (g_GameWindow.glContext)
    {
        SDL_GL_DeleteContext(g_GameWindow.glContext);
        g_GameWindow.glContext = nullptr;
    }

    SDL_Quit();

    FileSystem::WriteDataToFile(TH_CONFIG_FILE, &g_Supervisor.cfg, sizeof(g_Supervisor.cfg));
    SDL_ShowCursor(SDL_ENABLE);
    g_GameErrorContext.Flush();
}

static void restart_game()
{
    g_GameErrorContext.ResetContext();
    GameErrorContext::Log(&g_GameErrorContext, TH_ERR_OPTION_CHANGED_RESTART);

    if (!g_Supervisor.cfg.windowed)
        SDL_ShowCursor(SDL_ENABLE);

    // Reinitialize game
    if (initialize_game())
    {
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(main_loop, 0, 1);
#else
        while (running)
            main_loop();
#endif
    }
}

// ------------------------------------------------
// Entry point
// ------------------------------------------------

int main(int argc, char *argv[])
{
    if (!initialize_game())
        return -1;

#ifdef __EMSCRIPTEN__
    // Non-blocking browser main loop
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    // Desktop blocking loop
    while (running)
        main_loop();
#endif

    cleanup();
    return 0;
}
