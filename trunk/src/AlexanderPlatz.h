#pragma once
#include "Engine.h"
#include "ShaderDynTexture.h"
#include "ShaderStaticTexture.h"

struct alexander_platz_const_buffer
{
	int width = 0;
	int height = 0;
	float time = 0;

	float bass_coef = 0;
	float smooth_bass_coef = 0;

	float c1_time = -10;
	float c2_time = -10;

	float eye_time = -1;
	float eye_id = -1;
	float eye_stage = -1;

	float time_end = 0;
};

class AlexanderPlatz : public Engine {
public:
	AlexanderPlatz();
	~AlexanderPlatz();

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

	alexander_platz_const_buffer cbuffer;
	ShaderConstBuffer scb;

	float time;
	float tent_len;
	float cam_pos;

	float fft_prev[fft_res_size];
	unsigned char fft_data[fft_res_size];

	bool inited;
};