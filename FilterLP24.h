#ifndef FILTER_LP24_H
#define FILTER_LP24_H
#include <stdint.h>
#include <Filter.h>

#define FILTER_LP24_NUM_COEFFICIENTS 256

class FilterLP24 : public Filter {
public:

	FilterLP24();
	~FilterLP24(){}

	void generateFilterCoefficients();

	void process() override;

	uint64_t fc[FILTER_LP24_NUM_COEFFICIENTS]; // 32bit representation of fc/sample_rate
	uint64_t Gstage[FILTER_LP24_NUM_COEFFICIENTS];

protected:
	int sample_rate;
	int64_t u;
	int64_t x0;
	int64_t v1, y1, z1;
	int64_t v2, y2, z2;
	int64_t v3, y3, z3;
	int64_t v4, y4, z4;

};

#endif // FILTER_LP24_H
