#pragma once

class IRecorder {
public:
	virtual ~IRecorder() {}

	virtual void SaveNewFrame(unsigned int *data) = 0;
};