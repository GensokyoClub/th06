#include "stdafx.h"

#include "MidiOutput.hpp"
#include "MidiSub.hpp"

namespace th06
{
MidiDevice::MidiDevice()
{
    this->handle = NULL;
    this->deviceId = 0;
}

MidiDevice::~MidiDevice()
{
    this->Close();
}

ZunBool MidiDevice::OpenDevice(u32 uDeviceId)
{
    if (this->handle != NULL)
    {
        if (this->deviceId != uDeviceId)
        {
            this->Close();
        }
        else
        {
            return false;
        }
    }

    this->deviceId = uDeviceId;

    return midiOutOpen(&this->handle, uDeviceId, (DWORD_PTR)g_Supervisor.hwndGameWindow, NULL, CALLBACK_WINDOW) !=
           MMSYSERR_NOERROR;
}

ZunResult MidiDevice::Close()
{
    if (this->handle == NULL)
    {
        return ZUN_ERROR;
    }

    midiOutReset(this->handle);
    midiOutClose(this->handle);
    this->handle = NULL;

    return ZUN_SUCCESS;
}

ZunBool MidiDevice::SendLongMsg(LPMIDIHDR pmh)
{
    if (this->handle == NULL)
    {
        return false;
    }
    else
    {
        if (midiOutPrepareHeader(this->handle, pmh, sizeof(MIDIHDR)) != MMSYSERR_NOERROR)
        {
            return true;
        }

        return midiOutLongMsg(this->handle, pmh, sizeof(MIDIHDR)) != MMSYSERR_NOERROR;
    }
}

ZunBool MidiDevice::SendShortMsg(DWORD msg)
{
    if (this->handle == NULL)
    {
        return false;
    }
    else
    {
        return midiOutShortMsg(this->handle, msg) != MMSYSERR_NOERROR;
    }
}

ZunBool MidiDevice::SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte)
{
    u8 data[4];

    if (this->handle == NULL)
    {
        return false;
    }
    else
    {
        data[0] = midiStatus;
        data[1] = firstByte;
        data[2] = secondByte;
        return midiOutShortMsg(this->handle, *(DWORD *)data) != MMSYSERR_NOERROR;
    }
}

void MidiDevice::GetDeviceName(u32 uDeviceId, char *name)
{
    MIDIOUTCAPS caps;

    midiOutGetDevCaps(uDeviceId, &caps, sizeof(caps));
    strcpy(name, caps.szPname);
}

MidiTimer::MidiTimer()
{
    timeGetDevCaps(&this->timeCaps, sizeof(TIMECAPS));
    this->timerId = 0;
}

MidiTimer::~MidiTimer()
{
    this->StopTimer();
    timeEndPeriod(this->timeCaps.wPeriodMin);
}

u32 MidiTimer::StartTimer(u32 delay, LPTIMECALLBACK cb, DWORD_PTR data)
{
    this->StopTimer();
    timeBeginPeriod(this->timeCaps.wPeriodMin);

    if (cb != NULL)
    {
        this->timerId = timeSetEvent(delay, this->timeCaps.wPeriodMin, cb, data, TIME_PERIODIC);
    }
    else
    {
        this->timerId = timeSetEvent(delay, this->timeCaps.wPeriodMin, (LPTIMECALLBACK)MidiTimer_DefaultTimerCallback,
                                     (DWORD_PTR)this, TIME_PERIODIC);
    }
    return this->timerId;
}

i32 MidiTimer::StopTimer()
{
    if (this->timerId != 0)
    {
        timeKillEvent(this->timerId);
    }
    timeEndPeriod(this->timeCaps.wPeriodMin);
    this->timerId = 0;
    return 1;
}

void CALLBACK MidiTimer_DefaultTimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    MidiTimer *timer = (MidiTimer *)dwUser;

    timer->OnTimerElapsed();
}

// These bodies and Win32 types follow SSG PBGMIDI.C and KOG MidiSub.h.
// KOG makes all three static inline in MidiSub.h. ConvDWord remains inline
// here because removing it changes ParseFile in the retail /Ob1 build.
WORD ConvWord(WORD data)
{
    WORD temp;

    ((BYTE *)&temp)[0] = ((BYTE *)&data)[1];
    ((BYTE *)&temp)[1] = ((BYTE *)&data)[0];

    return temp;
}

