#include "stdafx.h"

#include "SoundPlayer.hpp"

namespace th06
{
static DWORD WINAPI SoundPlayer_BackgroundMusicPlayerThread(LPVOID lpThreadParameter);

#define BACKGROUND_MUSIC_BUFFER_SIZE 0x8000
#define BACKGROUND_MUSIC_WAV_NUM_CHANNELS 2
#define BACKGROUND_MUSIC_WAV_BITS_PER_SAMPLE 16
#define BACKGROUND_MUSIC_WAV_BLOCK_ALIGN BACKGROUND_MUSIC_WAV_BITS_PER_SAMPLE / 8 * BACKGROUND_MUSIC_WAV_NUM_CHANNELS

enum SoundEffectFiles
{
    SFX_PLST00,
    SFX_ENEP00,
    SFX_PLDEAD00,
    SFX_POWER0,
    SFX_POWER1,
    SFX_TAN00,
    SFX_TAN01,
    SFX_TAN02,
    SFX_OK00,
    SFX_CANCEL00,
    SFX_SELECT00,
    SFX_GUN00,
    SFX_CAT00,
    SFX_LAZER00,
    SFX_LAZER01,
    SFX_ENEP01,
    SFX_NEP00,
    SFX_DAMAGE00,
    SFX_ITEM00,
    SFX_KIRA00,
    SFX_KIRA01,
    SFX_KIRA02,
    SFX_EXTEND,
    SFX_TIMEOUT,
#if BUILT_SINCE(100)
    SFX_GRAZE,
    SFX_POWERUP
#endif
};

SoundEffectData g_SoundEffects[] = {
#if BUILT_SINCE(100)
    {SFX_PLST00, -1500, 0},     {SFX_PLST00, -2000, 0},   {SFX_ENEP00, -1200, 5},  {SFX_ENEP00, -1400, 5},
    {SFX_PLDEAD00, -1000, 100}, {SFX_POWER0, -500, 100},  {SFX_POWER1, -500, 100}, {SFX_TAN00, -1700, 50},
    {SFX_TAN01, -1700, 50},     {SFX_TAN02, -1700, 50},   {SFX_OK00, -1000, 100},  {SFX_CANCEL00, -1000, 100},
    {SFX_SELECT00, -1900, 10},  {SFX_GUN00, -1200, 10},   {SFX_CAT00, -900, 100},  {SFX_TAN00, -1500, 50},
    {SFX_LAZER00, -900, 50},    {SFX_LAZER01, -900, 50},  {SFX_ENEP01, -600, 100}, {SFX_NEP00, -400, 100},
    {SFX_DAMAGE00, -1100, 0},   {SFX_ITEM00, -900, 0},    {SFX_TAN00, -1800, 20},  {SFX_TAN01, -1800, 20},
    {SFX_TAN02, -1800, 20},     {SFX_KIRA00, -300, 50},   {SFX_KIRA01, -600, 50},  {SFX_KIRA02, -800, 50},
    {SFX_EXTEND, -100, 140},    {SFX_TIMEOUT, -500, 100}, {SFX_GRAZE, -1000, 20},  {SFX_POWERUP, -1000, 90},
#else
    {SFX_PLST00, -2500, 0},     {SFX_PLST00, -3000, 0},   {SFX_ENEP00, -1200, 5},  {SFX_ENEP00, -1700, 5},
    {SFX_PLDEAD00, -1500, 100}, {SFX_POWER0, -300, 100},  {SFX_POWER1, -300, 100}, {SFX_TAN00, -1500, 50},
    {SFX_TAN01, -1500, 50},     {SFX_TAN02, -1500, 50},   {SFX_OK00, -800, 100},   {SFX_CANCEL00, -1400, 100},
    {SFX_SELECT00, -1700, 10},  {SFX_GUN00, -1500, 10},   {SFX_CAT00, -800, 100},  {SFX_TAN00, -1000, 50},
    {SFX_LAZER00, -700, 50},    {SFX_LAZER01, -700, 50},  {SFX_ENEP01, -500, 100}, {SFX_NEP00, -200, 100},
    {SFX_DAMAGE00, -1600, 0},   {SFX_ITEM00, -900, 0},    {SFX_TAN00, -2200, 20},  {SFX_TAN01, -2200, 20},
    {SFX_TAN02, -2200, 20},     {SFX_KIRA00, -500, 50},   {SFX_KIRA01, -800, 50},  {SFX_KIRA02, -1200, 50},
    {SFX_EXTEND, -300, 140},    {SFX_TIMEOUT, -500, 100},
#endif
};
const char *g_SoundEffectFiles[] = {
    "data/wav/plst00.wav", "data/wav/enep00.wav",   "data/wav/pldead00.wav", "data/wav/power0.wav",
    "data/wav/power1.wav", "data/wav/tan00.wav",    "data/wav/tan01.wav",    "data/wav/tan02.wav",
    "data/wav/ok00.wav",   "data/wav/cancel00.wav", "data/wav/select00.wav", "data/wav/gun00.wav",
    "data/wav/cat00.wav",  "data/wav/lazer00.wav",  "data/wav/lazer01.wav",  "data/wav/enep01.wav",
    "data/wav/nep00.wav",  "data/wav/damage00.wav", "data/wav/item00.wav",   "data/wav/kira00.wav",
    "data/wav/kira01.wav", "data/wav/kira02.wav",   "data/wav/extend.wav",   "data/wav/timeout.wav",
#if BUILT_SINCE(100)
    "data/wav/graze.wav",  "data/wav/powerup.wav",
#endif
};

SoundPlayer g_SoundPlayer;

#pragma var_order(bufDesc, audioBuffer2Start, audioBuffer2Len, audioBuffer1Len, audioBuffer1Start, wavFormat)
ZunResult SoundPlayer::InitializeDSound(HWND gameWindow)
{
#if BUILT_SINCE(100)
    DSBUFFERDESC bufDesc;
    WAVEFORMATEX wavFormat;
    LPVOID audioBuffer1Start;
    DWORD audioBuffer1Len;
    LPVOID audioBuffer2Start;
    DWORD audioBuffer2Len;
#endif

    this->manager = new CSoundManager();
    if (FAILED(this->manager->Initialize(gameWindow, 2, 2, 44100, 16)))
    {
        g_GameErrorContext.Log(TH_ERR_SOUNDPLAYER_FAILED_TO_INITIALIZE_OBJECT);
        SAFE_DELETE(this->manager);
        return ZUN_ERROR;
    }
    this->dsoundHdl = this->manager->GetDirectSound();

#if BUILT_SINCE(100)
    this->backgroundMusicThreadHandle = NULL;
    memset(&bufDesc, 0, sizeof(DSBUFFERDESC));
    bufDesc.dwSize = sizeof(DSBUFFERDESC);
    bufDesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE;
    bufDesc.dwBufferBytes = BACKGROUND_MUSIC_BUFFER_SIZE;
    memset(&wavFormat, 0, sizeof(WAVEFORMATEX));
    wavFormat.cbSize = 0;
    wavFormat.wFormatTag = WAVE_FORMAT_PCM;
    wavFormat.nChannels = BACKGROUND_MUSIC_WAV_NUM_CHANNELS;
    wavFormat.nSamplesPerSec = 44100;
    wavFormat.nAvgBytesPerSec = 176400;
    wavFormat.nBlockAlign = BACKGROUND_MUSIC_WAV_BLOCK_ALIGN;
    wavFormat.wBitsPerSample = BACKGROUND_MUSIC_WAV_BITS_PER_SAMPLE;
    bufDesc.lpwfxFormat = &wavFormat;
    if (FAILED(this->dsoundHdl->CreateSoundBuffer(&bufDesc, &this->initSoundBuffer, NULL)))
    {
        return ZUN_ERROR;
    }
    if (FAILED(this->initSoundBuffer->Lock(0, BACKGROUND_MUSIC_BUFFER_SIZE, &audioBuffer1Start, &audioBuffer1Len,
                                           &audioBuffer2Start, &audioBuffer2Len, 0)))
    {
        return ZUN_ERROR;
    }

    memset(audioBuffer1Start, 0, BACKGROUND_MUSIC_BUFFER_SIZE);
    this->initSoundBuffer->Unlock(audioBuffer1Start, audioBuffer1Len, audioBuffer2Start, audioBuffer2Len);
    this->initSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
#else
    // 0.08 creates one streaming thread here and keeps it until Release().
    this->backgroundMusicThreadHandle =
        CreateThread(NULL, 0, SoundPlayer_BackgroundMusicPlayerThread, gameWindow, 0, &this->backgroundMusicThreadId);
    this->backgroundMusicUpdateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
    /* 4 times per second */
    SetTimer(gameWindow, 0, 250, NULL);
    this->gameWindow = gameWindow;
    g_GameErrorContext.Log(TH_DBG_SOUNDPLAYER_INIT_SUCCESS);
    return ZUN_SUCCESS;
}

ZunResult SoundPlayer::Release()
{
#if BUILT_SINCE(100)
    if (this->manager == NULL)
    {
        return ZUN_SUCCESS;
    }
    for (i32 i = 0; i < SOUND_EFFECT_COUNT; i++)
    {
#if BUILT_SINCE(101)
        SAFE_RELEASE(this->duplicateSoundBuffers[i]);
#endif
        // 1.00 never releases the duplicated buffers.
        SAFE_RELEASE(this->soundBuffers[i]);
    }
    KillTimer(this->gameWindow, 1);
#if BUILT_SINCE(101)
    StopBGM();
#else
    // 1.00 only stops the streaming thread here, and never releases initSoundBuffer.
    if (this->backgroundMusicThreadHandle != NULL)
    {
        PostThreadMessage(this->backgroundMusicThreadId, WM_QUIT, 0, 0);
        WaitForSingleObject(this->backgroundMusicThreadHandle, INFINITE);
        CloseHandle(this->backgroundMusicThreadHandle);
        CloseHandle(this->backgroundMusicUpdateEvent);
        this->backgroundMusicThreadHandle = NULL;
    }
#endif
    this->dsoundHdl = NULL;
    this->initSoundBuffer->Stop();
#if BUILT_SINCE(101)
    SAFE_RELEASE(this->initSoundBuffer);
#endif
    SAFE_DELETE(this->backgroundMusic);
    SAFE_DELETE(this->manager);
    return ZUN_SUCCESS;
#else
    if (this->manager == NULL)
    {
        return ZUN_SUCCESS;
    }
    for (i32 i = 0; i < SOUND_EFFECT_COUNT; i++)
    {
        SAFE_RELEASE(this->soundBuffers[i]);
    }
    KillTimer(this->gameWindow, 1);
    SAFE_DELETE(this->backgroundMusic);
    SAFE_DELETE(this->manager);
    PostThreadMessage(this->backgroundMusicThreadId, WM_QUIT, 0, 0);
    WaitForSingleObject(this->backgroundMusicThreadHandle, INFINITE);
    CloseHandle(this->backgroundMusicThreadHandle);
    CloseHandle(this->backgroundMusicUpdateEvent);
    this->dsoundHdl = NULL;
    return ZUN_SUCCESS;
#endif
}

static WAVEFORMATEX *GetWavFormatData(u8 *soundData, const char *formatString, i32 *formatSize,
                                      u32 fileSizeExcludingFormat)
{
    while (fileSizeExcludingFormat > 0)
    {
        *formatSize = *(i32 *)(soundData + 4);
        if (strncmp((char *)soundData, formatString, 4) == 0)
        {
            return (WAVEFORMATEX *)(soundData + 8);
        }
        fileSizeExcludingFormat -= (*formatSize + 8);
        soundData += *formatSize + 8;
    }
    return NULL;
}

#pragma var_order(sFDCursor, dsBuffer, wavDataPtr, formatSize, audioPtr2, audioSize2, audioSize1, audioPtr1,           \
                  soundFileData, wavData, fileSize)
