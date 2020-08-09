#pragma once
#include "IDynamicTexture.h"

class ShaderDynTexture : public IDynamicTexture {
public:
	ShaderDynTexture(void *tex_data, int tex_h, int tex_w, DynTextureType tex_type, int reg) : data(tex_data), height(tex_h), width(tex_w), type(tex_type), register_id(reg) {}

	virtual int Height() { return height; }
	virtual int Width() { return width; }

	virtual void GetData(void *out_data, unsigned int stride);

	virtual DynTextureType Type() { return type; }

	virtual int RegisterId() { return register_id; };

private:
	void *data;
	int height;
	int width;

	int register_id;

	DynTextureType type;
};