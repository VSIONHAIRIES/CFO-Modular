// #include <math.h>
// #include <EEPROM.h>
// #include "NewExample.h"
#include <Arduino.h>
#include "Bodyseq.h"
#include "MIDI.h"
#include "Sequencer.h"
#include "EnvelopeLinear.h"
#include "OscillatorSAW.h"
#include "FilterLP24.h"
#include "Amplifier.h"
#include "Teensy3DAC.h"

#define MIDI_CHANNEL 1

// Some helper variables for module inputs
int noSignal = 0;
int fullSignal = SIGNED_BIT_32_HIGH;    // The highest signed 32bit number = 2147483647
int lowSignal = SIGNED_BIT_32_LOW;      // The lowest signed 32bit number = -2147483648
int halfSignal = SIGNED_BIT_32_HIGH >> 1;
int quarterSignal = SIGNED_BIT_32_HIGH >> 2;

// Forward declaration of setupMidiAndSound function used at the bottom
void setupMidiAndSound();


//////////////////////////////////////////////////////////
// THIS IS WHERE YOU START DEFINING YOUR SYNTH TOPOLOGY //
//////////////////////////////////////////////////////////

// When using the Bodyseq interface you ned the 6 lines below.
// No need to touch them, except maybe sample_rate if you want to experiment
Bodyseq interface; // Declare the interface module we want to use
IntervalTimer isrTimer; // The interval timer that is running our synth engine
int sample_rate = 48000; // This sample rate gives us exactly 2000 cycles between each call to the synth_isr
Teensy3DAC t3dac(sample_rate); // The Digital-to-Analog-Converter (DAC) on the Teensy3.2 board
MIDI midi; // The MIDI module that we need to talk to other machines
Sequencer seq; // The sequencer is needed for the Bodyseq interface, but also trigger the sounds

// Declare the synth modules you want to use
EnvelopeLinear env1;
EnvelopeLinear env2;
OscillatorSAW saw;
FilterLP24 fltr;
Amplifier amp;

// Declare any additional variables you want to use in this sketch
int cutoffModAmount = 0;
int cutoff = 0;
int _bpm;
int portamento = 0;

// The Interrupt Service Routine (ISR) for the synthesizer
void synth_isr() {
    env1.process();
    env2.process();
    saw.process();
    fltr.process();
    amp.process();
    t3dac.process();
}


void setup() {

    // wipe_eeprom(); // DO NOT TURN THIS ON UNLESS YOU WANT TO WIPE ALL MEMORY
    interface.attachSequencer(&seq);
    seq.init(120);
    seq.setInternalClock(true);
    interface.init();

    // Below hook up the various modules to each other
    // The order doesn't matter
    env1.gateIn_ptr = &seq.gateOut;
    env2.gateIn_ptr = &seq.gateOut;
    saw.portamentoIn_ptr = &portamento;
    saw.frequencyIn_ptr = &seq.noteOut;
    fltr.audioIn_ptr = &saw.audioOut;
    fltr.cutoffIn_ptr = &cutoff;
    fltr.cutoffModSourceIn_ptr = &env2.envelopeOut;
    fltr.cutoffModAmountIn_ptr = &cutoffModAmount;
    amp.audioIn_ptr = &fltr.audioOut;
    amp.ampModSourceIn_ptr = &env1.envelopeOut;
    amp.ampModAmountIn_ptr = &fullSignal;
    t3dac.audioIn_ptr = &amp.audioOut;

    // Attach the functions to the Pots in the interface. Each Pot can have
    // a separate function for each mode.

    interface.pots.attachCallback(NO_BUTTONS, 0, &setCutoff);
    // interface.pots.attachCallback(NO_BUTTONS, 1, &setCutoffModAmount);
    interface.pots.attachCallback(NO_BUTTONS, 1, &setPortamento);
    interface.pots.attachCallback(BUTTON_1, 0, &setEnv2Attack);
    interface.pots.attachCallback(BUTTON_1, 1, &setEnv2DecayRelease);
    interface.pots.attachCallback(BUTTON_1_3, 0, &setBPM);

    // This function sets up MIDI and Sequencer objects,
    // prepares the DAC and initiate the synth ISR
    setupMidiAndSound();
}


void loop() {
    seq.update();
    usbMIDI.read();
    midi.checkSerialMidi();
    interface.update();
}

// Here you define the functions you want to trigger with the pots
void setCutoff(int value) {
    cutoff = value << 21;
}

void setCutoffModAmount(int value) {
    cutoffModAmount = value << 21;
}

void setPortamento(int value) {
    portamento = value << 21;
}

void setEnv2Attack(int value) {
    env2.setAttack(value >> 3);
}

void setEnv2DecayRelease(int value) {
    env2.setDecay(value >> 3);
    env2.setRelease(value >> 3);
}

void setBPM(int value) {
    int bpm = value >> 2;
    if(bpm == 0) {
        midi.setMidiIn(true);
        midi.setMidiThru(true);
        midi.setMidiOut(false);
        midi.setMidiClockIn(true);
        midi.setMidiClockThru(true);
        midi.setMidiClockOut(false);
        seq.setInternalClock(false);
    } else {
        midi.setMidiIn(false);
        midi.setMidiThru(true);
        midi.setMidiOut(false);
        midi.setMidiClockIn(false);
        midi.setMidiClockThru(false);
        midi.setMidiClockOut(true);
        seq.setInternalClock(true);
        //      Sequencer.sequencerContinue();
        if(seq.getbpm() == 0) seq.start();
    }
    seq.setbpm(bpm);
}


///////////////////////////////////////////
// THE BELOW FUNCTIONS ARE NEEDED FOR    //
// THE SYSTEM TO PLAY SOUND AND RESPOND  //
// TO MIDI. NO NEED TO TOUCH THEM UNLESS //
// YOU KNOW WHAT YOU ARE DOING.          //
///////////////////////////////////////////

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
