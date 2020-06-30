#pragma once
#include <string>

class ISound {
public:
	virtual ~ISound() { }

	virtual const std::string &Path() = 0;

	virtual float *Data() = 0;
	virtual int DataSize() = 0;
};