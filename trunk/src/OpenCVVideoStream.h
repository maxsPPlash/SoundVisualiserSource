#pragma once
#include "IVideoStream.h"
#include "opencv2/opencv.hpp"

class OpenCVVideoStream : public IVideoStream {
public:
	OpenCVVideoStream();
	~OpenCVVideoStream();

	bool Initialise(const char* path);

	virtual bool Update(float cur_time);
	virtual bool GetFrame(const unsigned char *& data, int &size);
	virtual bool CopyFrame(void *data, unsigned int size);

private:
	cv::VideoCapture video;
	cv::Mat frame;
};