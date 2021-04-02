#include "SimpleSound.h"

static const char *frame_save_path = "imgs\\img_";
//const char *file_path = "./test_shader.wav";
//const char *file_path = "./Summer.wav";
//static const char *file_path = "./GoFuckYourself.wav";
//static const char *file_path = "./techno_test.wav";
static const char *file_path = "./Alexander-Platz.wav";
//const char *file_path = "./BrianJames.wav";

constexpr int sound_step = 2048;

SimpleSound::SimpleSound() {
	recorder = 0;
	snd = 0;
	snd_stream = 0;
	inited = false;

	tent_len = 0.f;
	cam_pos = 0.f;

	for (int i = 0; i < fft_res_size; ++i) {
		fft_data[i] = 0;
		fft_prev[i] - 0;
	}
}

SimpleSound::~SimpleSound() {
	delete snd;
	delete snd_stream;

	delete audo_data_tex;
	delete image;
}

bool SimpleSound::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height) {
	//	recorder = new Recorder(frame_save_path, width, height);
	snd = new WAVSoundFile(file_path);
	snd_stream = new SoundStreamFile(snd, sound_step);

//	player.Play(snd);
	prev_time = start_time = std::chrono::steady_clock::now();

	scb.Data(&cbuffer);
	scb.Size(sizeof(cbuffer));

	audo_data_tex = new ShaderDynTexture(fft_data, 1, 512, DT_U8R, 0);
	std::vector<IDynamicTexture*> dyn_textures;
	dyn_textures.push_back(audo_data_tex);

	std::vector<IStaticTexture*> stat_textures;

	cbuffer.width = width;
	cbuffer.height = height;

	return Engine::Initialize(hInstance, window_title, window_class, width, height, recorder, L"test_adv_projection", &scb, dyn_textures, stat_textures);
}

template <typename T, int size, int window_h_size>
void smooth_array(T *data) {
//	const int window_h_size = 16;

	T inner[size];
	memcpy(inner, data, sizeof(T) * size);

	for (int i = 0; i < size; ++i) {
		float mmax = data[i];
		for(int j = -window_h_size; j <= window_h_size; j++) {
			int id = i + j;
			if (id < 0 || id >= size) continue;

			float coef = 1.f - float(abs(j)) / float(window_h_size);
			float smooth_val = float(inner[id]) * pow(coef, 0.25f);
			mmax = max(mmax, smooth_val);
		}
		data[i] = mmax;
	}
}

void SimpleSound::Update() {
	bool snd_updated = snd_stream->Update(time);

	const int bass_samples_cnt = 32;

	if (snd_stream->Finished()) return;

// FOR CAPTURE
//	float dt = 1.f/30.f;
//	time += dt;

	std::chrono::time_point<std::chrono::steady_clock> cur_time = std::chrono::steady_clock::now();
	// FOR REALTIME
	std::chrono::duration<float> cdt = cur_time - prev_time;
	std::chrono::duration<float> diff = cur_time - start_time;
	float dt = cdt.count();
	time = diff.count();

	cbuffer.time = time;
	prev_time = cur_time;

	if (snd_updated || !inited) {
		float fft_res[fft_res_size];
		analyser.CalcFFT_log(snd_stream->CurData(), sound_step, snd_stream->File()->Channels(), fft_res, fft_res_size);

		cbuffer.bass_coef = 0;
		float tent_len_max = 0.f;
		for (int i = 0; i < fft_res_size; ++i) {
//			float new_data = clamp((fft_prev[i] + fft_res[i]) / 2 * 255., 0., 255.);
			float new_data = clamp((fft_res[i] - fft_prev[i]) * 255., 0., 255.);
			if (new_data >= fft_data[i])
				fft_data[i] = new_data;
			else
				fft_data[i] -= min(10, fft_data[i] - new_data);

			if (i < bass_samples_cnt)
				cbuffer.bass_coef += new_data;
		}
		CopyMemory(fft_prev, fft_res, fft_res_size*sizeof(float));
		cbuffer.bass_coef /= bass_samples_cnt;

		inited = true;
	}

	Engine::Update();
}