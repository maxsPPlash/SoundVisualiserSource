#pragma once
#include "IDynamicTexture.h"
#include "IVideoStream.h"

class ShaderVideoTexture : public IDynamicTexture {
public:
	ShaderVideoTexture(IVideoStream *tex_data, int tex_h, int tex_w, int reg) : video(tex_data), height(tex_h), width(tex_w), register_id(reg) {}

	virtual int Height() { return height; }
	virtual int Width() { return width; }

	virtual void GetData(void *out_data, unsigned int stride) { video->CopyFrame(out_data, stride); }

	virtual DynTextureType Type() { return DT_U8RGBA; } // right now only this type

	virtual int RegisterId() { return register_id; };

private:
	IVideoStream *video;
	int height;
	int width;

	int register_id;
};