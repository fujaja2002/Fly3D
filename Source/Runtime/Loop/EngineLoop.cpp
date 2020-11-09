#include "Runtime/Loop/EngineLoop.h"
#include "Runtime/Windows/WindowsWindow.h"
#include "Runtime/Windows/WindowsApplication.h"
#include "Runtime/Windows/WindowsMisc.h"
#include "Runtime/Math/Math.h"
#include "Runtime/Core/Globals.h"

static void FitWindowSize(float widthBias, float heightBias, std::shared_ptr<FWindowDefinition>& def)
{
	int32 width  = -1;
	int32 height = -1;
	FWindowsMisc::GetDesktopResolution(width, height);

	int32 realWidth  = FMath::TruncToInt(width  * widthBias);
	int32 realHeight = FMath::TruncToInt(height * heightBias);

	def->xDesiredPositionOnScreen = (width - realWidth) / 2;
	def->yDesiredPositionOnScreen = (height - realHeight) / 2;
	def->widthDesiredOnScreen  = realWidth;
	def->heightDesiredOnScreen = realHeight;
}

FEngineLoop::FEngineLoop()
{

}

FEngineLoop::~FEngineLoop()
{

}

int32 FEngineLoop::PreInit(int32 argc, WIDECHAR* argv)
{
	std::shared_ptr<FWindowDefinition> def = std::make_shared<FWindowDefinition>();
	FitWindowSize(0.8f, 0.8f, def);

	FWindowsApplication::CreateApplication(Globals::HInstance, NULL);
	WindowPtr window = FWindowsApplication::GetApplication()->MakeWindow(def, nullptr, true);

	return 0;
}

int32 FEngineLoop::Init()
{

	return 0;
}

int32 FEngineLoop::PostInit()
{

	return 0;
}

void FEngineLoop::Exit()
{

}

void FEngineLoop::Tick()
{
	FWindowsApplication::GetApplication()->PumpMessages(0.016f);
}

void FEngineLoop::PreInitRHI()
{

}

void FEngineLoop::InitRHI()
{

}

void FEngineLoop::PostInitRHI()
{

}

void FEngineLoop::PreInitApp()
{

}

void FEngineLoop::InitApp()
{

}

void FEngineLoop::PostInitApp()
{

}

void FEngineLoop::PreExitApp()
{
	
}

void FEngineLoop::ExitApp()
{
	FWindowsApplication::DestroyApplication();
}