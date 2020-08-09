#pragma once
#include "IConstBufferData.h"

class ShaderConstBuffer : public IConstBufferData {
public:
	virtual void *Data() { return data; };
	virtual int Size() { return size; };

	void Data(void * d) { data = d; };
	void Size(int sz) { size = sz; };
private:
	void *data;
	int size;
};