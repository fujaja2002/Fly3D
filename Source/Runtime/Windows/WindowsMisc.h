#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Math/Vector2D.h"
#include "Runtime/Math/Color.h"

#include <string>

struct FWindowsMisc
{
	static void RequestMinimize();
	static bool IsThisApplicationForeground();
	static int32 GetAppIcon();
	static void PreventScreenSaver();
	static FLinearColor GetScreenPixelColor(const FVector2D& inScreenPos, float inGamma = 1.0f);
	static bool GetWindowTitleMatchingText(const WIDECHAR* titleStartsWith, std::wstring& outTitle);
	static float GetDPIScaleFactorAtPoint(int32 x, int32 y);
	static void ClipboardCopy(const WIDECHAR* str);
	static void ClipboardPaste(std::wstring& dest);
	static void GetDesktopResolution(int32& outWidth, int32& outHeight);
};