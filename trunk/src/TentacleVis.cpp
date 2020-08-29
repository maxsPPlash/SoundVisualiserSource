#include "TentacleVis.h"

static const char *frame_save_path = "imgs\\img_";
//const char *file_path = "./test_shader.wav";
//const char *file_path = "./Summer.wav";
//static const char *file_path = "./GoFuckYourself.wav";
static const char *file_path = "./techno_test.wav";
//const char *file_path = "./BrianJames.wav";

constexpr int sound_step = 2048;

constexpr bool use_freqs = false;

TentacleVis::TentacleVis() {
	recorder = 0;
	snd = 0;
	snd_stream = 0;
	inited = false;

	tent_len = 0.f;
	cam_pos = 0.f;

	for (int i = 0; i < fft_res_size; ++i) {
		tent_len_data[i] = 0;
		tent_len_float[i] = 0.f;
	}
}

TentacleVis::~TentacleVis() {
	delete snd;
	delete snd_stream;

	delete audo_data_tex;
	delete image;
}

bool TentacleVis::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height) {
	//	recorder = new Recorder(frame_save_path, width, height);
	snd = new WAVSoundFile(file_path);
	snd_stream = new SoundStreamFile(snd, sound_step);

	player.Play(snd);
	prev_time = start_time = std::chrono::steady_clock::now();

	scb.Data(&cbuffer);
	scb.Size(sizeof(cbuffer));

	audo_data_tex = new ShaderDynTexture(tent_len_data, 1, 512, DT_U16R, 0);
	std::vector<IDynamicTexture*> dyn_textures;
	dyn_textures.push_back(audo_data_tex);

	image = new ShaderStaticTexture(L"Data\\Textures\\piano.png", 1);
	std::vector<IStaticTexture*> stat_textures;
	stat_textures.push_back(image);

	cbuffer.width = width;
	cbuffer.height = height;
	cbuffer.tent_len = 0;

	return Engine::Initialize(hInstance, window_title, window_class, width, height, recorder, use_freqs ? L"tentacle_freqs" : L"tentacle_bass", &scb, dyn_textures, stat_textures);
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

void TentacleVis::Update() {
	if (use_freqs)
		UpdateFreqs();
	else
		UpdateBass();
}


void TentacleVis::UpdateFreqs()
{
	bool snd_updated = snd_stream->Update(time);

	const int bass_samples_cnt = 8;

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
			float new_data = fft_res[i];
//			tent_len_float[i] = new_data;
//			tent_len_float[i] += new_data * dt * 4.f; // += (0.05f+smoothstep(0.4f, 0.9f, new_data)) * dt * 2;
			tent_len_float[i] += (0.05f+smoothstep(0.05f, 0.3f, new_data)) * dt * 5.f;
			tent_len_max = max(tent_len_max, tent_len_float[i]);

			if (i < bass_samples_cnt)
				cbuffer.bass_coef += new_data;
		}
		cbuffer.bass_coef /= bass_samples_cnt;

//		smooth_array<fft_res_size, 32>(tent_len_float);

//		tent_len += (0.05f+smoothstep(0.4f, 0.9f, cbuffer.bass_coef)) * dt * 2;
//		tent_len_float[0] = tent_len;
//		tent_len_max = tent_len;

		const float min_tent_len = 0.3f;
		float cam_move = 0.f;
		if (min_tent_len < tent_len_max) {
			cam_move = (tent_len_max - min_tent_len) * dt * 4.f;
			cam_pos += cam_move;
			tent_len -= cam_move;
		}
//		cam_move = 1000;
		for (int i = 0; i < fft_res_size; ++i) {
			if (min_tent_len < tent_len_float[i]) {
				tent_len_float[i] = clamp(tent_len_float[i] - min(cam_move, (tent_len_float[i] - min_tent_len) * dt * 4.f), 0.f, 1.f);
			}
			tent_len_data[i] = snd_float2short(tent_len_float[i]);
		}
		smooth_array<unsigned short, fft_res_size, 32>(tent_len_data);

		cbuffer.cam_pos = cam_pos;
		inited = true;
	}

//	for (int i = 0; i < 512; ++i)
//		tent_len_data[i] += (0.05+smoothstep(0.4f, 0.9f, sdata.fft_bass)) * dt * 2;
//	tent_len += (0.05+smoothstep(0.4f, 0.9f, sdata.fft_bass)) * dt * 2;
//	const float min_tent_len = 0.5f;
//	if (min_tent_len < tent_len) {
//		float cam_move = (tent_len - min_tent_len)* dt * 4.f;
//		cam_pos += cam_move;
//		for (int i = 0; i < 512; ++i)
//			tent_len_data[i] -= cam_move;
////		tent_len -= cam_move;
//	}
//	cbuffer.tent_len = tent_len;
//	cbuffer.cam_pos = cam_pos;
//	cbuffer.bass_coef = sdata.fft_bass;

//	gfx.tdata(tent_len_data);

	Engine::Update();
}

void TentacleVis::UpdateBass() {
	bool snd_updated = snd_stream->Update(time) && !snd_stream->Finished();

	const int bass_samples_cnt = 8;

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

// FOR CAPTURE
//	float dt = 1.f/30.f;
//	time += dt;

	float fft_res[fft_res_size];
	if (snd_updated || !inited) {
		analyser.CalcFFT_log(snd_stream->CurData(), sound_step, snd_stream->File()->Channels(), fft_res, fft_res_size);
		inited = false;
	} else {
//		for (int i = 0; i < fft_res_size; ++i) {
//			fft_res[i] = 0;
//		}
	}
	{
		cbuffer.bass_coef = 0;
		float tent_len_max = 0.f;
		for (int i = 0; i < fft_res_size; ++i) {
			float new_data = fft_res[i];
//			tent_len_float[i] = new_data;
//			tent_len_float[i] += new_data * dt * 4.f; // += (0.05f+smoothstep(0.4f, 0.9f, new_data)) * dt * 2;
			tent_len_float[i] += (0.05f+smoothstep(0.05f, 0.3f, new_data)) * dt * 5.f;
			tent_len_max = max(tent_len_max, tent_len_float[i]);

			if (i < bass_samples_cnt)
				cbuffer.bass_coef += new_data;
		}
		cbuffer.bass_coef /= bass_samples_cnt*1.5;

		tent_len += (0.05+smoothstep(0.4f, 0.9f, cbuffer.bass_coef)) * dt * 2;
		cbuffer.tent_len = tent_len;
		const float min_tent_len = 0.5f;
		if (cam_pos + min_tent_len < tent_len) {
			cam_pos += (tent_len - min_tent_len - cam_pos)* dt * 4.f;
		}
		cbuffer.cam_pos = cam_pos;
	}
}
