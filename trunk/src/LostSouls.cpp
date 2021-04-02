#include "LostSouls.h"

static const char *frame_save_path = "imgs\\img_";
//const char *file_path = "./test_shader.wav";
//const char *file_path = "./Summer.wav";
//static const char *file_path = "./GoFuckYourself.wav";
//static const char *file_path = "./techno_test.wav";
static const char *file_path = "./Spork_Lost_Souls1.wav";
//const char *file_path = "./BrianJames.wav";

constexpr int sound_step = 2048;

LostSouls::LostSouls() {
	recorder = 0;
	snd = 0;
	snd_stream = 0;
	inited = false;

	tent_len = 0.f;
	cam_pos = 0.f;

	nohats_cnt = 0;

	for (int i = 0; i < fft_res_size; ++i) {
		fft_data[i] = 0;
		fft_prev[i] - 0;
	}
}

LostSouls::~LostSouls() {
	delete snd;
	delete snd_stream;

	delete audo_data_tex;
	delete image;
}

bool LostSouls::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height) {
	recorder = new Recorder(frame_save_path, width, height);
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

	return Engine::Initialize(hInstance, window_title, window_class, width, height, recorder, L"lost_souls", &scb, dyn_textures, stat_textures);
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

void LostSouls::Update() {
	bool snd_updated = snd_stream->Update(time);

	const int bass_samples_cnt = 23;

	if (snd_stream->Finished()) return;

//	std::chrono::time_point<std::chrono::steady_clock> cur_time = std::chrono::steady_clock::now();
//	// FOR REALTIME
//	std::chrono::duration<float> cdt = cur_time - prev_time;
//	std::chrono::duration<float> diff = cur_time - start_time;
//	float dt = cdt.count();
//	time = diff.count();
//	prev_time = cur_time;

	cbuffer.time = time;

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

		if (fft_res[23] > 0.55/* && fft_res[6] < 5.5*/)
		{
			if (time - cbuffer.click_time > 1.0) {
				cbuffer.click_time = time;
				cbuffer.click_id++;
			}
		}

		if (fft_res[40] > 0.6/* && fft_res[6] < 5.5*/)
		{
			if (time - cbuffer.horn_time > 7.0) {
				cbuffer.horn_time = time;
				cbuffer.horn_id++;
			}
		}

		if (fft_res[301] > 0.15/* && fft_res[6] < 5.5*/)
		{
			if (cbuffer.hats_id < 1 && nohats_cnt > 10) {
				cbuffer.hats_time = time;
				cbuffer.hats_id++;
			}
			nohats_cnt = 0;
		} else {
			nohats_cnt++;
		}

		if (fft_res[300] > 0.05/* && fft_res[6] < 5.5*/)
		{
			if (nohatsin_cnt > 5) {
				cbuffer.hatsin_time = time;
			}
			nohatsin_cnt = 0;
		} else {
			nohatsin_cnt++;
		}

		float new300 = fft_res[301];
		if (new300 >= cbuffer.power300 && new300 > 0.45) {
			cbuffer.power300 = new300;
			last_300 = time;
		}
		else if (new300 < cbuffer.power300 && time - last_300 > 4) {
			cbuffer.power300 -= 0.02;
		}

		inited = true;
	}

// FOR CAPTURE
	float dt = 1.f/60.f;
	time += dt;

	Engine::Update();
}