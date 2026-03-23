//-----------------------------------------------------------------------------
// File: DSUtil.cpp
//
// Desc: DirectSound framework classes for reading and writing wav files and
//       playing them in DirectSound buffers. Feel free to use this class
//       as a starting point for adding extra functionality.
//
// Copyright (c) 1999-2000 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include "zwave.hpp"
#include "dxutil.hpp"
#include "utils.hpp"
#include "Windows.h"
#include <dsound.h>
#include <dxerr8.h>
#include <mmsystem.h>

namespace th06
{

//-----------------------------------------------------------------------------
// Name: CSoundManager::CSoundManager()
// Desc: Constructs the class
//-----------------------------------------------------------------------------
CSoundManager::CSoundManager()
{
    m_pDS = NULL;
}

//-----------------------------------------------------------------------------
// Name: CSoundManager::~CSoundManager()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CSoundManager::~CSoundManager()
{
    SAFE_RELEASE(m_pDS);
}

//-----------------------------------------------------------------------------
// Name: CSoundManager::Initialize()
// Desc: Initializes the IDirectSound object and also sets the primary buffer
//       format.  This function must be called before any others.
//-----------------------------------------------------------------------------
HRESULT CSoundManager::Initialize(HWND hWnd, DWORD dwCoopLevel, DWORD dwPrimaryChannels, DWORD dwPrimaryFreq,
                                  DWORD dwPrimaryBitRate)
{
    HRESULT hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    SAFE_RELEASE(m_pDS);

    // Create IDirectSound using the primary sound device
    if (FAILED(hr = DirectSoundCreate8(NULL, &m_pDS, NULL)))
        return DXTRACE_ERR(TEXT("DirectSoundCreate8"), hr);

    // Set DirectSound coop level
    if (FAILED(hr = m_pDS->SetCooperativeLevel(hWnd, dwCoopLevel)))
        return DXTRACE_ERR(TEXT("SetCooperativeLevel"), hr);

    // Set primary buffer format
    SetPrimaryBufferFormat(dwPrimaryChannels, dwPrimaryFreq, dwPrimaryBitRate);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CSoundManager::SetPrimaryBufferFormat()
// Desc: Set primary buffer to a specified format
//       For example, to set the primary buffer format to 22kHz stereo, 16-bit
//       then:   dwPrimaryChannels = 2
//               dwPrimaryFreq     = 22050,
//               dwPrimaryBitRate  = 16
//-----------------------------------------------------------------------------
HRESULT CSoundManager::SetPrimaryBufferFormat(DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate)
{
    HRESULT hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    if (m_pDS == NULL)
        return CO_E_NOTINITIALIZED;

    // Get the primary buffer
    DSBUFFERDESC dsbd;
    ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat = NULL;

    if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pDSBPrimary, NULL)))
        return DXTRACE_ERR(TEXT("CreateSoundBuffer"), hr);

    WAVEFORMATEX wfx;
    ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = (WORD)dwPrimaryChannels;
    wfx.nSamplesPerSec = dwPrimaryFreq;
    wfx.wBitsPerSample = (WORD)dwPrimaryBitRate;
    wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    if (FAILED(hr = pDSBPrimary->SetFormat(&wfx)))
        return DXTRACE_ERR(TEXT("SetFormat"), hr);

    SAFE_RELEASE(pDSBPrimary);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CSoundManager::CreateStreaming()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CSoundManager::CreateStreaming(CStreamingSound **ppStreamingSound, LPTSTR strWaveFileName,
                                       DWORD dwCreationFlags, GUID guid3DAlgorithm, DWORD dwNotifyCount,
                                       DWORD dwNotifySize, HANDLE hNotifyEvent)
{
    HRESULT hr;

    if (m_pDS == NULL)
        return CO_E_NOTINITIALIZED;

    LPDIRECTSOUNDBUFFER pDSBuffer = NULL;
    DWORD dwDSBufferSize;
    CWaveFile *pWaveFile = NULL;
    DSBPOSITIONNOTIFY *aPosNotify = NULL;
    LPDIRECTSOUNDNOTIFY pDSNotify = NULL;

    pWaveFile = new CWaveFile();
    pWaveFile->Open(strWaveFileName, NULL, WAVEFILE_READ);

    // Figure out how big the DSound buffer should be
    dwDSBufferSize = dwNotifySize * dwNotifyCount;

    // Set up the direct sound buffer.  Request the NOTIFY flag, so
    // that we are notified as the sound buffer plays.  Note, that using this flag
    // may limit the amount of hardware acceleration that can occur.
    DSBUFFERDESC dsbd;
    ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = dwCreationFlags | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 |
                   DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;
    dsbd.dwBufferBytes = dwDSBufferSize;
    dsbd.guid3DAlgorithm = guid3DAlgorithm;
    dsbd.lpwfxFormat = pWaveFile->m_pwfx;

    if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pDSBuffer, NULL)))
    {
        return E_FAIL;
    }

    // Create the notification events, so that we know when to fill
    // the buffer as the sound plays.
    if (FAILED(hr = pDSBuffer->QueryInterface(IID_IDirectSoundNotify, (VOID **)&pDSNotify)))
    {
        return E_FAIL;
    }

    aPosNotify = new DSBPOSITIONNOTIFY[dwNotifyCount];
    if (aPosNotify == NULL)
        return E_OUTOFMEMORY;

    for (DWORD i = 0; i < dwNotifyCount; i++)
    {
        aPosNotify[i].dwOffset = (dwNotifySize * i) + dwNotifySize - 1;
        aPosNotify[i].hEventNotify = hNotifyEvent;
    }

    // Tell DirectSound when to notify us. The notification will come in the from
    // of signaled events that are handled in WinMain()
    if (FAILED(hr = pDSNotify->SetNotificationPositions(dwNotifyCount, aPosNotify)))
    {
        SAFE_RELEASE(pDSNotify);
        SAFE_DELETE(aPosNotify);
        return E_FAIL;
    }

    SAFE_RELEASE(pDSNotify);
    SAFE_DELETE(aPosNotify);

    // Create the sound
    *ppStreamingSound = new CStreamingSound(pDSBuffer, dwDSBufferSize, pWaveFile, dwNotifySize);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CSound::CSound()
