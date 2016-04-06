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
EnvelopeWithDivision env1;
EnvelopeWithDivision env2;
OscillatorWAVE wave1;
OscillatorWAVE wave2;
Amplifier amp;
Mixer mix;
Sequencer seq;
FilterLP6 fltr;

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

/*
void MIDI::controller(uint8_t channel, uint8_t number, uint8_t value) {

    if(value > 127) value = 127;
    // instrument[number] = value;

    switch(number) {

        case IS_12_BIT:
            if(value) Music.set12bit(true);
            else Music.set12bit(false);
            break;
        case PORTAMENTO:
            Music.setPortamento(portamentoTimeTable[value]);
            break;
        case CUTOFF:
            Music.setCutoff(value * 512);
            break;
        case RESONANCE:
            Music.setResonance(value * 512);
            break;
        case FILTER_TYPE:
            Music.setFilterType(value);
            break;
        case CUTOFF_MOD_AMOUNT:
            Music.setCutoffModAmount((value-64) * 1024);
            break;
        case CUTOFF_SOURCE:
            Music.setCutoffModSource(value);
            break;
        case CUTOFF_SHAPE:
            Music.setCutoffModShape(value);
            break;
        case ZERO_HZ_FM:
            if(value) Music.fmToZeroHertz(true);
            else Music.fmToZeroHertz(false);
            break;
        case FM_OCTAVES:
            Music.setFMoctaves(value+1);
            break;
        case LFO1:
            if(value) {
                Music.setOsc1LFO(true);
                Music.setFrequency1(Music.getNoteFrequency(value)/1024.0);
            } else {
                Music.setOsc1LFO(false);
            }
            break;
        case LFO2:
            if(value) {
                Music.setOsc2LFO(true);
                Music.setFrequency2(Music.getNoteFrequency(value)/1024.0);
            } else {
                Music.setOsc2LFO(false);
            }
            break;
        case LFO3:
            if(value) {
                Music.setOsc3LFO(true);
                Music.setFrequency3(Music.getNoteFrequency(value)/1024.0);
            } else {
                Music.setOsc3LFO(false);
            }
            break;
        case DETUNE1:
            Music.setDetune1(map(value,0,127,-100,100)*0.0005946);
            break;
        case DETUNE2:
            Music.setDetune2(map(value,0,127,-100,100)*0.0005946);
            //Music.setDetune2((value-64.0)*0.0005946);
            //Music.setDetune2(value/5120.0);
            break;
        case DETUNE3:
            Music.setDetune3(map(value,0,127,-100,100)*0.0005946);
            //Music.setDetune3((value-64.0)*0.0005946);
            //Music.setDetune3(value/5120.0);
            break;
        case SEMITONE1:
//          if(15 < value && value < 113) {
//              int8_t val = (((value-16)/2)-24);
//              Music.setSemitone1(val);
//          } else if (value < 16) {
//              Music.setSemitone1(-24);
//          } else {
//              Music.setSemitone1(24);
//          }
            if(40 <= value && value <= 88) {
                Music.setSemitone1(value-64);
            } else if (value < 40) {
                Music.setSemitone1(-24);
            } else {
                Music.setSemitone1(24);
            }
            break;
        case SEMITONE2:
//          if(15 < value && value < 113) {
//              int8_t val = (((value-16)/2)-24);
//              Music.setSemitone2(val);
//          } else if (value < 16) {
//              Music.setSemitone2(-24);
//          } else {
//              Music.setSemitone2(24);
//          }
            if(40 <= value && value <= 88) {
                Music.setSemitone2(value-64);
            } else if (value < 40) {
                Music.setSemitone2(-24);
            } else {
                Music.setSemitone2(24);
            }
            break;
        case SEMITONE3:
//          if(15 < value && value < 113) {
//              int8_t val = (((value-16)/2)-24);
//              Music.setSemitone3(val);
//          } else if (value < 16) {
//              Music.setSemitone3(-24);
//          } else {
//              Music.setSemitone3(24);
//          }
            if(40 <= value && value <= 88) {
                Music.setSemitone3(value-64);
            } else if (value < 40) {
                Music.setSemitone3(-24);
            } else {
                Music.setSemitone3(24);
            }
            break;
        case GAIN1:
            Music.setGain1(value / 127.0);
            break;
        case GAIN2:
            Music.setGain2(value / 127.0);
            break;
        case GAIN3:
            Music.setGain3(value / 127.0);
            break;
        case WAVEFORM1:
            Music.setWaveform1(value);
            break;
        case WAVEFORM2:
            Music.setWaveform2(value);
            break;
        case WAVEFORM3:
            Music.setWaveform3(value);
            break;
        case FM1:
            Music.setFM1(value);
            break;
        case FM2:
            Music.setFM2(value);
            break;
        case FM3:
            Music.setFM3(value);
            break;
//      case FM1_OCTAVES:
//          Music.setFM1octaves(value+1);
//          break;
//      case FM2_OCTAVES:
//          Music.setFM2octaves(value+1);
//          break;
//      case FM3_OCTAVES:
//          Music.setFM3octaves(value+1);
//          break;
        case FM1_SOURCE:
            Music.setFM1Source(value);
            break;
        case FM2_SOURCE:
            Music.setFM2Source(value);
            break;
        case FM3_SOURCE:
            Music.setFM3Source(value);
            break;
        case FM1_SHAPE:
            Music.setFM1Shape(value);
            break;
        case FM2_SHAPE:
            Music.setFM2Shape(value);
            break;
        case FM3_SHAPE:
            Music.setFM3Shape(value);
            break;
        case ENV1_ENABLE:
            if(value) Music.enableEnvelope1();
            else Music.disableEnvelope1();
            break;
        case ENV1_ATTACK:
            Music.setEnv1Attack(value);
            break;
        case ENV1_DECAY:
            Music.setEnv1Decay(value);
            break;
        case ENV1_SUSTAIN:
            Music.setEnv1Sustain(value);
            break;
        case ENV1_RELEASE:
            Music.setEnv1Release(value);
            break;
        case ENV2_ENABLE:
            if(value) Music.enableEnvelope2();
            else Music.disableEnvelope2();
            break;
        case ENV2_ATTACK:
            Music.setEnv2Attack(value);
            break;
        case ENV2_DECAY:
            Music.setEnv2Decay(value);
            break;
        case ENV2_SUSTAIN:
            Music.setEnv2Sustain(value);
            break;
        case ENV2_RELEASE:
            Music.setEnv2Release(value);
            break;
        case PRESET_SAVE:
            Music.savePreset(value);
            break;
        case PRESET_RECALL:
            Music.getPreset(value);
            Music.sendInstrument();
            break;
        case SEQ_INTERNAL_CLOCK:
//            Sequencer.setInternalClock(value);
            break;
        case SEQ_CLOCK_IN:
//            setMidiClockIn(value);
            break;
        case SEQ_CLOCK_THRU:
//            setMidiClockThru(value);
            break;
        case SEQ_CLOCK_OUT:
//            setMidiClockOut(value);
            break;
        case SEQ_BPM:
//            Sequencer.setbpm(60 + value);
            break;
        case SEQ_SEQUENCE:
//            Sequencer.setSelectedSequence(value);
            break;
        default:
            break;
    }
}
*/

#endif // SIMPLE_SEQUENCER_H
