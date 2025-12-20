#include "SoundPlayer.hpp"

#include "FileSystem.hpp"
#include "Supervisor.hpp"
#include "i18n.hpp"
#include "utils.hpp"

namespace th06
{

#define BACKGROUND_MUSIC_BUFFER_SIZE 0x8000
#define BACKGROUND_MUSIC_WAV_NUM_CHANNELS 2
#define BACKGROUND_MUSIC_WAV_BITS_PER_SAMPLE 16
#define BACKGROUND_MUSIC_WAV_BLOCK_ALIGN BACKGROUND_MUSIC_WAV_BITS_PER_SAMPLE / 8 * BACKGROUND_MUSIC_WAV_NUM_CHANNELS

DIFFABLE_STATIC_ARRAY_ASSIGN(SoundBufferIdxVolume, 32, g_SoundBufferIdxVol) = {
    {0, -1500, 0},   {0, -2000, 0},   {1, -1200, 5},   {1, -1400, 5},  {2, -1000, 100}, {3, -500, 100},
    {4, -500, 100},  {5, -1700, 50},  {6, -1700, 50},  {7, -1700, 50}, {8, -1000, 100}, {9, -1000, 100},
    {10, -1900, 10}, {11, -1200, 10}, {12, -900, 100}, {5, -1500, 50}, {13, -900, 50},  {14, -900, 50},
    {15, -600, 100}, {16, -400, 100}, {17, -1100, 0},  {18, -900, 0},  {5, -1800, 20},  {6, -1800, 20},
    {7, -1800, 20},  {19, -300, 50},  {20, -600, 50},  {21, -800, 50}, {22, -100, 140}, {23, -500, 100},
    {24, -1000, 20}, {25, -1000, 90},
};
DIFFABLE_STATIC_ARRAY_ASSIGN(char *, 26, g_SFXList) = {
    "data/wav/plst00.wav", "data/wav/enep00.wav",   "data/wav/pldead00.wav", "data/wav/power0.wav",
    "data/wav/power1.wav", "data/wav/tan00.wav",    "data/wav/tan01.wav",    "data/wav/tan02.wav",
    "data/wav/ok00.wav",   "data/wav/cancel00.wav", "data/wav/select00.wav", "data/wav/gun00.wav",
    "data/wav/cat00.wav",  "data/wav/lazer00.wav",  "data/wav/lazer01.wav",  "data/wav/enep01.wav",
    "data/wav/nep00.wav",  "data/wav/damage00.wav", "data/wav/item00.wav",   "data/wav/kira00.wav",
    "data/wav/kira01.wav", "data/wav/kira02.wav",   "data/wav/extend.wav",   "data/wav/timeout.wav",
    "data/wav/graze.wav",  "data/wav/powerup.wav",
};
DIFFABLE_STATIC(SoundPlayer, g_SoundPlayer)

SoundPlayer::SoundPlayer()
{
    memset(this, 0, sizeof(SoundPlayer));
    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->unk408); i++)
    {
        this->unk408[i] = -1;
    }
}

