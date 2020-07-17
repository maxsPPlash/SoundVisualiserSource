#pragma once

class ISoundStream {
public:
	ISoundStream() {}
	virtual ~ISoundStream() {}

	virtual bool Update(float cur_time) = 0;
};