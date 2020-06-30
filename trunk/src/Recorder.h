#pragma once
#include "IRecorder.h"
#include <string>


class Recorder : public IRecorder {
public:
	Recorder(const char * save_path, int frame_width, int frame_height) : path(save_path), width(frame_width), height(frame_height), frame_id(0) {
	}

	virtual void SaveNewFrame(unsigned int *data);

private:
	void SavePNG(const char *filename, unsigned int *data);

private:
	std::string path;
	int width, height;
	int frame_id;
};