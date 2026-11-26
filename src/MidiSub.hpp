#pragma once

#include "decomp.hpp"
#include <Windows.h>

namespace th06
{
// SMF Header (Standard MIDI File)
typedef struct tagSMFHeader
{
    u32 magic;
    u32 size;
} SMFHeader;
typedef SMFHeader MidiHeader;

// SMF Info
typedef struct tagSMFMainInfo
{
    u16 format;
    u16 numTracks;
    u16 divisions;
} SMFMainInfo;
typedef SMFMainInfo MidiInfo;

// SMF Track Header
typedef struct tagSMFTrack
{
    u32 magic;
    u32 length;
} SMFTrack;
typedef SMFTrack MidiTrackHeader;

enum MidiOpcode
{
    MIDI_OPCODE_CHANNEL_1 = 0x01,
    MIDI_OPCODE_CHANNEL_2 = 0x02,
    MIDI_OPCODE_CHANNEL_3 = 0x03,
    MIDI_OPCODE_CHANNEL_4 = 0x04,
    MIDI_OPCODE_CHANNEL_5 = 0x05,
    MIDI_OPCODE_CHANNEL_6 = 0x06,
    MIDI_OPCODE_CHANNEL_7 = 0x07,
    MIDI_OPCODE_CHANNEL_8 = 0x08,
    MIDI_OPCODE_CHANNEL_9 = 0x09,
    MIDI_OPCODE_CHANNEL_A = 0x0A,
    MIDI_OPCODE_CHANNEL_B = 0x0B,
    MIDI_OPCODE_CHANNEL_C = 0x0C,
    MIDI_OPCODE_CHANNEL_D = 0x0D,
    MIDI_OPCODE_CHANNEL_E = 0x0E,
    MIDI_OPCODE_CHANNEL_F = 0x0F,
    MIDI_OPCODE_NOTE_OFF = 0x80,
    MIDI_OPCODE_NOTE_ON = 0x90,
    MIDI_OPCODE_POLYPHONIC_AFTERTOUCH = 0xA0,
    MIDI_OPCODE_CONTROL_CHANGE = 0xB0,
    MIDI_OPCODE_PROGRAM_CHANGE = 0xC0,
    MIDI_OPCODE_CHANNEL_AFTERTOUCH = 0xD0,
    MIDI_OPCODE_PITCH_BEND_CHANGE = 0xE0,
    MIDI_OPCODE_SYSTEM_EXCLUSIVE = 0xF0,
    MIDI_OPCODE_MIDI_TIME_CODE_QTR_FRAME = 0xF1,
    MIDI_OPCODE_SONG_POSITION_POINTER = 0xF2,
    MIDI_OPCODE_SONG_SELECT = 0xF3,
    MIDI_OPCODE_RESERVED_F4 = 0xF4,
    MIDI_OPCODE_RESERVED_F5 = 0xF5,
    MIDI_OPCODE_TUNE_REQUEST = 0xF6,
    MIDI_OPCODE_END_OF_SYSEX = 0xF7,
    MIDI_OPCODE_TIMING_CLOCK = 0xF8,
    MIDI_OPCODE_RESERVED_F9 = 0xF9,
    MIDI_OPCODE_START = 0xFA,
    MIDI_OPCODE_CONTINUE = 0xFB,
    MIDI_OPCODE_STOP = 0xFC,
    MIDI_OPCODE_RESERVED_FD = 0xFD,
    MIDI_OPCODE_ACTIVE_SENSING = 0xFE,
    MIDI_OPCODE_SYSTEM_RESET = 0xFF,
};

enum MidiMetaEvent
{
    MIDI_META_EVENT_END_OF_TRACK = 0x2f,
    MIDI_META_EVENT_TEMPO_CHANGE = 0x51
};

enum MidiCC
{
    MIDI_CC_BANK_SELECT_MSB = 0,
    MIDI_CC_BREATH_CONTROL = 2,
    MIDI_CC_FOOT_PEDAL = 4,
    MIDI_CC_VOLUME = 7,
    MIDI_CC_PAN = 10,
    MIDI_CC_REVERB_DEPTH = 91,
    MIDI_CC_CHORUS_DEPTH = 93
};

struct MidiTrack
{
    BOOL isPlaying;
    i32 time;
    u32 length;
    u8 opcode;
    alignment_padding(0x3);
    u8 *data;
    u8 *readCursor;
    u8 *loopPointCursor;
    i32 loopPointTime;
};
ZUN_ASSERT_TYPE(MidiTrack, 0x20, 4);

} // namespace th06
