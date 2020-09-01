#include <windows.h>
#include "MainWnd.h"
using namespace System;
using namespace System::Windows;

[STAThread]
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmd, int nCmd)
{
	MainWnd^ win = gcnew MainWnd();
	win->Title = "Hello World";

	Application^ app = gcnew Application();
	app->Run(win);
}