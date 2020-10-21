#include "Runtime/Windows/WindowsMisc.h"
#include "Runtime/Windows/WindowsApplication.h"

#include <Windows.h>
#include <tchar.h>

void WindowsMisc::GetDesktopResolution(int32& outWidth, int32& outHeight)
{
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	outWidth  = desktop.right;
	outHeight = desktop.bottom;
}

void WindowsMisc::RequestMinimize()
{
	::ShowWindow(::GetActiveWindow(), SW_MINIMIZE);
}

bool WindowsMisc::IsThisApplicationForeground()
{
	uint32 foregroundProcess;
	::GetWindowThreadProcessId(GetForegroundWindow(), (::DWORD*)&foregroundProcess);
	return (foregroundProcess == GetCurrentProcessId());
}

int32 WindowsMisc::GetAppIcon()
{
	return 0;
}

void WindowsMisc::PreventScreenSaver()
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

LinearColor WindowsMisc::GetScreenPixelColor(const Vector2D& inScreenPos, float inGamma)
{
	COLORREF pixelColorRef = GetPixel(GetDC(HWND_DESKTOP), inScreenPos.x, inScreenPos.y);

	uint8 r = (pixelColorRef & 0xFF);
	uint8 g = (pixelColorRef & 0xFF00) >> 8;
	uint8 b = (pixelColorRef & 0xFF0000) >> 16;
	uint8 a = 255;

	return LinearColor(Color32(r, g, b, a));
}

bool WindowsMisc::GetWindowTitleMatchingText(const WIDECHAR* titleStartsWith, std::wstring& outTitle)
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

float WindowsMisc::GetDPIScaleFactorAtPoint(float x, float y)
{
	HDC context = GetDC(nullptr);
	int32 dpi   = GetDeviceCaps(context, LOGPIXELSX);
	float scale = (float)dpi / 96.0f;

	ReleaseDC(nullptr, context);

	return scale;
}

void WindowsMisc::ClipboardCopy(const WIDECHAR* str)
{
	if (OpenClipboard(GetActiveWindow()) )
	{
		int32 strLen = _tcslen(str);
		HGLOBAL globalMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(WIDECHAR) * (strLen + 1));
		WIDECHAR* data = (WIDECHAR*)GlobalLock(globalMem);
		_tcscpy(data, str);
		GlobalUnlock(globalMem);
	}
}

void WindowsMisc::ClipboardPaste(std::wstring& dest)
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