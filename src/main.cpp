#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_main.h>
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

#ifdef __3DS__
#include <3ds.h>
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

#ifdef __EMSCRIPTEN__
#define EM_TH_CONFIG_FILE "/persistent/" TH_CONFIG_FILE
#endif

static bool initialize_game()
{
#ifdef _WIN32
    // TODO: Find a better solution for locales / encoding on Windows, because it's all a mess
    setlocale(LC_ALL, ".UTF8");
#endif

#ifdef __EMSCRIPTEN__
    bool loadConfig = g_Supervisor.LoadConfig(EM_TH_CONFIG_FILE);
#else
    bool loadConfig = g_Supervisor.LoadConfig(TH_CONFIG_FILE);

#endif

    if (!loadConfig)
    {
        g_GameErrorContext.Flush();
        return false;
    }

    GameWindow::CreateGameWindow();

    g_AnmManager = new AnmManager();

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

#ifdef __EMSCRIPTEN__
    FileSystem::WriteDataToFile(EM_TH_CONFIG_FILE, &g_Supervisor.cfg, sizeof(g_Supervisor.cfg));
#else
    FileSystem::WriteDataToFile(TH_CONFIG_FILE, &g_Supervisor.cfg, sizeof(g_Supervisor.cfg));
#endif

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

int main(int argc, char **argv)
{
    #ifdef __3DS__
        gfxInitDefault();
    #endif

    if (!initialize_game())
        return -1;

#ifdef __EMSCRIPTEN__
    // Non-blocking browser main loop
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (running)
        main_loop();
#endif

    cleanup();
    return 0;
}
