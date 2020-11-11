#include "Runtime/Loop/EngineLoop.h"
#include "Runtime/Windows/WindowsApplication.h"
#include "Runtime/Core/Globals.h"

#include "Runtime/Template/SharedPointer.h"

#include <string>
#include <stdio.h>
#include <Windows.h>

FEngineLoop GEngineLoop;

void SetupDebugConsole()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
}

int32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int32 iCmdShow)
{
#if FLY_DEBUG
	SetupDebugConsole();
#endif

	Globals::HInstance = hInstance;

	struct MyStruct
	{
		float x;
		float y;

		MyStruct(float xx, float yy)
		{
			LOGE("init\n");
		}

		~MyStruct()
		{
			LOGE("delete\n");
		}
	};

	{
		TSharedPtr<MyStruct> m = MakeShared<MyStruct>(1.0f, 2.0f);

		TSharedPtr<MyStruct> a(new MyStruct(1.2f, 2.0f));
	}

	struct EngineLoopCleanup
	{
		~EngineLoopCleanup()
		{
			Globals::IsRequestingExit = true;
			GEngineLoop.PreExitApp();
			GEngineLoop.Exit();
		}
	} cleanupEngine;
	
	int32 errorLevel = GEngineLoop.PreInit(iCmdShow, ::GetCommandLineW());
	if (errorLevel != 0 || Globals::IsRequestingExit)
	{
		return errorLevel;
	}
	
	errorLevel = GEngineLoop.Init();
	errorLevel = GEngineLoop.PostInit();

	GEngineLoop.PreInitRHI();
	GEngineLoop.InitRHI();
	GEngineLoop.PostInitRHI();

	GEngineLoop.PreInitApp();
	GEngineLoop.InitApp();
	GEngineLoop.PostInitApp();

	while (!Globals::IsRequestingExit)
	{
		GEngineLoop.Tick();
	}

	return errorLevel;
}