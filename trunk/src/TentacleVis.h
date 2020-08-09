#pragma once
#include "Engine.h"
#include "ShaderDynTexture.h"
#include "ShaderStaticTexture.h"

struct tentavle_t_const_buffer
{
	int width = 0;
	int height = 0;
	float time = 0;

	float tent_len = 0;
	float cam_pos = 0;
	float bass_coef = 0;
};

class TentacleVis : public Engine {
public:
	TentacleVis();
	~TentacleVis();

	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	virtual void Update() override;

	void UpdateFreqs();
	void UpdateBass();

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

	tentavle_t_const_buffer cbuffer;
	ShaderConstBuffer scb;

	float time;
	float tent_len;
	float cam_pos;

	unsigned short tent_len_data[fft_res_size];
	float tent_len_float[fft_res_size];

	bool inited;
};