#pragma once
#include "Engine.h"

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

	std::chrono::time_point<std::chrono::steady_clock> start_time;
	std::chrono::time_point<std::chrono::steady_clock> prev_time;

	float time;
	float tent_len;
	float cam_pos;

	unsigned short tent_len_data[fft_res_size];
	float tent_len_float[fft_res_size];

	bool inited;
};