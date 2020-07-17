#pragma once
#include <string>

class ISoundFile {
public:
	virtual ~ISoundFile() { }

	virtual const std::string &Path() = 0;

	virtual float *Data() = 0;
	virtual int SampleCount() = 0;
	virtual int SampleRate() = 0;
	virtual int Channels() = 0;
};