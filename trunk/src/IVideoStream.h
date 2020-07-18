#pragma once

class IVideoStream {
public:
	IVideoStream() {}
	virtual ~IVideoStream() {}

	virtual bool Update(float cur_time) = 0;
	virtual bool GetFrame(const unsigned char *& data, int &size) = 0;
	virtual bool CopyFrame(void *data, unsigned int size) = 0;
};