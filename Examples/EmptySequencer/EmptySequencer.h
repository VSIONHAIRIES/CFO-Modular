#ifndef EMPTY_SEQUENCER_H
#define EMPTY_SEQUENCER_H

#include <Arduino.h>
#include "MIDI.h"
#include "Sequencer.h"
#include "Teensy3DAC.h"

IntervalTimer isrTimer;
int sample_rate = 48000;

// Declare the synth modules you want to use
MIDI midi;
Sequencer seq;
Teensy3DAC t3dac(sample_rate);

// Forward declaration of setupMidiAndSound function used at the bottom
void setupMidiAndSound();

// Some helper variables for module inputs
int noSignal = 0;
int halfSignal = SIGNED_BIT_32_HIGH >> 1;
int quarterSignal = SIGNED_BIT_32_HIGH >> 2;
int fullSignal = SIGNED_BIT_32_HIGH;
int lowSignal = SIGNED_BIT_32_LOW;

// The Interrupt Service Routine (ISR) for the synthesizer
void synth_isr() {
    t3dac.process();
}

// Creating the class for this type of synth
class EmptySequencer {
public:
    EmptySequencer() {}
    ~EmptySequencer() {}
    void start();
};

void EmptySequencer::start() {

    // Below hook up the various modules to each other
    // The order doesn't matter

    t3dac.audioIn_ptr = &noSignal;

    // This function sets up MIDI and Sequencer objects,
    // prepares the DAC and initiate the synth ISR
    setupMidiAndSound();
}


//////////////////////////////////////////////
// THE BELOW FUNCTIONS ARE NEEDED FOR THE   //
// SYSTEM TO PLAY SOUND AND RESPOND TO MIDI //
//////////////////////////////////////////////

void OnNoteOn(byte channel, byte note, byte velocity) {
    channel = channel - 1;
    Serial.print("NOTEON on channel ");
    Serial.println(channel);
    if(channel == midi.getChannel()) {
        midi.noteOn(channel, note, velocity);
    }
    MIDI_SERIAL.write(byte(0x90 | (channel & 0x0F)));
    MIDI_SERIAL.write(byte(0x7F & note));
    MIDI_SERIAL.write(byte(0x7F & velocity));
}


void OnNoteOff(byte channel, byte note, byte velocity) {
    channel = channel - 1;
    Serial.print("NOTEOFF on channel ");
    if(channel == midi.getChannel()) {
        midi.noteOff(channel, note, velocity);
    }
    channel = channel - 1;
    MIDI_SERIAL.write(byte(0x80 | (channel & 0x0F)));
    MIDI_SERIAL.write(byte(0x7F & note));
    MIDI_SERIAL.write(byte(0x7F & velocity));
}


void OnControlChange(byte channel, byte control, byte value) {
    channel = channel - 1;
    Serial.print("CONTROLLER on channel ");
    if(channel == midi.getChannel()) {
        midi.controller(channel, control, value);
    }
    channel = channel - 1;
    MIDI_SERIAL.write(byte(0xB0 | (channel & 0x0F)));
    MIDI_SERIAL.write(byte(0x7F & control));
    MIDI_SERIAL.write(byte(0x7F & value));
}


void RealTimeSystem(byte realtimebyte) {
    midi.midiRealTimeHandler(realtimebyte);
}


// utility funtions to help pass MIDI Clock messages between
// MIDI and Sequencer objects
void seqTriggerClock() { seq.clock(); }
void seqTriggerStart() { seq.start(); }
void seqTriggerStop() { seq.stop(); }
void seqTriggerContinue() { seq.continues(); }
void midiSendClock() { midi.sendClock(); }
void midiSendStart() { midi.sendStart(); }
void midiSendStop() { midi.sendStop(); }
void midiSendContinue() { midi.sendContinue(); }


void setupMidiAndSound() {
    usbMIDI.setHandleNoteOff(OnNoteOff);
    usbMIDI.setHandleNoteOn(OnNoteOn);
    usbMIDI.setHandleControlChange(OnControlChange);
    usbMIDI.setHandleRealTimeSystem(RealTimeSystem);

    seq.attachSequencerClockCallbackClock(&midiSendClock);
    seq.attachSequencerClockCallbackStart(&midiSendStart);
    seq.attachSequencerClockCallbackStop(&midiSendStop);
    seq.attachSequencerClockCallbackContinue(&midiSendContinue);

    midi.attachMidiClockCallbackClock(&seqTriggerClock);
    midi.attachMidiClockCallbackStart(&seqTriggerStart);
    midi.attachMidiClockCallbackStop(&seqTriggerStop);
    midi.attachMidiClockCallbackContinue(&seqTriggerContinue);

    analogWriteResolution(12);

    cli();
    isrTimer.begin(synth_isr, 1000000.0 / sample_rate);
    sei();
}

#endif // EMPTY_SEQUENCER_H
