#pragma once
#include "ISound.h"
#include <string>

class WAVSound : public ISound {
public:
	WAVSound(const char *file_path) { LoadSound(file_path); }
	~WAVSound() { free(buf); }

	void Update(float cur_time);
	bool LoadSound(const char *file_path);

	virtual const std::string &Path() { return path; };
	virtual float *Data() { return buf + cur_sample; }
	virtual int DataSize() { return sample_count - cur_sample; }

private:
	std::string path;
	float *buf;
	float cur_time;
	int sample_count, samplerate;
	int cur_sample;
};