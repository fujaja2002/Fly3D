#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Template/Noncopyable.h"
#include "Runtime/Windows/WindowDefinition.h"

#include <Windows.h>

#include <vector>
#include <memory>

class FWindowsWindow;

typedef std::shared_ptr<FWindowsWindow> WindowPtr;

struct FDeferredWindowsMessage
{
	FDeferredWindowsMessage(const WindowPtr& inNativeWindow, HWND inHWnd, uint32 inMessage, WPARAM inWParam, LPARAM inLParam, int32 inX = 0, int32 inY = 0, uint32 inRawInputFlags = 0)
		: nativeWindow(inNativeWindow)
		, hWND(inHWnd)
		, message(inMessage)
		, wParam(inWParam)
		, lParam(inLParam)
		, x(inX)
		, y(inY)
		, rawInputFlags(inRawInputFlags)
	{ 

	}

	WindowPtr		nativeWindow;

	HWND			hWND;

	uint32			message;

	WPARAM			wParam;
	LPARAM			lParam;

	int32			x;
	int32			y;
	uint32			rawInputFlags;
};

class FWindowsApplication : public Noncopyable
{
public:

	static void CreateApplication(const HINSTANCE instanceHandle, const HICON iconHandle);

	static void DestroyApplication();

	static FWindowsApplication* GetApplication();

public:

	virtual ~FWindowsApplication();

	WindowPtr MakeWindow(const std::shared_ptr<FWindowDefinition>& definition, const WindowPtr& parent, const bool showImmediately);

	void PumpMessages(const float deltaTime);

protected:

	static LRESULT CALLBACK AppWndProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);

	static bool RegisterWindowClass(const HINSTANCE hInstance, const HICON hIcon);

	int64 ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);

	WindowPtr FindWindowByHWND(HWND handle);

	bool IsKeyboardInputMessage(uint32 msg);

	bool IsMouseInputMessage(uint32 msg);

	bool IsInputMessage(uint32 msg);

private:

	FWindowsApplication(const HINSTANCE instanceHandle, const HICON iconHandle);

private:

	struct EModifierKey
	{
		enum Type
		{
			LeftShift,		// VK_LSHIFT
			RightShift,		// VK_RSHIFT
			LeftControl,	// VK_LCONTROL
			RightControl,	// VK_RCONTROL
			LeftAlt,		// VK_LMENU
			RightAlt,		// VK_RMENU
			CapsLock,		// VK_CAPITAL
			Count,
		};
	};

private:

	HINSTANCE								m_InstanceHandle;
	HICON									m_IconHandle;
	std::vector<WindowPtr>					m_Windows;
	std::vector<FDeferredWindowsMessage>	m_DeferredMessages;
	bool									m_Resizing;

	bool									m_ModifierKeyState[EModifierKey::Count];
};