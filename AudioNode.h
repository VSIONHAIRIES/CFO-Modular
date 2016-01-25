#pragma once
#include <stdint.h>

#define BIT_32 4294967296
#define BIT_32_FLOAT 4294967296.0f
#define SIGNED_BIT_32_HIGH 2147483647
#define SIGNED_BIT_32_LOW -2147483647
#define UNSIGNED_BIT_32_HIGH 4294967295
#define UNSIGNED_BIT_32_LOW 0 

class AudioNode {
public:

	AudioNode();
	~AudioNode() {}

	// virtual int process(int audio) { return 0; }
	// virtual int process() { return 0; }
	virtual void process() {}
	virtual int convertFloatInput(float value);

	int* audioIn_ptr;
	int audioOut;

	static uint8_t midiNotePlaying;

protected:
	int _audioIn;
	int _audioOut; 
	int _noSignal; // a variable that all child classes can point to as default value.
};