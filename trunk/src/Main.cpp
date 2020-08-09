#include "TentacleVis.h"
#include "RaymarchingVis.h"
#include "VideoVis.h"
#include "TileVis.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		Log(hr, "Failed to call CoInitialize.");
		return -1;
	}

	TileVis engine;
	if (engine.Initialize(hInstance, "Title", "MyWindowClass", 800, 450)) // , 1280, 720 // 512, 512
	{
		while (engine.ProcessMessages() == true)
		{
			engine.Update();
			engine.RenderFrame();
		}
	}
	return 0;
}