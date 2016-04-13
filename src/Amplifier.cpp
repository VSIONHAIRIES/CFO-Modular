#include <Amplifier.h>

void Amplifier::process() {
	_ampModAmountIn = *ampModAmountIn_ptr;
	_ampModSourceIn = *ampModSourceIn_ptr;
	_audioSigned = *audioIn_ptr + SIGNED_BIT_32_HIGH; // we can't bitshift signed signals so we got to make it unsigned
	if(_ampModAmountIn < 0) _ampModAmountIn = 0;
	if(_ampModSourceIn < 0) _ampModSourceIn = -_ampModSourceIn;
	uint64_t ampModSignal = (uint64_t(_ampModSourceIn) * uint64_t(_ampModAmountIn)) >> 31;
	audioOut = ((_audioSigned * ampModSignal) >> 31) - SIGNED_BIT_32_HIGH; // bring back the signal in signed range
}