#pragma var_order(bufDesc, audioBuffer2Start, audioBuffer2Len, audioBuffer1Len, audioBuffer1Start, wavFormat)
ZunResult SoundPlayer::InitializeDSound(HWND gameWindow)
{
    DSBUFFERDESC bufDesc;
    tWAVEFORMATEX wavFormat;
    LPVOID audioBuffer1Start;
    DWORD audioBuffer1Len;
    LPVOID audioBuffer2Start;
    DWORD audioBuffer2Len;

    this->manager = new CSoundManager();
    if (this->manager->Initialize(gameWindow, 2, 2, 44100, 16) < ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_SOUNDPLAYER_FAILED_TO_INITIALIZE_OBJECT);
        if (this->manager != NULL)
        {
            delete this->manager;
            this->manager = NULL;
        }
        return ZUN_ERROR;
    }

    this->dsoundHdl = this->manager->GetDirectSound();
    this->backgroundMusicThreadHandle = NULL;
    memset(&bufDesc, 0, sizeof(DSBUFFERDESC));
    bufDesc.dwSize = sizeof(DSBUFFERDESC);
    bufDesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE;
    bufDesc.dwBufferBytes = BACKGROUND_MUSIC_BUFFER_SIZE;
    memset(&wavFormat, 0, sizeof(tWAVEFORMATEX));
    wavFormat.cbSize = 0;
    wavFormat.wFormatTag = WAVE_FORMAT_PCM;
    wavFormat.nChannels = BACKGROUND_MUSIC_WAV_NUM_CHANNELS;
    wavFormat.nSamplesPerSec = 44100;
    wavFormat.nAvgBytesPerSec = 176400;
    wavFormat.nBlockAlign = BACKGROUND_MUSIC_WAV_BLOCK_ALIGN;
    wavFormat.wBitsPerSample = BACKGROUND_MUSIC_WAV_BITS_PER_SAMPLE;
    bufDesc.lpwfxFormat = &wavFormat;
    if (this->dsoundHdl->CreateSoundBuffer(&bufDesc, &this->initSoundBuffer, NULL) < ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (this->initSoundBuffer->Lock(0, BACKGROUND_MUSIC_BUFFER_SIZE, &audioBuffer1Start, &audioBuffer1Len,
                                    &audioBuffer2Start, &audioBuffer2Len, 0) < ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    memset(audioBuffer1Start, 0, BACKGROUND_MUSIC_BUFFER_SIZE);
    this->initSoundBuffer->Unlock(audioBuffer1Start, audioBuffer1Len, audioBuffer2Start, audioBuffer2Len);
    this->initSoundBuffer->Play(0, 0, 1);
    /* 4 times per second */
    SetTimer(gameWindow, 0, 250, NULL);
    this->gameWindow = gameWindow;
    g_GameErrorContext.Log(TH_DBG_SOUNDPLAYER_INIT_SUCCESS);
    return ZUN_SUCCESS;
}

ZunResult SoundPlayer::Release(void)
{
    i32 i;

    if (this->manager == NULL)
    {
        return ZUN_SUCCESS;
    }
    for (i = 0; i < 0x80; i++)
    {
        if (this->duplicateSoundBuffers[i] != NULL)
        {
            this->duplicateSoundBuffers[i]->Release();
            this->duplicateSoundBuffers[i] = NULL;
        }
        if (this->soundBuffers[i] != NULL)
        {
            this->soundBuffers[i]->Release();
            this->soundBuffers[i] = NULL;
        }
    }
    KillTimer(this->gameWindow, 1);
    StopBGM();
    this->dsoundHdl = NULL;
    this->initSoundBuffer->Stop();
    if (this->initSoundBuffer != NULL)
    {
        this->initSoundBuffer->Release();
        this->initSoundBuffer = NULL;
    }
    if (this->backgroundMusic != NULL)
    {
        delete this->backgroundMusic;
        this->backgroundMusic = NULL;
    }
    if (this->manager != NULL)
    {
        delete this->manager;
        this->manager = NULL;
    }
    return ZUN_SUCCESS;
}

void SoundPlayer::StopBGM()
{
    if (this->backgroundMusic != NULL)
    {
        this->backgroundMusic->Stop();
        if (this->backgroundMusicThreadHandle != NULL)
        {
            PostThreadMessageA(this->backgroundMusicThreadId, WM_QUIT, 0, 0);
            utils::DebugPrint2("stop m_dwNotifyThreadID\n");
            WaitForSingleObject(this->backgroundMusicThreadHandle, INFINITE);
            utils::DebugPrint2("comp\n");
            CloseHandle(this->backgroundMusicThreadHandle);
            CloseHandle(this->backgroundMusicUpdateEvent);
            this->backgroundMusicThreadHandle = NULL;
        }
        if (this->backgroundMusic != NULL)
        {
            delete this->backgroundMusic;
            this->backgroundMusic = NULL;
        }
        utils::DebugPrint2("stop BGM\n");
    }
    return;
}

#pragma var_order(notifySize, waveFile, res, numSamplesPerSec, blockAlign, curTime, startTime, waitTime, curTime2,     \
                  startTime2, waitTime2)