ZunResult SoundPlayer::LoadSound(i32 idx, const char *path)
{
    u8 *soundFileData;
    u8 *sFDCursor;
    i32 fileSize;
    WAVEFORMATEX *wavDataPtr;
    WAVEFORMATEX *audioPtr1;
    WAVEFORMATEX *audioPtr2;
    DWORD audioSize1;
    DWORD audioSize2;
    WAVEFORMATEX wavData;
    i32 formatSize;
    DSBUFFERDESC dsBuffer;

    if (this->manager == NULL)
    {
        return ZUN_SUCCESS;
    }
    SAFE_RELEASE(this->soundBuffers[idx]);
    soundFileData = FileSystem::OpenPath(path);
    sFDCursor = soundFileData;
    if (sFDCursor == NULL)
    {
        return ZUN_ERROR;
    }
    if (strncmp((char *)sFDCursor, "RIFF", 4))
    {
        g_GameErrorContext.Log(TH_ERR_NOT_A_WAV_FILE_A, path);
        ZUN_FREE(soundFileData);
        return ZUN_ERROR;
    }
    sFDCursor += 4;

    fileSize = *(i32 *)sFDCursor;
    sFDCursor += 4;

    if (strncmp((char *)sFDCursor, "WAVE", 4))
    {
        g_GameErrorContext.Log(TH_ERR_NOT_A_WAV_FILE_B, path);
        ZUN_FREE(soundFileData);
        return ZUN_ERROR;
    }
    sFDCursor += 4;
    wavDataPtr = GetWavFormatData(sFDCursor, "fmt ", &formatSize, fileSize - 12);
    if (wavDataPtr == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_NOT_A_WAV_FILE_B, path);
        ZUN_FREE(soundFileData);
        return ZUN_ERROR;
    }
    wavData = *wavDataPtr;

    wavDataPtr = GetWavFormatData(sFDCursor, "data", &formatSize, fileSize - 12);
    if (wavDataPtr == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_NOT_A_WAV_FILE_B, path);
        ZUN_FREE(soundFileData);
        return ZUN_ERROR;
    }
    memset(&dsBuffer, 0, sizeof(dsBuffer));
    dsBuffer.dwSize = sizeof(dsBuffer);
