#include "ShaderDynTexture.h"

#include <windows.h>

void ShaderDynTexture::GetData(void *out_data, unsigned int stride) {
	CopyMemory(out_data, data, ElementSize() * height * width);
};