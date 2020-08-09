#pragma once

class IConstBufferData {
public:
	virtual void *Data() = 0;
	virtual int Size() = 0;
};