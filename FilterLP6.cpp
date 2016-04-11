#include "FilterLP6.h"
#define USE_MATH_DEFINES
#include <math.h>

const int64_t filterCoefficient[] = {
	#include <filterCoefficients_1poleLP.inc>
};

#define SAMPLE_RATE 48000.0f


FilterLP6::FilterLP6() : Filter() {

	for (int i=0; i<128; i++) {
		float fc = ( ( pow ( 2, ( ( float(i) - 69.0) / 12.0 ) ) ) * 440.0 ) / SAMPLE_RATE;
		_filterCoefficient[i] = int64_t ( BIT_32_FLOAT * ( exp ( M_PI * -2.0 * fc ) ) );
	}

	_a0 = 0;
	_a1 = 0;
	_b0 = 0;
	_b1 = 0;
	_x0 = 0;
	_x1 = 0;
	_y0 = 0;
	_y1 = 0;
}


void FilterLP6::process() {

	_audioIn = *audioIn_ptr;
	_cutoffIn = *cutoffIn_ptr;
	_cutoffModSourceIn = *cutoffModSourceIn_ptr;
	_cutoffModAmountIn = *cutoffModAmountIn_ptr;

	int64_t mod = (int64_t(_cutoffModAmountIn) * (int64_t(_cutoffModSourceIn)))>>32;
	int64_t c = (mod + int64_t(_cutoffIn));
	if(c > SIGNED_BIT_32_HIGH) c = SIGNED_BIT_32_HIGH;
	else if(c < SIGNED_BIT_32_LOW) c = SIGNED_BIT_32_LOW;
	// int64_t c = _cutoffIn;

	_x0 = _audioIn + SIGNED_BIT_32_HIGH;
    _b1 = _filterCoefficient[c>>24];
    _a0 = BIT_32 - _b1;

    _y0 = (_a0 * _x0 + _b1 * _y1) >> 32;
    _y1 = _y0;

	_audioOut = _y0 - SIGNED_BIT_32_HIGH;

	audioOut = _audioOut;

}
