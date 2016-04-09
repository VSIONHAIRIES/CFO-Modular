#include "FilterLP24.h"
#include <math.h>

#define SAMPLE_RATE 48000.0f
#define RESOLUTION_FACTOR_THRESHOLD 230
#define RESOLUTION_FACTOR_HIGH 32
#define RESOLUTION_FACTOR_LOW 24

FilterLP24::FilterLP24() : Filter() {

	sample_rate = SAMPLE_RATE;
	generateFilterCoefficients();

	x0 = 0;
	v1 = y1 = z1 = 0;
	v2 = y2 = z2 = 0;
	v3 = y3 = z3 = 0;
	v4 = y4 = z4 = 0;

}


void FilterLP24::generateFilterCoefficients() {
	for(int i=0; i < FILTER_LP24_NUM_COEFFICIENTS; i++) {
		// double freq = (( pow ( 2, (i - 69.0) / 12.0 ) ) * 440.0);
		double freq = (( pow ( 2, (i - 117.0) / 24.0 ) ) * 440.0);
		fc[i] = uint64_t(freq / sample_rate * BIT_32_FLOAT);

		double wd = 2 * M_PI * freq;
		double T  = 1.0 / (double)sample_rate;
		double wa = (2 / T) * tan(wd * T / 2);

		double g_float  = wa * T / 2;
		double Gstage_float = g_float / ( 1.0 + g_float );

		int resolution_factor;
		if(i < RESOLUTION_FACTOR_THRESHOLD) resolution_factor = RESOLUTION_FACTOR_HIGH;
		else resolution_factor = RESOLUTION_FACTOR_LOW;

		Gstage[i] = uint64_t(Gstage_float * double((uint64_t(1) << resolution_factor)));
	}
}


void FilterLP24::process() {

	_audioIn = *audioIn_ptr;
	_cutoffIn = *cutoffIn_ptr;
	_cutoffModSourceIn = *cutoffModSourceIn_ptr;
	_cutoffModAmountIn = *cutoffModAmountIn_ptr;

	int64_t mod = (int64_t(_cutoffModAmountIn) * (int64_t(_cutoffModSourceIn))) >> 31;
	int64_t c = (mod + int64_t(_cutoffIn));

	if(c > SIGNED_BIT_32_HIGH) c = SIGNED_BIT_32_HIGH;
	else if(c < 0) c = 0;

    int f = c>>23;
    if(f > 233) f = 233;
	int resolution_factor;
	if(f < RESOLUTION_FACTOR_THRESHOLD) resolution_factor = RESOLUTION_FACTOR_HIGH;
	if(f >= RESOLUTION_FACTOR_THRESHOLD) resolution_factor = RESOLUTION_FACTOR_LOW;

	x0 = int64_t(_audioIn);
	int64_t Gs = Gstage[f];

	u = (x0 * uint64_t(UNSIGNED_BIT_32_HIGH)) >> 32; // <<---- not sure why it breaks without...

	v1 = ((u - z1) * Gs) >> resolution_factor;
    y1 = (v1 + z1);
    z1 = y1 + v1;

    v2 = ((y1 - z2) * Gs) >> resolution_factor;
    y2 = (v2 + z2);
    z2 = y2 + v2;

    v3 = ((y2 - z3) * Gs) >> resolution_factor;
    y3 = (v3 + z3);
    z3 = y3 + v3;

    v4 = ((y3 - z4) * Gs) >> resolution_factor;
    y4 = (v4 + z4);
    z4 = y4 + v4;

	_audioOut = y4;
	audioOut = _audioOut;

}
