#include "Engine.h"
#include "Recorder.h"
#include "WAVSound.h"
#include "SoundPlayer.h"
#include "SoundAnalyser.h"

const char *frame_save_path = "imgs\\img_";
const char *file_path = "./test_shader.wav";
//const char *file_path = "./Summer.wav";

Engine::Engine() {
	recorder = 0;
	snd = 0;
	player = 0;
	analyser = 0;

	tent_len = 0.f;
	cam_pos = 0.f;
}

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int w, int h)
{
//	recorder = new Recorder(frame_save_path, width, height);
	snd = new WAVSound(file_path);
	player = new SoundPlayer();
	analyser = new SoundAnalyser(snd);
	width = w;
	height = h;

	player->Play(snd);
	prev_time = start_time = std::chrono::steady_clock::now();

	if (!this->render_window.Initialize(hInstance, window_title, window_class, width, height))
		return false;

	if (!gfx.Initialize(this->render_window.GetHWND(), width, height, recorder))
		return false;

	return true;
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::Update()
{
	snd->Update(time);
	analyser->Update();

	SoundData sdata;
	analyser->GetData(sdata);

// FOR CAPTURE
//	float dt = 1.f/30.f;
//	time += dt;

	gfx.tdata(sdata.fft_data);

	CB_VS_vertexshader &data = gfx.data();

	data.width = width;
	data.height = height;
	std::chrono::time_point<std::chrono::steady_clock> cur_time = std::chrono::steady_clock::now();
	// FOR REALTIME
	std::chrono::duration<float> cdt = cur_time - prev_time;
	std::chrono::duration<float> diff = cur_time - start_time;
	data.time = diff.count();
	float dt = cdt.count();
	time = diff.count();

	data.time = time;
	prev_time = cur_time;
	tent_len += (0.05+smoothstep(0.4f, 0.9f, sdata.fft_bass)) * dt * 2;
	data.tent_len = tent_len;
	const float min_tent_len = 0.5f;
	if (cam_pos + min_tent_len < tent_len) {
		cam_pos += (tent_len - min_tent_len - cam_pos)* dt * 4.f;
	}
	data.cam_pos = cam_pos;
	data.bass_coef = sdata.fft_bass;
}

void Engine::RenderFrame()
{
	this->gfx.RenderFrame();
}