inline DWORD ConvDWord(DWORD data)
{
    DWORD temp;

    ((BYTE *)&temp)[0] = ((BYTE *)&data)[3];
    ((BYTE *)&temp)[1] = ((BYTE *)&data)[2];
    ((BYTE *)&temp)[2] = ((BYTE *)&data)[1];
    ((BYTE *)&temp)[3] = ((BYTE *)&data)[0];

    return temp;
}

DWORD GetDeltaTime(LPBYTE *data)
{
    BYTE temp;
    DWORD count = 0;

    do
    {
        temp = **data;
        ++*data;
        count = (count << 7) + (temp & 0x7f);
    } while (temp & 0x80);

    return count;
}

MidiOutput::MidiOutput()
{
    this->tracks = NULL;
    this->timebase = 0;
    this->tempo = 0;
    this->numTracks = 0;
    this->unk2c4 = 0;
    this->fadeOutVolumeMultiplier = 0.0f;
    this->fadeOutLastSetVolume = 0;
    this->unk2d0 = 0;
    this->unk2d4 = 0;
    this->unk2d8 = 0;
    this->unk2dc = 0;
    this->fadeOutFlag = false;

    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->midiFileData); i++)
    {
        this->midiFileData[i] = NULL;
    }

    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->midiHeaders); i++)
    {
        this->midiHeaders[i] = NULL;
    }

    this->midiHeadersCursor = 0;
}

MidiOutput::~MidiOutput()
{
    this->StopPlayback();
    this->ClearTracks();
    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->midiFileData); i++)
    {
        this->ReleaseFileData(i);
    }
}

ZunResult MidiOutput::ReadFileData(u32 idx, const char *path)
{
    if (g_Supervisor.GetMusicMode() != MIDI)
    {
        return ZUN_SUCCESS;
    }

    this->StopPlayback();
    this->ReleaseFileData(idx);

    this->midiFileData[idx] = FileSystem::OpenPath(path);

    if (this->midiFileData[idx] == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_MIDI_FAILED_TO_READ_FILE, path);
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

void MidiOutput::ReleaseFileData(u32 idx)
{
    ZUN_FREE(this->midiFileData[idx]);
    this->midiFileData[idx] = NULL;
}

void MidiOutput::ClearTracks()
{
    for (i32 i = 0; i < this->numTracks; i++)
    {
        ZUN_FREE(this->tracks[i].data);
    }

    ZUN_FREE(this->tracks);
    this->tracks = NULL;
    this->numTracks = 0;
}

#pragma var_order(i, currentCursor, currentTrack, fileData, hdrLength, header, trackLength, info)
ZunResult MidiOutput::ParseFile(i32 fileIdx)
{
    this->ClearTracks();

    u8 *currentCursor = this->midiFileData[fileIdx];
    u8 *fileData = currentCursor;
    if (currentCursor == NULL)
    {
        DebugPrint(TH_JP_ERR_MIDI_NOT_LOADED);
        return ZUN_ERROR;
    }

    // This must be a byte buffer to
    // generate a security cookie...
    u8 header[sizeof(MidiHeader)];

    // Read midi header chunk
    // First, read the header len
    memcpy(&header, currentCursor, sizeof(MidiHeader));

    // Get a pointer to the end of the header chunk
    currentCursor += sizeof(MidiHeader);
    u32 hdrLength = ConvDWord(((MidiHeader *)&header)->size);

    MidiInfo *info = (MidiInfo *)currentCursor;
    currentCursor += hdrLength;

    // Read the format. Only three values of format are specified:
    //  0: the file contains a single multi-channel track
    //  1: the file contains one or more simultaneous tracks (or MIDI outputs) of a
    //  sequence
    //  2: the file contains one or more sequentially independent single-track
    //  patterns
    this->format = ConvWord(info->format);

    // Read the timebase (divisions) of this file. Note that this doesn't appear to support
    // "negative SMPTE format", which happens when the MSB is set.
    this->timebase = ConvWord(info->timeBase);
    // Read the number of tracks in this midi file.
    this->numTracks = ConvWord(info->numTracks);

    this->tracks = ZUN_ALLOC_ARRAY(MidiTrack, this->numTracks);
    memset(this->tracks, 0, sizeof(MidiTrack) * this->numTracks);

    u32 trackLength;
    MidiTrackHeader *currentTrack;

    i32 i;
    for (i = 0; i < this->numTracks; i += 1)
    {
        currentTrack = (MidiTrackHeader *)currentCursor;
        currentCursor += sizeof(MidiTrackHeader);

        // Read a track (MTrk) chunk.
        //
        // First, read the length of the chunk
        trackLength = ConvDWord(currentTrack->size);
        this->tracks[i].size = trackLength;
        this->tracks[i].data = ZUN_ALLOC(trackLength);
        this->tracks[i].play = true;
        memcpy(this->tracks[i].data, currentCursor, trackLength);
        currentCursor += trackLength;
    }
    this->tempo = 1000000;
    return ZUN_SUCCESS;
}

ZunResult MidiOutput::LoadFile(const char *midiPath)
{
    if (this->ReadFileData(0x1f, midiPath) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    this->ParseFile(0x1f);
    this->ReleaseFileData(0x1f);

    return ZUN_SUCCESS;
}

void MidiOutput::LoadTracks()
{
    i32 trackIndex;
    MidiTrack *track = this->tracks;

    this->fadeOutVolumeMultiplier = 1.0f;
    this->unk2dc = 0;
    this->fadeOutFlag = false;
    this->playcount1 = 0;
    this->playcount2 = 0;

    for (trackIndex = 0; trackIndex < this->numTracks; trackIndex++, track++)
    {
        track->curTrackDataCursor = track->data;
        track->loopTrackDataCursor = track->curTrackDataCursor;
        track->play = true;
        track->count = GetDeltaTime(&track->curTrackDataCursor);
    }
}

ZunResult MidiOutput::Play()
{
    if (this->tracks == NULL)
    {
        return ZUN_ERROR;
    }

    this->LoadTracks();
    this->midiOutDev.OpenDevice(0xFFFFFFFF);
    this->StartTimer(1, NULL, NULL);

    return ZUN_SUCCESS;
}

ZunResult MidiOutput::StopPlayback()
{
    if (this->tracks == NULL)
    {
        return ZUN_ERROR;
    }
    else
    {
        for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->midiHeaders); i++)
        {
            if (this->midiHeaders[this->midiHeadersCursor] != NULL)
            {
                this->UnprepareHeader(this->midiHeaders[this->midiHeadersCursor]);
            }
        }

        this->StopTimer();
        this->midiOutDev.Close();

        return ZUN_SUCCESS;
    }
}

