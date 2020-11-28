#include "Runtime/Loop/EngineLoop.h"
#include "Runtime/Windows/WindowsApplication.h"
#include "Runtime/Core/Globals.h"
#include "Runtime/Template/SharedPointer.h"
#include "Runtime/Core/Containers/Array.h"

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

		MyStruct()
			: x(0)
			, y(0)
		{

		}

		MyStruct(float inX, float inY)
			: x(inX)
			, y(inY)
		{

		}
	};

	TArray<MyStruct> arr;
	arr.AddUninitialized(10);
	arr.Emplace(1.0f, 2.0f);

	for (int32 i = 0; i < arr.Num(); ++i)
	{
		LOGI("(x=%f,y=%f)\n", arr[i].x, arr[i].y);
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