#pragma once

#include "DAC.h"
#include <stdint.h>

class Teensy3DAC : public DAC {
public:

	Teensy3DAC(int samplerate);
	~Teensy3DAC() {}

	void process() override;


};
