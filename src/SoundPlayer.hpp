#pragma once
#include <Windows.h>

#include <string.h>
#include "ZunResult.hpp"
#include "decomp.hpp"
#include "zwave.hpp"

namespace th06
{
enum SoundIdx
{
    NO_SOUND = -1,
    SOUND_SHOOT = 0,
    SOUND_1 = 1,
    SOUND_2 = 2,
    SOUND_3 = 3,
    SOUND_PICHUN = 4,
    SOUND_5 = 5,
    SOUND_BOMB_REIMARI = 6,
    SOUND_7 = 7,
    SOUND_8 = 8,
    SOUND_SHOOT_BOSS = 9,
    SOUND_SELECT = 10,
    SOUND_BACK = 11,
    SOUND_MOVE_MENU = 12,
    SOUND_BOMB_REIMU_A = 13,
    SOUND_BOMB = 14,
    SOUND_F = 15,
    SOUND_BOSS_LASER = 16,
    SOUND_BOSS_LASER_2 = 17,
    SOUND_12 = 18,
    SOUND_BOMB_MARISA_B = 19,
    SOUND_TOTAL_BOSS_DEATH = 20,
    SOUND_15 = 21,
    SOUND_16 = 22,
    SOUND_17 = 23,
    SOUND_18 = 24,
    SOUND_WTF_IS_THAT_LMAO = 25,
    SOUND_1A = 26,
    SOUND_1B = 27,
    SOUND_1UP = 28,
    SOUND_1D = 29,
    SOUND_GRAZE = 30,
    SOUND_POWERUP = 31,
};

struct SoundEffectData
{
    i32 bufferIdx;
    i16 volume;
    i16 unk;
};
ZUN_ASSERT_TYPE(SoundEffectData, 0x8, 4);

// This is more than the actual number of sound effects
#define SOUND_EFFECT_COUNT 128

#define SFX_QUEUE_LENGTH 3

// 0.08 uses 64 notifications; 1.00 uses eight. 1.02h halved the later buffer and notification count.
#if BUILT_BEFORE(100)
#define BACKGROUND_MUSIC_STREAM_SECONDS 4
#define BACKGROUND_MUSIC_STREAM_NOTIFICATIONS 64
#elif BUILT_SINCE(102h)
#define BACKGROUND_MUSIC_STREAM_SECONDS 2
#define BACKGROUND_MUSIC_STREAM_NOTIFICATIONS 4
#else
#define BACKGROUND_MUSIC_STREAM_SECONDS 4
#define BACKGROUND_MUSIC_STREAM_NOTIFICATIONS 8
#endif

struct SoundPlayer
{
    SoundPlayer()
    {
        memset(this, 0, sizeof(SoundPlayer));
        for (i32 i = 0; i < SOUND_EFFECT_COUNT; i++)
        {
            this->unk408[i] = -1;
        }
    }

    ZunResult InitializeDSound(HWND window);
    ZunResult InitSoundBuffers();
    ZunResult Release(void);

    ZunResult LoadSound(i32 idx, const char *path);
#if BUILT_SINCE(100)
    void PlaySounds();
#endif
    void PlaySoundByIdx(SoundIdx idx, i32 unused = 0);
    ZunResult PlayBGM(BOOL isLooping);
    void StopBGM();
    void UpdateFades()
    {
        if (this->backgroundMusic != NULL)
        {
            this->backgroundMusic->UpdateFadeOut();
        }
    }

    void FadeOut(float seconds)
    {
        if (this->backgroundMusic != NULL)
        {
            this->backgroundMusic->StartFadeOut(seconds);
        }
    }

    ZunResult LoadWav(char *path);
    ZunResult LoadPos(const char *path);

    LPDIRECTSOUND dsoundHdl;
    unreferenced_fields(0x4); // possibly LPDIRECTSOUNDBUFFER primarySoundBuffer based on PBG code and later games
    LPDIRECTSOUNDBUFFER soundBuffers[SOUND_EFFECT_COUNT];
    LPDIRECTSOUNDBUFFER duplicateSoundBuffers[SOUND_EFFECT_COUNT];
    i32 unk408[SOUND_EFFECT_COUNT];
#if BUILT_SINCE(100)
    LPDIRECTSOUNDBUFFER initSoundBuffer;
#endif
    HWND gameWindow;
    CSoundManager *manager;
    DWORD backgroundMusicThreadId;
    HANDLE backgroundMusicThreadHandle;
    unreferenced_fields(0x4);
#if BUILT_SINCE(100)
    i32 soundBuffersToPlay[SFX_QUEUE_LENGTH];
#endif
    CStreamingSound *backgroundMusic;
    HANDLE backgroundMusicUpdateEvent;
    BOOL isLooping;
};

#if BUILT_SINCE(100)
ZUN_ASSERT_TYPE(SoundPlayer, 0x638, 4);
#else
ZUN_ASSERT_TYPE(SoundPlayer, 0x628, 4);
#endif

#if BUILT_SINCE(100)
extern SoundEffectData g_SoundBufferIdxVol[32];
extern const char *g_SFXList[26];
#else
extern SoundEffectData g_SoundBufferIdxVol[30];
extern const char *g_SFXList[24];
#endif
extern SoundPlayer g_SoundPlayer;
} // namespace th06
