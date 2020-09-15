#include "AudioProcessor.h"
#include "../WAVSoundFile.h"
#include "../SoundStreamFile.h"
#include "../SoundAnalyser.h"

#include <Windows.h>

//const char *file_path = "./test_shader.wav";
//const char *file_path = "./../Summer.wav";
//static const char *file_path = "./../GoFuckYourself.wav";
static const char *file_path = "./../Alexander-Platz.wav";
//static const char *file_path = "./../techno_test.wav";
//const char *file_path = "./BrianJames.wav";

constexpr int sound_step = 2048;
constexpr int fft_res_size = 512;

AudioProcessor::AudioProcessor() {
	WAVSoundFile snd(file_path);
	SoundAnalyser analyser;

	int cur_pointer = 0;
	const int sample_count = snd.SampleCount();
	const int chans = snd.Channels();
	sample_rate = snd.SampleRate();

	float max_val = 0;

	while (cur_pointer + sound_step < sample_count) {
		std::vector<float> fft_res;
		fft_res.resize(fft_res_size);
		analyser.CalcFFT_log(snd.Data() + cur_pointer * chans, sound_step, chans, fft_res.data(), fft_res_size);

		for (float v : fft_res) {
			max_val = max(max_val, v);
		}

		cur_pointer += sound_step;

		FFT.data.push_back(fft_res);
	}

	FFT.max_val = max_val;
	FFT.xmul = 1;
}

void AudioProcessor::UpdateSum(int window/* = -1*/) {
	if (window < 0) {
		window = 0;
	}

	sum_data.data.resize(fft_res_size);

	float max_val = 0;
	for (int j = 0; j < fft_res_size; ++j) {
		for (int i = 0, size = FFT.data.size(); i < size; ++i) {
			float sum = 0.f;
			sum = FFT.data[i][j];
//			if (j > 0) {
//				sum += FFT.data[i][j-1];
//			}
//			for (float v : FFT.data[i]) {
//				sum += v;
//			}
			max_val = max(max_val, sum);
			sum_data.data[j].push_back(sum);
		}
	}

	sum_data.max_val = max_val;
	sum_data.xmul = float(sound_step) / float(sample_rate);
}