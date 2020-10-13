#pragma once
#include "RenderWindow.h"
#include "Graphics/Graphics.h"
#include <chrono>

#include "Recorder.h"
#include "WAVSoundFile.h"
#include "SoundPlayer.h"
#include "SoundAnalyser.h"
#include "SoundStreamFile.h"
#include "IVideoStream.h"
#include "ShaderConstBuffer.h"

//class Recorder;
//class WAVSound;
//class SoundPlayer;
//class SoundAnalyser;

constexpr int fft_res_size = 512;

class Engine
{
public:
	Engine();
	virtual ~Engine();

	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height, IRecorder *recorder, const std::wstring &shader_name, ShaderConstBuffer *cbuf, const std::vector<IDynamicTexture*> &dyn_texs, const std::vector<IStaticTexture*> &stat_texs);
	bool ProcessMessages();
	virtual void Update();
	void RenderFrame();

protected:
	RenderWindow render_window;
	Graphics gfx;

	int width, height;
};