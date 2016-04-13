#include <Mixer.h>

Mixer::Mixer() : AudioNode() {
	ch1audioIn_ptr = &_noSignal;
	ch2audioIn_ptr = &_noSignal;
	ch3audioIn_ptr = &_noSignal;
	ch4audioIn_ptr = &_noSignal;
	ch1gainIn_ptr = &_noSignal;
	ch2gainIn_ptr = &_noSignal;
	ch3gainIn_ptr = &_noSignal;
	ch4gainIn_ptr = &_noSignal;
}

void Mixer::process() {
	_ch1audio = *ch1audioIn_ptr + SIGNED_BIT_32_HIGH;
	_ch2audio = *ch2audioIn_ptr + SIGNED_BIT_32_HIGH;
	_ch3audio = *ch3audioIn_ptr + SIGNED_BIT_32_HIGH;
	_ch4audio = *ch4audioIn_ptr + SIGNED_BIT_32_HIGH;
	_ch1gain = *ch1gainIn_ptr;
	_ch2gain = *ch2gainIn_ptr;
	_ch3gain = *ch3gainIn_ptr;
	_ch4gain = *ch4gainIn_ptr;
	if(_ch1gain < 0) _ch1gain = 0;
	if(_ch2gain < 0) _ch2gain = 0;
	if(_ch3gain < 0) _ch3gain = 0;
	if(_ch4gain < 0) _ch4gain = 0;

	_audioOut = (_ch1audio * _ch1gain) >> 31;
	_audioOut += (_ch2audio * _ch2gain) >> 31;
	_audioOut += (_ch3audio * _ch3gain) >> 31;
	_audioOut += (_ch4audio * _ch4gain) >> 31;
	if(_audioOut > UNSIGNED_BIT_32_HIGH) _audioOut = UNSIGNED_BIT_32_HIGH;
	audioOut = _audioOut - SIGNED_BIT_32_HIGH;
}