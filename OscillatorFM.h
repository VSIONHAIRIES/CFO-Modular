#pragma once

#include <Oscillator.h>

class OscillatorFM : public Oscillator {
public:

	OscillatorFM();
	~OscillatorFM(){}

	// int process() override;
	void process() override;

};