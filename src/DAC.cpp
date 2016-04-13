#include <Arduino.h>
#include "DAC.h"


// static void dac_isr() {}


DAC::DAC(int samplerate) : AudioNode() {
	_sampleRate = samplerate;
}


int DAC::getSampleRate() {
	return _sampleRate;
}
