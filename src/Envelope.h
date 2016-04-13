#ifndef ENVELOPE_H
#define ENVELOPE_H
#include "AudioNode.h"

class Envelope : public AudioNode {
public:

	Envelope();
	~Envelope(){}

	void enableEnvelope();
	void disableEnvelope();

	int* gateIn_ptr;
	int envelopeOut;

protected:

	int _gateIn;
	int _lastGateIn;

	int _envelopeOn;

};

#endif // ENVELOPE_H
