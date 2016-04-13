#ifndef SIMPLE_SEQUENCER_H
#define SIMPLE_SEQUENCER_H

#include <Arduino.h>
// #include "synthesizer.h"
#include "Teensy3DAC.h"
#include "Amplifier.h"
// #include "OscillatorSAW.h"
#include "EnvelopeWithDivision.h"
#include "MIDI.h"
#include "Sequencer.h"
#include "FilterLP6.h"
#include "Mixer.h"
#include "OscillatorWAVE.h"

IntervalTimer isrTimer;
int sample_rate = 48000;

// Declare the synth modules you want to use
Teensy3DAC t3dac(sample_rate);
MIDI midi;
Sequencer seq;
EnvelopeWithDivision env1;
EnvelopeWithDivision env2;
OscillatorWAVE wave1;
OscillatorWAVE wave2;
Mixer mix;
FilterLP6 fltr;
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
    wave1.process();
    wave2.process();
    mix.process();
    fltr.process();
    amp.process();
    t3dac.process();
}

// Creating the class for this type of synth
class SimpleSequencer {
public:
    SimpleSequencer() {}
    ~SimpleSequencer() {}
    void start();
};

void SimpleSequencer::start() {

    // Below hook up the various modules to each other
    // The order doesn't matter

    // env1.gateIn_ptr = &midi.gateOut;
    env1.gateIn_ptr = &seq.gateOut;
    env2.gateIn_ptr = &seq.gateOut;

    // saw1.frequencyIn_ptr = &midi.noteOut;
    wave1.frequencyIn_ptr = &seq.noteOut;
    wave2.frequencyIn_ptr = &seq.noteOut;

    mix.ch1audioIn_ptr = &wave1.audioOut;
    mix.ch2audioIn_ptr = &wave2.audioOut;
    mix.ch1gainIn_ptr = &halfSignal;
    mix.ch2gainIn_ptr = &halfSignal;

    fltr.audioIn_ptr = &mix.audioOut;
    fltr.cutoffIn_ptr = &fullSignal;
    fltr.cutoffModSourceIn_ptr = &env2.envelopeOut;
    fltr.cutoffModAmountIn_ptr = &fullSignal;

    amp.audioIn_ptr = &fltr.audioOut;
    amp.ampModSourceIn_ptr = &env1.envelopeOut;
    // amp.ampModSourceIn_ptr = &fullSignal;
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
    // Serial.write("sent MIDI noteOn on MIDI OUT????");
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

#endif // SIMPLE_SEQUENCER_H
