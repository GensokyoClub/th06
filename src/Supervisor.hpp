#pragma once
#include <d3d8.h>
#include <d3dx8math.h>
#include <dinput.h>

#include "Chain.hpp"
#include "Global.hpp"
#include "ZunBool.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"
#include "pbg3/Pbg3Archive.hpp"

namespace th06
{
struct MidiOutput;

// The 0.13a trial's archive marker differs from its config and replay format version.
// TODO: Check 0.08p
#if BUILD_VERSION == BUILD_VERSION_013 || BUILD_VERSION == BUILD_VERSION_013a
#define CONFIG_VERSION 0x0102
#define REPLAY_VERSION 0x0102
#elif BUILD_VERSION == BUILD_VERSION_101
// 1.01 keeps 1.00's replay version, so 1.00 replays still load.
#define CONFIG_VERSION GAME_VERSION
#define REPLAY_VERSION 0x0100
#else
#define CONFIG_VERSION GAME_VERSION
#define REPLAY_VERSION GAME_VERSION
#endif

struct GameConfigOpts
{
    u32 useSwTextureBlending : 1;
    u32 dontUseVertexBuf : 1;
    u32 force16bitColorMode : 1;
    u32 clearBackBufferOnRefresh : 1;
    u32 displayMinimumGraphics : 1;
    u32 suppressUseOfGoroudShading : 1;
    u32 disableDepthTest : 1;
    u32 force60Fps : 1;
    u32 disableColorCompositing : 1;
    u32 referenceRasterizerMode : 1;
    u32 disableFog : 1;
    u32 dontUseDirectInput : 1;
    alignment_bitfields(u32, 20);
};
ZUN_ASSERT_TYPE(GameConfigOpts, 0x4, 4);

#define GAME_WINDOW_WIDTH 640
#define GAME_WINDOW_HEIGHT 480

#define GAME_REGION_POS_X 32.0f
#define GAME_REGION_POS_Y 16.0f

#define GAME_REGION_WIDTH 384.0f
#define GAME_REGION_HEIGHT 448.0f

#define GAME_REGION_POS_RIGHT (GAME_REGION_POS_X + GAME_REGION_WIDTH)
#define GAME_REGION_POS_BOTTOM (GAME_REGION_POS_Y + GAME_REGION_HEIGHT)

#define GAME_REGION_LEFT 0.0f
#define GAME_REGION_TOP 0.0f

#define GAME_REGION_RIGHT (GAME_REGION_LEFT + GAME_REGION_WIDTH)
#define GAME_REGION_BOTTOM (GAME_REGION_TOP + GAME_REGION_HEIGHT)

enum MusicMode
{
    OFF = 0,
    WAV = 1,
    MIDI = 2
};

struct GameConfiguration
{
    ControllerMapping controllerMapping;
#if BUILT_SINCE(102)
    // Always 0x102 for 1.02
    i32 version;
#endif
    u8 lifeCount;
    u8 bombCount;
    u8 colorMode16bit;
    u8 musicMode;
    u8 playSounds;
    u8 defaultDifficulty;
    u8 windowed;
    // 0 = fullspeed, 1 = 1/2 speed, 2 = 1/4 speed.
    u8 frameskipConfig;
#if BUILT_SINCE(102)
    i16 padXAxis;
    i16 padYAxis;
    unreferenced_fields(0x10);
#endif
    // GameConfigOpts bitfield.
    GameConfigOpts opts;
};
#if BUILT_SINCE(102)
ZUN_ASSERT_TYPE(GameConfiguration, 0x38, 4);
#else
ZUN_ASSERT_TYPE(GameConfiguration, 0x2c, 4);
#endif

#define IN_PBG3_INDEX 0
#define MD_PBG3_INDEX 1
#define ST_PBG3_INDEX 2
#define TL_PBG3_INDEX 3
#define CM_PBG3_INDEX 4
#define ED_PBG3_INDEX 5

typedef char Pbg3ArchiveName[32];

enum SupervisorState
{
    SUPERVISOR_STATE_INIT,
    SUPERVISOR_STATE_MAINMENU,
    SUPERVISOR_STATE_GAMEMANAGER,
    SUPERVISOR_STATE_NEXT_STAGE,
    SUPERVISOR_STATE_EXITSUCCESS,
    SUPERVISOR_STATE_EXITERROR,
    SUPERVISOR_STATE_RESULTSCREEN,
    SUPERVISOR_STATE_RESULTSCREEN_FROMGAME,
    SUPERVISOR_STATE_MAINMENU_REPLAY,
    SUPERVISOR_STATE_MUSICROOM,
    SUPERVISOR_STATE_ENDING,
};

struct Supervisor
{
    Supervisor()
    {
        memset(this, 0, sizeof(Supervisor));
    }

    ZunBool ReadMidiFile(u32 midiFileIdx, const char *path);
    ZunBool PlayMidiFile(i32 midiFileIdx);
    ZunResult PlayAudio(const char *path);
    ZunResult StopAudio();
    ZunResult SetupMidiPlayback(const char *path);
    ZunResult FadeOutMusic(f32 fadeOutSeconds);

