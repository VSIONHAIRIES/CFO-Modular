#include <Arduino.h>
#include "OscillatorWAVE.h"

const uint8_t waveTable[] = {
	#include "data/waveTable8bit.inc"
};


OscillatorWAVE::OscillatorWAVE(): Oscillator() {
	_waveform = 0;
	indx = 0;
	lowestIndx = 0;
	highestIndx = 0;
}

void OscillatorWAVE::process() {

	_frequencyIn = *frequencyIn_ptr;
	_frequencyIn += (_detune << 14);
	_frequencyIn += _semi << 24;
	_accumulator += getExpFrequency(_frequencyIn);

	indx = _accumulator + uint64_t(SIGNED_BIT_32_HIGH);
	indx = indx >> 24;

	// if(indx < lowestIndx) lowestIndx = indx;
	// if(indx > highestIndx) highestIndx = indx;
	// _indx += 128; // make wavetables signed
	indx += _waveform;
	indx = waveTable[indx];
	// _osc -= 128; // make wavetables signed
	indx = indx << 23;
	osc = int64_t(indx) - SIGNED_BIT_32_HIGH;
	audioOut = osc;
	// _osc = int((int64_t(_osc) * int64_t(_gain)) >> 31);

}

void OscillatorWAVE::setWaveform(int waveform) {
	if(waveform < 0) return;
	if(waveform < 16) _waveform = waveform << 8;
	// Serial.println(_waveform);
}