#if BUILT_SINCE(100)
    dsBuffer.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;
#else
    dsBuffer.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_LOCDEFER;
#endif
    dsBuffer.dwBufferBytes = formatSize;
    dsBuffer.lpwfxFormat = &wavData;
    if (FAILED(this->dsoundHdl->CreateSoundBuffer(&dsBuffer, &this->soundBuffers[idx], NULL)))
    {
        ZUN_FREE(soundFileData);
        return ZUN_ERROR;
    }
    if (FAILED(this->soundBuffers[idx]->Lock(0, formatSize, (LPVOID *)&audioPtr1, (LPDWORD)&audioSize1,
                                             (LPVOID *)&audioPtr2, (LPDWORD)&audioSize2, NULL)))
    {
        ZUN_FREE(soundFileData);
        return ZUN_ERROR;
    }
    memcpy(audioPtr1, wavDataPtr, audioSize1);
    if (audioSize2 != 0)
    {
        memcpy(audioPtr2, (i8 *)wavDataPtr + audioSize1, audioSize2);
    }
    this->soundBuffers[idx]->Unlock((LPVOID *)audioPtr1, audioSize1, (LPVOID *)audioPtr2, audioSize2);
    ZUN_FREE(soundFileData);
    return ZUN_SUCCESS;
}

