#include <math.h>
#include <EnvelopeLinear.h>

#define SAMPLE_RATE 48000

#define MAX_ENV_GAIN SIGNED_BIT_32_HIGH

EnvelopeLinear::EnvelopeLinear() : Envelope() {

	for(int i=0; i<=256;i++) {
		_attackTable[i] = i << 23; // bring 8bit up to 31 bit
		_decayTable[256-i] = i << 23; // bring 8bit up to 31 bit
	}

	for (int i=0; i<128; i++) {
		_envelopeTimeTable[127 - i] = (int64_t(pow(2,((double(i) - 82.0 - 69.0) / 12.0)) * 440.0) << 32) / SAMPLE_RATE;  // dividing by sample rate to avoid doing it in interrupt
	}

	_attackIndex = 0;
	_decayIndex = 0;
	_releaseIndex = 0;

	_lastGateIn = 0;

	setStage(0);
	setAttack(4);
	setDecay(64);
	setSustain(0);
	setRelease(48);
	setVelSustain(127);
	setVelPeak(127);

}


void EnvelopeLinear::process() {

	_gateIn = *gateIn_ptr;
	// if(_lastGateIn < 0  && _gateIn < 0); // Continous LOW: release (stage 4) or no gain (stage 0)
	if(_lastGateIn < 0  && _gateIn > 0) _stage = 1; // transition from LOW to HIGH: attack (stage 1) starts
	// if(_lastGateIn > 0  && _gateIn > 0); // Continous HIGH: attack (stage 1), decay (stage 2) and sustain (stage 3). Wait for release
	if(_lastGateIn > 0  && _gateIn < 0) _stage = 4; // transition from HIGH to LOW: Release (stage 4) begins
	_lastGateIn = _gateIn;
	
	// Attack
	if(_stage == 1) {
		_attackIndex += _attack;


		_env = getFractionalAttack(_attackIndex);
		// _env = _attackTable[_attackIndex >> 24];
		// _env += (MAX_ENV_GAIN - _env) / _attack;
		if(_velPeak < _env) {
			_env = _velPeak;
			_stage = 2;
		}
	}
	// Decay
	else if(_stage == 2) {
		_env += -1;	// to make sure the envelope decreases when (velSustain1-_env) is smaller than decay1
		_env += (_velSustain - _env) / _decay;
		if(_env < _velSustain || MAX_ENV_GAIN < _env) {
			_env = _velSustain;
			_stage = 3;
		}
	}
	// Sustain
	else if (_stage == 3) {
		_env = _velSustain;
	}

	// Release
	else if (_stage == 4) {
		_env += -1; // to make sure the envelope decreases when (0-_env) is smaller than release1
		_env += (0 - _env) / _release;
		if(_env < 0 || MAX_ENV_GAIN < _env) {
			_env = 0;
			_stage = 0;
		}
	}
			 
	// No gain
	else if (_stage == 0) {
		_env = 0;
	}

	envelopeOut = _env << 15;

}


int EnvelopeLinear::getFractionalAttack(int indx) {

	if(indx > SIGNED_BIT_32_HIGH) indx = SIGNED_BIT_32_HIGH;

	_indx = indx >> 23;  // take it down to the range 0-127
	_ifrac = indx - (_indx << 23);	// find the remainder in the indx after substracting the _indx for the array.

	_phase0 = _attackTable[_indx];
	_phase1 = _attackTable[_indx + 1];

	_dphase = _phase1 - _phase0;
	_pfrac = (_ifrac * _dphase) >> 24;
	_phase0 += _pfrac;

	return _phase0;
}


void EnvelopeLinear::setStage(uint8_t stage)
{
	_stage = stage;
}


void EnvelopeLinear::setAttack(uint8_t att)
{
	if(att>127) att = 127;
	_attack = _envelopeTimeTable[att];
}


void EnvelopeLinear::setDecay(uint8_t dec)
{
	if(dec>127) dec = 127;
	_decay = _envelopeTimeTable[dec];
}


void EnvelopeLinear::setSustain(uint8_t sus)
{
	_sustain = (sus * MAX_ENV_GAIN) >> 7;	
}


void EnvelopeLinear::setRelease(uint8_t rel)
{
	if(rel>127) rel = 127;
	_release = _envelopeTimeTable[rel];
}


void EnvelopeLinear::setVelSustain(uint8_t vel)
{
	_velSustain = vel * (_sustain >> 7);	
}


void EnvelopeLinear::setVelPeak(uint8_t vel)
{
	_velPeak = vel * (MAX_ENV_GAIN >> 7);	
}
