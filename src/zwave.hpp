//-----------------------------------------------------------------------------
// File: DSUtil.h
//
// Desc:
//
// Copyright (c) 1999-2000 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef DSUTIL_H
#define DSUTIL_H

#include "inttypes.hpp"
#include "Windows.h"
#include <dsound.h>
#include <mmreg.h>
#include <mmsystem.h>

namespace th06
{
//-----------------------------------------------------------------------------
// Classes used by this header
//-----------------------------------------------------------------------------
class CSoundManager;
class CSound;
class CStreamingSound;
class CWaveFile;

//-----------------------------------------------------------------------------
// Typing macros
//-----------------------------------------------------------------------------
#define WAVEFILE_READ 1
#define WAVEFILE_WRITE 2

#define DSUtil_StopSound(s)                                                                                            \
    {                                                                                                                  \
        if (s)                                                                                                         \
            s->Stop();                                                                                                 \
    }
#define DSUtil_PlaySound(s)                                                                                            \
    {                                                                                                                  \
        if (s)                                                                                                         \
            s->Play(0, 0);                                                                                             \
    }
#define DSUtil_PlaySoundLooping(s)                                                                                     \
    {                                                                                                                  \
        if (s)                                                                                                         \
            s->Play(0, DSBPLAY_LOOPING);                                                                               \
    }

//-----------------------------------------------------------------------------
// Name: class CSoundManager
// Desc:
//-----------------------------------------------------------------------------
class CSoundManager
{
  protected:
    LPDIRECTSOUND8 m_pDS;

  public:
    CSoundManager();
    ~CSoundManager();

    HRESULT Initialize(HWND hWnd, DWORD dwCoopLevel, DWORD dwPrimaryChannels, DWORD dwPrimaryFreq,
                       DWORD dwPrimaryBitRate);
    inline LPDIRECTSOUND GetDirectSound()
    {
        return m_pDS;
    }
    HRESULT SetPrimaryBufferFormat(DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate);

    HRESULT CreateStreaming(CStreamingSound **ppStreamingSound, LPTSTR strWaveFileName, DWORD dwCreationFlags,
                            GUID guid3DAlgorithm, DWORD dwNotifyCount, DWORD dwNotifySize, HANDLE hNotifyEvent);
};

//-----------------------------------------------------------------------------
// Name: class CSound
// Desc: Encapsulates functionality of a DirectSound buffer.
//-----------------------------------------------------------------------------
class CSound
{
  protected:
    LPDIRECTSOUNDBUFFER *m_apDSBuffer;
    DWORD m_dwDSBufferSize;

  public:
    CWaveFile *m_pWaveFile;

  protected:
    DWORD m_dwNumBuffers;

  public:
    // th06 extensions for fadeout
    i32 m_dwCurFadeoutProgress;
    i32 m_dwTotalFadeout;
    DWORD m_dwIsFadingOut;

  protected:
    HRESULT RestoreBuffer(LPDIRECTSOUNDBUFFER pDSB, BOOL *pbWasRestored);

  public:
    CSound(LPDIRECTSOUNDBUFFER *apDSBuffer, DWORD dwDSBufferSize, DWORD dwNumBuffers, CWaveFile *pWaveFile);
    virtual ~CSound();

    HRESULT FillBufferWithSound(LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger);
    LPDIRECTSOUNDBUFFER GetFreeBuffer();
    LPDIRECTSOUNDBUFFER GetBuffer(DWORD dwIndex);

    HRESULT Play(DWORD dwPriority, DWORD dwFlags);
    HRESULT Stop();
    HRESULT Reset();
};

//-----------------------------------------------------------------------------
// Name: class CStreamingSound
// Desc: Encapsulates functionality to play a wave file with DirectSound.
//       The Create() method loads a chunk of wave file into the buffer,
//       and as sound plays more is written to the buffer by calling
//       HandleWaveStreamNotification() whenever hNotifyEvent is signaled.
//-----------------------------------------------------------------------------
class CStreamingSound : public CSound
{
  protected:
    DWORD m_dwLastPlayPos;
    DWORD m_dwPlayProgress;
    DWORD m_dwNotifySize;
    DWORD m_dwNextWriteOffset;
    BOOL m_bFillNextNotificationWithSilence;

  public:
    CStreamingSound(LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize, CWaveFile *pWaveFile, DWORD dwNotifySize);
    ~CStreamingSound();

    HRESULT UpdateFadeOut();
    HRESULT HandleWaveStreamNotification(BOOL bLoopedPlay);
    HRESULT Reset();
};

//-----------------------------------------------------------------------------
// Name: class CWaveFile
// Desc: Encapsulates reading or writing sound data to or from a wave file
//-----------------------------------------------------------------------------
class CWaveFile
{
  public:
    WAVEFORMATEX *m_pwfx; // Pointer to WAVEFORMATEX structure
    HMMIO m_hmmio;        // MM I/O handle for the WAVE
    MMCKINFO m_ck;        // Multimedia RIFF chunk
    MMCKINFO m_ckRiff;    // Use in opening a WAVE file
    DWORD m_dwSize;       // The size of the wave file
    MMIOINFO m_mmioinfoOut;
    DWORD m_dwFlags;
    BOOL m_bIsReadingFromMemory;
    BYTE *m_pbData;
    BYTE *m_pbDataCur;
    ULONG m_ulDataSize;

    // Zun extensions
    i32 m_loopStartPoint; // Start of the loop. When the wave file reaches the
                          // loop end point, it will restart playback from this
                          // point.
    i32 m_loopEndPoint;   // End of the loop

  protected:
    HRESULT ReadMMIO();

  public:
    CWaveFile();
    ~CWaveFile();

    HRESULT Open(LPTSTR strFileName, WAVEFORMATEX *pwfx, DWORD dwFlags);
    HRESULT OpenFromMemory(BYTE *pbData, ULONG ulDataSize, WAVEFORMATEX *pwfx, DWORD dwFlags);
    HRESULT Close();

    HRESULT Read(BYTE *pBuffer, DWORD dwSizeToRead, DWORD *pdwSizeRead);

    DWORD GetSize();
    HRESULT ResetFile(bool loop);
    WAVEFORMATEX *GetFormat()
    {
        return m_pwfx;
    };
};
}; // namespace th06

#endif // DSUTIL_H
