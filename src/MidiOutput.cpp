#include "inttypes.hpp"
#include <Windows.h>
#include <mmreg.h>
#include <mmsystem.h>

#include "FileSystem.hpp"
#include "MidiOutput.hpp"
#include "Supervisor.hpp"
#include "ZunMemory.hpp"
#include "i18n.hpp"
#include "utils.hpp"

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
    if (this->handle != 0)
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
    if (this->handle == 0)
    {
        return ZUN_ERROR;
    }

    midiOutReset(this->handle);
    midiOutClose(this->handle);
    this->handle = 0;

    return ZUN_SUCCESS;
}

union MidiShortMsg {
    struct
    {
        u8 midiStatus;
        i8 firstByte;
        i8 secondByte;
        i8 unused;
    } msg;
    u32 dwMsg;
};

ZunBool MidiDevice::SendLongMsg(LPMIDIHDR pmh)
{
    if (this->handle == 0)
    {
        return false;
    }
    else
    {
        if (midiOutPrepareHeader(this->handle, pmh, sizeof(*pmh)) != MMSYSERR_NOERROR)
        {
            return true;
        }

        return midiOutLongMsg(this->handle, pmh, sizeof(*pmh)) != MMSYSERR_NOERROR;
    }
}

ZunBool MidiDevice::SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte)
{
    MidiShortMsg pkt;

    if (this->handle == 0)
    {
        return false;
    }
    else
    {
        pkt.msg.midiStatus = midiStatus;
        pkt.msg.firstByte = firstByte;
        pkt.msg.secondByte = secondByte;
        return midiOutShortMsg(this->handle, pkt.dwMsg) != MMSYSERR_NOERROR;
    }
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
        this->timerId = timeSetEvent(delay, this->timeCaps.wPeriodMin, (LPTIMECALLBACK)MidiTimer::DefaultTimerCallback,
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

void CALLBACK MidiTimer::DefaultTimerCallback(u32 uTimerID, u32 uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    MidiTimer *timer = (MidiTimer *)dwUser;

    timer->OnTimerElapsed();
}

u16 MidiOutput::Ntohs(u16 val)
{
    u8 tmp[2];

    tmp[0] = ((u8 *)&val)[1];
    tmp[1] = ((u8 *)&val)[0];

    return *(const u16 *)(&tmp);
}

u32 MidiOutput::SkipVariableLength(u8 **curTrackDataCursor)
{
    u32 length;
    u8 tmp;

    length = 0;
    do
    {
        tmp = **curTrackDataCursor;
        *curTrackDataCursor = *curTrackDataCursor + 1;
        length = length * 0x80 + (tmp & 0x7f);
    } while ((tmp & 0x80) != 0);

    return length;
}

MidiOutput::MidiOutput()
{
    this->tracks = NULL;
    this->divisions = 0;
    this->tempo = 0;
    this->numTracks = 0;
    this->unk2c4 = 0;
    this->fadeOutVolumeMultiplier = 0;
    this->fadeOutLastSetVolume = 0;
    this->unk2d0 = 0;
    this->unk2d4 = 0;
    this->unk2d8 = 0;
    this->unk2dc = 0;
    this->fadeOutFlag = 0;

    for (int i = 0; i < ARRAY_SIZE_SIGNED(this->midiFileData); i++)
    {
        this->midiFileData[i] = NULL;
    }

    for (int i = 0; i < ARRAY_SIZE_SIGNED(this->midiHeaders); i++)
    {
        this->midiHeaders[i] = NULL;
    }

    this->midiHeadersCursor = 0;
}

MidiOutput::~MidiOutput()
{
    this->StopPlayback();
    this->ClearTracks();
    for (i32 i = 0; i < 32; i++)
    {
        this->ReleaseFileData(i);
    }
}

ZunResult MidiOutput::ReadFileData(u32 idx, char *path)
{
    if (g_Supervisor.cfg.musicMode != MIDI)
    {
        return ZUN_SUCCESS;
    }

    this->StopPlayback();
    this->ReleaseFileData(idx);

    this->midiFileData[idx] = FileSystem::OpenPath(path, false);

    if (this->midiFileData[idx] == (byte *)0x0)
    {
        g_GameErrorContext.Log(TH_ERR_MIDI_FAILED_TO_READ_FILE, path);
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

void MidiOutput::ReleaseFileData(u32 idx)
{
    u8 *data = this->midiFileData[idx];
    free(data);

    this->midiFileData[idx] = NULL;
}

#pragma var_order(trackIndex, data, tracks)
void MidiOutput::ClearTracks()
{
    i32 trackIndex;
    u8 *data;
    MidiTrack *tracks;

    for (trackIndex = 0; trackIndex < this->numTracks; trackIndex++)
    {
        data = this->tracks[trackIndex].trackData;
        free(data);
    }

    tracks = this->tracks;
    free(tracks);
    this->tracks = NULL;
    this->numTracks = 0;
}

#pragma var_order(trackIdx, currentCursor, currentCursorTrack, fileData, hdrLength, hdrRaw, trackLength,               \
                  endOfHeaderPointer)
ZunResult MidiOutput::ParseFile(i32 fileIdx)
{
    u8 hdrRaw[8];
    u32 trackLength;
    u8 *currentCursor, *currentCursorTrack, *endOfHeaderPointer;
    i32 trackIdx;
    u8 *fileData;
    u32 hdrLength;

    this->ClearTracks();
    currentCursor = this->midiFileData[fileIdx];
    fileData = currentCursor;
    if (currentCursor == NULL)
    {
        utils::DebugPrint2(TH_JP_ERR_MIDI_NOT_LOADED);
        return ZUN_ERROR;
    }

    // Read midi header chunk
    // First, read the header len
    memcpy(&hdrRaw, currentCursor, 8);

    // Get a pointer to the end of the header chunk
    currentCursor += sizeof(hdrRaw);
    hdrLength = MidiOutput::Ntohl(*(u32 *)(hdrRaw + 4));

    endOfHeaderPointer = currentCursor;
    currentCursor += hdrLength;

    // Read the format. Only three values of format are specified:
    //  0: the file contains a single multi-channel track
    //  1: the file contains one or more simultaneous tracks (or MIDI outputs) of a
    //  sequence
    //  2: the file contains one or more sequentially independent single-track
    //  patterns
    this->format = MidiOutput::Ntohs(*(u16 *)endOfHeaderPointer);

    // Read the divisions in this track. Note that this doesn't appear to support
    // "negative SMPTE format", which happens when the MSB is set.
    this->divisions = MidiOutput::Ntohs(*(u16 *)(endOfHeaderPointer + 4));
    // Read the number of tracks in this midi file.
    this->numTracks = MidiOutput::Ntohs(*(u16 *)(endOfHeaderPointer + 2));

    // Allocate this->divisions * 32 bytes.
    this->tracks = (MidiTrack *)ZunMemory::Alloc(sizeof(MidiTrack) * this->numTracks);
    memset(this->tracks, 0, sizeof(MidiTrack) * this->numTracks);
    for (trackIdx = 0; trackIdx < this->numTracks; trackIdx += 1)
    {
        currentCursorTrack = currentCursor;
        currentCursor += 8;

        // Read a track (MTrk) chunk.
        //
        // First, read the length of the chunk
        trackLength = MidiOutput::Ntohl(*(u32 *)(currentCursorTrack + 4));
        this->tracks[trackIdx].trackLength = trackLength;
        this->tracks[trackIdx].trackData = (u8 *)ZunMemory::Alloc(trackLength);
        this->tracks[trackIdx].trackPlaying = 1;
        memcpy(this->tracks[trackIdx].trackData, currentCursor, trackLength);
        currentCursor += trackLength;
    }
    this->tempo = 1000000;
    return ZUN_SUCCESS;
}

ZunResult MidiOutput::LoadFile(char *midiPath)
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

    this->fadeOutVolumeMultiplier = 1.0;
    this->unk2dc = 0;
    this->fadeOutFlag = 0;
    this->volume = 0;
    this->unk130 = 0;

    for (trackIndex = 0; trackIndex < this->numTracks; trackIndex++, track++)
    {
        track->curTrackDataCursor = track->trackData;
        track->startTrackDataMaybe = track->curTrackDataCursor;
        track->trackPlaying = 1;
        track->trackLengthOther = MidiOutput::SkipVariableLength(&track->curTrackDataCursor);
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
        utils::DebugPrint2("error :\n");
    }

    if (this->midiOutDev.handle == 0)
    {
        utils::DebugPrint2("error :\n");
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
        utils::DebugPrint2("error :\n");
    }

    void *lpData = pmh->lpData;
    free(lpData);
    free(pmh);
    return ZUN_SUCCESS;
}

u32 MidiOutput::SetFadeOut(u32 ms)
{
    this->fadeOutVolumeMultiplier = 0.0;
    this->fadeOutInterval = ms;
    this->fadeOutElapsedMS = 0;
    this->unk2dc = 0;
    this->fadeOutFlag = 1;

    return 0;
}

#pragma var_order(trackIndex, local_14, trackLoaded)
void MidiOutput::OnTimerElapsed()
{
    unsigned __int64 local_14;
    i32 trackIndex;
    BOOL trackLoaded;

    trackLoaded = false;
    // longlong multiplication. Oh god.
    local_14 = this->unk130 + (this->volume * this->divisions * 1000) / this->tempo;
    if (this->fadeOutFlag != 0)
    {
        if (this->fadeOutElapsedMS < this->fadeOutInterval)
        {
            this->fadeOutVolumeMultiplier = 1.0f - (f32)this->fadeOutElapsedMS / (f32)this->fadeOutInterval;
            if ((u32)(this->fadeOutVolumeMultiplier * 128.0f) != this->fadeOutLastSetVolume)
            {
                this->FadeOutSetVolume(0);
            }
            this->fadeOutLastSetVolume = this->fadeOutVolumeMultiplier * 128.0f;
            this->fadeOutElapsedMS = this->fadeOutElapsedMS + 1;
        }
        else
        {
            this->fadeOutVolumeMultiplier = 0.0;
            return;
        }
    }
    for (trackIndex = 0; trackIndex < this->numTracks; trackIndex += 1)
    {
        if (this->tracks[trackIndex].trackPlaying)
        {
            trackLoaded = true;
            while (this->tracks[trackIndex].trackPlaying)
            {
                if (this->tracks[trackIndex].trackLengthOther <= local_14)
                {
                    this->ProcessMsg(&this->tracks[trackIndex]);
                    local_14 = this->unk130 + (this->volume * this->divisions * 1000 / this->tempo);
                    continue;
                }
                break;
            }
        }
    }
    this->volume += 1;
    if (!trackLoaded)
    {
        this->LoadTracks();
    }
    return;
}

#pragma var_order(nextTrackLength, idx, arg2, lVar5, opcodeLow, opcodeHigh, opcode, arg1, curTrackLength, midiHdr,     \
                  cVar1, unk24, local_2c, local_30, midiHeaderSize, lpdata)
void MidiOutput::ProcessMsg(MidiTrack *track)
{
    i32 lVar5;
    i32 curTrackLength, nextTrackLength;
    MidiTrack *local_30;
    MidiTrack *local_2c;
    u8 arg1, arg2;
    u8 opcode, opcodeHigh, opcodeLow;
    u8 cVar1;
    size_t midiHeaderSize;
    MIDIHDR *midiHdr;
    i32 idx;
    LPSTR lpdata;
    i32 unk24;

    opcode = *track->curTrackDataCursor;
    if (opcode < MIDI_OPCODE_NOTE_OFF)
    {
        opcode = track->opcode;
    }
    else
    {
        track->curTrackDataCursor += 1;
    }
    // we AND the opcode to filter out the channel
    opcodeHigh = opcode & 0xf0;
    opcodeLow = opcode & 0x0f;
    switch (opcodeHigh)
    {
    case MIDI_OPCODE_SYSTEM_EXCLUSIVE:
        if (opcode == MIDI_OPCODE_SYSTEM_EXCLUSIVE)
        {
            if (this->midiHeaders[this->midiHeadersCursor] != NULL)
            {
                this->UnprepareHeader(this->midiHeaders[this->midiHeadersCursor]);
            }
            midiHeaderSize = sizeof(MIDIHDR);
            midiHdr = this->midiHeaders[this->midiHeadersCursor] = (MIDIHDR *)malloc(midiHeaderSize);
            curTrackLength = MidiOutput::SkipVariableLength(&track->curTrackDataCursor);
            memset(midiHdr, 0, sizeof(MIDIHDR));
            midiHdr->lpData = (LPSTR)malloc(curTrackLength + 1);
            midiHdr->lpData[0] = -0x10;
            midiHdr->dwFlags = 0;
            midiHdr->dwBufferLength = curTrackLength + 1;
            for (idx = 0; idx < curTrackLength; idx += 1)
            {
                midiHdr->lpData[idx + 1] = *track->curTrackDataCursor;
                track->curTrackDataCursor += 1;
            }
            if (this->midiOutDev.SendLongMsg(midiHdr))
            {
                lpdata = midiHdr->lpData;
                free(lpdata);
                free(midiHdr);
                this->midiHeaders[this->midiHeadersCursor] = NULL;
            }
            this->midiHeadersCursor += 1;
            this->midiHeadersCursor = this->midiHeadersCursor % 32;
        }
        else if (opcode == MIDI_OPCODE_SYSTEM_RESET)
        {
            // Meta-Event. In a MIDI file, SYSTEM_RESET gets reused as a
            // sort of escape code to introducde its own meta-events system,
            // which are events that make sense in the context of a MIDI
            // file, but not in the context of the MIDI protocol itself.
            cVar1 = *track->curTrackDataCursor;
            track->curTrackDataCursor += 1;
            curTrackLength = MidiOutput::SkipVariableLength(&track->curTrackDataCursor);
            // End of Track meta-event.
            if (cVar1 == 0x2f)
            {
                track->trackPlaying = 0;
                return;
            }
            // Set Tempo meta-event.
            if (cVar1 == 0x51)
            {
                this->unk130 += (this->volume * this->divisions * 1000 / this->tempo);
                this->volume = 0;
                this->tempo = 0;
                for (idx = 0; idx < curTrackLength; idx += 1)
                {
                    this->tempo += this->tempo * 0x100 + *track->curTrackDataCursor;
                    track->curTrackDataCursor += 1;
                }
                unk24 = 60000000 / this->tempo;
                break;
            }
            track->curTrackDataCursor = track->curTrackDataCursor + curTrackLength;
        }
        break;
    case MIDI_OPCODE_NOTE_OFF:
    case MIDI_OPCODE_NOTE_ON:
    case MIDI_OPCODE_POLYPHONIC_AFTERTOUCH:
    case MIDI_OPCODE_MODE_CHANGE:
    case MIDI_OPCODE_PITCH_BEND_CHANGE:
        arg1 = *track->curTrackDataCursor;
        track->curTrackDataCursor += 1;
        arg2 = *track->curTrackDataCursor;
        track->curTrackDataCursor += 1;
        break;
    case MIDI_OPCODE_PROGRAM_CHANGE:
    case MIDI_OPCODE_CHANNEL_AFTERTOUCH:
        arg1 = *track->curTrackDataCursor;
        track->curTrackDataCursor += 1;
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
            lVar5 = (f32)arg2 * this->fadeOutVolumeMultiplier;
            if (lVar5 < 0)
            {
                lVar5 = 0;
            }
            else if (0x7f < lVar5)
            {
                lVar5 = 0x7f;
            }
            arg2 = this->channels[opcodeLow].modifiedVolume = lVar5;
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
        case 2:
            // Breath control
            for (local_2c = &this->tracks[0], idx = 0; idx < this->numTracks; idx += 1, local_2c += 1)
            {
                local_2c->startTrackDataMaybe = local_2c->curTrackDataCursor;
                local_2c->unk1c = local_2c->trackLengthOther;
            }
            this->unk2ec = this->tempo;
            this->unk2f0 = this->volume;
            this->unk2f8 = this->unk130;
            break;
        case 4:
            // Foot controller
            for (local_30 = &this->tracks[0], idx = 0; idx < this->numTracks; idx += 1, local_30 += 1)
            {
                local_30->curTrackDataCursor = (byte *)local_30->startTrackDataMaybe;
                local_30->trackLengthOther = local_30->unk1c;
            }
            this->tempo = this->unk2ec;
            this->volume = this->unk2f0;
            this->unk130 = this->unk2f8;
            break;
        }
        break;
    }
    if (opcode < MIDI_OPCODE_SYSTEM_EXCLUSIVE)
    {
        this->midiOutDev.SendShortMsg(opcode, arg1, arg2);
    }
    track->opcode = opcode;
    nextTrackLength = MidiOutput::SkipVariableLength(&track->curTrackDataCursor);
    track->trackLengthOther = track->trackLengthOther + nextTrackLength;
    return;
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
        midiStatus = (idx + 0xb0) & 0xff;
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
    return;
}

}; // namespace th06
