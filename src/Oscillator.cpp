#include <Arduino.h>
#include "Oscillator.h"

const float semiToneTable[] = {0.25,0.2648658,0.2806155,0.29730177,0.31498027,0.33370996,0.35355338,0.37457678,0.39685026,0.4204482,0.44544938,0.47193715,0.5,0.5297315,0.561231,0.59460354,0.62996054,0.6674199,0.70710677,0.74915355,0.7937005,0.8408964,0.8908987,0.9438743,1.0,1.0594631,1.122462,1.1892071,1.2599211,1.3348398,1.4142135,1.4983071,1.587401,1.6817929,1.7817974,1.8877486,2.0,2.1189263,2.244924,2.3784142,2.5198421,2.6696796,2.828427,2.9966142,3.174802,3.3635857,3.563595,3.7754972,4.0};

Oscillator::Oscillator() : AudioNode() {

	_period = 0;
	_phase = 0;
	_osc = 0;
	_accumulator = 0;

	_isPlaying = 0;

	setFrequency(110.0f);
	setDetune(0);
	setBend(0);
	setPortamento(1);
	setSemitone(0);
	setGain(1.0f);

	_frequencyIn = 0;
	_fmSourceIn = 0;
	_fmAmountIn = 0;
	_portamento = 0;

	_nosignal = 0;
	frequencyIn_ptr = &_nosignal;
	fmSourceIn_ptr = &_nosignal;
	fmAmountIn_ptr = &_nosignal;
	portamentoIn_ptr = &_nosignal;

	setFrequencyIn(57); // midinote for 220Hz

	// Low Pass filter table for the portamento function	for (int i=0; i<128; i++) {
	for (int i=0; i<128; i++) {
		double fc = ( ( pow ( 2, ( ( double(i) - 140.0 - 69.0) / 24.0 ) ) ) * 440.0 ) / SAMPLE_RATE;
		_portamentoLPCoefficient[127 - i] = int64_t ( BIT_32_FLOAT * ( exp ( M_PI * -2.0 * fc ) ) );
	}


	// this extends the standard midi frequencies from 128 to 256 discrete values,
	// extending the frequency range with -117 positions below to 0.00949 Hz
	// and +11 above to 23679 Hz.
	for (int i=0; i<=128; i++) {
		_expFrequency[i] = (int64_t(pow(2,((float(i) - 69.0) / 12.0)) * 440.0) << 32) / SAMPLE_RATE;  // dividing by sample rate to avoid doing it in interrupt
		// Serial.print("_expFrequency[");
		// Serial.print(i);
		// Serial.print("] = ");
		// Serial.println(int(_expFrequency[i] >> 32));
	}

	for (int i=0; i<=128; i++) {
		_lfoFrequency[i] = (int64_t(pow(2,((double(i) - 117.0 - 69.0) / 12.0)) * 440.0) << 32) / SAMPLE_RATE;  // dividing by sample rate to avoid doing it in interrupt
	}

	_dir = 1;
	_indx = 0;
	_ifrac = 0;
	_freq0 = 0;
	_freq1 = 0;
	_dfreq = 0;
	_ffrac = 0;
	_dPhase = 0;
	_lfo = false;
}


void Oscillator::accumulator() {
	_accumulator = _accumulator + _phase;

}


// FREQUENCY AND DETUNE FUNCTIONS
void Oscillator::setFrequency(float freq) {
	_freq = freq;
	setFrequency();
}


void inline Oscillator::setFrequency() {
	// _period = int(((_freq * _semi * (1 + _detune + _bend)) * PERIOD_MAX) / SAMPLE_RATE);
	// _dPhase = _period;
}


void Oscillator::setFrequencyIn(int midinote) {
	if(midinote < 0) midinote = 0;
	else if(midinote > 127) midinote = 127;
	_frequencyIn = midinote << 24;
}


int Oscillator::getExpFrequencyFromMidinote(int indx) {
	_indx = indx >> 24;
	return _expFrequency[_indx];
}


int Oscillator::getExpFrequency(int indx) {

	_dir = 1;
	if(indx < 0) {
		indx = -indx;
		_dir = -1;
	}
	if(indx > SIGNED_BIT_32_HIGH) indx = SIGNED_BIT_32_HIGH;

	_indx = indx >> 24;  // take it down to the range 0-127
	_ifrac = indx - (_indx << 24);	// find the remainder in the indx after substracting the _indx for the array.

	if(_lfo) {
		_freq0 = _lfoFrequency[_indx];
		_freq1 = _lfoFrequency[_indx + 1];
	} else {
		_freq0 = _expFrequency[_indx];
		_freq1 = _expFrequency[_indx + 1];
	}

	_dfreq = _freq1 - _freq0;
	_ffrac = (_ifrac * _dfreq) >> 24;
	_freq0 += _ffrac;

	// _dPhase = _freq0 * _dir;
	return _freq0 * _dir;
	// _accumulator += _dPhase;
}


void Oscillator::setSemitone(int semi) {
	if(-24 <= semi && semi <= 24){
		_semi = semi;
	} else if (semi < -24) {
		_semi = -24 << 24;
	} else {
		_semi = 24 << 24;
	}
	// setFrequency();

}


void Oscillator::setDetune(int detune) {
	_detune = detune;
}


void Oscillator::setBend(int bend) {
	_bend = 0;
	// setFrequency();
}


void Oscillator::setPortamento(int port) {
	if(port == 0) port = 1; // should this check for negative values?
	_portamento = port;
}


// GAIN FUNCTIONS
void Oscillator::setGain(float gain){
	_gain = convertFloatInput(gain);
	// if(gain < -1.0f) _gain = SIGNED_BIT_32_LOW;
	// else if(gain > 1.0f) _gain = SIGNED_BIT_32_HIGH;
	// else _gain = int(int64_t(gain * SIGNED_BIT_32_HIGH));
}


float Oscillator::getGain() {
	return float(_gain)/SIGNED_BIT_32_HIGH;
}