ZunResult SoundPlayer::LoadWav(char *path)
{
    HRESULT res;
    CWaveFile waveFile;
    DWORD startTime;
    DWORD curTime;
    u32 waitTime;
    u32 blockAlign;
    u32 numSamplesPerSec;
    u32 notifySize;
    DWORD startTime2;
    DWORD curTime2;
    u32 waitTime2;

    if (this->manager == NULL)
    {
        return ZUN_ERROR;
    }
    if (g_Supervisor.cfg.playSounds == 0)
    {
        return ZUN_ERROR;
    }
    if (this->dsoundHdl == NULL)
    {
        return ZUN_ERROR;
    }
    this->StopBGM();
    utils::DebugPrint2("load BGM\n");
    res = waveFile.Open(path, NULL, WAVEFILE_READ);
    if (FAILED(res))
    {
        utils::DebugPrint2("error : wav file load error %s\n", path);
        waveFile.Close();
        return ZUN_ERROR;
    }
    if (waveFile.GetSize() == 0)
    {
        waveFile.Close();
        return ZUN_ERROR;
    }
    // Sleep 100ms?
    startTime = timeGetTime();
    curTime = startTime;
    waitTime = 100;
    while (curTime < startTime + waitTime && curTime >= startTime)
    {
        curTime = timeGetTime();
    }
    waveFile.Close();
    blockAlign = waveFile.m_pwfx->nBlockAlign;
    numSamplesPerSec = waveFile.m_pwfx->nSamplesPerSec;
    notifySize = numSamplesPerSec * 2 * blockAlign >> 2;
    notifySize -= (notifySize % blockAlign);
    this->backgroundMusicUpdateEvent = CreateEventA(NULL, 0, 0, NULL);
    this->backgroundMusicThreadHandle = CreateThread(NULL, 0, SoundPlayer::BackgroundMusicPlayerThread,
                                                     g_Supervisor.hwndGameWindow, 0, &this->backgroundMusicThreadId);
    res = this->manager->CreateStreaming(&this->backgroundMusic, path,
                                         DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY, GUID_NULL, 4,
                                         notifySize, this->backgroundMusicUpdateEvent);
    if (FAILED(res))
    {
        utils::DebugPrint2(TH_ERR_SOUNDPLAYER_FAILED_TO_CREATE_BGM_SOUND_BUFFER);
        return ZUN_ERROR;
    }
    utils::DebugPrint2("comp\n");
    startTime2 = timeGetTime();
    curTime2 = startTime2;
    waitTime2 = 100;
    while (curTime2 < startTime2 + waitTime2 && curTime2 >= startTime2)
    {
        curTime2 = timeGetTime();
    }
    return ZUN_SUCCESS;
}

#pragma var_order(fileData, bgmFile, loopEnd, loopStart)
ZunResult SoundPlayer::LoadPos(char *path)
{
    u8 *fileData;
    CWaveFile *bgmFile;
    i32 loopEnd;
    i32 loopStart;

    if (this->manager == NULL)
    {
        return ZUN_ERROR;
    }
    if (g_Supervisor.cfg.playSounds == NULL)
    {
        return ZUN_ERROR;
    }
    if (this->backgroundMusic == NULL)
    {
        return ZUN_ERROR;
    }

    fileData = FileSystem::OpenPath(path, 0);
    if (fileData == NULL)
    {
        return ZUN_ERROR;
    }
    bgmFile = this->backgroundMusic->m_pWaveFile;
    loopEnd = *(i32 *)(fileData + 4) * 4;
    loopStart = *(i32 *)(fileData) * 4;
    bgmFile->m_loopStartPoint = loopStart;
    bgmFile->m_loopEndPoint = loopEnd;
    free(fileData);
    return ZUN_SUCCESS;
}

ZunResult SoundPlayer::InitSoundBuffers()
{
    i32 idx;
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
        for (idx = 0; idx < 3; idx++)
        {
            this->soundBuffersToPlay[idx] = -1;
        }
        for (idx = 0; idx < ARRAY_SIZE_SIGNED(g_SFXList); idx++)
        {
            if (this->LoadSound(idx, g_SFXList[idx]) != ZUN_SUCCESS)
            {
                g_GameErrorContext.Log(TH_ERR_SOUNDPLAYER_FAILED_TO_LOAD_SOUND_FILE, g_SFXList[idx]);
                return ZUN_ERROR;
            }
        }
        for (idx = 0; idx < ARRAY_SIZE(g_SoundBufferIdxVol); idx++)
        {
            this->dsoundHdl->DuplicateSoundBuffer(this->soundBuffers[g_SoundBufferIdxVol[idx].bufferIdx],
                                                  &this->duplicateSoundBuffers[idx]);
            this->duplicateSoundBuffers[idx]->SetCurrentPosition(0);
            this->duplicateSoundBuffers[idx]->SetVolume(g_SoundBufferIdxVol[idx].volume);
        }
    }
    return ZUN_SUCCESS;
}

