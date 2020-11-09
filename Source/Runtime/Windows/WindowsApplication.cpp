#include "Runtime/Windows/WindowsApplication.h"
#include "Runtime/Windows/WindowsWindow.h"
#include "Runtime/Platform/Platform.h"
#include "Runtime/Math/Math.h"
#include "Runtime/Core/Globals.h"

static FWindowsApplication* g_Application = nullptr;

FWindowsApplication* GetApplication()
{
	Assert(g_Application);
	return g_Application;
}

LRESULT CALLBACK FWindowsApplication::AppWndProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
{
	return GetApplication()->ProcessMessage(hwnd, msg, wParam, lParam);
}

bool FWindowsApplication::RegisterWindowClass(const HINSTANCE hInstance, const HICON hIcon)
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = AppWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = FWindowsWindow::AppWindowClass;

	if (!::RegisterClass(&wc))
	{
		MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	return true;
}

void FWindowsApplication::CreateApplication(const HINSTANCE instanceHandle, const HICON iconHandle)
{
	if (g_Application)
	{
		return;
	}

	FWindowsApplication::RegisterWindowClass(instanceHandle, iconHandle);

	g_Application = new FWindowsApplication(instanceHandle, iconHandle);
}

void FWindowsApplication::DestroyApplication()
{
	if (!g_Application)
	{
		return;
	}

	delete g_Application;

	g_Application = nullptr;
}

FWindowsApplication* FWindowsApplication::GetApplication()
{
	Assert(g_Application);
	return g_Application;
}

WindowPtr FWindowsApplication::MakeWindow(const std::shared_ptr<FWindowDefinition>& definition, const WindowPtr& parent, const bool showImmediately)
{
	WindowPtr window = FWindowsWindow::MakeWindow();
	window->Initialize(definition, m_InstanceHandle, parent, showImmediately);

	m_Windows.push_back(window);

	return window;
}

FWindowsApplication::FWindowsApplication(const HINSTANCE instanceHandle, const HICON iconHandle)
	: m_InstanceHandle(instanceHandle)
	, m_IconHandle(iconHandle)
	, m_Windows()
	, m_DeferredMessages()
	, m_Resizing(false)
{
	
}

FWindowsApplication::~FWindowsApplication()
{
	for (int32 i = 0; i < m_Windows.size(); ++i)
	{
		m_Windows[i]->Destroy();
	}

	m_Windows.clear();
}

void FWindowsApplication::PumpMessages(const float deltaTime)
{
	MSG message;

	while(PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
	{ 
		TranslateMessage(&message);
		DispatchMessage(&message); 
	}
}

WindowPtr FWindowsApplication::FindWindowByHWND(HWND handle)
{
	for (int32 i = 0; i < m_Windows.size(); ++i)
	{
		if (m_Windows[i]->GetHWnd() == handle)
		{
			return m_Windows[i];
		}
	}

	return std::shared_ptr<FWindowsWindow>(nullptr);
}

bool FWindowsApplication::IsKeyboardInputMessage(uint32 msg)
{
	switch(msg)
	{
		case WM_CHAR:
		case WM_SYSCHAR:
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYUP:
		case WM_SYSCOMMAND:
		return true;
	}

	return false;
}

bool FWindowsApplication::IsMouseInputMessage(uint32 msg)
{
	switch(msg)
	{
		case WM_MOUSEHWHEEL:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHOVER:
		case WM_MOUSELEAVE:
		case WM_MOUSEMOVE:
		case WM_NCMOUSEHOVER:
		case WM_NCMOUSELEAVE:
		case WM_NCMOUSEMOVE:
		case WM_NCMBUTTONDBLCLK:
		case WM_NCMBUTTONDOWN:
		case WM_NCMBUTTONUP:
		case WM_NCRBUTTONDBLCLK:
		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONUP:
		case WM_NCXBUTTONDBLCLK:
		case WM_NCXBUTTONDOWN:
		case WM_NCXBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_XBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
			return true;
	}

	return false;
}

bool FWindowsApplication::IsInputMessage(uint32 msg)
{
	if (IsKeyboardInputMessage(msg) || IsMouseInputMessage(msg))
	{
		return true;
	}

	switch(msg)
	{
		case WM_INPUT:
		case WM_INPUT_DEVICE_CHANGE:
			return true;
	}

	return false;
}

int64 FWindowsApplication::ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
{
	WindowPtr currentNativeWindow = FindWindowByHWND(hwnd);
	if (currentNativeWindow == nullptr)
	{
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	switch (msg)
	{
		case WM_ERASEBKGND:
		{
			return 1;
		}
		case WM_PAINT:
		{
			return 0;
		}
		case WM_ENTERSIZEMOVE:
		{
			break;
		}
		case WM_EXITSIZEMOVE:
		{
			break;
		}
		case WM_SIZE:
		{
			break;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
