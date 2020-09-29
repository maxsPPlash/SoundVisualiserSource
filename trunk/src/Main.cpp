#include "TentacleVis.h"
#include "RaymarchingVis.h"
#include "VideoVis.h"
#include "TileVis.h"
#include "SinCircle.h"
#include "SimpleSound.h"
#include "Composite.h"
#include "Projector.h"
#include "RayEye.h"
#include "AlexanderPlatz.h"
#include "LostSouls.h"

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

	LostSouls engine;
	if (engine.Initialize(hInstance, "Title", "MyWindowClass", 640, 360)) //3840, 2160 //426, 240 // 640, 360 //800, 450 // 1280, 720 // 512, 512
	{
		while (engine.ProcessMessages() == true)
		{
			engine.Update();
			engine.RenderFrame();
		}
	}
	return 0;
}