#include "Runtime/Windows/WindowsApplication.h"
#include "Runtime/Windows/WindowsWindow.h"
#include "Runtime/Platform/Platform.h"

static WindowsApplication* g_Application = nullptr;

WindowsApplication* GetApplication()
{
	Assert(g_Application);
	return g_Application;
}

LRESULT CALLBACK WindowsApplication::AppWndProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool WindowsApplication::RegisterWindowClass(const HINSTANCE hInstance, const HICON hIcon)
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
	wc.lpszClassName = WindowsWindow::AppWindowClass;

	if (!::RegisterClass(&wc))
	{
		MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	return true;
}

void WindowsApplication::CreateApplication(const HINSTANCE instanceHandle, const HICON iconHandle)
{
	if (g_Application)
	{
		return;
	}

	WindowsApplication::RegisterWindowClass(instanceHandle, iconHandle);

	g_Application = new WindowsApplication(instanceHandle, iconHandle);
}

void WindowsApplication::DestroyApplication()
{
	if (!g_Application)
	{
		return;
	}

	delete g_Application;

	g_Application = nullptr;
}

WindowPtr WindowsApplication::MakeWindow(const std::shared_ptr<WindowDefinition>& definition, const WindowPtr& parent, const bool showImmediately)
{
	WindowPtr window = WindowsWindow::MakeWindow();
	window->Initialize(definition, m_InstanceHandle, parent, showImmediately);

	m_Windows.push_back(window);

	return window;
}

WindowsApplication::WindowsApplication(const HINSTANCE instanceHandle, const HICON iconHandle)
	: m_InstanceHandle(instanceHandle)
	, m_IconHandle(iconHandle)
{
	
}

WindowsApplication::~WindowsApplication()
{
	for (int32 i = 0; i < m_Windows.size(); ++i)
	{
		m_Windows[i]->Destroy();
	}

	m_Windows.clear();
}

void WindowsApplication::PumpMessages(const float deltaTime)
{
	MSG message;

	while(PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
	{ 
		TranslateMessage(&message);
		DispatchMessage(&message); 
	}
}

int32 WindowsApplication::ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
{
	
	return 0;
}
