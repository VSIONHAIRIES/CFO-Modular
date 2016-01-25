#include <Arduino.h>
#include <stdint.h>
#include "MIDI.h"




MIDI::MIDI() {

	noteOut = 0;

	pinMode(0, INPUT);
    pinMode(1, OUTPUT);
    Serial.begin(115200);
    MIDI_SERIAL.begin(31250);
    
    setMidiIn(true);
    setMidiOut(true);
    setMidiThru(true);
    setMidiClockIn(true);
    setMidiClockThru(true);
    setMidiClockOut(true);
    
	midiBufferIndex = 0;
	midiChannel = 0;
    midiRead = false;
}


void MIDI::setChannel(uint8_t channel)
{
    if(channel < 1 || channel > 16) {
        Serial.println("MIDI channel must be set to a number between 1 and 16");
    }
    else midiChannel = channel - 1;
}

uint8_t MIDI::getChannel() {
    return midiChannel;
}


void MIDI::sendNoteOff(uint8_t channel, uint8_t note) {
    if(getMidiOut()) {
        MIDI_SERIAL.write(0x80 | (channel & 0x0F));
        MIDI_SERIAL.write(byte(note & 0x7F));
        MIDI_SERIAL.write(0x00);
    }
}


void MIDI::sendNoteOff(uint8_t channel, uint8_t note, uint8_t vel) {
    if(getMidiOut()) {
        MIDI_SERIAL.write(0x80 | (channel & 0x0F));
        MIDI_SERIAL.write(byte(note & 0x7F));
        MIDI_SERIAL.write(byte(vel & 0x7F));
    }
}


void MIDI::sendNoteOn(uint8_t channel, uint8_t note, uint8_t vel) {
    if(getMidiOut()) {
        MIDI_SERIAL.write(0x90 | (channel & 0x0F));
        MIDI_SERIAL.write(byte(note & 0x7F));
        MIDI_SERIAL.write(byte(vel & 0x7F));
    }
}


void MIDI::sendController(uint8_t channel, uint8_t number, uint8_t value) {
    if(getMidiOut()) {
        MIDI_SERIAL.write(0xB0 | (channel & 0x0F));
        MIDI_SERIAL.write(byte(number & 0x7F));
        MIDI_SERIAL.write(byte(value & 0x7F));
    }
}


void MIDI::sendClock() {
    if(getMidiClockOut()) {
        MIDI_SERIAL.write(MIDI_CLOCK);
    }
}


void MIDI::sendStart() {
    if(getMidiClockOut()) {
        MIDI_SERIAL.write(MIDI_START);
    }
}


void MIDI::sendContinue() {
    if(getMidiClockOut()) {
        MIDI_SERIAL.write(MIDI_CONTINUE);
    }
}


void MIDI::sendStop() {
    if(getMidiClockOut()) {
        MIDI_SERIAL.write(MIDI_STOP);
    }
}


void MIDI::setMidiIn(bool i)
{
    midiIn = i;
}


bool MIDI::getMidiIn()
{
    return midiIn;
}


void MIDI::setMidiOut(bool o)
{
    midiOut = o;
}


bool MIDI::getMidiOut()
{
    return midiOut;
}


void MIDI::setMidiThru(bool t)
{
    midiThru = t;
}


bool MIDI::getMidiThru()
{
    return midiThru;
}


void MIDI::setMidiClockIn(bool i)
{
    midiClockIn = i;
}


bool MIDI::getMidiClockIn()
{
    return midiClockIn;
}


void MIDI::setMidiClockOut(bool o)
{
    midiClockOut = o;
}


bool MIDI::getMidiClockOut()
{
    return midiClockOut;
}


void MIDI::setMidiClockThru(bool t)
{
    midiClockThru = t;
}


bool MIDI::getMidiClockThru()
{
    return midiClockThru;
}


