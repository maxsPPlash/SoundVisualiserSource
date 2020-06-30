#pragma once
#include "RenderWindow.h"
#include "Graphics/Graphics.h"
#include <chrono>

class Recorder;
class WAVSound;
class SoundPlayer;
class SoundAnalyser;

class Engine
{
protected:
	RenderWindow render_window;
	Graphics gfx;

	Recorder *recorder;
	WAVSound *snd;
	SoundPlayer *player;
	SoundAnalyser *analyser;

	std::chrono::time_point<std::chrono::steady_clock> start_time;
	std::chrono::time_point<std::chrono::steady_clock> prev_time;

	int width, height;
	float time;
	float tent_len;
	float cam_pos;

public:
	Engine();

	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	bool ProcessMessages();
	void Update();
	void RenderFrame();
};