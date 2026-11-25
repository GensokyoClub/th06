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

BOOL MidiDevice::OpenDevice(u32 uDeviceId)
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

BOOL MidiDevice::SendLongMsg(LPMIDIHDR pmh)
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

BOOL MidiDevice::SendShortMsg(DWORD msg)
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

BOOL MidiDevice::SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte)
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
    this->divisions = 0;
    this->tempo = 0;
    this->numTracks = 0;
    this->transpose = 0;
    this->volumeMultiplier = 0.0f;
    this->prevFadeOutVolume = 0;
    this->unk2d0 = 0;
    this->isVolumeLocked = false;
    this->unk2d8 = 0;
    this->unk2dc = 0;
    this->isFadingOut = false;

    for (i32 i = 0; i < MAX_MIDI_FILES; i++)
    {
        this->midiFileData[i] = NULL;
    }

    for (i32 i = 0; i < MAX_MIDI_FILES; i++)
    {
        this->midiHeaders[i] = NULL;
    }

    this->midiHeadersCursor = 0;
}

MidiOutput::~MidiOutput()
{
    this->StopPlayback();
    this->ClearTracks();
    for (i32 i = 0; i < MAX_MIDI_FILES; i++)
    {
        this->ReleaseFileData(i);
    }
}

ZunResult MidiOutput::ReadFileData(u32 idx, const char *path)
{
    if (g_Supervisor.cfg.musicMode != MIDI)
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

    // Read the divisions of this file. Note that this doesn't appear to support
    // "negative SMPTE format", which happens when the MSB is set.
    this->divisions = ConvWord(info->divisions);
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
        trackLength = ConvDWord(currentTrack->length);
        this->tracks[i].length = trackLength;
        this->tracks[i].data = ZUN_ALLOC(trackLength);
        this->tracks[i].isPlaying = true;
        memcpy(this->tracks[i].data, currentCursor, trackLength);
        currentCursor += trackLength;
    }
    this->tempo = 1000000;
    return ZUN_SUCCESS;
}