#pragma var_order(notifySize, waveFile, res, numSamplesPerSec, blockAlign, curTime, startTime, waitTime, curTime2,     \
                  startTime2, waitTime2)
ZunResult SoundPlayer::LoadWav(char *path)
{
    HRESULT res;
    CWaveFile waveFile;
#if BUILT_SINCE(101)
    DWORD startTime;
    DWORD curTime;
    u32 waitTime;
#endif
    u32 blockAlign;
    u32 numSamplesPerSec;
    u32 notifySize;
#if BUILT_SINCE(101)
    DWORD startTime2;
    DWORD curTime2;
    u32 waitTime2;
#endif

    if (this->manager == NULL)
    {
        return ZUN_ERROR;
    }
    if (!g_Supervisor.cfg.playSounds)
    {
        return ZUN_ERROR;
    }
    if (this->dsoundHdl == NULL)
    {
        return ZUN_ERROR;
    }
    this->StopBGM();
#if BUILT_BEFORE(100)
    SAFE_DELETE(this->backgroundMusic);
#endif
    DebugPrint("load BGM\n");
    res = waveFile.Open(path, NULL, WAVEFILE_READ);
    if (FAILED(res))
    {
        DebugPrint("error : wav file load error %s\n", path);
        waveFile.Close();
        return ZUN_ERROR;
    }
    if (waveFile.GetSize() == 0)
    {
        waveFile.Close();
        return ZUN_ERROR;
    }
#if BUILT_SINCE(101)
    // Sleep 100ms? 1.00 has neither of these waits.
    startTime = timeGetTime();
    curTime = startTime;
    waitTime = 100;
    while (curTime < startTime + waitTime && curTime >= startTime)
    {
        curTime = timeGetTime();
    }
#endif
    waveFile.Close();
    blockAlign = waveFile.m_pwfx->nBlockAlign;
    numSamplesPerSec = waveFile.m_pwfx->nSamplesPerSec;
    notifySize =
        numSamplesPerSec * BACKGROUND_MUSIC_STREAM_SECONDS * blockAlign / BACKGROUND_MUSIC_STREAM_NOTIFICATIONS;
    notifySize -= (notifySize % blockAlign);
#if BUILT_SINCE(100)
#if BUILT_SINCE(101)
    this->backgroundMusicUpdateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
    this->backgroundMusicThreadHandle = CreateThread(NULL, 0, SoundPlayer_BackgroundMusicPlayerThread,
                                                     g_Supervisor.hwndGameWindow, 0, &this->backgroundMusicThreadId);
#if BUILT_BEFORE(101)
    // 1.00 starts the streaming thread before creating the event it waits on.
    this->backgroundMusicUpdateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
#endif
    res = this->manager->CreateStreaming(
        &this->backgroundMusic, path, DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY, GUID_NULL,
        BACKGROUND_MUSIC_STREAM_NOTIFICATIONS, notifySize, this->backgroundMusicUpdateEvent);
    if (FAILED(res))
    {
#if BUILT_SINCE(100)
        DebugPrint(TH_ERR_SOUNDPLAYER_FAILED_TO_CREATE_BGM_SOUND_BUFFER);
#else
        g_GameErrorContext.Log(TH_ERR_SOUNDPLAYER_FAILED_TO_CREATE_BGM_SOUND_BUFFER);
#endif
        return ZUN_ERROR;
    }
    DebugPrint("comp\n");
#if BUILT_SINCE(101)
    startTime2 = timeGetTime();
    curTime2 = startTime2;
    waitTime2 = 100;
    while (curTime2 < startTime2 + waitTime2 && curTime2 >= startTime2)
    {
        curTime2 = timeGetTime();
    }
#endif
    return ZUN_SUCCESS;
}

