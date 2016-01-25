#pragma once
#include "Envelope.h"

class EnvelopeLinear : public Envelope {
public:

	EnvelopeLinear();
	~EnvelopeLinear(){}

	void process() override;

	void setStage(uint8_t stage);
	void setAttack(uint8_t att);
	void setDecay(uint8_t dec);
	void setSustain(uint8_t sus);
	void setRelease(uint8_t rel);
	void setVelSustain(uint8_t vel);
	void setVelPeak(uint8_t vel);

	int getFractionalAttack(int indx);

protected:

	int _attackIndex;
	int _decayIndex;
	int _releaseIndex;

	int _attackTable[257];
	int _decayTable[257];
	int _envelopeTimeTable[128];

	int _env;
	int _stage;
	
	int _attack;
	int _decay;
	int _sustain;
	int _release;

	int _velSustain;
	int _velPeak;

	//variables for direct audiorate calculation of index positions
	int64_t		_dir;
	int64_t 	_indx;
	int64_t 	_ifrac;
	int64_t		_phase0;
	int64_t 	_phase1;
	int64_t		_dphase;
	int64_t 	_pfrac;
	// int 		_dPhase;



};