void MIDI::checkSerialMidi()
{
    while(MIDI_SERIAL.available()) {
        
		data = MIDI_SERIAL.read();
		
        if(data >= 0xF8) {
            midiRealTimeHandler(data);
//            RealTimeSystem(byte(data));
            continue;
        }
        
//		if(data & 0x80 && (data & 0x0F) == midiChannel) {	// bitmask with 10000000 to see if byte is over 127 (data&0x80)
//			midiBufferIndex = 0;							// and check if the midi channel corresponds to the midiChannel
//			midiRead = true;								// the device is set to listen to.
//		} else if(data & 0x80) {							// Else if the byte is over 127 (but not on the device's
//			midiRead = false;								// midiChannel, don't read this or any following bytes.
//		}
		
        if(data >= 0x80 && data < 0xF0) {       // check if incoming byte is a status byte (above 127)but less than sysEx (0xF0)
            if((data & 0x0F) == midiChannel) {  // if it is, check if it is the right MIDI channel
                midiBufferIndex = 0;
                midiRead = true;
            } else if(data >= 0x80) {           // if above check fails, check if it is still a status byte
                midiRead = false;
            } else {}                           // if it is below 128 just continue
        }
        
        if(midiRead) {
			midiBuffer[midiBufferIndex] = data;
			midiBufferIndex++;
			if (midiBufferIndex > 2) {
				midiRead = false;
				midiHandler();
//                Serial.println("MIDI RECEIVED");
//                Serial.println(midiBuffer[0], HEX);
//                Serial.println(midiBuffer[1], HEX);
//                Serial.println(midiBuffer[2], HEX);
			}
		}
	}	
}


void MIDI::midiRealTimeHandler(uint8_t data) {
    
    if(getMidiClockThru()) {
        MIDI_SERIAL.write(data);
    }
    if(getMidiClockIn()) {
        switch(data) {
            case 0xF8:
                clock();
                break;
                
            case 0xFA:
                start();
                break;
                
            case 0xFB:
                continues();
                break;
                
            case 0xFC:
                stop();
                break;
                
            default:
                break;
        }

    }
}


void MIDI::clock()
{
        // Sequencer.clock();
}


void MIDI::start()
{
        // Sequencer.start();
}


void MIDI::continues()
{
        // Sequencer.continues();
}


void MIDI::stop()
{
        // Sequencer.stop();
}


void MIDI::midiHandler() {

    if(getMidiThru()) {
        MIDI_SERIAL.write(midiBuffer[0]);
        MIDI_SERIAL.write(midiBuffer[1]);
        MIDI_SERIAL.write(midiBuffer[2]);
    }
    if(getMidiIn()) {
        if((midiBuffer[0] & 0x0F) == midiChannel) {
            switch(midiBuffer[0] & 0xF0) { // bit mask with &0xF0
                case 0x80:
                    noteOff			(midiBuffer[0] & 0x0F,     // midi channel 0-15
                                     midiBuffer[1] & 0x7F,   // note value 0-127
                                     midiBuffer[2] & 0x7F);  // note velocity 0-127
                    break;
                    
                case 0x90:
                    noteOn			(midiBuffer[0] & 0x0F,     // midi channel 0-15
                                     midiBuffer[1] & 0x7F,   // note value 0-127
                                     midiBuffer[2] & 0x7F);  // note velocity 0-127
                    break;
                    
                case 0xA0:
                    aftertouch		(midiBuffer[0] & 0x0F,   // midi channel 0-15
                                     midiBuffer[1] & 0x7F, // note value 0-127
                                     midiBuffer[2] & 0x7F);// note velocity 0-127
                    break;
                    
                case 0xB0:
                    controller		(midiBuffer[0] & 0x0F,   // midi channel 0-15
                                     midiBuffer[1] & 0x7F, // controller number 0-127
                                     midiBuffer[2] & 0x7F);// controller value 0-127
                    break;
                    
                case 0xC0:
                    programChange	(midiBuffer[0]  & 0x0F,    // midi channel 0-15
                                     midiBuffer[1] & 0x7F);  // program number 0-127
                    break;
                    
                case 0xD0:
                    channelPressure	(midiBuffer[0]  & 0x0F,    // midi channel 0-15
                                     midiBuffer[1] & 0x7F);  // pressure amount 0-127
                    break;
                    
                case 0xE0:
                    pitchWheel		(midiBuffer[0] & 0x0F,   // midi channel 0-15
                                     midiBuffer[1] & 0x7F, // higher bits 0-6
                                     midiBuffer[2] & 0x7F);// lower bits 7-13
                    break;
                    
                default:
                    break;
            }
        }
        else Serial.println("Skipped MIDI message on another channel");
    }
}


