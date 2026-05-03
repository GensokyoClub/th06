#pragma once

#include <windows.h>

#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
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

struct SoundBufferIdxVolume
{
    i32 bufferIdx;
    i16 volume;
    i16 unk;
};
ZUN_ASSERT_SIZE(SoundBufferIdxVolume, 0x8);

struct SoundPlayer
{
    SoundPlayer();

    ZunResult InitializeDSound(HWND window);
    ZunResult InitSoundBuffers();
    ZunResult Release(void);

    ZunResult LoadSound(i32 idx, char *path);
    static WAVEFORMATEX *GetWavFormatData(u8 *soundData, char *formatString, i32 *formatSize,
                                          u32 fileSizeExcludingFormat);
    void PlaySounds();
    void PlaySoundByIdx(SoundIdx idx, i32 unused);
    ZunResult PlayBGM(BOOL isLooping);
    void StopBGM();
    void FadeOut(f32 seconds)
    {
        CStreamingSound *bgm;

        if (this->backgroundMusic != NULL)
        {
            bgm = this->backgroundMusic;
            bgm->m_dwIsFadingOut = 1;
            bgm->m_dwCurFadeoutProgress = seconds * 60;
            bgm->m_dwTotalFadeout = bgm->m_dwCurFadeoutProgress;
        }
    }

    static DWORD __stdcall BackgroundMusicPlayerThread(LPVOID lpThreadParameter);

    ZunResult LoadWav(char *path);
    ZunResult LoadPos(char *path);

    LPDIRECTSOUND dsoundHdl;
    i32 unk4;
    LPDIRECTSOUNDBUFFER soundBuffers[128];
    LPDIRECTSOUNDBUFFER duplicateSoundBuffers[128];
    i32 unk408[128];
    LPDIRECTSOUNDBUFFER initSoundBuffer;
    HWND gameWindow;
    CSoundManager *manager;
    DWORD backgroundMusicThreadId;
    HANDLE backgroundMusicThreadHandle;
    i32 unk61c;
    i32 soundBuffersToPlay[3];
    CStreamingSound *backgroundMusic;
    HANDLE backgroundMusicUpdateEvent;
    BOOL isLooping;
};
ZUN_ASSERT_SIZE(SoundPlayer, 0x638);

DIFFABLE_EXTERN(SoundBufferIdxVolume, g_SoundBufferIdxVol[32]);
DIFFABLE_EXTERN(char, *g_SFXList[26]);
DIFFABLE_EXTERN(SoundPlayer, g_SoundPlayer)
}; // namespace th06
