#include "MidiCoreAudio.hpp"
#include "utils.hpp"

static AudioUnit CreateUnit(OSType type, OSType subType)
{
    AudioComponentDescription desc = {};
    desc.componentType = type;
    desc.componentSubType = subType;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;

    AudioComponent component = AudioComponentFindNext(NULL, &desc);
    if (component == NULL)
    {
        return NULL;
    }

    AudioUnit unit = NULL;
    if (AudioComponentInstanceNew(component, &unit) != noErr)
    {
        return NULL;
    }

    return unit;
}

MidiDevice::MidiDevice()
{
    this->synthUnit = NULL;
    this->outputUnit = NULL;
}

MidiDevice::~MidiDevice()
{
    this->Close();
}

bool MidiDevice::OpenDevice(u32 uDeviceId)
{
    (void)uDeviceId;

    if (this->synthUnit != NULL)
    {
        return true;
    }

    this->synthUnit = CreateUnit(kAudioUnitType_MusicDevice, kAudioUnitSubType_DLSSynth);
    if (this->synthUnit == NULL)
    {
        utils::DebugPrint2("error : couldn't create the DLS synth audio unit\n");
        this->Close();
        return false;
    }

    this->outputUnit = CreateUnit(kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput);
    if (this->outputUnit == NULL)
    {
        utils::DebugPrint2("error : couldn't create the default output audio unit\n");
        this->Close();
        return false;
    }

    AudioUnitConnection connection;
    connection.sourceAudioUnit = this->synthUnit;
    connection.sourceOutputNumber = 0;
    connection.destInputNumber = 0;
    if (AudioUnitSetProperty(this->outputUnit, kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input, 0, &connection,
                             sizeof(connection)) != noErr)
    {
        utils::DebugPrint2("error : couldn't connect the synth to the audio output\n");
        this->Close();
        return false;
    }

    if (AudioUnitInitialize(this->synthUnit) != noErr || AudioUnitInitialize(this->outputUnit) != noErr ||
        AudioOutputUnitStart(this->outputUnit) != noErr)
    {
        utils::DebugPrint2("error : couldn't start the audio output\n");
        this->Close();
        return false;
    }

    utils::DebugPrint2("Playing midi through the DLS software synthesizer");

    return true;
}

ZunResult MidiDevice::Close()
{
    if (this->synthUnit == NULL && this->outputUnit == NULL)
    {
        return ZUN_ERROR;
    }

    if (this->outputUnit != NULL)
    {
        AudioOutputUnitStop(this->outputUnit);
        AudioUnitUninitialize(this->outputUnit);
        AudioComponentInstanceDispose(this->outputUnit);
        this->outputUnit = NULL;
    }

    if (this->synthUnit != NULL)
    {
        AudioUnitUninitialize(this->synthUnit);
        AudioComponentInstanceDispose(this->synthUnit);
        this->synthUnit = NULL;
    }

    return ZUN_SUCCESS;
}

bool MidiDevice::SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte)
{
    if (this->synthUnit == NULL)
    {
        return true;
    }

    return MusicDeviceMIDIEvent(this->synthUnit, midiStatus, firstByte, secondByte, 0) == noErr;
}

bool MidiDevice::SendLongMsg(const u8 *buf, u32 len)
{
    if (this->synthUnit == NULL)
    {
        return true;
    }

    return MusicDeviceSysEx(this->synthUnit, buf, len) == noErr;
}
