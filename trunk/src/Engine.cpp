#include "Engine.h"

Engine::Engine() {

}

Engine::~Engine() {

}

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int w, int h, IRecorder *recorder, const std::wstring &shader_name, ShaderConstBuffer *cbuf, const std::vector<IDynamicTexture*> &dyn_texs, const std::vector<IStaticTexture*> &stat_texs)
{

	width = w;
	height = h;

	if (!this->render_window.Initialize(hInstance, window_title, window_class, width, height))
		return false;

	if (!gfx.Initialize(this->render_window.GetHWND(), width, height, recorder, shader_name, cbuf, dyn_texs, stat_texs))
		return false;

	return true;
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::Update()
{
}

void Engine::RenderFrame()
{
	this->gfx.RenderFrame();
}

