#include "SinCircle.h"

static const char *frame_save_path = "imgs\\img_";
//const char *file_path = "./test_shader.wav";
static const char *file_path = "./Summer.wav";
//const char *file_path = "./GoFuckYourself.wav";
//const char *file_path = "./BrianJames.wav";

constexpr int sound_step = 2048;

SinCircle::SinCircle() {
	recorder = 0;
	snd = 0;
	snd_stream = 0;
}

SinCircle::~SinCircle() {
	delete snd;
	delete snd_stream;
}

bool SinCircle::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height) {
	//	recorder = new Recorder(frame_save_path, width, height);
//	snd = new WAVSoundFile(file_path);
//	snd_stream = new SoundStreamFile(snd, sound_step);

//	player.Play(snd);
	prev_time = start_time = std::chrono::steady_clock::now();

	scb.Data(&cbuffer);
	scb.Size(sizeof(cbuffer));

	cbuffer.width = width;
	cbuffer.height = height;

	std::vector<IDynamicTexture*> dyn_textures;
	std::vector<IStaticTexture*> stat_textures;

	return Engine::Initialize(hInstance, window_title, window_class, width, height, recorder, L"sin_circle", &scb, dyn_textures, stat_textures);
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

void SinCircle::Update() {
//	bool snd_updated = snd_stream->Update(time);

	const int bass_samples_cnt = 32;

//	if (snd_stream->Finished()) return;

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

//	if (snd_updated) {
//		float fft_res[fft_res_size];
//		analyser.CalcFFT_log(snd_stream->CurData(), sound_step, snd_stream->File()->Channels(), fft_res, fft_res_size);
//	}

	Engine::Update();
}
