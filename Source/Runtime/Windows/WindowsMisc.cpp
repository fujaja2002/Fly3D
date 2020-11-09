#include "Runtime/Windows/WindowsMisc.h"
#include "Runtime/Windows/WindowsApplication.h"

#include <Windows.h>
#include <tchar.h>

void FWindowsMisc::GetDesktopResolution(int32& outWidth, int32& outHeight)
{
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	outWidth  = desktop.right;
	outHeight = desktop.bottom;
}

void FWindowsMisc::RequestMinimize()
{
	::ShowWindow(::GetActiveWindow(), SW_MINIMIZE);
}

bool FWindowsMisc::IsThisApplicationForeground()
{
	uint32 foregroundProcess;
	::GetWindowThreadProcessId(GetForegroundWindow(), (::DWORD*)&foregroundProcess);
	return (foregroundProcess == GetCurrentProcessId());
}

int32 FWindowsMisc::GetAppIcon()
{
	return 0;
}

void FWindowsMisc::PreventScreenSaver()
{
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dx = 0;
	input.mi.dy = 0;	
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	input.mi.time = 0;
	input.mi.dwExtraInfo = 0;

	SendInput(1, &input, sizeof(INPUT));
}

FLinearColor FWindowsMisc::GetScreenPixelColor(const FVector2D& inScreenPos, float inGamma)
{
	COLORREF pixelColorRef = GetPixel(GetDC(HWND_DESKTOP), (int32)(inScreenPos.x), (int32)(inScreenPos.y));

	uint8 r = (uint8)(pixelColorRef & 0xFF);
	uint8 g = (uint8)((pixelColorRef & 0xFF00) >> 8);
	uint8 b = (uint8)((pixelColorRef & 0xFF0000) >> 16);
	uint8 a = 255;

	return FLinearColor(FColor32(r, g, b, a));
}

bool FWindowsMisc::GetWindowTitleMatchingText(const WIDECHAR* titleStartsWith, std::wstring& outTitle)
{
	WIDECHAR buffer[8192];

	HWND hwnd = FindWindowW(NULL, NULL);
	bool wasFound = false;

	if (hwnd != NULL)
	{
		size_t titleStartsWithLen = _tcslen(titleStartsWith);

		do
		{
			GetWindowTextW(hwnd, buffer, 8192);

			if (_tcsnccmp(titleStartsWith, buffer, titleStartsWithLen) == 0)
			{
				outTitle = buffer;
				hwnd     = NULL;
				wasFound = true;
			}
			else
			{
				hwnd = GetWindow(hwnd, GW_HWNDNEXT);
			}
		}
		while (hwnd != NULL);
	}

	return wasFound;
}

float FWindowsMisc::GetDPIScaleFactorAtPoint(int32 x, int32 y)
{
	HDC context = GetDC(nullptr);
	int32 dpi   = GetDeviceCaps(context, LOGPIXELSX);
	float scale = (float)dpi / 96.0f;

	ReleaseDC(nullptr, context);

	return scale;
}

void FWindowsMisc::ClipboardCopy(const WIDECHAR* str)
{
	if (OpenClipboard(GetActiveWindow()) )
	{
		size_t strLen = _tcslen(str);
		HGLOBAL globalMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(WIDECHAR) * (strLen + 1));
		WIDECHAR* data = (WIDECHAR*)GlobalLock(globalMem);
		_tcscpy(data, str);
		GlobalUnlock(globalMem);
	}
}

void FWindowsMisc::ClipboardPaste(std::wstring& dest)
{
	if (OpenClipboard(GetActiveWindow()))
	{
		HGLOBAL globalMem = GetClipboardData(CF_UNICODETEXT);
		bool unicode = true;
		if (!globalMem)
		{
			globalMem = GetClipboardData(CF_TEXT);
			unicode   = false;
		}

		if (!globalMem)
		{
			dest = L"";
		}
		else
		{
			void* data = GlobalLock(globalMem);
			if (unicode)
			{
				dest = (WIDECHAR*)data;
			}
			else
			{
				ANSICHAR* ach = (ANSICHAR*)data;
				dest.resize(strlen(ach));
				for (int32 i = 0; i < dest.size(); ++i)
				{
					dest[i] = ach[i];
				}
			}
			GlobalUnlock(globalMem);
		}
	}
	else 
	{
		dest = L"";
	}
}