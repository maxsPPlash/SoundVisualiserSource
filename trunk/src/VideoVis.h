#pragma once
#include "Engine.h"
#include "ShaderVideoTexture.h"

struct video_t_const_buffer
{
	int width = 0;
	int height = 0;
	float time = 0;

	float bass_coef = 0;
};

class VideoVis : public Engine {
public:
	VideoVis();
	~VideoVis();

	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	virtual void Update() override;

private:
	Recorder *recorder;
	WAVSoundFile *snd;
	SoundStreamFile *snd_stream;
	IVideoStream *video_stream;
	SoundPlayer player;
	SoundAnalyser analyser;

	video_t_const_buffer cbuffer;
	ShaderConstBuffer scb;

	ShaderVideoTexture *video_tex;

	std::chrono::time_point<std::chrono::steady_clock> start_time;
	std::chrono::time_point<std::chrono::steady_clock> prev_time;

	float time;

	unsigned short tent_len_data[fft_res_size];

	bool inited;
};