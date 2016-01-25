#pragma once

#include <Oscillator.h>

enum WaveformType {
SINE  = 0
, SQUARE = 1
, PULSE = 2
, TRIANGLE = 3
, SAW = 4
, FUZZ = 5
, DIGI1 = 6
, DIGI2 = 7
, DIGI3 = 8
, DIGI4 = 9
, NOISE = 10
, DIGI6 = 11
, TAN1 = 12
, TAN2 = 13
, TAN3 = 14
, TAN4 = 15
};

class OscillatorWAVE : public Oscillator {
public:

	OscillatorWAVE();
	~OscillatorWAVE(){}

	// int process() override;
	void process() override;

	void setWaveform(int waveform);

	unsigned int lowestIndx;
	unsigned int highestIndx;

	uint64_t indx;
	int64_t osc;

protected:
	int _waveform;

};