struct PosFile
{
    i32 loopStart;
    i32 loopEnd;
};

#pragma var_order(fileData, bgmFile)
ZunResult SoundPlayer::LoadPos(const char *path)
{
    PosFile *fileData;
    CWaveFile *bgmFile;

    if (this->manager == NULL)
    {
        return ZUN_ERROR;
    }
    if (!g_Supervisor.cfg.playSounds)
    {
        return ZUN_ERROR;
    }
    if (this->backgroundMusic == NULL)
    {
        return ZUN_ERROR;
    }

    fileData = (PosFile *)FileSystem::OpenPath(path);
    if (fileData == NULL)
    {
        return ZUN_ERROR;
    }
    bgmFile = this->backgroundMusic->GetWaveFile();
    bgmFile->SetLoopPoints(fileData->loopStart * 4, fileData->loopEnd * 4);
    ZUN_FREE(fileData);
    return ZUN_SUCCESS;
}

#pragma var_order(buffer, res)
ZunResult SoundPlayer::PlayBGM(BOOL isLooping)
{
    LPDIRECTSOUNDBUFFER buffer;
    HRESULT res;

    DebugPrint("play BGM\n");
    if (this->backgroundMusic == NULL)
    {
        return ZUN_ERROR;
    }
    res = this->backgroundMusic->Reset();
    if (FAILED(res))
    {
        return ZUN_ERROR;
    }

    buffer = this->backgroundMusic->GetBuffer(0);
    res = this->backgroundMusic->FillBufferWithSound(buffer, isLooping);
    if (FAILED(res))
    {
        return ZUN_ERROR;
    }
    res = this->backgroundMusic->Play(0, DSBPLAY_LOOPING);
    if (FAILED(res))
    {
        return ZUN_ERROR;
    }
    DebugPrint("comp\n");
    this->isLooping = isLooping;
    return ZUN_SUCCESS;
}

