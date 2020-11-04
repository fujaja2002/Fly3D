#include "Runtime/Windows/WindowsWindow.h"
#include "Runtime/Windows/WindowsApplication.h"
#include "Runtime/Windows/WindowsMisc.h"
#include "Runtime/Math/Math.h"

#include "Runtime/Template/Function.h"

#include <string>
#include <stdio.h>
#include <Windows.h>

void SetupDebugConsole()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
}

void FitWindowSize(float widthBias, float heightBias, std::shared_ptr<WindowDefinition>& def)
{
	int32 width  = -1;
	int32 height = -1;
	WindowsMisc::GetDesktopResolution(width, height);

	int32 realWidth  = Math::TruncToInt(width  * widthBias);
	int32 realHeight = Math::TruncToInt(height * heightBias);

	def->xDesiredPositionOnScreen = (width - realWidth) / 2;
	def->yDesiredPositionOnScreen = (height - realHeight) / 2;
	def->widthDesiredOnScreen  = realWidth;
	def->heightDesiredOnScreen = realHeight;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
#if FLY_DEBUG
	SetupDebugConsole();
#endif

	std::shared_ptr<WindowDefinition> def = std::make_shared<WindowDefinition>();
	FitWindowSize(0.8f, 0.8f, def);
	
	WindowsApplication::CreateApplication(hInstance, NULL);
	
	WindowPtr window = GetApplication()->MakeWindow(def, nullptr, true);

	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);

	while (true)
	{
		GetApplication()->PumpMessages(0.16f);
	}

	WindowsApplication::DestroyApplication();

    return 0;
}