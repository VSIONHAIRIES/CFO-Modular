#include <Envelope.h>

Envelope::Envelope() : AudioNode() {

	_lastGateIn = 0;
	envelopeOut = 0;

}


void Envelope::enableEnvelope()
{
	_envelopeOn = 1;
}


void Envelope::disableEnvelope()
{
	_envelopeOn = 0;
}