    BOOL LoadPbg3(i32 pbg3FileIdx, const char *filename);
    void ReleasePbg3(i32 pbg3FileIdx);

    ZunResult LoadConfig(const char *path);

    void TickTimer(i32 *frames, f32 *subframes);

    i32 GetMusicMode()
    {
        return this->cfg.musicMode;
    }

    ZunBool IsHardwareBlendingDisabled()
    {
        return this->cfg.opts.useSwTextureBlending;
    }

    ZunBool IsVertexBufferDisabled()
    {
        return this->cfg.opts.dontUseVertexBuf;
    }

    ZunBool Is16bitColorMode()
    {
        return this->cfg.opts.force16bitColorMode;
    }

    ZunBool IsMinimumGraphicsMode()
    {
        return this->cfg.opts.displayMinimumGraphics;
    }

    ZunBool IsShadingDisabled()
    {
        return this->cfg.opts.suppressUseOfGoroudShading;
    }

    ZunBool IsDepthTestDisabled()
    {
        return this->cfg.opts.disableDepthTest;
    }

    ZunBool IsForced60Fps()
    {
        return this->cfg.opts.force60Fps;
    }

    ZunBool IsColorCompositingDisabled()
    {
        return this->cfg.opts.disableColorCompositing;
    }

    ZunBool IsReferenceRasterizerMode()
    {
        return this->cfg.opts.referenceRasterizerMode;
    }

    ZunBool IsFogDisabled()
    {
        return this->cfg.opts.disableFog;
    }

    ZunBool IsDInputDisabled()
    {
        return this->cfg.opts.dontUseDirectInput;
    }

    ZunBool ShouldForceBackbufferClear()
    {
        return this->cfg.opts.clearBackBufferOnRefresh | this->cfg.opts.displayMinimumGraphics;
    }

    ZunBool IsSoftwareTexturing()
    {
        return this->cfg.opts.disableColorCompositing | this->cfg.opts.useSwTextureBlending;
    }

#if BUILT_SINCE(102f)
    // The later frame-pacing condition also checks vsyncEnabled, which earlier builds lack.
    ZunBool ShouldRunAt60Fps()
    {
        return this->IsForced60Fps() && this->vsyncEnabled;
    }
#endif

    ZunBool IsWindowed()
    {
        return this->cfg.windowed;
    }

    ZunBool IsNotLoadingNextStage()
    {
        return this->curState != SUPERVISOR_STATE_NEXT_STAGE;
    }

    HINSTANCE hInstance;
    LPDIRECT3D8 d3dIface;
    LPDIRECT3DDEVICE8 d3dDevice;
    LPDIRECTINPUT8 dinputIface;
    LPDIRECTINPUTDEVICE8 keyboard;
    LPDIRECTINPUTDEVICE8 controller;
#if BUILT_SINCE(101)
    DIDEVCAPS controllerCaps;
#endif
    HWND hwndGameWindow;
    D3DXMATRIX viewMatrix;
    D3DXMATRIX projectionMatrix;
    D3DVIEWPORT8 viewport;
    D3DPRESENT_PARAMETERS presentParameters;
    GameConfiguration cfg;
#if BUILT_SINCE(102g)
    // NOTE: This is not even close to a default config
    GameConfiguration defaultConfig;
#endif
    i32 calcCount;
    i32 wantedState;
    i32 curState;
    i32 prevState;

    unreferenced_fields(0x4);
    i32 forceRedrawFrames;
    ZunBool isInEnding;

#if BUILT_SINCE(102f)
    ZunBool vsyncEnabled;
#endif
    i32 lastFrameTime;
    f32 effectiveFramerateMultiplier;
    f32 framerateMultiplier;

    MidiOutput *midiOutput;

    f32 unk1b4;
    f32 unk1b8;

    Pbg3Archive *pbg3Archives[16];
    Pbg3ArchiveName pbg3ArchiveNames[16];

    u8 hasD3dHardwareVertexProcessing;
    u8 lockableBackbuffer;
    u8 colorMode16Bits;
    alignment_padding(0x1);

    u32 startupTimeBeforeMenuMusic;
    D3DCAPS8 d3dCaps;
};
ZunResult Supervisor_RegisterChain();

#if BUILT_SINCE(102g)
ZUN_ASSERT_SIZE(Supervisor, 0x4d8);
#elif BUILT_SINCE(102f)
ZUN_ASSERT_SIZE(Supervisor, 0x4a0);
#elif BUILT_SINCE(102)
ZUN_ASSERT_SIZE(Supervisor, 0x49c);
#elif BUILT_SINCE(101)
ZUN_ASSERT_SIZE(Supervisor, 0x490);
#else
ZUN_ASSERT_SIZE(Supervisor, 0x464);
#endif

extern Supervisor g_Supervisor;

} // namespace th06
