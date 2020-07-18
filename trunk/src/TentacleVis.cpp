#include "TentacleVis.h"

static const char *frame_save_path = "imgs\\img_";
//const char *file_path = "./test_shader.wav";
//const char *file_path = "./Summer.wav";
static const char *file_path = "./GoFuckYourself.wav";
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
}

bool TentacleVis::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height) {
	//	recorder = new Recorder(frame_save_path, width, height);
	snd = new WAVSoundFile(file_path);
	snd_stream = new SoundStreamFile(snd, sound_step);

	player.Play(snd);
	prev_time = start_time = std::chrono::steady_clock::now();

	CB_VS_vertexshader &data = gfx.data();
	data.width = width;
	data.height = height;
	data.tent_len = 0;
	gfx.tdata(tent_len_data);

	return Engine::Initialize(hInstance, window_title, window_class, width, height, recorder, use_freqs ? L"tentacle_freqs" : L"tentacle_bass");
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

	const int bass_samples_cnt = 32;

	if (snd_stream->Finished()) return;

	CB_VS_vertexshader &data = gfx.data();

// FOR CAPTURE
//	float dt = 1.f/30.f;
//	time += dt;

	std::chrono::time_point<std::chrono::steady_clock> cur_time = std::chrono::steady_clock::now();
	// FOR REALTIME
	std::chrono::duration<float> cdt = cur_time - prev_time;
	std::chrono::duration<float> diff = cur_time - start_time;
	float dt = cdt.count();
	time = diff.count();

	data.time = time;
	prev_time = cur_time;

	if (snd_updated || !inited) {
		float fft_res[fft_res_size];
		analyser.CalcFFT_log(snd_stream->CurData(), sound_step, snd_stream->File()->Channels(), fft_res, fft_res_size);

		data.bass_coef = 0;
		float tent_len_max = 0.f;
		for (int i = 0; i < fft_res_size; ++i) {
			float new_data = fft_res[i];
//			tent_len_float[i] = new_data;
//			tent_len_float[i] += new_data * dt * 4.f; // += (0.05f+smoothstep(0.4f, 0.9f, new_data)) * dt * 2;
			tent_len_float[i] += (0.05f+smoothstep(0.05f, 0.3f, new_data)) * dt * 5.f;
			tent_len_max = max(tent_len_max, tent_len_float[i]);

			if (i < bass_samples_cnt)
				data.bass_coef += new_data;
		}
		data.bass_coef /= bass_samples_cnt;

//		smooth_array<fft_res_size, 32>(tent_len_float);

//		tent_len += (0.05f+smoothstep(0.4f, 0.9f, data.bass_coef)) * dt * 2;
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

		data.cam_pos = cam_pos;
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
//	data.tent_len = tent_len;
//	data.cam_pos = cam_pos;
//	data.bass_coef = sdata.fft_bass;

//	gfx.tdata(tent_len_data);

	Engine::Update();
}

void TentacleVis::UpdateBass() {
	bool snd_updated = snd_stream->Update(time) && !snd_stream->Finished();

	const int bass_samples_cnt = 32;

//	if (snd_stream->Finished()) return;

	CB_VS_vertexshader &data = gfx.data();

// FOR CAPTURE
//	float dt = 1.f/30.f;
//	time += dt;

	std::chrono::time_point<std::chrono::steady_clock> cur_time = std::chrono::steady_clock::now();
	// FOR REALTIME
	std::chrono::duration<float> cdt = cur_time - prev_time;
	std::chrono::duration<float> diff = cur_time - start_time;
	float dt = cdt.count();
	time = diff.count();

	data.time = time;
	prev_time = cur_time;

// FOR CAPTURE
//	float dt = 1.f/30.f;
//	time += dt;

	float fft_res[fft_res_size];
	if (snd_updated || !inited) {
		analyser.CalcFFT_log(snd_stream->CurData(), sound_step, snd_stream->File()->Channels(), fft_res, fft_res_size);
		inited = false;
	} else {
		for (int i = 0; i < fft_res_size; ++i) {
			fft_res[i] = 0;
		}
	}
	{
		data.bass_coef = 0;
		float tent_len_max = 0.f;
		for (int i = 0; i < fft_res_size; ++i) {
			float new_data = fft_res[i];
//			tent_len_float[i] = new_data;
//			tent_len_float[i] += new_data * dt * 4.f; // += (0.05f+smoothstep(0.4f, 0.9f, new_data)) * dt * 2;
			tent_len_float[i] += (0.05f+smoothstep(0.05f, 0.3f, new_data)) * dt * 5.f;
			tent_len_max = max(tent_len_max, tent_len_float[i]);

			if (i < bass_samples_cnt)
				data.bass_coef += new_data;
		}
		data.bass_coef /= bass_samples_cnt;

		tent_len += (0.05+smoothstep(0.4f, 0.9f, data.bass_coef)) * dt * 2;
		data.tent_len = tent_len;
		const float min_tent_len = 0.5f;
		if (cam_pos + min_tent_len < tent_len) {
			cam_pos += (tent_len - min_tent_len - cam_pos)* dt * 4.f;
		}
		data.cam_pos = cam_pos;
	}
}
