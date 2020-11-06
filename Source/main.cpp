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

void FitWindowSize(float widthBias, float heightBias, std::shared_ptr<FWindowDefinition>& def)
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
#if FLY_DEBUG
	SetupDebugConsole();
#endif

	TFunction<float(int32)> f0 = [](int32 num) -> float {
		return 1.0f;
	};

	TFunctionRef<int32(const float abc)> f1 = [](const float abc) -> int32 {
		return 2;
	};

	TUniqueFunction<void()> f2 = []() -> void {
		printf("TUniqueFunction\n");
	};

	printf("value=%f\n", f0(1));
	printf("value=%d\n", f1(1));
	f2();

	struct MyStruct
	{
		float x;
		float y;

		MyStruct()
		{
			printf("MyStruct()");
		}

		~MyStruct()
		{
			printf("~MyStruct()");
		}
	};

	MyStruct* mm = new(EAllocatorType::kMemTypeRegular, alignof(MyStruct), __FILE__, __LINE__) MyStruct;

	mm->~MyStruct();

	char* a = new(EAllocatorType::kMemTypeRegular, alignof(char[128]), __FILE__, __LINE__) char[128];


	std::shared_ptr<FWindowDefinition> def = std::make_shared<FWindowDefinition>();
	FitWindowSize(0.8f, 0.8f, def);
	
	FWindowsApplication::CreateApplication(hInstance, NULL);
	
	WindowPtr window = GetApplication()->MakeWindow(def, nullptr, true);

	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);

	while (true)
	{
		GetApplication()->PumpMessages(0.16f);
	}

	FWindowsApplication::DestroyApplication();

    return 0;
}