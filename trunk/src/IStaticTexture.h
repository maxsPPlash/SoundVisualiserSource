#pragma once

class IStaticTexture {
public:
	virtual const std::wstring &Path() = 0;

	virtual int RegisterId() = 0;
};