#ifndef EXAMPLE_SEQUENCER_H
#define EXAMPLE_SEQUENCER_H

#include <Arduino.h>
#include "MIDI.h"
#include "Sequencer.h"
#include "EnvelopeLinear.h"
#include "OscillatorSAW.h"
#include "FilterLP24.h"
#include "Amplifier.h"
#include "Teensy3DAC.h"

IntervalTimer isrTimer;
int sample_rate = 48000;

// Declare the synth modules you want to use
Teensy3DAC t3dac(sample_rate);
MIDI midi;
Sequencer seq;
EnvelopeLinear env1;
EnvelopeLinear env2;
OscillatorSAW saw;
FilterLP24 fltr;
Amplifier amp;

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
    env1.process();
    env2.process();
    saw.process();
    fltr.process();
    amp.process();
    t3dac.process();
}

// Creating the class for this type of synth
class ExampleSequencer {
public:
    ExampleSequencer() {}
    ~ExampleSequencer() {}
    void start();
};

void ExampleSequencer::start() {

    // Below hook up the various modules to each other
    // The order doesn't matter

    env1.gateIn_ptr = &seq.gateOut;
    env2.gateIn_ptr = &seq.gateOut;

    saw.frequencyIn_ptr = &seq.noteOut;

    fltr.audioIn_ptr = &saw.audioOut;
    fltr.cutoffIn_ptr = &fullSignal;
    fltr.cutoffModSourceIn_ptr = &env2.envelopeOut;
    fltr.cutoffModAmountIn_ptr = &fullSignal;

    amp.audioIn_ptr = &fltr.audioOut;
    amp.ampModSourceIn_ptr = &env1.envelopeOut;
    amp.ampModAmountIn_ptr = &fullSignal;

    t3dac.audioIn_ptr = &amp.audioOut;

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

#endif // EXAMPLE_SEQUENCER_H