void MIDI::noteOff(uint8_t channel, uint8_t note, uint8_t vel) {
   Serial.print("NoteOff received on channel: ");
   Serial.println(channel + 1, HEX);

    // Music.noteOff(note);
   gateOut = SIGNED_BIT_32_LOW;
    // isPlaying = 0;
}


void MIDI::noteOn(uint8_t channel, uint8_t note, uint8_t vel) {
	// Serial.print("NoteOn received on channel: ");
	// Serial.println(channel + 1, HEX);
	// Serial.print("Playing note ");
	// Serial.print(note);
	// Serial.print(" with velocity ");
	// Serial.println(vel);
	noteOut = note << 24;
	gateOut = SIGNED_BIT_32_HIGH;
	// isPlaying = 1;
	// if(vel == 0) isPlaying = 0;

}


void MIDI::aftertouch(uint8_t channel, uint8_t note, uint8_t pressure) {
	// Write code here for Aftertouch 
}


void MIDI::controller(uint8_t channel, uint8_t number, uint8_t value) {
	
	if(value > 127) value = 127;
	// instrument[number] = value;
	
	switch(number) {
/*		
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
//			if(15 < value && value < 113) {
//				int8_t val = (((value-16)/2)-24);
//				Music.setSemitone1(val);
//			} else if (value < 16) {
//				Music.setSemitone1(-24);				
//			} else {
//				Music.setSemitone1(24);
//			}
			if(40 <= value && value <= 88) {
				Music.setSemitone1(value-64);
			} else if (value < 40) {
				Music.setSemitone1(-24);				
			} else {
				Music.setSemitone1(24);
			}
			break;
		case SEMITONE2:
//			if(15 < value && value < 113) {
//				int8_t val = (((value-16)/2)-24);
//				Music.setSemitone2(val);
//			} else if (value < 16) {
//				Music.setSemitone2(-24);				
//			} else {
//				Music.setSemitone2(24);
//			}
			if(40 <= value && value <= 88) {
				Music.setSemitone2(value-64);
			} else if (value < 40) {
				Music.setSemitone2(-24);				
			} else {
				Music.setSemitone2(24);
			}
			break;
		case SEMITONE3:
//			if(15 < value && value < 113) {
//				int8_t val = (((value-16)/2)-24);
//				Music.setSemitone3(val);
//			} else if (value < 16) {
//				Music.setSemitone3(-24);				
//			} else {
//				Music.setSemitone3(24);
//			}
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
//		case FM1_OCTAVES:
//			Music.setFM1octaves(value+1);
//			break;
//		case FM2_OCTAVES:
//			Music.setFM2octaves(value+1);
//			break;
//		case FM3_OCTAVES:
//			Music.setFM3octaves(value+1);
//			break;
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
*/            
        default:
			break;
	} 
}


void MIDI::programChange(uint8_t channel, uint8_t number) {
//	Music.getPreset(number);
}


void MIDI::channelPressure(uint8_t channel, uint8_t pressure) {
	// Write code here for Channel Pressure 
}


void MIDI::pitchWheel(uint8_t channel, uint8_t highBits, uint8_t lowBits) {
	// Write code here for Pitch Wheel
}