void SoundPlayer::StopBGM()
{
    if (this->backgroundMusic != NULL)
    {
        this->backgroundMusic->Stop();
#if BUILT_SINCE(100)
        if (this->backgroundMusicThreadHandle != NULL)
        {
            PostThreadMessage(this->backgroundMusicThreadId, WM_QUIT, 0, 0);
            DebugPrint("stop m_dwNotifyThreadID\n");
            WaitForSingleObject(this->backgroundMusicThreadHandle, INFINITE);
            DebugPrint("comp\n");
            CloseHandle(this->backgroundMusicThreadHandle);
            CloseHandle(this->backgroundMusicUpdateEvent);
            this->backgroundMusicThreadHandle = NULL;
        }
        SAFE_DELETE(this->backgroundMusic);
        DebugPrint("stop BGM\n");
#else
        this->backgroundMusic->Reset();
#endif
    }
}

ZunResult SoundPlayer::InitSoundBuffers()
{
    if (this->manager == NULL)
    {
        return ZUN_ERROR;
    }
    else if (this->dsoundHdl == NULL)
    {
        return ZUN_SUCCESS;
    }
    else
    {
        i32 i;
#if BUILT_SINCE(100)
        for (i = 0; i < SFX_QUEUE_LENGTH; i++)
        {
            this->soundBuffersToPlay[i] = NO_SOUND;
        }
#endif
        for (i = 0; i < ARRAY_SIZE_SIGNED(g_SoundEffectFiles); i++)
        {
            if (this->LoadSound(i, g_SoundEffectFiles[i]) != ZUN_SUCCESS)
            {
                g_GameErrorContext.Log(TH_ERR_SOUNDPLAYER_FAILED_TO_LOAD_SOUND_FILE, g_SoundEffectFiles[i]);
                return ZUN_ERROR;
            }
        }
        for (i = 0; i < ARRAY_SIZE(g_SoundEffects); i++)
        {
            this->dsoundHdl->DuplicateSoundBuffer(this->soundBuffers[g_SoundEffects[i].fileIdx],
                                                  &this->duplicateSoundBuffers[i]);
            this->duplicateSoundBuffers[i]->SetCurrentPosition(0);
            this->duplicateSoundBuffers[i]->SetVolume(g_SoundEffects[i].volume);
        }
    }
    return ZUN_SUCCESS;
}