WAVEFORMATEX *SoundPlayer::GetWavFormatData(u8 *soundData, char *formatString, i32 *formatSize,
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
ZunResult SoundPlayer::LoadSound(i32 idx, char *path)
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
    if (this->soundBuffers[idx] != NULL)
    {
        this->soundBuffers[idx]->Release();
        this->soundBuffers[idx] = NULL;
    }
    soundFileData = (u8 *)FileSystem::OpenPath(path, 0);
    sFDCursor = soundFileData;
    if (sFDCursor == NULL)
    {
        return ZUN_ERROR;
    }
    if (strncmp((char *)sFDCursor, "RIFF", 4))
    {
        g_GameErrorContext.Log(TH_ERR_NOT_A_WAV_FILE, path);
        free(soundFileData);
        return ZUN_ERROR;
    }
    sFDCursor += 4;

    fileSize = *(i32 *)sFDCursor;
    sFDCursor += 4;

    if (strncmp((char *)sFDCursor, "WAVE", 4))
    {
        g_GameErrorContext.Log(TH_ERR_NOT_A_WAV_FILE, path);
        free(soundFileData);
        return ZUN_ERROR;
    }
    sFDCursor += 4;
    wavDataPtr = GetWavFormatData(sFDCursor, "fmt ", &formatSize, fileSize - 12);
    if (wavDataPtr == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_NOT_A_WAV_FILE, path);
        free(soundFileData);
        return ZUN_ERROR;
    }
    wavData = *wavDataPtr;

    wavDataPtr = GetWavFormatData(sFDCursor, "data", &formatSize, fileSize - 12);
    if (wavDataPtr == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_NOT_A_WAV_FILE, path);
        free(soundFileData);
        return ZUN_ERROR;
    }
    memset(&dsBuffer, 0, sizeof(dsBuffer));
    dsBuffer.dwSize = sizeof(dsBuffer);
    dsBuffer.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;
    dsBuffer.dwBufferBytes = formatSize;
    dsBuffer.lpwfxFormat = &wavData;
    if (FAILED(this->dsoundHdl->CreateSoundBuffer(&dsBuffer, &this->soundBuffers[idx], NULL)))
    {
        free(soundFileData);
        return ZUN_ERROR;
    }
    if (FAILED(soundBuffers[idx]->Lock(0, formatSize, (LPVOID *)&audioPtr1, (LPDWORD)&audioSize1, (LPVOID *)&audioPtr2,
                                       (LPDWORD)&audioSize2, NULL)))
    {
        free(soundFileData);
        return ZUN_ERROR;
    }
    memcpy(audioPtr1, wavDataPtr, audioSize1);
    if (audioSize2 != 0)
    {
        memcpy(audioPtr2, (i8 *)wavDataPtr + audioSize1, audioSize2);
    }
    soundBuffers[idx]->Unlock((LPVOID *)audioPtr1, audioSize1, (LPVOID *)audioPtr2, audioSize2);
    free(soundFileData);
    return ZUN_SUCCESS;
}

#pragma var_order(buffer, res)
ZunResult SoundPlayer::PlayBGM(BOOL isLooping)
{
    LPDIRECTSOUNDBUFFER buffer;
    HRESULT res;

    utils::DebugPrint2("play BGM\n");
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
    utils::DebugPrint2("comp\n");
    this->isLooping = isLooping;
    return ZUN_SUCCESS;
}

#pragma var_order(idx, sndBufIdx)
void SoundPlayer::PlaySounds()
{
    i32 idx;
    i32 sndBufIdx;

    if (this->manager == NULL)
    {
        return;
    }
    if (!g_Supervisor.cfg.playSounds)
    {
        return;
    }
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->soundBuffersToPlay); idx++)
    {
        if (this->soundBuffersToPlay[idx] < 0)
        {
            break;
        }
        sndBufIdx = this->soundBuffersToPlay[idx];
        this->soundBuffersToPlay[idx] = -1;
        if (this->duplicateSoundBuffers[sndBufIdx] == NULL)
        {
            continue;
        }
        this->duplicateSoundBuffers[sndBufIdx]->Stop();
        this->duplicateSoundBuffers[sndBufIdx]->SetCurrentPosition(0);
        this->duplicateSoundBuffers[sndBufIdx]->Play(0, 0, 0);
    }
    return;
}

#pragma var_order(i, SFXToPlay)
void SoundPlayer::PlaySoundByIdx(SoundIdx idx, i32 unused)
{
    i32 SFXToPlay;
    i32 i;

    SFXToPlay = g_SoundBufferIdxVol[idx].unk;
    for (i = 0; i < 3; i++)
    {
        if (this->soundBuffersToPlay[i] < 0)
        {
            break;
        }
        if (this->soundBuffersToPlay[i] == idx)
        {
            return;
        }
    }
    if (i >= 3)
    {
        return;
    }
    this->soundBuffersToPlay[i] = idx;
    this->unk408[idx] = SFXToPlay;
    return;
}

#pragma var_order(msg, looped, lpThreadParameterCopy, waitObj, res, stopped)
DWORD __stdcall SoundPlayer::BackgroundMusicPlayerThread(LPVOID lpThreadParameter)
{
    DWORD waitObj;
    MSG msg;
    u32 stopped;
    u32 looped;
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
            stopped = true;
        }
        switch (waitObj)
        {
        case 0:
            if (g_SoundPlayer.backgroundMusic != NULL)
            {
                res = g_SoundPlayer.backgroundMusic->HandleWaveStreamNotification(looped);
            }
            break;
        case 1:
            while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE) != 0)
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
}; // namespace th06
