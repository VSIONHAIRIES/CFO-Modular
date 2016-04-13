#pragma once

#include <Oscillator.h>

class OscillatorSAW : public Oscillator {
public:

	OscillatorSAW();
	~OscillatorSAW(){}

	// int process() override;
	void process() override;

};