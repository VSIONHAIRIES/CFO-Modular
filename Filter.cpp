#include <Filter.h>


Filter::Filter() : AudioNode() {


	_cutoff = SIGNED_BIT_32_HIGH;
	_resonance = 0;
	_cutoffModSource = 0;
	_cutoffModAmount = 0;
}


void Filter::setCutoff(float cutoff) {
	_cutoff = int(((cutoff * 2.0) - 1.0) * SIGNED_BIT_32_HIGH);
}


float Filter::getCutoff() {
	return (float(_cutoff) / SIGNED_BIT_32_HIGH);
}


void Filter::setCutoffModAmount(float cutoffModAmount) {
	_cutoffModAmount = int(cutoffModAmount * SIGNED_BIT_32_HIGH);
}


float Filter::getCutoffModAmount() {
	return (float(_cutoffModAmount) / SIGNED_BIT_32_HIGH);
}


void Filter::setResonance(float resonance) {
	_resonance = int(resonance * SIGNED_BIT_32_HIGH);
}


float Filter::getResonance() {
	return (float(_resonance) / SIGNED_BIT_32_HIGH);
}
