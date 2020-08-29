#pragma once
#include "Engine.h"

struct projector_const_buffer
{
	int width = 0;
	int height = 0;
	float time = 0;
};

class Projector : public Engine {
public:
	Projector();
	~Projector();

	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	virtual void Update() override;

private:
	Recorder *recorder;
	WAVSoundFile *snd;
	SoundStreamFile *snd_stream;
	SoundPlayer player;
	SoundAnalyser analyser;

	std::chrono::time_point<std::chrono::steady_clock> start_time;
	std::chrono::time_point<std::chrono::steady_clock> prev_time;

	projector_const_buffer cbuffer;
	ShaderConstBuffer scb;

	float time;

	unsigned short tent_len_data[fft_res_size];

//	bool inited;
};