#pragma var_order(i, unk)
void SoundPlayer::PlaySoundId(i32 sound, i32 unused)
{
#if BUILT_SINCE(100)
    i32 unk;
    i32 i;

    unk = g_SoundEffects[sound].unk;
    for (i = 0; i < SFX_QUEUE_LENGTH; i++)
    {
        if (!IS_VALID_SFX(this->soundBuffersToPlay[i]))
        {
            break;
        }
        if (this->soundBuffersToPlay[i] == sound)
        {
            return;
        }
    }
    if (i >= SFX_QUEUE_LENGTH)
    {
        return;
    }
    this->soundBuffersToPlay[i] = sound;
    this->unk408[sound] = unk;
#else
    if (this->manager == NULL)
    {
        return;
    }
    if (this->duplicateSoundBuffers[sound] == NULL)
    {
        return;
    }
    if (!g_Supervisor.cfg.playSounds)
    {
        return;
    }
    this->unk408[sound] = g_SoundEffects[sound].unk;
    this->duplicateSoundBuffers[sound]->Stop();
    this->duplicateSoundBuffers[sound]->SetCurrentPosition(0);
    this->duplicateSoundBuffers[sound]->Play(0, 0, 0);
#endif
}

#if BUILT_SINCE(100)
void SoundPlayer::PlaySounds()
{
    if (this->manager == NULL)
    {
        return;
    }
    if (!g_Supervisor.cfg.playSounds)
    {
        return;
    }
    for (i32 i = 0; i < SFX_QUEUE_LENGTH; i++)
    {
        if (!IS_VALID_SFX(this->soundBuffersToPlay[i]))
        {
            break;
        }
        i32 sndBufIdx = this->soundBuffersToPlay[i];
        this->soundBuffersToPlay[i] = NO_SOUND;
        if (this->duplicateSoundBuffers[sndBufIdx] == NULL)
        {
            continue;
        }
        this->duplicateSoundBuffers[sndBufIdx]->Stop();
        this->duplicateSoundBuffers[sndBufIdx]->SetCurrentPosition(0);
        this->duplicateSoundBuffers[sndBufIdx]->Play(0, 0, 0);
    }
}

#endif

#pragma var_order(msg, looped, lpThreadParameterCopy, waitObj, res, stopped)
static DWORD WINAPI SoundPlayer_BackgroundMusicPlayerThread(LPVOID lpThreadParameter)
{
    DWORD waitObj;
    MSG msg;
    BOOL stopped;
    BOOL looped;
    LPVOID lpThreadParameterCopy;
    HRESULT res;

    lpThreadParameterCopy = lpThreadParameter;
    stopped = false;
    looped = true;
    while (!stopped)
    {
        waitObj =
            MsgWaitForMultipleObjects(1, &g_SoundPlayer.backgroundMusicUpdateEvent, FALSE, INFINITE, QS_ALLEVENTS);
        if (g_SoundPlayer.backgroundMusic == NULL)
        {
#if BUILT_SINCE(100)
            stopped = true;
#else
            break;
#endif
        }
        switch (waitObj)
        {
        case 0:
            if (g_SoundPlayer.backgroundMusic != NULL)
            {
                res = g_SoundPlayer.backgroundMusic->HandleWaveStreamNotification(looped);
#if BUILT_BEFORE(100)
                if (FAILED(res))
                {
                    stopped = true;
                }
#endif
            }
            break;
        case 1:
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    stopped = true;
                }
            }
            break;
        }
    }
    return 0;
}
} // namespace th06
