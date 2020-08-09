#pragma once
#include <string>
#include "IStaticTexture.h"

class ShaderStaticTexture : public IStaticTexture {
public:
	ShaderStaticTexture(const std::wstring &tex_path, int reg) : path(tex_path), register_id(reg) {}

	virtual const std::wstring &Path() { return path; }

	virtual int RegisterId() { return register_id; }

private:
	std::wstring path;

	int register_id;
};