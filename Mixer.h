#pragma once
#include "AudioNode.h"

class Mixer : public AudioNode {
public:

	Mixer();
	~Mixer(){}

	void process() override;

	int* ch1audioIn_ptr;
	int* ch2audioIn_ptr;
	int* ch3audioIn_ptr;
	int* ch4audioIn_ptr;
	int* ch1gainIn_ptr;
	int* ch2gainIn_ptr;
	int* ch3gainIn_ptr;
	int* ch4gainIn_ptr;

protected:

	uint64_t _ch1audio;
	uint64_t _ch2audio;
	uint64_t _ch3audio;
	uint64_t _ch4audio;
	int _ch1gain;
	int _ch2gain;
	int _ch3gain;
	int _ch4gain;
	uint64_t _audioOut;
};