// Desc: Constructs the class
//-----------------------------------------------------------------------------
CSound::CSound(LPDIRECTSOUNDBUFFER *apDSBuffer, DWORD dwDSBufferSize, DWORD dwNumBuffers, CWaveFile *pWaveFile)
{
    DWORD i;

    m_apDSBuffer = new LPDIRECTSOUNDBUFFER[dwNumBuffers];
    for (i = 0; i < dwNumBuffers; i++)
        m_apDSBuffer[i] = apDSBuffer[i];

    m_dwDSBufferSize = dwDSBufferSize;
    m_dwNumBuffers = dwNumBuffers;
    m_pWaveFile = pWaveFile;

    FillBufferWithSound(m_apDSBuffer[0], FALSE);

    // Make DirectSound do pre-processing on sound effects
    for (i = 0; i < dwNumBuffers; i++)
        m_apDSBuffer[i]->SetCurrentPosition(0);
}

//-----------------------------------------------------------------------------
// Name: CSound::~CSound()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CSound::~CSound()
{
    for (DWORD i = 0; i < m_dwNumBuffers; i++)
    {
        SAFE_RELEASE(m_apDSBuffer[i]);
    }

    SAFE_DELETE_ARRAY(m_apDSBuffer);
    SAFE_DELETE(m_pWaveFile);
}

