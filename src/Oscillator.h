#pragma once
#include "AudioNode.h"
#include <stdint.h>
// #include <math.h>

#define PERIOD_MAX BIT_32 /// need to move this
#define SAMPLE_RATE 48000 /// need to move this

class Oscillator : public AudioNode {
public:

	Oscillator();
	~Oscillator(){}

	void accumulator();

	// FREQUENCY AND DETUNE FUNCTIONS
	void setFrequency(float frequency);
	void inline setFrequency();

	void setSemitone(int semi); // -24 to 24 semitones
	void setDetune(int detune); // one semitone is 1024
	void setBend(int bend);
	void setPortamento(int port);

	void setFrequencyIn(int midinote); // set the frequency by MIDI note number 0-127
	int getExpFrequencyFromMidinote(int indx);
	int getExpFrequency(int indx);

	// GAIN FUNCTIONS
	void setGain(float value); // 0.0 - 1.0
	float getGain();       // 0.0 - 1.0

	bool _lfo;

	int* frequencyIn_ptr;
	int* fmSourceIn_ptr;
	int* fmAmountIn_ptr;
	int* portamentoIn_ptr;

protected:

	int64_t		_expFrequency[256];
	int64_t		_lfoFrequency[256];
	int64_t		_portamentoLPCoefficient[128];

	int _nosignal;

	float 		_freq;
	int 		_detune;
	int			_semi;
	int			_portamento;
	int			_bend;
	int			_gain;

	int 		_period;
	int			_phase;
	int 		_accumulator;
	int			_osc;

	// variables for portamento frequency change LP filter
	int64_t _a0, _b1, _x0, _y0;

	//variables for direct audiorate calculation of frequency
	int64_t		_dir;
	int64_t 	_indx;
	int64_t 	_ifrac;
	int64_t		_freq0;
	int64_t 	_freq1;
	int64_t		_dfreq;
	int64_t 	_ffrac;
	int 		_dPhase;

	int _midinoteIn;
	int _frequencyIn;
	int _fmSourceIn;
	int _fmAmountIn;
	int _portamentoIn;
	int _isPlaying;
	// int _audioOut;

};
