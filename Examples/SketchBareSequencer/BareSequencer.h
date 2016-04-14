#ifndef BARE_SEQUENCER_H
#define BARE_SEQUENCER_H

#include <Arduino.h>
#include <EEPROM.h>
#include "MIDI.h"
#include "Sequencer.h"
#include "OscillatorWAVE.h"
#include "EnvelopeWithDivision.h"
#include "Amplifier.h"
#include "Mixer.h"
#include "FilterLP6.h"
#include "Teensy3DAC.h"

#define MIDI_CHANNEL 1
#define NUM_TRACKS 8
#define NUM_STEPS 8
#define OCT_EEPROM_OFFSET (NUM_TRACKS * NUM_STEPS)

const int sample_rate = 48000;
const int scale[] = {0, 2, 3, 5, 7, 8, 10, 12};
const int rootNote = 36;

IntervalTimer isrTimer;
MIDI midi;
Sequencer seq;
EnvelopeWithDivision env1;
EnvelopeWithDivision env2;
OscillatorWAVE wave1;
OscillatorWAVE wave2;
Mixer mix;
FilterLP6 fltr;
Amplifier amp;
Teensy3DAC t3dac(sample_rate);

int notes[64];
int octave[64];
int track[8];
int noteValues[8];

void setupMidiAndSound();
void readSequencesFromEEPROM();

// Some helper variables for module inputs
int noSignal = 0;
int halfSignal = SIGNED_BIT_32_HIGH >> 1;
int quarterSignal = SIGNED_BIT_32_HIGH >> 2;
int fullSignal = SIGNED_BIT_32_HIGH;
int lowSignal = SIGNED_BIT_32_LOW;

class BareSequencer {

  public:
    BareSequencer() {}
    ~BareSequencer() {}
    void patch_modules();
    void setup();
    void update(int);
};

void BareSequencer::patch_modules() {

  // Below hook up the various modules to each other
  // The order doesn't matter
  env1.gateIn_ptr = &seq.gateOut;
  env2.gateIn_ptr = &seq.gateOut;

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
  amp.ampModAmountIn_ptr = &fullSignal;

  t3dac.audioIn_ptr = &amp.audioOut;

  // This function sets up MIDI and Sequencer objects,
  // prepares the DAC and initiate the synth ISR
  setupMidiAndSound();
}

void BareSequencer::setup() {
  patch_modules();

  wave1.setWaveform(SAW);
  seq.init(120);
  seq.setInternalClock(true);

  readSequencesFromEEPROM();
}

void BareSequencer::update(int pTrack) {
  seq.update();
  seq.setInternal(track[pTrack], true);
  usbMIDI.read();
  midi.checkSerialMidi();
}

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


void OnNoteOn(byte channel, byte note, byte velocity) {
  channel = channel - 1;
  Serial.print("NOTEON on channel ");
  Serial.println(channel);
  if (channel == midi.getChannel()) {
    midi.noteOn(channel, note, velocity);
  }
  MIDI_SERIAL.write(byte(0x90 | (channel & 0x0F)));
  MIDI_SERIAL.write(byte(0x7F & note));
  MIDI_SERIAL.write(byte(0x7F & velocity));
}


void OnNoteOff(byte channel, byte note, byte velocity) {
  channel = channel - 1;
  Serial.print("NOTEOFF on channel ");
  if (channel == midi.getChannel()) {
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
  if (channel == midi.getChannel()) {
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
void seqTriggerClock() {
  seq.clock();
}
void seqTriggerStart() {
  seq.start();
}
void seqTriggerStop() {
  seq.stop();
}
void seqTriggerContinue() {
  seq.continues();
}
void midiSendClock() {
  midi.sendClock();
}
void midiSendStart() {
  midi.sendStart();
}
void midiSendStop() {
  midi.sendStop();
}
void midiSendContinue() {
  midi.sendContinue();
}

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

void readSequencesFromEEPROM() {
  for (int i = 0; i < NUM_TRACKS; i++) {
    for (int j = 0; j < NUM_STEPS; j++) {
      notes[NUM_TRACKS * i + j] = EEPROM.read(i * NUM_TRACKS + j);
      octave[NUM_TRACKS * i + j] = EEPROM.read(i * NUM_TRACKS + j + OCT_EEPROM_OFFSET);
      if (octave[NUM_TRACKS * i + j] > 1) {
        octave[NUM_TRACKS * i + j] = 1;
      }
    }
    track[i] = seq.newSequence(NOTE_16, 8, LOOP);

    for (int j = 0; j < NUM_STEPS; j++) {
      int note = notes[8 * i + j];
      if (note == 255) {
        noteValues[j] = 0;
      }
      else {
        noteValues[j] = rootNote + scale[notes[8 * i + j]] + octave[8 * i + j] * 12;
      }
    }
    seq.insertNotes(track[i], noteValues, 8, 0);
    seq.setInternal(track[i], false);
    seq.setExternal(track[i], false);
  }
}

#endif // BARE_SEQUENCER_H
