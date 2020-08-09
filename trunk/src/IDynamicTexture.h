#pragma once

enum DynTextureType {
	DT_U8RGBA = 0,
	DT_U16R,
};

class IDynamicTexture {
public:
	virtual int Height() = 0;
	virtual int Width() = 0;

	virtual void GetData(void *data, unsigned int stride) = 0;

	virtual DynTextureType Type() = 0;

	virtual int RegisterId() = 0;

	int ElementSize() {
		switch (Type()) {
		case DT_U8RGBA:
			return sizeof(unsigned char) * 4;
			break;
		case DT_U16R:
			return sizeof(unsigned short);
		}
	}
};