#pragma once
//#include "ISound.h"

class SoundAnalyser  {
public:
	SoundAnalyser() {}

	bool CalcFFT_log(float *input, int input_sz, int input_stride, float *output, int output_sz);

//	SoundAnalyser(ISound *sound) : snd(sound) { }

//	virtual void Update();
//	virtual void GetData(SoundData &res);

public:
//	ISound *snd;

//	SoundData data;

//	unsigned char fft_data[512];
};