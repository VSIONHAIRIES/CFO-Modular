#include <Arduino.h>
#include "Teensy3DAC.h"


Teensy3DAC::Teensy3DAC(int samplerate) : DAC(samplerate) {

}

void Teensy3DAC::process() {	

	_audioIn = *audioIn_ptr;
	_audioIn += SIGNED_BIT_32_HIGH; // upper limit of signed 32bit integer to bring it into unsigned int space.
	analogWrite(A14, _audioIn>>20);

}