ZunResult MidiOutput::UnprepareHeader(LPMIDIHDR pmh)
{
    if (pmh == NULL)
    {
        DebugPrint("error :\n");
    }

    if (this->midiOutDev.handle == 0)
    {
        DebugPrint("error :\n");
    }

    i32 i;
    for (i = 0; i < ARRAY_SIZE_SIGNED(this->midiHeaders); i++)
    {
        if (this->midiHeaders[i] == pmh)
        {
            this->midiHeaders[i] = NULL;
            goto success;
        }
    }

    return ZUN_ERROR;

success:
    MMRESULT res = midiOutUnprepareHeader(this->midiOutDev.handle, pmh, sizeof(*pmh));
    if (res != MMSYSERR_NOERROR)
    {
        DebugPrint("error :\n");
    }

    ZUN_FREE(pmh->lpData);
    ZUN_FREE(pmh);
    return ZUN_SUCCESS;
}

u32 MidiOutput::SetFadeOut(u32 ms)
{
    this->fadeOutVolumeMultiplier = 0.0f;
    this->fadeOutInterval = ms;
    this->fadeOutElapsedMS = 0;
    this->unk2dc = 0;
    this->fadeOutFlag = true;

    return 0;
}

#pragma var_order(trackIndex, now, trackLoaded)
void MidiOutput::OnTimerElapsed()
{
    ULONGLONG now;
    i32 trackIndex;
    BOOL trackLoaded;

    trackLoaded = false;
    now = this->playcount2 + (this->playcount1 * this->timebase * 1000) / this->tempo;
    if (this->fadeOutFlag)
    {
        if (this->fadeOutElapsedMS < this->fadeOutInterval)
        {
            this->fadeOutVolumeMultiplier = 1.0f - (f32)this->fadeOutElapsedMS / (f32)this->fadeOutInterval;
            if ((u32)(this->fadeOutVolumeMultiplier * 128.0f) != this->fadeOutLastSetVolume)
            {
                this->FadeOutSetVolume(0);
            }
            this->fadeOutLastSetVolume = this->fadeOutVolumeMultiplier * 128.0f;
            this->fadeOutElapsedMS++;
        }
        else
        {
            this->fadeOutVolumeMultiplier = 0.0f;
            return;
        }
    }
    for (trackIndex = 0; trackIndex < this->numTracks; trackIndex++)
    {
        if (this->tracks[trackIndex].play)
        {
            trackLoaded = true;
            while (this->tracks[trackIndex].play && this->tracks[trackIndex].count <= now)
            {
                this->ProcessMsg(&this->tracks[trackIndex]);
                now = this->playcount2 + (this->playcount1 * this->timebase * 1000 / this->tempo);
            }
        }
    }
    this->playcount1++;
    if (!trackLoaded)
    {
        this->LoadTracks();
    }
}

