#ifndef MIDI_H
#define MIDI_H

#include "AudioNode.h"

// MIDI specific constants
#define MIDI_SERIAL Serial1
#define MIDI_THROUGH true

#ifndef MIDI_CHANNEL
	#define MIDI_CHANNEL 1
#elif (MIDI_CHANNEL > 0) && (MIDI_CHANNEL < 17)
#else
	#error MIDI_CHANNEL should be between 1 - 16
#endif

// Constants defined for MIDI CLOCK
#define MIDI_CLOCK 0xF8     // 248
#define MIDI_START 0xFA     // 250;
#define MIDI_CONTINUE 0xFB  // 251;
#define MIDI_STOP 0xFC      // 252;



class MIDI : public AudioNode {
public:

	MIDI();
	~MIDI() {}

	void process() override;

	// void init();
	void checkSerialMidi();

    void setChannel(uint8_t channel);
    uint8_t getChannel();
    uint8_t midiChannel;

	void midiHandler();
    void midiRealTimeHandler(uint8_t data);

	void noteOff(uint8_t channel, uint8_t note, uint8_t vel);
	void noteOn(uint8_t channel, uint8_t note, uint8_t vel);
	void aftertouch(uint8_t channel, uint8_t note, uint8_t pressure);
	void controller(uint8_t channel, uint8_t number, uint8_t value);
	void programChange(uint8_t channel, uint8_t number);
	void channelPressure(uint8_t channel, uint8_t pressure);
	void pitchWheel(uint8_t channel, uint8_t highBits, uint8_t lowBits);
	void pitchChange(uint8_t channel, int pitch); // extra pitchWheel function for USB MIDI interfacing
    void clock();
    void stop();
    void start();
    void continues();

    void sendNoteOff(uint8_t channel, uint8_t note);
    void sendNoteOff(uint8_t channel, uint8_t note, uint8_t vel);
    void sendNoteOn(uint8_t channel, uint8_t note, uint8_t vel);
    void sendController(uint8_t channel, uint8_t number, uint8_t value);

    void sendClock();
    void sendStart();
    void sendContinue();
    void sendStop();

    void setMidiIn(bool i);
    bool getMidiIn();

    void setMidiOut(bool o);
    bool getMidiOut();

    void setMidiThru(bool t);
    bool getMidiThru();

    void setMidiClockIn(bool i);
    bool getMidiClockIn();

    void setMidiClockOut(bool o);
    bool getMidiClockOut();

    void setMidiClockThru(bool t);
    bool getMidiClockThru();

    int noteOut;
    int gateOut;

		int* noteOffIn_ptr;
		int* noteOnIn_ptr;
		int* controllerIn_ptr;
		bool* clockIn_ptr;
	  bool* startIn_ptr;
	  bool* continueIn_ptr;
	  bool* stopIn_ptr;
	  bool clockOut;
	  bool startOut;
	  bool continueOut;
	  bool stopOut;


protected:

	// MIDI
	uint8_t data;
	uint8_t midiBuffer[3];

    bool midiIn;
    bool midiOut;
    bool midiThru;
    bool midiClockIn;
    bool midiClockOut;
    bool midiClockThru;

	int midiBufferIndex;
	uint16_t frequency;
	uint8_t notePlayed;
    bool midiRead;
//    int notesPlayed[16];
//    int noteIndex;

    // int midiNoteToSignal[128];

		int _noteOffIn[4];
		int _noteOnIn[4];
		int _controllerIn[4];
		bool _clockIn;
		bool _startIn;
		bool _continueIn;
		bool _stopIn;


};

#endif // MIDI_H
