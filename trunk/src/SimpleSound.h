#pragma once
#include "Engine.h"
#include "ShaderDynTexture.h"
#include "ShaderStaticTexture.h"

struct simple_sound_const_buffer
{
	int width = 0;
	int height = 0;
	float time = 0;

	float bass_coef = 0;
};

class SimpleSound : public Engine {
public:
	SimpleSound();
	~SimpleSound();

	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	virtual void Update() override;

private:
	Recorder *recorder;
	WAVSoundFile *snd;
	SoundStreamFile *snd_stream;
	SoundPlayer player;
	SoundAnalyser analyser;

	ShaderDynTexture *audo_data_tex;
	ShaderStaticTexture *image;

	std::chrono::time_point<std::chrono::steady_clock> start_time;
	std::chrono::time_point<std::chrono::steady_clock> prev_time;

	simple_sound_const_buffer cbuffer;
	ShaderConstBuffer scb;

	float time;
	float tent_len;
	float cam_pos;

	float fft_prev[fft_res_size];
	unsigned char fft_data[fft_res_size];

	bool inited;
};