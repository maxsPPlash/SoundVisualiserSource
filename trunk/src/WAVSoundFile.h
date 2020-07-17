#pragma once
#include "ISoundFile.h"
#include <string>

class WAVSoundFile : public ISoundFile {
public:
	WAVSoundFile(const char *file_path) { LoadSound(file_path); }
	~WAVSoundFile() { free(data); }

	bool LoadSound(const char *file_path);

	virtual const std::string &Path() { return path; };
	virtual float *Data() { return data; }
	virtual int SampleCount() { return sample_count; }
	virtual int SampleRate() { return samplerate; }
	virtual int Channels() { return channels; }

private:
	std::string path;
	float *data;

	int sample_count;
	int samplerate;
	int channels;
};