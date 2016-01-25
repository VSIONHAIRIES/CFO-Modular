#pragma once
#include "AudioNode.h"


class DAC : public AudioNode {
public:

	DAC(int samplerate);
	~DAC() {}

	virtual int getSampleRate();

	// virtual void setSampleRate(int samplerate);

protected:

	int _sampleRate;

};
