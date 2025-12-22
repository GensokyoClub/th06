#include "MidiDefault.hpp"
#include "GameErrorContext.hpp"
#include "i18n.hpp"

MidiDevice::MidiDevice() {
    printedWarning = false;
}

MidiDevice::~MidiDevice() {
}

bool MidiDevice::OpenDevice(u32 uDeviceId) {
    (void)uDeviceId;

    if (!printedWarning) {
        GameErrorContext::Log(&g_GameErrorContext, TH_ERR_NO_MIDI_SUPPORT);
        printedWarning = true;
    }

    return true;
}

bool MidiDevice::Close() {
    return true;
}

bool MidiDevice::SendLongMsg(u8 *buf, u32 len) {
    return true;
}

bool MidiDevice::SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte) {
    return true;
}
