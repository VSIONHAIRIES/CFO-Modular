#include <math.h>
#include <EnvelopeLinear.h>
#include <Arduino.h>

#define SAMPLE_RATE 48000

#define MAX_ENV_GAIN SIGNED_BIT_32_HIGH
#define MAX_ENV_TARGET UNSIGNED_BIT_32_HIGH

EnvelopeLinear::EnvelopeLinear() : Envelope() {

	// default linear implementation of rising and falling curves
	// for(int i=0; i<=256;i++) {
	// 	_attackTable[i] = i << 23; // bring 8bit up to 31 bit
	// 	_decayTable[256-i] = i << 23; // bring 8bit up to 31 bit
	// }

	// for (int i=0; i<128; i++) {
	// 	_envelopeSpeedTable[i] = int64_t(pow(2,((double(i) - 100.0 - 69.0) / 6.0)) * 440.0 * pow(2,31) / SAMPLE_RATE);  // dividing by sample rate to avoid doing it in interrupt
	// }

	// _attackIndex = 0;
	// _decayIndex = 0;
	// _releaseIndex = 0;

	_lastGateIn = 0;

	for (int i=0; i<128; i++) {
		double fc = ( ( pow ( 2, ( ( double(i) - 100.0 - 69.0) / 12.0 ) ) ) * 440.0 ) / SAMPLE_RATE;
		_filterCoefficient[127 - i] = int64_t ( BIT_32_FLOAT * ( exp ( M_PI * -2.0 * fc ) ) );
	}

	_a0 = 0;
	_a1 = 0;
	_b0 = 0;
	_b1 = 0;
	_x0 = 0;
	_x1 = 0;
	_y0 = 0;
	_y1 = 0;

	setStage(0);
	setAttack(4);
	setDecay(64);
	setSustain(0);
	setRelease(64);
	setVelSustain(0);
	setVelPeak(0);

}


void EnvelopeLinear::process() {

	_gateIn = *gateIn_ptr;
	// if(_lastGateIn < 0  && _gateIn < 0); // Continous LOW: release (stage 4) or no gain (stage 0)
	if(_lastGateIn < 0  && _gateIn > 0) {
		_stage = 1; // transition from LOW to HIGH: attack (stage 1) starts
		_env_target = MAX_ENV_TARGET;
		_b1 = _b1_attack;
		// Serial.print("STAGE 1 - ");
		// Serial.println((long)_env);
	}
	// if(_lastGateIn > 0  && _gateIn > 0); // Continous HIGH: attack (stage 1), decay (stage 2) and sustain (stage 3). Wait for release
	if(_lastGateIn > 0  && _gateIn < 0) {
		_stage = 4; // transition from HIGH to LOW: Release (stage 4) begins
		_env_target = 127;
		_b1 = _b1_release;
		// Serial.print("STAGE 4 - ");
		// Serial.println((long)_env);
	}
	_lastGateIn = _gateIn;

	switch (_stage) {
		// Attack
		case 1:
			// _env_target = MAX_ENV_TARGET;
			// _b1 = _b1_attack;
			if(_env > MAX_ENV_GAIN) {
				_stage = 2;
				_env_target = _sustain;
				_b1 = _b1_decay;
				// Serial.print("STAGE 2 - ");
				// Serial.println((long)_env);
			}

		break;

		// Decay
		case 2: {
			// _env_target = _sustain;
			// _b1 = _b1_decay;
			if(_env < _sustain) {

				_stage = 3;
				_env_target = _sustain;
				// Serial.print("STAGE 3 - ");
				// Serial.println((long)_env);
				// Serial.print("_sustain: ");
				// Serial.println((long)_sustain);
			}
		}
		break;

		// Sustain
		case 3: {
			// _env_target = _sustain;
		}
		break;

		// Release
		case 4: {
			// _env_target = 127;
			// _b1 = _b1_release;
			if(_env < 127) {
				_env = 127;
				_stage = 0;
				// Serial.print("STAGE 0 - ");
				// Serial.println((long)_env);
			}
		}
		break;


		case 0: {
		}
		break;

		// No gain
		default: {
			if(_env > 0) _env -= 1;
		}
		break;
	}

	// run the filter that actually moves the envelope. _b1 is set for each
	_x0 = _env_target;
	_a0 = BIT_32 - _b1;
	_y0 = ((_a0 * _x0) >> 32) + ((_b1 * _y0) >> 32);
	// if(_y0 > MAX_ENV_GAIN) _y0 = MAX_ENV_GAIN;
	// _y1 = _y0;
	_env = _y0;
	envelopeOut = _env;

}


// int EnvelopeLinear::interpolateFromTable(int indx, int *table) {
//
// 	if(indx > SIGNED_BIT_32_HIGH) indx = SIGNED_BIT_32_HIGH;
// 	else if (indx < 0) indx = 0;
//
// 	_indx = indx >> 23;  // take it down to the range 0-127
// 	_ifrac = indx - (_indx << 23);	// find the remainder in the indx after substracting the _indx for the array.
//
// 	_phase0 = table[_indx];
// 	_phase1 = table[_indx + 1];
//
// 	_dphase = _phase1 - _phase0;
// 	_pfrac = (_ifrac * _dphase) >> 24;
// 	_phase0 += _pfrac;
//
// 	return _phase0;
// }


void EnvelopeLinear::setStage(uint8_t stage)
{
	_stage = stage;
}


void EnvelopeLinear::setAttack(uint8_t att)
{
	if(att>127) att = 127;
	_b1_attack = _filterCoefficient[att];
}


void EnvelopeLinear::setDecay(uint8_t dec)
{
	if(dec>127) dec = 127;
	_b1_decay = _filterCoefficient[dec];
}


void EnvelopeLinear::setSustain(uint8_t sus)
{
	_sustain = uint32_t(sus * uint32_t(MAX_ENV_GAIN >> 7));
}


void EnvelopeLinear::setRelease(uint8_t rel)
{
	if(rel>127) rel = 127;
	_b1_release = _filterCoefficient[rel];
}


void EnvelopeLinear::setVelSustain(uint8_t vel)
{
	_velSustain = vel * (_sustain >> 7);
}


void EnvelopeLinear::setVelPeak(uint8_t vel)
{
	_velPeak = vel * (MAX_ENV_GAIN >> 7);
}