#pragma var_order(count, idx, arg2, volume, opcodeLow, opcodeHigh, opcode, arg1)
void MidiOutput::ProcessMsg(MidiTrack *track)
{
    i32 volume;
    u8 arg1, arg2;
    u8 opcode, opcodeHigh, opcodeLow;
    i32 idx;

    opcode = *track->curTrackDataCursor;
    if (opcode < MIDI_OPCODE_NOTE_OFF)
    {
        opcode = track->status;
    }
    else
    {
        track->curTrackDataCursor++;
    }
    // we AND the opcode to filter out the channel
    opcodeHigh = opcode & 0xf0;
    opcodeLow = opcode & 0x0f;
    switch (opcodeHigh)
    {
    case MIDI_OPCODE_SYSTEM_EXCLUSIVE: {
        i32 curTrackLength;
        if (opcode == MIDI_OPCODE_SYSTEM_EXCLUSIVE)
        {
            if (this->midiHeaders[this->midiHeadersCursor] != NULL)
            {
                this->UnprepareHeader(this->midiHeaders[this->midiHeadersCursor]);
            }
            MIDIHDR *midiHdr = this->midiHeaders[this->midiHeadersCursor] = ZUN_ALLOC_TYPE(MIDIHDR);
            curTrackLength = GetDeltaTime(&track->curTrackDataCursor);
            memset(midiHdr, 0, sizeof(MIDIHDR));
            midiHdr->lpData = (LPSTR)ZUN_ALLOC(curTrackLength + 1);
            midiHdr->lpData[0] = (u8)0xf0;
            midiHdr->dwFlags = 0;
            midiHdr->dwBufferLength = curTrackLength + 1;
            for (idx = 0; idx < curTrackLength; idx++)
            {
                midiHdr->lpData[idx + 1] = *(track->curTrackDataCursor++);
            }
            if (this->midiOutDev.SendLongMsg(midiHdr))
            {
                ZUN_FREE(midiHdr->lpData);
                ZUN_FREE(midiHdr);
                this->midiHeaders[this->midiHeadersCursor] = NULL;
            }
            this->midiHeadersCursor++;
            this->midiHeadersCursor = this->midiHeadersCursor % 32;
        }
        else if (opcode == MIDI_OPCODE_SYSTEM_RESET)
        {
            // Meta-Event. In a MIDI file, SYSTEM_RESET gets reused as a
            // sort of escape code to introducde its own meta-events system,
            // which are events that make sense in the context of a MIDI
            // file, but not in the context of the MIDI protocol itself.
            u8 code = *(track->curTrackDataCursor++);
            curTrackLength = GetDeltaTime(&track->curTrackDataCursor);
            // End of Track meta-event.
            if (code == 0x2f)
            {
                track->play = false;
                return;
            }
            // Set Tempo meta-event.
            if (code == 0x51)
            {
                this->playcount2 += (this->playcount1 * this->timebase * 1000 / this->tempo);
                this->playcount1 = 0;
                this->tempo = 0;
                for (idx = 0; idx < curTrackLength; idx++)
                {
                    this->tempo += (this->tempo << 8) + *(track->curTrackDataCursor++);
                }

                i32 bpm = 60000000 / this->tempo;
                break;
            }
            track->curTrackDataCursor += curTrackLength;
        }
        break;
    }
    case MIDI_OPCODE_NOTE_OFF:
    case MIDI_OPCODE_NOTE_ON:
    case MIDI_OPCODE_POLYPHONIC_AFTERTOUCH:
    case MIDI_OPCODE_MODE_CHANGE:
    case MIDI_OPCODE_PITCH_BEND_CHANGE:
        arg1 = *(track->curTrackDataCursor++);
        arg2 = *(track->curTrackDataCursor++);
        break;
    case MIDI_OPCODE_PROGRAM_CHANGE:
    case MIDI_OPCODE_CHANNEL_AFTERTOUCH:
        arg1 = *(track->curTrackDataCursor++);
        arg2 = 0;
        break;
    }
    switch (opcodeHigh)
    {
    case MIDI_OPCODE_NOTE_ON:
        if (arg2 != 0)
        {
            arg1 += this->unk2c4;
            this->channels[opcodeLow].keyPressedFlags[arg1 >> 3] |= (1 << (arg1 & 7)) & 0xff;
            break;
        }
    case MIDI_OPCODE_NOTE_OFF:
        arg1 += this->unk2c4;
        this->channels[opcodeLow].keyPressedFlags[arg1 >> 3] &= (~(1 << (arg1 & 7))) & 0xff;
        break;
    case MIDI_OPCODE_PROGRAM_CHANGE:
        // Program Change
        this->channels[opcodeLow].instrument = arg1;
        break;
    case MIDI_OPCODE_MODE_CHANGE:
        switch (arg1)
        {
        case 0:
            // Bank Select
            this->channels[opcodeLow].instrumentBank = arg2;
            break;
        case 7:
            // Channel Volume
            this->channels[opcodeLow].channelVolume = arg2;
            volume = (f32)arg2 * this->fadeOutVolumeMultiplier;
            if (volume < 0)
            {
                volume = 0;
            }
            else if (volume > 0x7f)
            {
                volume = 0x7f;
            }
            arg2 = this->channels[opcodeLow].modifiedVolume = volume;
            break;
        case 91:
            // Effects 1 Depth
            this->channels[opcodeLow].effectOneDepth = arg2;
            break;
        case 93:
            // Effects 3 Depth
            this->channels[opcodeLow].effectThreeDepth = arg2;
            break;
        case 10:
            // Pan
            this->channels[opcodeLow].pan = arg2;
            break;
        case 2: {
            // Breath control, used by ZUN as the loop start marker
            MidiTrack *track;
            for (track = &this->tracks[0], idx = 0; idx < this->numTracks; idx++, track++)
            {
                track->loopTrackDataCursor = track->curTrackDataCursor;
                track->loopCount = track->count;
            }
            this->loopTempo = this->tempo;
            this->loopPlaycount1 = this->playcount1;
            this->loopPlaycount2 = this->playcount2;
            break;
        }
        case 4: {
            // Foot controller, used by ZUN as the loop end marker
            MidiTrack *track;
            for (track = &this->tracks[0], idx = 0; idx < this->numTracks; idx++, track++)
            {
                track->curTrackDataCursor = track->loopTrackDataCursor;
                track->count = track->loopCount;
            }
            this->tempo = this->loopTempo;
            this->playcount1 = this->loopPlaycount1;
            this->playcount2 = this->loopPlaycount2;
            break;
        }
        }
        break;
    }
    if (opcode < MIDI_OPCODE_SYSTEM_EXCLUSIVE)
    {
        this->midiOutDev.SendShortMsg(opcode, arg1, arg2);
    }
    track->status = opcode;
    i32 count = GetDeltaTime(&track->curTrackDataCursor);
    track->count += count;
}

#pragma var_order(arg1, idx, volumeByte, midiStatus, volumeClamped)
void MidiOutput::FadeOutSetVolume(i32 volume)
{
    i32 volumeClamped;
    u32 volumeByte;
    i32 idx;
    i32 arg1;
    u32 midiStatus;

    if (this->unk2d4 != 0)
    {
        return;
    }
    arg1 = 7;
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->channels); idx += 1)
    {
        midiStatus = (u8)(idx + 0xb0);
        volumeClamped = (i32)(this->channels[idx].channelVolume * this->fadeOutVolumeMultiplier) + volume;
        if (volumeClamped < 0)
        {
            volumeClamped = 0;
        }
        else if (volumeClamped > 127)
        {
            volumeClamped = 127;
        }
        volumeByte = volumeClamped & 0xff;
        this->midiOutDev.SendShortMsg(midiStatus, arg1, volumeByte);
    }
}

void MidiTimer::OnTimerElapsed()
{
}
}; // namespace th06
