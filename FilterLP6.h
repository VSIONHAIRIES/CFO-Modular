#pragma once
#include <stdint.h>
#include <Filter.h>

class FilterLP6 : public Filter {
public:

	FilterLP6();
	~FilterLP6(){}

	void process() override;

protected:

	int64_t _filterCoefficient[128];

	int64_t lastSampleOutLP;
	int64_t _a0;
	int64_t _a1;
	int64_t _b0;
	int64_t _b1;
    int64_t _y1;
    int64_t _x1;
    int64_t _y0;
    int64_t _x0;
    	
};