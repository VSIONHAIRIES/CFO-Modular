#pragma once
#include "Envelope.h"

#define MAX_ENV_GAIN 65535

class EnvelopeWithDivision : public Envelope {
public:

	EnvelopeWithDivision();
	~EnvelopeWithDivision(){}

	void process() override;

	void enableEnvelope();
	void disableEnvelope();
	void setStage(uint8_t stage);
	void setAttack(uint8_t att);
	void setDecay(uint8_t dec);
	void setSustain(uint8_t sus);
	void setRelease(uint8_t rel);
	void setVelSustain(uint8_t vel);
	void setVelPeak(uint8_t vel);

protected:

	int _envTimeTable[128];

	int _env;
	int _stage;
	
	int _attack;
	int _decay;
	int _sustain;
	int _release;

	int _velSustain;
	int _velPeak;


};