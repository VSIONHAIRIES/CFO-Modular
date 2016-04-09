#include "FilterMoog.h"
#define USE_MATH_DEFINES
#include <math.h>
#include <Arduino.h>

#define SAMPLE_RATE 48000.0f
#define RESOLUTION_FACTOR_THRESHOLD 230
#define RESOLUTION_FACTOR_HIGH 32
#define RESOLUTION_FACTOR_LOW 24
FilterMoog::FilterMoog() : Filter() {

	sample_rate = SAMPLE_RATE;
	generateFilterCoefficientsMoogLadder();

	k = 0;

	x0 = 0;
	v1 = y1 = z1 = 0;
	v2 = y2 = z2 = 0;
	v3 = y3 = z3 = 0;
	v4 = y4 = z4 = 0;

}


void FilterMoog::generateFilterCoefficientsMoogLadder() {
	for(int i=0; i < FILTER_MOOG_NUM_COEFFICIENTS; i++) {
		// double freq = (( pow ( 2, (i - 69.0) / 12.0 ) ) * 440.0);
		double freq = (( pow ( 2, (i - 117.0) / 24.0 ) ) * 440.0);
		fc[i] = uint64_t(freq / sample_rate * BIT_32_FLOAT);
		// freq = 220.0;

		// double wd = 2 * M_PI * 440;
		double wd = 2 * M_PI * freq;
		double T  = 1.0 / (double)sample_rate;
		double wa = (2 / T) * tan(wd * T / 2);

		double g_float  = wa * T / 2;
		double gg_float = g_float * g_float;
		double ggg_float = g_float * g_float * g_float;
		double G_float = g_float * g_float * g_float * g_float;
		double Gstage_float = g_float / ( 1.0 + g_float );

		int resolution_factor;
		if(i < RESOLUTION_FACTOR_THRESHOLD) resolution_factor = RESOLUTION_FACTOR_HIGH;
		else if(i >= RESOLUTION_FACTOR_THRESHOLD) resolution_factor = RESOLUTION_FACTOR_LOW;

		g[i] = uint64_t(g_float * double((uint64_t(1) << resolution_factor)));
		gg[i] = uint64_t(gg_float * double((uint64_t(1) << resolution_factor)));
		ggg[i] = uint64_t(ggg_float * double((uint64_t(1) << resolution_factor)));
		G[i] = uint64_t(G_float * double((uint64_t(1) << resolution_factor)));
		Gstage[i] = uint64_t(Gstage_float * double((uint64_t(1) << resolution_factor)));

		// MOST OF THESE WILL BE ZERO. ADJUST THE BITSHIFTS TO INCREASE RESOLUTION
		for(int k=0; k < FILTER_MOOG_NUM_K; k++) {
			u_divisor[k][i] = uint64_t(1 / (1.0 + k * G_float) * double((uint64_t(1) << 32)));

			// u_divisor[k-1][i] = uint64_t(1 / (1.0 + k * G_float) * double((uint64_t(1) << 32)));
		}
		// double u = (xn - k*S)/(1 + k*G);

	}
}


void FilterMoog::setResonance(int res) {
	if(res < 0) res = 0;
	else if(res > FILTER_MOOG_NUM_K) res = FILTER_MOOG_NUM_K;
	k = res;
}


void FilterMoog::process() {

	_audioIn = *audioIn_ptr;
	_cutoffIn = *cutoffIn_ptr;
	_cutoffModSourceIn = *cutoffModSourceIn_ptr;
	_cutoffModAmountIn = *cutoffModAmountIn_ptr;

	int64_t mod = (int64_t(_cutoffModAmountIn) * (int64_t(_cutoffModSourceIn))) >> 31;
	int64_t c = (mod + int64_t(_cutoffIn));
	// uint64_t c = _cutoffIn;
	if(c > SIGNED_BIT_32_HIGH) c = SIGNED_BIT_32_HIGH;
	// else if(c < 0) c = 0;

    int f = c>>23;
    // if(f > 234) f = 234;
	int resolution_factor;
	if(f < RESOLUTION_FACTOR_THRESHOLD) resolution_factor = RESOLUTION_FACTOR_HIGH;
	if(f >= RESOLUTION_FACTOR_THRESHOLD) resolution_factor = RESOLUTION_FACTOR_LOW;

	x0 = int64_t(_audioIn);
	int64_t S = (z1 * ggg[f] + z2 * gg[f] + z3 * g[f] + z4) >> resolution_factor;
	int64_t Gs = Gstage[f];

	// double u = (xn - k*S)/(1 + k*G);
	u = ((x0 - k * S) * u_divisor[k][f]) >> 32;
    // u = x0 - k * S;
	// if(u > SIGNED_BIT_32_HIGH) u = SIGNED_BIT_32_HIGH;
	// if(u < SIGNED_BIT_32_LOW) u = SIGNED_BIT_32_LOW;

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
