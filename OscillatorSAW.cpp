#include <Arduino.h>
#include "OscillatorSAW.h"


OscillatorSAW::OscillatorSAW(): Oscillator() {
}


void OscillatorSAW::process() {

		_frequencyIn = *frequencyIn_ptr;
		_frequencyIn += (_detune << 14);
		_frequencyIn += _semi << 24;
		// _accumulator += getExpFrequency(indx);
		// _accumulator += getExpFrequencyFromMidinote(_frequencyIn);
		_accumulator += getExpFrequency(_frequencyIn);
		audioOut = SIGNED_BIT_32_HIGH - _accumulator;
}

