#include <Arduino.h>
#include "OscillatorFM.h"


OscillatorFM::OscillatorFM(): Oscillator() {
}


void OscillatorFM::process() {

		_frequencyIn = *frequencyIn_ptr;
		_fmSourceIn = *fmSourceIn_ptr;
		_fmAmountIn = *fmAmountIn_ptr;

		int64_t mod = (int64_t(_fmSourceIn) * int64_t(_fmAmountIn)) >> 32;
		int indx = int64_t(_frequencyIn) + mod;

		_accumulator += getExpFrequency(indx);

		_audioOut = SIGNED_BIT_32_HIGH - _accumulator;
		audioOut = _audioOut;
}