//-----------------------------------------------------------------------------
// Name: CSound::FillBufferWithSound()
// Desc: Fills a DirectSound buffer with a sound file
//-----------------------------------------------------------------------------
HRESULT CSound::FillBufferWithSound(LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger)
{
    HRESULT hr;
    VOID *pDSLockedBuffer = NULL;   // Pointer to locked buffer memory
    DWORD dwDSLockedBufferSize = 0; // Size of the locked DirectSound buffer
    DWORD dwWavDataRead = 0;        // Amount of data read from the wav file

    if (pDSB == NULL)
        return CO_E_NOTINITIALIZED;

    // Make sure we have focus, and we didn't just switch in from
    // an app which had a DirectSound device
    if (FAILED(hr = RestoreBuffer(pDSB, NULL)))
        return DXTRACE_ERR(TEXT("RestoreBuffer"), hr);

    // Lock the buffer down
    if (FAILED(hr = pDSB->Lock(0, m_dwDSBufferSize, &pDSLockedBuffer, &dwDSLockedBufferSize, NULL, NULL, 0L)))
        return DXTRACE_ERR(TEXT("Lock"), hr);

    // Reset the wave file to the beginning
    m_pWaveFile->ResetFile(false);

    if (FAILED(hr = m_pWaveFile->Read((BYTE *)pDSLockedBuffer, dwDSLockedBufferSize, &dwWavDataRead)))
        return DXTRACE_ERR(TEXT("Read"), hr);

    if (dwWavDataRead == 0)
    {
        // Wav is blank, so just fill with silence
        FillMemory((BYTE *)pDSLockedBuffer, dwDSLockedBufferSize,
                   (BYTE)(m_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0));
    }
    else if (dwWavDataRead < dwDSLockedBufferSize)
    {
        // If the wav file was smaller than the DirectSound buffer,
        // we need to fill the remainder of the buffer with data
        if (bRepeatWavIfBufferLarger)
        {
            // Reset the file and fill the buffer with wav data
            DWORD dwReadSoFar = dwWavDataRead; // From previous call above.
            while (dwReadSoFar < dwDSLockedBufferSize)
            {
                // This will keep reading in until the buffer is full
                // for very short files
                if (FAILED(hr = m_pWaveFile->ResetFile(false)))
                    return DXTRACE_ERR(TEXT("ResetFile"), hr);

                hr = m_pWaveFile->Read((BYTE *)pDSLockedBuffer + dwReadSoFar, dwDSLockedBufferSize - dwReadSoFar,
                                       &dwWavDataRead);
                if (FAILED(hr))
                    return DXTRACE_ERR(TEXT("Read"), hr);

                dwReadSoFar += dwWavDataRead;
            }
        }
        else
        {
            // Don't repeat the wav file, just fill in silence
            FillMemory((BYTE *)pDSLockedBuffer + dwWavDataRead, dwDSLockedBufferSize - dwWavDataRead,
                       (BYTE)(m_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0));
        }
    }

    // Unlock the buffer, we don't need it anymore.
    pDSB->Unlock(pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0);

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CSound::RestoreBuffer()
// Desc: Restores the lost buffer. *pbWasRestored returns TRUE if the buffer was
//       restored.  It can also NULL if the information is not needed.
//-----------------------------------------------------------------------------
HRESULT CSound::RestoreBuffer(LPDIRECTSOUNDBUFFER pDSB, BOOL *pbWasRestored)
{
    HRESULT hr;

    if (pDSB == NULL)
        return CO_E_NOTINITIALIZED;
    if (pbWasRestored)
        *pbWasRestored = FALSE;

    DWORD dwStatus;
    if (FAILED(hr = pDSB->GetStatus(&dwStatus)))
        return DXTRACE_ERR(TEXT("GetStatus"), hr);

    if (dwStatus & DSBSTATUS_BUFFERLOST)
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so
        // the restoring the buffer may fail.
        // If it does, sleep until DirectSound gives us control.
        do
        {
            hr = pDSB->Restore();
            if (hr == DSERR_BUFFERLOST)
                Sleep(10);
        } while (hr = pDSB->Restore());

        if (pbWasRestored != NULL)
            *pbWasRestored = TRUE;

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

//-----------------------------------------------------------------------------
// Name: CSound::GetFreeBuffer()
// Desc: Checks to see if a buffer is playing and returns TRUE if it is.
//-----------------------------------------------------------------------------
#pragma var_order(bIsPlaying, i)
LPDIRECTSOUNDBUFFER CSound::GetFreeBuffer()
{
    BOOL bIsPlaying = FALSE;

    if (m_apDSBuffer == NULL)
        return FALSE;

    DWORD i;
    for (i = 0; i < m_dwNumBuffers; i++)
    {
        if (m_apDSBuffer[i])
        {
            DWORD dwStatus = 0;
            m_apDSBuffer[i]->GetStatus(&dwStatus);
            if ((dwStatus & DSBSTATUS_PLAYING) == 0)
                break;
        }
    }

    if (i != m_dwNumBuffers)
        return m_apDSBuffer[i];
    else
        return m_apDSBuffer[rand() % m_dwNumBuffers];
}

//-----------------------------------------------------------------------------
// Name: CSound::GetBuffer()
// Desc:
//-----------------------------------------------------------------------------
LPDIRECTSOUNDBUFFER CSound::GetBuffer(DWORD dwIndex)
{
    if (m_apDSBuffer == NULL)
        return NULL;
    if (dwIndex >= m_dwNumBuffers)
        return NULL;

    return m_apDSBuffer[dwIndex];
}

//-----------------------------------------------------------------------------
// Name: CSound::Play()
// Desc: Plays the sound using voice management flags.  Pass in DSBPLAY_LOOPING
//       in the dwFlags to loop the sound
//-----------------------------------------------------------------------------
HRESULT CSound::Play(DWORD dwPriority, DWORD dwFlags)
{
    HRESULT hr;
    BOOL bRestored;

    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    LPDIRECTSOUNDBUFFER pDSB = GetFreeBuffer();

    if (pDSB == NULL)
        return DXTRACE_ERR(TEXT("GetFreeBuffer"), E_FAIL);

    // Restore the buffer if it was lost
    if (FAILED(hr = RestoreBuffer(pDSB, &bRestored)))
        return DXTRACE_ERR(TEXT("RestoreBuffer"), hr);

    if (bRestored)
    {
        // The buffer was restored, so we need to fill it with new data
        if (FAILED(hr = FillBufferWithSound(pDSB, FALSE)))
            return DXTRACE_ERR(TEXT("FillBufferWithSound"), hr);

        // Make DirectSound do pre-processing on sound effects
        Reset();
    }

    this->m_dwIsFadingOut = 0;
    this->m_dwCurFadeoutProgress = 0;
    this->m_dwTotalFadeout = 0;

    return pDSB->Play(0, dwPriority, dwFlags);
}

//-----------------------------------------------------------------------------
// Name: CSound::Stop()
// Desc: Stops the sound from playing
//-----------------------------------------------------------------------------
HRESULT CSound::Stop()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    HRESULT hr = 0;

    utils::DebugPrint2("CSound::Stop ");

    for (DWORD i = 0; i < m_dwNumBuffers; i++)
    {
        utils::DebugPrint2("%d ", i);
        hr |= m_apDSBuffer[i]->Stop();
    }

    utils::DebugPrint2("\n");

    this->m_dwIsFadingOut = 0;

    return hr;
}

//-----------------------------------------------------------------------------
// Name: CSound::Reset()
// Desc: Reset all of the sound buffers
//-----------------------------------------------------------------------------
HRESULT CSound::Reset()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    HRESULT hr = 0;

    for (DWORD i = 0; i < m_dwNumBuffers; i++)
        hr |= m_apDSBuffer[i]->SetCurrentPosition(0);

    return hr;
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::CStreamingSound()
// Desc: Setups up a buffer so data can be streamed from the wave file into
//       buffer.  This is very useful for large wav files that would take a
//       while to load.  The buffer is initially filled with data, then
//       as sound is played the notification events are signaled and more data
//       is written into the buffer by calling HandleWaveStreamNotification()
//-----------------------------------------------------------------------------
CStreamingSound::CStreamingSound(LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize, CWaveFile *pWaveFile,
                                 DWORD dwNotifySize)
    : CSound(&pDSBuffer, dwDSBufferSize, 1, pWaveFile)
{
    m_dwLastPlayPos = 0;
    m_dwPlayProgress = 0;
    m_dwNotifySize = dwNotifySize;
    m_dwNextWriteOffset = 0;
    m_bFillNextNotificationWithSilence = FALSE;
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::~CStreamingSound()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CStreamingSound::~CStreamingSound()
{
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::UpdateFadeOut()
// Desc: Handle the notification that tell us to put more wav data in the
//       circular buffer
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::UpdateFadeOut()
{
    if (this->m_dwIsFadingOut != 0)
    {
        this->m_dwCurFadeoutProgress = this->m_dwCurFadeoutProgress - 1;
        if (this->m_dwCurFadeoutProgress <= 0)
        {
            this->m_dwIsFadingOut = 0;
            this->m_apDSBuffer[0]->Stop();
            return 1;
        }
        DWORD vol = ((this->m_dwCurFadeoutProgress * 5000) / this->m_dwTotalFadeout) - 5000;
        HRESULT res = this->m_apDSBuffer[0]->SetVolume(vol);
    }
    return 0;
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::HandleWaveStreamNotification()
// Desc: Handle the notification that tell us to put more wav data in the
//       circular buffer
//-----------------------------------------------------------------------------
#pragma var_order(dwDSLockedBufferSize2, pDSLockedBuffer, dwBytesWrittenToBuffer, pDSLockedBuffer2, bRestored,         \
                  dwPlayDelta, hr, dwDSLockedBufferSize, dwCurrentPlayPos)
HRESULT CStreamingSound::HandleWaveStreamNotification(BOOL bLoopedPlay)
{
    HRESULT hr;
    DWORD dwCurrentPlayPos;
    DWORD dwPlayDelta;
    DWORD dwBytesWrittenToBuffer;
    VOID *pDSLockedBuffer;
    VOID *pDSLockedBuffer2;
    DWORD dwDSLockedBufferSize;
    DWORD dwDSLockedBufferSize2;

    if (m_apDSBuffer == NULL || m_pWaveFile == NULL)
        return CO_E_NOTINITIALIZED;

    // Restore the buffer if it was lost
    BOOL bRestored;
    if (FAILED(hr = RestoreBuffer(m_apDSBuffer[0], &bRestored)))
    {
        utils::DebugPrint2("error : RetoreBuffer in HandleWaveStreamNotification\n");
        return DXTRACE_ERR(TEXT("RestoreBuffer"), hr);
    }

    if (bRestored)
    {
        // The buffer was restored, so we need to fill it with new data
        if (FAILED(hr = FillBufferWithSound(m_apDSBuffer[0], FALSE)))
        {
            utils::DebugPrint2("error : FillBufferWithSound in HandleWaveStreamNotification\n");
            return DXTRACE_ERR(TEXT("FillBufferWithSound"), hr);
        }
        return S_OK;
    }

    // Lock the DirectSound buffer
    pDSLockedBuffer = NULL;
    pDSLockedBuffer2 = NULL;
    if (FAILED(hr = m_apDSBuffer[0]->Lock(m_dwNextWriteOffset, m_dwNotifySize, &pDSLockedBuffer, &dwDSLockedBufferSize,
                                          &pDSLockedBuffer2, &dwDSLockedBufferSize2, 0L)))
    {
        utils::DebugPrint2("error : Buffer->Lock in HandleWaveStreamNotification\n");
        return DXTRACE_ERR(TEXT("Lock"), hr);
    }

    // m_dwDSBufferSize and m_dwNextWriteOffset are both multiples of m_dwNotifySize,
    // it should the second buffer should never be valid
    if (pDSLockedBuffer2 != NULL)
        return E_UNEXPECTED;

    if (!m_bFillNextNotificationWithSilence)
    {
        // Fill the DirectSound buffer with wav data
        if (FAILED(hr = m_pWaveFile->Read((BYTE *)pDSLockedBuffer, dwDSLockedBufferSize, &dwBytesWrittenToBuffer)))
        {
            utils::DebugPrint2("error : m_pWaveFile->Read in HandleWaveStreamNotification\n");
            return DXTRACE_ERR(TEXT("Read"), hr);
        }
    }
    else
    {
        // Fill the DirectSound buffer with silence
        FillMemory(pDSLockedBuffer, dwDSLockedBufferSize, (BYTE)(m_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0));
        dwBytesWrittenToBuffer = dwDSLockedBufferSize;
    }

    // If the number of bytes written is less than the
    // amount we requested, we have a short file.
    if (dwBytesWrittenToBuffer < dwDSLockedBufferSize)
    {
        if (!bLoopedPlay)
        {
            // Fill in silence for the rest of the buffer.
            FillMemory((BYTE *)pDSLockedBuffer + dwBytesWrittenToBuffer, dwDSLockedBufferSize - dwBytesWrittenToBuffer,
                       (BYTE)(m_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0));

            // Any future notifications should just fill the buffer with silence
            m_bFillNextNotificationWithSilence = TRUE;
        }
        else
        {
            // We are looping, so reset the file and fill the buffer with wav data
            DWORD dwReadSoFar = dwBytesWrittenToBuffer; // From previous call above.
            while (dwReadSoFar < dwDSLockedBufferSize)
            {
                // This will keep reading in until the buffer is full (for very short files).
                if (FAILED(hr = m_pWaveFile->ResetFile(true)))
                {
                    utils::DebugPrint2("error : m_pWaveFile->ResetFile in HandleWaveStreamNotification\n");
                    return DXTRACE_ERR(TEXT("ResetFile"), hr);
                }

                if (FAILED(hr = m_pWaveFile->Read((BYTE *)pDSLockedBuffer + dwReadSoFar,
                                                  dwDSLockedBufferSize - dwReadSoFar, &dwBytesWrittenToBuffer)))
                {
                    utils::DebugPrint2("error : m_pWaveFile->Read(+) in HandleWaveStreamNotification\n");
                    return DXTRACE_ERR(TEXT("Read"), hr);
                }

                dwReadSoFar += dwBytesWrittenToBuffer;
            }
        }
    }

    // Unlock the DirectSound buffer
    m_apDSBuffer[0]->Unlock(pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0);

    // Figure out how much data has been played so far.  When we have played
    // passed the end of the file, we will either need to start filling the
    // buffer with silence or starting reading from the beginning of the file,
    // depending if the user wants to loop the sound
    if (FAILED(hr = m_apDSBuffer[0]->GetCurrentPosition(&dwCurrentPlayPos, NULL)))
    {
        utils::DebugPrint2("error : m_apDSBuffer[0]->GetCurrentPosition in HandleWaveStreamNotification\n");
        return DXTRACE_ERR(TEXT("GetCurrentPosition"), hr);
    }

    // Check to see if the position counter looped
    if (dwCurrentPlayPos < m_dwLastPlayPos)
        dwPlayDelta = (m_dwDSBufferSize - m_dwLastPlayPos) + dwCurrentPlayPos;
    else
        dwPlayDelta = dwCurrentPlayPos - m_dwLastPlayPos;

    m_dwPlayProgress += dwPlayDelta;
    m_dwLastPlayPos = dwCurrentPlayPos;

    // If we are now filling the buffer with silence, then we have found the end so
    // check to see if the entire sound has played, if it has then stop the buffer.
    if (m_bFillNextNotificationWithSilence)
    {
        // We don't want to cut off the sound before it's done playing.
        if (m_dwPlayProgress >= m_pWaveFile->GetSize())
        {
            m_apDSBuffer[0]->Stop();
        }
    }

    // Update where the buffer will lock (for next time)
    m_dwNextWriteOffset += dwDSLockedBufferSize;
    m_dwNextWriteOffset %= m_dwDSBufferSize; // Circular buffer

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CStreamingSound::Reset()
// Desc: Resets the sound so it will begin playing at the beginning
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::Reset()
{
    HRESULT hr;

    if (m_apDSBuffer[0] == NULL || m_pWaveFile == NULL)
        return CO_E_NOTINITIALIZED;

    m_dwLastPlayPos = 0;
    m_dwPlayProgress = 0;
    m_dwNextWriteOffset = 0;
    m_bFillNextNotificationWithSilence = FALSE;

    // Restore the buffer if it was lost
    BOOL bRestored;
    if (FAILED(hr = RestoreBuffer(m_apDSBuffer[0], &bRestored)))
        return DXTRACE_ERR(TEXT("RestoreBuffer"), hr);

    if (bRestored)
    {
        // The buffer was restored, so we need to fill it with new data
        if (FAILED(hr = FillBufferWithSound(m_apDSBuffer[0], FALSE)))
            return DXTRACE_ERR(TEXT("FillBufferWithSound"), hr);
    }

    m_pWaveFile->ResetFile(false);

    return m_apDSBuffer[0]->SetCurrentPosition(0L);
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::CWaveFile()
// Desc: Constructs the class.  Call Open() to open a wave file for reading.
//       Then call Read() as needed.  Calling the destructor or Close()
//       will close the file.
//-----------------------------------------------------------------------------
CWaveFile::CWaveFile()
{
    m_pwfx = NULL;
    m_hmmio = NULL;
    m_dwSize = 0;
    m_bIsReadingFromMemory = FALSE;
    m_loopEndPoint = 0;
    m_loopStartPoint = 0;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::~CWaveFile()
// Desc: Destructs the class
//-----------------------------------------------------------------------------
CWaveFile::~CWaveFile()
{
    Close();

    if (!m_bIsReadingFromMemory)
        SAFE_DELETE_ARRAY(m_pwfx);
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Open()
// Desc: Opens a wave file for reading
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Open(LPTSTR strFileName, WAVEFORMATEX *pwfx, DWORD dwFlags)
{
    HRESULT hr;

    m_dwFlags = dwFlags;
    m_bIsReadingFromMemory = FALSE;

    if (m_dwFlags == WAVEFILE_READ)
    {
        if (strFileName == NULL)
            return E_INVALIDARG;
        SAFE_DELETE_ARRAY(m_pwfx);

        MMIOINFO mmioInfo;

        ZeroMemory(&mmioInfo, sizeof(mmioInfo));
        m_hmmio = mmioOpen(strFileName, &mmioInfo, MMIO_ALLOCBUF | MMIO_READ);
        if (NULL == m_hmmio)
        {
            switch (mmioInfo.wErrorRet)
            {
            case MMIOERR_PATHNOTFOUND:
                utils::DebugPrint2("The directory specification is incorrect. \n");
                break;
            case MMIOERR_ACCESSDENIED:
                utils::DebugPrint2("The file is protected and cannot be opened. \n");
                break;
            case MMIOERR_SHARINGVIOLATION:
                utils::DebugPrint2("The file is being used by another application and is unavailable. \n");
                break;
            case MMIOERR_TOOMANYOPENFILES:
                utils::DebugPrint2("too Meny Open Files \n");
                break;
            case MMIOERR_INVALIDFILE:
                utils::DebugPrint2(
                    "Another failure condition occurred. This is the default error for an open-file failure. \n");
                break;
            }
            utils::DebugPrint2("error : mmioOpen in CWaveFile::Open()\n");
            return E_FAIL;
        }

        if (FAILED(hr = ReadMMIO()))
        {
            // ReadMMIO will fail if its an not a wave file
            mmioClose(m_hmmio, 0);
            utils::DebugPrint2("error : ReadOpen in CWaveFile::Open()\n");
            return E_FAIL;
        }

        if (FAILED(hr = ResetFile(false)))
        {
            utils::DebugPrint2("error : ResetFile in CWaveFile::Open()\n");
            return E_FAIL;
        }

        // After the reset, the size of the wav file is m_ck.cksize so store it now
        m_dwSize = m_ck.cksize;
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::OpenFromMemory()
// Desc: copy data to CWaveFile member variable from memory
//-----------------------------------------------------------------------------
HRESULT CWaveFile::OpenFromMemory(BYTE *pbData, ULONG ulDataSize, WAVEFORMATEX *pwfx, DWORD dwFlags)
{
    m_pwfx = pwfx;
    m_ulDataSize = ulDataSize;
    m_pbData = pbData;
    m_pbDataCur = m_pbData;
    m_bIsReadingFromMemory = TRUE;

    if (dwFlags != WAVEFILE_READ)
        return E_NOTIMPL;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::ReadMMIO()
// Desc: Support function for reading from a multimedia I/O stream.
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_pwfx.
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ReadMMIO()
{
    MMCKINFO ckIn;               // chunk info. for general use.
    PCMWAVEFORMAT pcmWaveFormat; // Temp PCM structure to load in.

    m_pwfx = NULL;

    if ((0 != mmioDescend(m_hmmio, &m_ckRiff, NULL, 0)))
        return DXTRACE_ERR(TEXT("mmioDescend"), E_FAIL);

    // Check to make sure this is a valid wave file
    if ((m_ckRiff.ckid != FOURCC_RIFF) || (m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E')))
        return DXTRACE_ERR_NOMSGBOX(TEXT("mmioFOURCC"), E_FAIL);

    // Search the input file for for the 'fmt ' chunk.
    ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (0 != mmioDescend(m_hmmio, &ckIn, &m_ckRiff, MMIO_FINDCHUNK))
        return DXTRACE_ERR(TEXT("mmioDescend"), E_FAIL);

    // Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
    // if there are extra parameters at the end, we'll ignore them
    if (ckIn.cksize < (LONG)sizeof(PCMWAVEFORMAT))
        return DXTRACE_ERR(TEXT("sizeof(PCMWAVEFORMAT)"), E_FAIL);

    // Read the 'fmt ' chunk into <pcmWaveFormat>.
    if (mmioRead(m_hmmio, (HPSTR)&pcmWaveFormat, sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat))
        return DXTRACE_ERR(TEXT("mmioRead"), E_FAIL);

    // Allocate the waveformatex, but if its not pcm format, read the next
    // word, and thats how many extra bytes to allocate.
    if (pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM)
    {
        m_pwfx = (WAVEFORMATEX *)new CHAR[sizeof(WAVEFORMATEX)];
        if (NULL == m_pwfx)
            return DXTRACE_ERR(TEXT("m_pwfx"), E_FAIL);

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy(m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat));
        m_pwfx->cbSize = 0;
    }
    else
    {
        // Read in length of extra bytes.
        WORD cbExtraBytes = 0L;
        if (mmioRead(m_hmmio, (CHAR *)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD))
            return DXTRACE_ERR(TEXT("mmioRead"), E_FAIL);

        m_pwfx = (WAVEFORMATEX *)new CHAR[sizeof(WAVEFORMATEX) + cbExtraBytes];
        if (NULL == m_pwfx)
            return DXTRACE_ERR(TEXT("new"), E_FAIL);

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy(m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat));
        m_pwfx->cbSize = cbExtraBytes;

        // Now, read those extra bytes into the structure, if cbExtraAlloc != 0.
        if (mmioRead(m_hmmio, (CHAR *)(((BYTE *)&(m_pwfx->cbSize)) + sizeof(WORD)), cbExtraBytes) != cbExtraBytes)
        {
            SAFE_DELETE(m_pwfx);
            return DXTRACE_ERR(TEXT("mmioRead"), E_FAIL);
        }
    }

    // Ascend the input file out of the 'fmt ' chunk.
    if (0 != mmioAscend(m_hmmio, &ckIn, 0))
    {
        SAFE_DELETE(m_pwfx);
        return DXTRACE_ERR(TEXT("mmioAscend"), E_FAIL);
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::GetSize()
// Desc: Retuns the size of the read access wave file
//-----------------------------------------------------------------------------
DWORD CWaveFile::GetSize()
{
    return m_dwSize;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::ResetFile()
// Desc: Resets the internal m_ck pointer so reading starts from the
//       beginning of the file again
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ResetFile(bool loop)
{
    if (m_bIsReadingFromMemory)
    {
        m_pbDataCur = m_pbData;
    }
    else
    {
        if (m_hmmio == NULL)
        {
            utils::DebugPrint2("error : m_hmmio\t== NULL in CWaveFile::ResetFile\n");
            return CO_E_NOTINITIALIZED;
        }

        if (m_dwFlags == WAVEFILE_READ)
        {
            // Seek to the data
            if (-1 == mmioSeek(m_hmmio, m_ckRiff.dwDataOffset + sizeof(FOURCC), SEEK_SET))
            {
                utils::DebugPrint2("error : mmioSeek in CWaveFile::ResetFile\n");
                return DXTRACE_ERR(TEXT("mmioSeek"), E_FAIL);
            }

            // Search the input file for the 'data' chunk.
            m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
            MMRESULT res = mmioDescend(m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK);
            if (0 != res)
            {
                utils::DebugPrint2("error : mmioDescend in CWaveFile::ResetFile\n");
                return DXTRACE_ERR(TEXT("mmioDescend"), E_FAIL);
            }

            if (0 < m_loopEndPoint)
            {
                m_ck.cksize = m_loopEndPoint;
            }
            if (loop && 0 < this->m_loopStartPoint)
            {
                MMIOINFO mmioinfoIn;
                if (0 != mmioGetInfo(this->m_hmmio, &mmioinfoIn, 0))
                {
                    utils::DebugPrint2("error : mmioGetInfo in CWaveFile::ResetFile\n");
                    return E_FAIL;
                }
                for (int i = 0; i < this->m_loopStartPoint; i++)
                {
                    if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
                    {
                        if (0 != mmioAdvance(this->m_hmmio, &mmioinfoIn, 0))
                        {
                            utils::DebugPrint2("error : mmioAdvance in CWaveFile::ResetFile\n");
                            return E_FAIL;
                        }
                        if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
                        {
                            utils::DebugPrint2(
                                "error : mmioinfoIn.pchNext == mmioinfoIn.pchEndRead in CWaveFile::ResetFile\n");
                            return E_FAIL;
                        }
                    }
                    mmioinfoIn.pchNext = mmioinfoIn.pchNext + 1;
                }
                this->m_ck.cksize = this->m_ck.cksize - this->m_loopStartPoint;
                if (mmioSetInfo(this->m_hmmio, &mmioinfoIn, 0) != 0)
                {
                    utils::DebugPrint2("error : mmioSetInfo in CWaveFile::ResetFile\n");
                    return 0x80004005;
                }
            }
        }
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Read()
// Desc: Reads section of data from a wave file into pBuffer and returns
//       how much read in pdwSizeRead, reading not more than dwSizeToRead.
//       This uses m_ck to determine where to start reading from.  So
//       subsequent calls will be continue where the last left off unless
//       Reset() is called.
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Read(BYTE *pBuffer, DWORD dwSizeToRead, DWORD *pdwSizeRead)
{
    if (m_bIsReadingFromMemory)
    {
        if (m_pbDataCur == NULL)
            return CO_E_NOTINITIALIZED;
        if (pdwSizeRead != NULL)
            *pdwSizeRead = 0;

        if ((BYTE *)(m_pbDataCur + dwSizeToRead) > (BYTE *)(m_pbData + m_ulDataSize))
        {
            dwSizeToRead = m_ulDataSize - (DWORD)(m_pbDataCur - m_pbData);
        }

        CopyMemory(pBuffer, m_pbDataCur, dwSizeToRead);

        if (pdwSizeRead != NULL)
            *pdwSizeRead = dwSizeToRead;

        return S_OK;
    }
    else
    {
        MMIOINFO mmioinfoIn; // current status of m_hmmio

        if (m_hmmio == NULL)
            return CO_E_NOTINITIALIZED;
        if (pBuffer == NULL || pdwSizeRead == NULL)
            return E_INVALIDARG;

        if (pdwSizeRead != NULL)
            *pdwSizeRead = 0;

        if (0 != mmioGetInfo(m_hmmio, &mmioinfoIn, 0))
        {
            utils::DebugPrint2("error :\t%s(%s)\n", __FILE__, 1060);
            return DXTRACE_ERR(TEXT("mmioGetInfo"), E_FAIL);
        }

        UINT cbDataIn = dwSizeToRead;
        if (cbDataIn > m_ck.cksize)
            cbDataIn = m_ck.cksize;

        m_ck.cksize -= cbDataIn;

        for (DWORD cT = 0; cT < cbDataIn; cT++)
        {
            // Copy the bytes from the io to the buffer.
            if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
            {
                if (0 != mmioAdvance(m_hmmio, &mmioinfoIn, MMIO_READ))
                {
                    // Note: 1075 here is _probably_ the line number. I'm not
                    // using __LINE__ to avoid mismatches due to reformatting.
                    utils::DebugPrint2("error :\t%s(%s)\n", __FILE__, 1075);
                    return DXTRACE_ERR(TEXT("mmioAdvance"), E_FAIL);
                }

                if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
                {
                    // Note: 1075 here is _probably_ the line number. I'm not
                    // using __LINE__ to avoid mismatches due to reformatting.
                    utils::DebugPrint2("error :\t%s(%s)\n", __FILE__, 1079);
                    return DXTRACE_ERR(TEXT("mmioinfoIn.pchNext"), E_FAIL);
                }
            }

            // Actual copy.
            *((BYTE *)pBuffer + cT) = *((BYTE *)mmioinfoIn.pchNext);
            mmioinfoIn.pchNext++;
        }

        if (0 != mmioSetInfo(m_hmmio, &mmioinfoIn, 0))
        {
            utils::DebugPrint2("error :\t%s(%s)\n", __FILE__, 1088);
            return DXTRACE_ERR(TEXT("mmioSetInfo"), E_FAIL);
        }

        if (pdwSizeRead != NULL)
            *pdwSizeRead = cbDataIn;

        return S_OK;
    }
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Close()
// Desc: Closes the wave file
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Close()
{
    if (m_dwFlags == WAVEFILE_READ)
    {
        mmioClose(m_hmmio, 0);
        m_hmmio = NULL;
    }
    return S_OK;
}
}; // namespace th06
