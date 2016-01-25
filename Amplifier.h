#pragma once
#include "AudioNode.h"

class Amplifier : public AudioNode {
public:

	Amplifier(){}
	~Amplifier(){}

	void process() override;

	int* ampModAmountIn_ptr;
	int* ampModSourceIn_ptr;

protected:

	uint64_t _audioSigned;
	int _ampModAmountIn;
	int _ampModSourceIn;
};