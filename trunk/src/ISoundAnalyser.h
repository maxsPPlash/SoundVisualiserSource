#pragma once

struct SoundData {
	float fft_bass;
	unsigned char *fft_data;
};

class ISoundAnalyser {
public:
	virtual void Update() = 0;
	/// Not thread safe!
	virtual void GetData(SoundData &res) = 0;
};