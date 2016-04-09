#ifndef FILTER_LP24_H
#define FILTER_LP24_H
#include <stdint.h>
#include <Filter.h>

#define FILTER_LP24_NUM_COEFFICIENTS 256
#define FILTER_LP24_NUM_K 4

class FilterLP24 : public Filter {
public:

	FilterLP24();
	~FilterLP24(){}

	void generateFilterCoefficientsMoogLadder();
	void setResonance(int res);

	void process() override;

	uint64_t fc[FILTER_LP24_NUM_COEFFICIENTS]; // 32bit representation of fc/sample_rate
	uint64_t g[FILTER_LP24_NUM_COEFFICIENTS];
	uint64_t gg[FILTER_LP24_NUM_COEFFICIENTS];
	uint64_t ggg[FILTER_LP24_NUM_COEFFICIENTS];
	uint64_t G[FILTER_LP24_NUM_COEFFICIENTS];
	uint64_t Gstage[FILTER_LP24_NUM_COEFFICIENTS];
	uint64_t u_divisor[FILTER_LP24_NUM_K][FILTER_LP24_NUM_COEFFICIENTS];

protected:
	int sample_rate;


	int64_t u;
    uint64_t S;
    uint8_t k;


	int64_t x0, x1;
	int64_t v1, y1, z1;
	int64_t v2, y2, z2;
	int64_t v3, y3, z3;
	int64_t v4, y4, z4;

};

#endif // FILTER_LP24_H
