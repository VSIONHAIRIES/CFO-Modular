#pragma once

#include "AudioNode.h"

class Filter : public AudioNode {
public:

	Filter();
	~Filter(){}

// FILTER FUNCTIONS
	virtual void setCutoff(float cutoff);
	virtual float getCutoff();
	virtual void setResonance(float resonance);
	virtual float getResonance();
	virtual void setCutoffModAmount(float amount);
	virtual float getCutoffModAmount();

	int* cutoffIn_ptr;
	int* cutoffModSourceIn_ptr;
	int* cutoffModAmountIn_ptr;

protected:

	int _cutoffIn;
	int _cutoffModSourceIn;
	int _cutoffModAmountIn;

	int	_cutoff;
	int	_resonance;
    int _cutoffModSource;
    int _cutoffModAmount;
	
};