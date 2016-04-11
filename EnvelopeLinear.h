#ifndef ENVELOPE_LINEAR_H
#define ENVELOPE_LINEAR_H

#include "Envelope.h"

class EnvelopeLinear : public Envelope {
public:

	EnvelopeLinear();
	~EnvelopeLinear(){}

	void process() override;

	// int interpolateFromTable(int indx, int *table);
	void setStage(uint8_t stage);
	void setAttack(uint8_t att);
	void setDecay(uint8_t dec);
	void setSustain(uint8_t sus);
	void setRelease(uint8_t rel);
	void setVelSustain(uint8_t vel);
	void setVelPeak(uint8_t vel);

	// int getFractionalAttack(int indx);



	// uint32_t _attackIndex;
	// uint32_t _decayIndex;
	// uint32_t _releaseIndex;

	// int _attackTable[257];
	// int _decayTable[257];
	// int64_t _envelopeSpeedTable[128];
	unsigned long _env;
	unsigned long _env_target;
	int _stage;

	uint32_t _attack;
	uint32_t _decay;
	uint32_t _sustain;
	uint32_t _release;

	uint32_t _velSustain;
	uint32_t _velPeak;

	//variables for direct audiorate calculation of index positions
	// int64_t		_dir;
	// int64_t 	_indx;
	// int64_t 	_ifrac;
	// int64_t		_phase0;
	// int64_t 	_phase1;
	// int64_t		_dphase;
	// int64_t 	_pfrac;

	// variables for low pass filter on _env_target
	uint64_t _filterCoefficient[128];
	uint64_t _a0;
	uint64_t _a1;
	uint64_t _b0;
	uint64_t _b1;
	uint64_t _b1_attack;
	uint64_t _b1_decay;
	uint64_t _b1_release;
	uint64_t _y1;
	uint64_t _x1;
	uint64_t _y0;
	uint64_t _x0;
protected:


};

#endif // ENVELOPE_LINEAR_H
