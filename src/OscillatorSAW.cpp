#include <Arduino.h>
#include "OscillatorSAW.h"


OscillatorSAW::OscillatorSAW(): Oscillator() {
}


void OscillatorSAW::process() {

		_frequencyIn = *frequencyIn_ptr;
		_frequencyIn += (_detune << 14);
		_frequencyIn += _semi << 24;

		_portamentoIn = *portamentoIn_ptr;
		if(_portamentoIn < 0) _portamentoIn = 0;
		_portamentoIn >>= 24;
		_b1 = _portamentoLPCoefficient[_portamentoIn];

		_x0 = _frequencyIn;
		_a0 = BIT_32 - _b1;
		_y0 = ((_a0 * _x0) >> 32) + ((_b1 * _y0) >> 32);
		_frequencyIn = _y0;


		// _accumulator += getExpFrequency(indx);
		// _accumulator += getExpFrequencyFromMidinote(_frequencyIn);
		_accumulator += getExpFrequency(_frequencyIn);
		audioOut = SIGNED_BIT_32_HIGH - _accumulator;
}