ZunResult MidiOutput::LoadFile(const char *midiPath)
{
    if (this->ReadFileData(MAX_MIDI_FILES - 1, midiPath) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    this->ParseFile(MAX_MIDI_FILES - 1);
    this->ReleaseFileData(MAX_MIDI_FILES - 1);

    return ZUN_SUCCESS;
}

void MidiOutput::PlayTracks()
{
    i32 i;
    MidiTrack *track = this->tracks;

    this->volumeMultiplier = 1.0f;
    this->unk2dc = 0;
    this->isFadingOut = false;
    this->msSinceLastTempoEvent = 0;
    this->msOfLastTempoEvent = 0;

    for (i = 0; i < this->numTracks; i++, track++)
    {
        track->readCursor = track->data;
        track->loopPointCursor = track->readCursor;
        track->isPlaying = true;
        track->time = GetDeltaTime(&track->readCursor);
    }
}

ZunResult MidiOutput::Play()
{
    if (this->tracks == NULL)
    {
        return ZUN_ERROR;
    }

    this->PlayTracks();
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
        for (i32 i = 0; i < MAX_MIDI_FILES; i++)
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

    if (this->midiOutDev.handle == NULL)
    {
        DebugPrint("error :\n");
    }

    for (i32 i = 0; i < MAX_MIDI_FILES; i++)
    {
        if (this->midiHeaders[i] == pmh)
        {
            this->midiHeaders[i] = NULL;
            goto success;
        }
    }

    return ZUN_ERROR;

success:
    MMRESULT res = midiOutUnprepareHeader(this->midiOutDev.handle, pmh, sizeof(MIDIHDR));
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
    this->volumeMultiplier = 0.0f;
    this->fadeOutDuration = ms;
    this->fadeOutTimer = 0;
    this->unk2dc = 0;
    this->isFadingOut = true;

    return 0;
}

#pragma var_order(i, now, tracksPlaying)
void MidiOutput::OnTimerElapsed()
{
    i32 i;
    BOOL tracksPlaying = false;
    u64 now = this->msOfLastTempoEvent + this->msSinceLastTempoEvent * this->divisions * 1000 / this->tempo;
    if (this->isFadingOut)
    {
        if (this->fadeOutTimer < this->fadeOutDuration)
        {
            this->volumeMultiplier = 1.0f - (f32)this->fadeOutTimer / this->fadeOutDuration;
            if ((u32)(this->volumeMultiplier * 128.0f) != this->prevFadeOutVolume)
            {
                this->SetVolume(0);
            }
            this->prevFadeOutVolume = this->volumeMultiplier * 128.0f;
            this->fadeOutTimer++;
        }
        else
        {
            this->volumeMultiplier = 0.0f;
            return;
        }
    }
    for (i = 0; i < this->numTracks; i++)
    {
        if (this->tracks[i].isPlaying)
        {
            tracksPlaying = true;
            while (this->tracks[i].isPlaying && this->tracks[i].time <= now)
            {
                this->ProcessMsg(&this->tracks[i]);
                now = this->msOfLastTempoEvent + this->msSinceLastTempoEvent * this->divisions * 1000 / this->tempo;
            }
        }
    }
    this->msSinceLastTempoEvent++;
    if (!tracksPlaying)
    {
        this->PlayTracks();
    }
}

#pragma var_order(delta, i, arg2, volume, opcodeLow, opcodeHigh, opcode, arg1)
void MidiOutput::ProcessMsg(MidiTrack *track)
{
    i32 volume;
    u8 arg1, arg2;
    i32 i;

    u8 opcode = *track->readCursor;
    if (opcode < MIDI_OPCODE_NOTE_OFF)
    {
        opcode = track->opcode;
    }
    else
    {
        track->readCursor++;
    }
    // we AND the opcode to filter out the channel
    u8 opcodeHigh = opcode & 0xf0;
    u8 opcodeLow = opcode & 0x0f;
    switch (opcodeHigh)
    {
    case MIDI_OPCODE_SYSTEM_EXCLUSIVE: {
        i32 length;
        if (opcode == MIDI_OPCODE_SYSTEM_EXCLUSIVE)
        {
            if (this->midiHeaders[this->midiHeadersCursor] != NULL)
            {
                this->UnprepareHeader(this->midiHeaders[this->midiHeadersCursor]);
            }
            MIDIHDR *midiHdr = this->midiHeaders[this->midiHeadersCursor] = ZUN_ALLOC_TYPE(MIDIHDR);
            length = GetDeltaTime(&track->readCursor);
            memset(midiHdr, 0, sizeof(MIDIHDR));
            midiHdr->lpData = (LPSTR)ZUN_ALLOC(length + 1);
            midiHdr->lpData[0] = (u8)0xf0;
            midiHdr->dwFlags = 0;
            midiHdr->dwBufferLength = length + 1;
            for (i = 0; i < length; i++)
            {
                midiHdr->lpData[i + 1] = *track->readCursor++;
            }
            if (this->midiOutDev.SendLongMsg(midiHdr))
            {
                ZUN_FREE(midiHdr->lpData);
                ZUN_FREE(midiHdr);
                this->midiHeaders[this->midiHeadersCursor] = NULL;
            }
            this->midiHeadersCursor++;
            this->midiHeadersCursor %= MAX_MIDI_FILES;
        }
        else if (opcode == MIDI_OPCODE_SYSTEM_RESET)
        {
            // Meta-Event. In a MIDI file, SYSTEM_RESET gets reused as a
            // sort of escape code to introducde its own meta-events system,
            // which are events that make sense in the context of a MIDI
            // file, but not in the context of the MIDI protocol itself.
            u8 code = *track->readCursor++;
            length = GetDeltaTime(&track->readCursor);

            if (code == MIDI_META_EVENT_END_OF_TRACK)
            {
                track->isPlaying = false;
                return;
            }

            if (code == MIDI_META_EVENT_TEMPO_CHANGE)
            {
                this->msOfLastTempoEvent += this->msSinceLastTempoEvent * this->divisions * 1000 / this->tempo;
                this->msSinceLastTempoEvent = 0;
                this->tempo = 0;
                for (i = 0; i < length; i++)
                {
                    this->tempo += (this->tempo << CHAR_BIT) + *track->readCursor++;
                }

                i32 bpm = 60000000 / this->tempo;
                break;
            }
            track->readCursor += length;
        }
        break;
    }
    case MIDI_OPCODE_NOTE_OFF:
    case MIDI_OPCODE_NOTE_ON:
    case MIDI_OPCODE_POLYPHONIC_AFTERTOUCH:
    case MIDI_OPCODE_CONTROL_CHANGE:
    case MIDI_OPCODE_PITCH_BEND_CHANGE:
        arg1 = *track->readCursor++;
        arg2 = *track->readCursor++;
        break;
    case MIDI_OPCODE_PROGRAM_CHANGE:
    case MIDI_OPCODE_CHANNEL_AFTERTOUCH:
        arg1 = *track->readCursor++;
        arg2 = 0;
        break;
    }
    // NOTE:
    // Most of this stuff doesn't actually need to be saved/calculated
    // in the channel structs? Only the loop point handling is functional.
    // And it's not even handling all of the things in the midi file either...
    switch (opcodeHigh)
    {
    case MIDI_OPCODE_NOTE_ON:
        if (arg2 != 0)
        {
            arg1 += this->transpose;
            this->channels[opcodeLow].keysPressed[arg1 >> 3] |= (u8)(1 << (arg1 & 7));
            break;
        }
        // fallthrough
    case MIDI_OPCODE_NOTE_OFF:
        arg1 += this->transpose;
        this->channels[opcodeLow].keysPressed[arg1 >> 3] &= (u8) ~(1 << (arg1 & 7));
        break;
    case MIDI_OPCODE_PROGRAM_CHANGE:
        this->channels[opcodeLow].program = arg1;
        break;
    case MIDI_OPCODE_CONTROL_CHANGE:
        switch (arg1)
        {
        case MIDI_CC_BANK_SELECT_MSB:
            this->channels[opcodeLow].bankMSB = arg2;
            break;
        case MIDI_CC_VOLUME:
            this->channels[opcodeLow].rawVolume = arg2;
            volume = arg2 * this->volumeMultiplier;
            if (volume < 0)
            {
                volume = 0;
            }
            else if (volume > 127)
            {
                volume = 127;
            }
            arg2 = this->channels[opcodeLow].mixedVolume = volume;
            break;
        case MIDI_CC_REVERB_DEPTH:
            this->channels[opcodeLow].reverbDepth = arg2;
            break;
        case MIDI_CC_CHORUS_DEPTH:
            this->channels[opcodeLow].chorusDepth = arg2;
            break;
        case MIDI_CC_PAN:
            this->channels[opcodeLow].pan = arg2;
            break;
        case MIDI_CC_BREATH_CONTROL: {
            // Used by ZUN as the loop start marker
            MidiTrack *track = &this->tracks[0];
            for (i = 0; i < this->numTracks; i++, track++)
            {
                track->loopPointCursor = track->readCursor;
                track->loopPointTime = track->time;
            }
            this->loopTempo = this->tempo;
            this->loopMsSinceLastTempoEvent = this->msSinceLastTempoEvent;
            this->loopMsOfLastTempoEvent = this->msOfLastTempoEvent;
            break;
        }
        case MIDI_CC_FOOT_PEDAL: {
            // Used by ZUN as the loop end marker
            MidiTrack *track = &this->tracks[0];
            for (i = 0; i < this->numTracks; i++, track++)
            {
                track->readCursor = track->loopPointCursor;
                track->time = track->loopPointTime;
            }
            this->tempo = this->loopTempo;
            this->msSinceLastTempoEvent = this->loopMsSinceLastTempoEvent;
            this->msOfLastTempoEvent = this->loopMsOfLastTempoEvent;
            break;
        }
        }
        break;
    }
    if (opcode < MIDI_OPCODE_SYSTEM_EXCLUSIVE)
    {
        this->midiOutDev.SendShortMsg(opcode, arg1, arg2);
    }
    track->opcode = opcode;
    i32 delta = GetDeltaTime(&track->readCursor);
    track->time += delta;
}

#pragma var_order(arg1, i, volumeByte, midiStatus, volumeClamped)
void MidiOutput::SetVolume(i32 volume)
{
    i32 volumeClamped;
    u32 volumeByte;
    i32 i;
    i32 arg1;
    u32 midiStatus;

    if (this->isVolumeLocked)
    {
        return;
    }
    arg1 = MIDI_CC_VOLUME;
    for (i = 0; i < MIDI_CHANNEL_COUNT; i++)
    {
        midiStatus = (u8)(i + MIDI_OPCODE_CONTROL_CHANGE);
        volumeClamped = (i32)(this->channels[i].rawVolume * this->volumeMultiplier) + volume;
        if (volumeClamped < 0)
        {
            volumeClamped = 0;
        }
        else if (volumeClamped > 127)
        {
            volumeClamped = 127;
        }
        volumeByte = (u8)volumeClamped;
        this->midiOutDev.SendShortMsg(midiStatus, arg1, volumeByte);
    }
}

void MidiTimer::OnTimerElapsed()
{
}
}; // namespace th06
