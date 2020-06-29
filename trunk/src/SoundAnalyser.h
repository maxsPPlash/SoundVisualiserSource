#pragma once
#include "ISoundAnalyser.h"
#include "ISound.h"

class SoundAnalyser : public ISoundAnalyser {
public:
	SoundAnalyser(ISound *sound) : snd(sound) { }

	virtual void Update();
	virtual void GetData(SoundData &res);

public:
	ISound *snd;

	SoundData data;

	unsigned char fft_data[512];
};