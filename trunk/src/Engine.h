#pragma once
#include "RenderWindow.h"
#include "Graphics/Graphics.h"

class Engine
{
protected:
	RenderWindow render_window;
	Graphics gfx;

public:
	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	bool ProcessMessages();
	void Update();
	void RenderFrame();
};