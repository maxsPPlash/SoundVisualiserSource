#pragma once
#include <vector>

struct ViewData {
	std::vector<std::vector<float>> data;
	float max_val;
};

class AudioProcessor {
public:
	AudioProcessor();

	ViewData &getDataFFT() { return FFT; }
	ViewData &getDataSum() { return sum_data; }

	void UpdateSum(int window = -1);
private:
	ViewData FFT;
	ViewData sum_data;
};