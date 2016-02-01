#include <EnvelopeWithDivision.h>

// Used in the functions that set the envelope timing
int envTimeTable[] = {1,5,9,14,19,26,34,42,53,65,79,95,113,134,157,182,211,243,278,317,359,405,456,511,570,633,702,776,854,939,1029,1124,1226,1333,1448,1568,1695,1829,1971,2119,2274,2438,2610,2789,2977,3172,3377,3590,3813,4044,4285,4535,4795,5065,5345,5635,5936,6247,6569,6902,7247,7602,7970,8349,8740,9143,9559,9986,10427,10880,11347,11827,12321,12828,13349,13883,14433,14996,15574,16167,16775,17398,18036,18690,19359,20045,20746,21464,22198,22949,23716,24501,25303,26122,26959,27813,28686,29577,30486,31413,32359,33325,34309,35312,36335,37378,38440,39522,40625,41748,42892,44056,45241,46448,47675,48925,50196,51489,52803,54141,55500,56883,58288,59716,61167,62642,64140,65662};


EnvelopeWithDivision::EnvelopeWithDivision() : Envelope() {

	_lastGateIn = 0;

	setStage(0);
	setAttack(12);
	setDecay(64);
	setSustain(127);
	setRelease(48);
	setVelSustain(127);
	setVelPeak(127);

}


void EnvelopeWithDivision::process() {

	_gateIn = *gateIn_ptr;
	// if(_lastGateIn < 0  && _gateIn < 0); // Continous LOW: release (stage 4) or no gain (stage 0)
	if(_lastGateIn < 0  && _gateIn > 0) _stage = 1; // transition from LOW to HIGH: attack (stage 1) starts
	// if(_lastGateIn > 0  && _gateIn > 0); // Continous HIGH: attack (stage 1), decay (stage 2) and sustain (stage 3). Wait for release
	if(_lastGateIn > 0  && _gateIn < 0) _stage = 4; // transition from HIGH to LOW: Release (stage 4) begins
	_lastGateIn = _gateIn;

	// Attack
	if(_stage == 1) {
		_env += 1; // to make sure the envelope increases when (MAX_ENV_GAIN-_env) is smaller than attack1
		_env += (MAX_ENV_GAIN - _env) / _attack;
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


void EnvelopeWithDivision::enableEnvelope()
{
	_envelopeOn = 1;
}


void EnvelopeWithDivision::disableEnvelope()
{
	_envelopeOn = 0;
}


void EnvelopeWithDivision::setStage(uint8_t stage)
{
	_stage = stage;
}


void EnvelopeWithDivision::setAttack(uint8_t att)
{
	if(att>127) att = 127;
	_attack = envTimeTable[att];
}


void EnvelopeWithDivision::setDecay(uint8_t dec)
{
	if(dec>127) dec = 127;
	_decay = envTimeTable[dec];
}


void EnvelopeWithDivision::setSustain(uint8_t sus)
{
	_sustain = (sus * MAX_ENV_GAIN) >> 7;
}


void EnvelopeWithDivision::setRelease(uint8_t rel)
{
	if(rel>127) rel = 127;
	_release = envTimeTable[rel];
}


void EnvelopeWithDivision::setVelSustain(uint8_t vel)
{
	_velSustain = (vel * _sustain) >> 7;
}


void EnvelopeWithDivision::setVelPeak(uint8_t vel)
{
	_velPeak = (vel * MAX_ENV_GAIN) >> 7;
}
