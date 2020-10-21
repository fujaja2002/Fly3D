#include "Runtime/Windows/WindowsWindow.h"
#include "Runtime/Windows/WindowsMisc.h"
#include "Runtime/Math/Math.h"

static int32 WindowsAeroBorderSize     = 8;
static int32 WindowsStandardBorderSize = 4;

const WIDECHAR WindowsWindow::AppWindowClass[] = L"Fly3DWindow";

std::shared_ptr<WindowsWindow> WindowsWindow::MakeWindow()
{
	return std::shared_ptr<WindowsWindow>(new WindowsWindow());
}

WindowsWindow::~WindowsWindow()
{

}

HWND WindowsWindow::GetHWnd() const
{
	return m_HWnd;
}

void WindowsWindow::Initialize(const std::shared_ptr<WindowDefinition>& inDefinition, HINSTANCE inHInstance, const std::shared_ptr<WindowsWindow>& inParent, const bool showImmediately)
{
	m_Definition   = inDefinition;
	m_RegionWidth  = -1;
	m_RegionHeight = -1;

	const float xInitialRect  = inDefinition->xDesiredPositionOnScreen;
	const float yInitialRect  = inDefinition->yDesiredPositionOnScreen;
	const float widthInitial  = inDefinition->widthDesiredOnScreen;
	const float heightInitial = inDefinition->heightDesiredOnScreen;

	m_DPIScaleFactor = WindowsMisc::GetDPIScaleFactorAtPoint(xInitialRect, yInitialRect);

	int32 clientX = Math::TruncToInt(xInitialRect);
	int32 clientY = Math::TruncToInt(yInitialRect);
	int32 clientWidth  = Math::TruncToInt(widthInitial);
	int32 clientHeight = Math::TruncToInt(heightInitial);
	int32 windowX = clientX;
	int32 windowY = clientY;
	int32 windowWidth  = clientWidth;
	int32 windowHeight = clientHeight;

	bool supportsPerPixelBlending = true;

	uint32 windowExStyle = 0;
	uint32 windowStyle   = 0;

	if (!inDefinition->hasOSWindowBorder)
	{
		windowExStyle = WS_EX_WINDOWEDGE;
		if (inDefinition->transparencySupport == WindowTransparency::PerWindow)
		{
			windowExStyle |= WS_EX_LAYERED;
		}
		else if (inDefinition->transparencySupport == WindowTransparency::PerPixel)
		{
			if (supportsPerPixelBlending )
			{
				windowExStyle |= WS_EX_COMPOSITED;
			}
		}

		windowStyle = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		if (inDefinition->appearsInTaskbar)
		{
			windowExStyle |= WS_EX_APPWINDOW;
		}
		else
		{
			windowExStyle |= WS_EX_TOOLWINDOW;
		}

		if (inDefinition->isTopmostWindow)
		{
			windowExStyle |= WS_EX_TOPMOST;
		}

		if (!inDefinition->acceptsInput)
		{
			windowExStyle |= WS_EX_TRANSPARENT;
		}
	}
	else
	{
		windowExStyle = WS_EX_APPWINDOW;
		windowStyle   = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;

		if (IsRegularWindow())
		{
			if (inDefinition->supportsMaximize)
			{
				windowStyle |= WS_MAXIMIZEBOX;
			}

			if (inDefinition->supportsMinimize)
			{
				windowStyle |= WS_MINIMIZEBOX;
			}

			if (inDefinition->hasSizingFrame)
			{
				windowStyle |= WS_THICKFRAME;
			}
			else
			{
				windowStyle |= WS_BORDER;
			}
		}
		else
		{
			windowStyle |= WS_POPUP | WS_BORDER;
		}

		RECT borderRect = { 0, 0, 0, 0 };
		::AdjustWindowRectEx(&borderRect, windowStyle, false, windowExStyle);

		windowX += borderRect.left;
		windowY += borderRect.top;

		windowWidth  += borderRect.right  - borderRect.left;
		windowHeight += borderRect.bottom - borderRect.top;
	}

	m_HWnd = CreateWindowEx(
		windowExStyle,
		AppWindowClass,
		inDefinition->title.c_str(),
		windowStyle,
		windowX, 
		windowY, 
		windowWidth, 
		windowHeight,
		inParent ? inParent->GetHWnd() : NULL,
		NULL, 
		inHInstance, 
		NULL
	);

	if (m_HWnd == NULL)
	{
		MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);

		const uint32 error = GetLastError();

		DWORD numHandles = 0;
		GetProcessHandleCount(GetCurrentProcess(), &numHandles);

		return;
	}

	ReshapeWindow(clientX, clientY, clientWidth, clientHeight);

	if (inDefinition->transparencySupport == WindowTransparency::PerWindow)
	{
		SetOpacity(inDefinition->opacity);
	}

	if (IsRegularWindow() && !inDefinition->hasOSWindowBorder)
	{
		windowStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
		if (inDefinition->supportsMaximize)
		{
			windowStyle |= WS_MAXIMIZEBOX;
		}

		if (inDefinition->supportsMinimize)
		{
			windowStyle |= WS_MINIMIZEBOX;
		}

		if (inDefinition->hasSizingFrame)
		{
			windowStyle |= WS_THICKFRAME;
		}

		SetWindowLong(m_HWnd, GWL_STYLE, windowStyle);

		uint32 SetWindowPositionFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED;

		if (inDefinition->activationPolicy == WindowActivationPolicy::Never)
		{
			SetWindowPositionFlags |= SWP_NOACTIVATE;
		}

		::SetWindowPos(m_HWnd, nullptr, 0, 0, 0, 0, SetWindowPositionFlags);

		AdjustWindowRegion(clientWidth, clientHeight);
	}
	else if (inDefinition->hasOSWindowBorder)
	{
		if (!inDefinition->hasCloseButton)
		{
			EnableMenuItem(GetSystemMenu(m_HWnd, false), SC_CLOSE, MF_GRAYED);
		}
	}

	if (IsRegularWindow())
	{
		RegisterDragDrop(m_HWnd, this);
	}

	if (showImmediately)
	{
		Show();
	}
}

bool WindowsWindow::IsRegularWindow() const
{
	return m_Definition->isRegularWindow;
}

void WindowsWindow::AdjustWindowRegion(int32 width, int32 height)
{
	m_RegionWidth  = width;
	m_RegionHeight = height;

	HRGN region = MakeWindowRegionObject(true);
	SetWindowRgn(m_HWnd, region, false);
}

void WindowsWindow::OnParentWindowMinimized()
{
	::GetWindowPlacement(m_HWnd, &m_PreParentMinimizedWindowPlacement);
}

void WindowsWindow::OnParentWindowRestored()
{
	::SetWindowPlacement(m_HWnd, &m_PreParentMinimizedWindowPlacement);
}

bool WindowsWindow::IsEnabled()
{
	return !!::IsWindowEnabled(m_HWnd);
}

bool WindowsWindow::IsManualManageDPIChanges() const
{
	return m_HandleManualDPIChanges;
}

void WindowsWindow::SetManualManageDPIChanges(const bool manualDPIChanges)
{
	m_HandleManualDPIChanges = manualDPIChanges;
}

void WindowsWindow::ReshapeWindow(int32 newX, int32 newY, int32 newWidth, int32 newHeight)
{
	m_AspectRatio = (float)newWidth / (float)newHeight;

	WINDOWINFO windowInfo = { };
	windowInfo.cbSize = sizeof(WINDOWINFO);
	::GetWindowInfo(m_HWnd, &windowInfo);

	if (m_Definition->hasOSWindowBorder)
	{
		RECT borderRect = { 0, 0, 0, 0 };
		::AdjustWindowRectEx(&borderRect, windowInfo.dwStyle, false, windowInfo.dwExStyle);

		newX += borderRect.left;
		newY += borderRect.top;

		newWidth  += borderRect.right - borderRect.left;
		newHeight += borderRect.bottom - borderRect.top;
	}

	int32 windowX = newX;
	int32 windowY = newY;

	if (IsMaximized())
	{
		Restore();
	}

	::SetWindowPos(m_HWnd, nullptr, windowX, windowY, newWidth, newHeight, SWP_NOZORDER | SWP_NOACTIVATE | ((m_WindowMode == WindowMode::Fullscreen) ? SWP_NOSENDCHANGING : 0));
}

bool WindowsWindow::GetFullScreenInfo(int32& x, int32& y, int32& width, int32& height) const
{
	bool trueFullscreen = m_WindowMode == WindowMode::Fullscreen;

	HMONITOR monitor = MonitorFromWindow(m_HWnd, trueFullscreen ? MONITOR_DEFAULTTOPRIMARY : MONITOR_DEFAULTTONEAREST);

	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &monitorInfo);

	x = monitorInfo.rcMonitor.left;
	y = monitorInfo.rcMonitor.top;
	width  = monitorInfo.rcMonitor.right  - x;
	height = monitorInfo.rcMonitor.bottom - y;

	return true;
}

void WindowsWindow::MoveWindowTo(int32 x, int32 y)
{
	if (m_Definition->hasOSWindowBorder)
	{
		const LONG windowStyle   = ::GetWindowLong(m_HWnd, GWL_STYLE);
		const LONG windowExStyle = ::GetWindowLong(m_HWnd, GWL_EXSTYLE);

		RECT borderRect = { 0, 0, 0, 0 };
		::AdjustWindowRectEx(&borderRect, windowStyle, false, windowExStyle);

		x += borderRect.left;
		y += borderRect.top;
	}

	::SetWindowPos(m_HWnd, nullptr, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}

void WindowsWindow::BringToFront(bool force)
{
	if (IsRegularWindow())
	{
		if (::IsIconic(m_HWnd))
		{
			::ShowWindow(m_HWnd, SW_RESTORE);
		}
		else
		{
			::SetActiveWindow(m_HWnd);
		}
	}
	else
	{
		HWND hwndInsertAfter = HWND_TOP;
		uint32 flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER;

		if (!force)
		{
			flags |= SWP_NOACTIVATE;
		}

		if (m_Definition->isTopmostWindow)
		{
			hwndInsertAfter = HWND_TOPMOST;
		}

		::SetWindowPos(m_HWnd, hwndInsertAfter, 0, 0, 0, 0, flags);
	}
}

void WindowsWindow::ForceToFront()
{
	::SetForegroundWindow(m_HWnd);
}

void WindowsWindow::Destroy()
{
	::DestroyWindow(m_HWnd);
}

void WindowsWindow::Minimize()
{
	if (!m_IsFirstTimeVisible)
	{
		::ShowWindow(m_HWnd, SW_MINIMIZE);
	}
	else
	{
		m_InitiallyMinimized = true;
		m_InitiallyMaximized = false;
	}
}

void WindowsWindow::Maximize()
{
	if (!m_IsFirstTimeVisible)
	{
		::ShowWindow(m_HWnd, SW_MAXIMIZE);
	}
	else
	{
		m_InitiallyMaximized = true;
		m_InitiallyMinimized = false;
	}
}

void WindowsWindow::Restore()
{
	if (!m_IsFirstTimeVisible)
	{
		::ShowWindow(m_HWnd, SW_RESTORE);
	}
	else
	{
		m_InitiallyMaximized = false;
		m_InitiallyMinimized = false;
	}
}

void WindowsWindow::Show()
{
	if (!m_IsVisible)
	{
		m_IsVisible = true;

		bool shouldActivate = false;
		if (m_Definition->acceptsInput)
		{
			shouldActivate = m_Definition->activationPolicy == WindowActivationPolicy::Always;
			if (m_IsFirstTimeVisible && m_Definition->activationPolicy == WindowActivationPolicy::FirstShown)
			{
				shouldActivate = true;
			}
		}

		int showWindowCommand = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
		if (m_IsFirstTimeVisible)
		{
			m_IsFirstTimeVisible = false;
			if (m_InitiallyMinimized)
			{
				showWindowCommand = shouldActivate ? SW_MINIMIZE : SW_SHOWMINNOACTIVE;
			}
			else if (m_InitiallyMaximized)
			{
				showWindowCommand = shouldActivate ? SW_SHOWMAXIMIZED : SW_MAXIMIZE;
			}
		}

		::ShowWindow(m_HWnd, showWindowCommand);
	}
}

void WindowsWindow::Hide()
{
	if (m_IsVisible)
	{
		m_IsVisible = false;
		::ShowWindow(m_HWnd, SW_HIDE);
	}
}

bool WindowsWindow::IsMaximized() const
{
	return !!::IsZoomed(m_HWnd);
}

bool WindowsWindow::IsMinimized() const
{
	return !!::IsIconic(m_HWnd);
}

bool WindowsWindow::IsVisible() const
{
	return m_IsVisible;
}

bool WindowsWindow::GetRestoredDimensions(int32& x, int32& y, int32& width, int32& height)
{
	WINDOWPLACEMENT windowPlacement;
	windowPlacement.length = sizeof(WINDOWPLACEMENT);

	if (::GetWindowPlacement(m_HWnd, &windowPlacement) != 0)
	{
		const RECT restored = windowPlacement.rcNormalPosition;

		x = restored.left;
		y = restored.top;
		width  = restored.right  - restored.left;
		height = restored.bottom - restored.top;

		const LONG windowExStyle = ::GetWindowLong(m_HWnd, GWL_EXSTYLE);
		if ((windowExStyle & WS_EX_TOOLWINDOW) == 0)
		{
			const bool trueFullscreen = (m_WindowMode == WindowMode::Fullscreen);
			HMONITOR monitor = MonitorFromWindow(m_HWnd, trueFullscreen ? MONITOR_DEFAULTTOPRIMARY : MONITOR_DEFAULTTONEAREST);
			MONITORINFO monitorInfo;
			monitorInfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(monitor, &monitorInfo);

			x += monitorInfo.rcWork.left - monitorInfo.rcMonitor.left;
			y += monitorInfo.rcWork.top  - monitorInfo.rcMonitor.top;
		}

		return true;
	}
	else
	{
		return false;
	}
}

void WindowsWindow::SetWindowFocus()
{
	if (GetFocus() != m_HWnd)
	{
		::SetFocus(m_HWnd);
	}
}

void WindowsWindow::SetOpacity(const float inOpacity)
{
	SetLayeredWindowAttributes(m_HWnd, 0, Math::TruncToInt(inOpacity * 255.0f), LWA_ALPHA);
}

void WindowsWindow::Enable(bool enable)
{
	::EnableWindow(m_HWnd, enable);
}

bool WindowsWindow::IsPointInWindow(int32 x, int32 y) const
{
	HRGN region = MakeWindowRegionObject(false);
	bool result = !!PtInRegion(region, x, y);

	DeleteObject(region);

	return result;
}

int32 WindowsWindow::GetWindowBorderSize() const
{
	if (!m_Definition->hasOSWindowBorder)
	{
		return 0;
	}

	WINDOWINFO windowInfo = { };
	windowInfo.cbSize = sizeof(WINDOWINFO);
	::GetWindowInfo(m_HWnd, &windowInfo);

	return windowInfo.cxWindowBorders;
}

int32 WindowsWindow::GetWindowTitleBarSize() const
{
	return GetSystemMetrics(SM_CYCAPTION);
}

bool WindowsWindow::IsForegroundWindow() const
{
	return ::GetForegroundWindow() == m_HWnd;
}

void WindowsWindow::SetText(const WIDECHAR* const text)
{
	SetWindowText(m_HWnd, text);
}

void WindowsWindow::SetWindowMode(WindowMode newWindowMode)
{
	if (newWindowMode != m_WindowMode)
	{
		WindowMode previousWindowMode = m_WindowMode;
		m_WindowMode = newWindowMode;

		const bool trueFullscreen = newWindowMode == WindowMode::Fullscreen;
		const LONG fullscreenModeStyle = WS_POPUP;

		LONG windowStyle = GetWindowLong(m_HWnd, GWL_STYLE);
		LONG windowedModeStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;

		if (IsRegularWindow())
		{
			if (m_Definition->supportsMaximize)
			{
				windowedModeStyle |= WS_MAXIMIZEBOX;
			}

			if (m_Definition->supportsMinimize)
			{
				windowedModeStyle |= WS_MINIMIZEBOX;
			}

			if (m_Definition->hasSizingFrame)
			{
				windowedModeStyle |= WS_THICKFRAME;
			}
			else
			{
				windowedModeStyle |= WS_BORDER;
			}
		}
		else
		{
			windowedModeStyle |= WS_POPUP | WS_BORDER;
		}

		if (newWindowMode == WindowMode::WindowedFullscreen || newWindowMode == WindowMode::Fullscreen)
		{
			if (previousWindowMode == WindowMode::Windowed)
			{
				m_PreFullscreenWindowPlacement.length = sizeof(WINDOWPLACEMENT);
				::GetWindowPlacement(m_HWnd, &m_PreFullscreenWindowPlacement);
			}

			windowStyle &= ~windowedModeStyle;
			windowStyle |= fullscreenModeStyle;

			SetWindowLong(m_HWnd, GWL_STYLE, windowStyle);
			::SetWindowPos(m_HWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			if (!trueFullscreen)
			{
				ShowWindow(m_HWnd, SW_RESTORE);
			}

			RECT clientRect;
			GetClientRect(m_HWnd, &clientRect);

			HMONITOR monitor = MonitorFromWindow(m_HWnd, trueFullscreen ? MONITOR_DEFAULTTOPRIMARY : MONITOR_DEFAULTTONEAREST);
			MONITORINFO monitorInfo;
			monitorInfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(monitor, &monitorInfo);

			LONG monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
			LONG targetClientWidth = trueFullscreen ? Math::Min(monitorWidth, clientRect.right - clientRect.left) : monitorWidth;
			LONG monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
			LONG targetClientHeight = trueFullscreen ? Math::Min(monitorHeight, clientRect.bottom - clientRect.top) : monitorHeight;

			ReshapeWindow(
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				targetClientWidth,
				targetClientHeight
			);
		}
		else
		{
			windowStyle &= ~fullscreenModeStyle;
			windowStyle |= windowedModeStyle;
			SetWindowLong(m_HWnd, GWL_STYLE, windowStyle);
			::SetWindowPos(m_HWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			if (m_PreFullscreenWindowPlacement.length)
			{
				::SetWindowPlacement(m_HWnd, &m_PreFullscreenWindowPlacement);
			}
		}
	}
}

HRESULT __stdcall WindowsWindow::DragEnter(__RPC__in_opt IDataObject *dataObjectPointer, ::DWORD keyState, POINTL cursorPosition, __RPC__inout::DWORD *cursorEffect)
{
	printf("DragEnter\n");
	return 0;
}

HRESULT __stdcall WindowsWindow::DragOver(::DWORD keyState, POINTL cursorPosition, __RPC__inout ::DWORD *cursorEffect)
{
	printf("DragOver\n");
	return 0;
}

HRESULT __stdcall WindowsWindow::DragLeave(void)
{
	printf("DragLeave\n");
	return 0;
}

HRESULT __stdcall WindowsWindow::Drop(__RPC__in_opt IDataObject *dataObjectPointer, ::DWORD keyState, POINTL cursorPosition, __RPC__inout::DWORD *cursorEffect)
{
	printf("Drop\n");
	return 0;
}

HRESULT __stdcall WindowsWindow::QueryInterface(REFIID iid, void ** ppvObject)
{
	printf("QueryInterface\n");
	return 0;
}

ULONG __stdcall WindowsWindow::AddRef(void)
{
	printf("AddRef\n");
	return 0;
}

ULONG __stdcall WindowsWindow::Release(void)
{
	printf("Release\n");
	return 0;
}

WindowsWindow::WindowsWindow()
	: m_Definition(nullptr)
	, m_HWnd(NULL)
	, m_RegionWidth(-1)
	, m_RegionHeight(-1)
	, m_WindowMode(WindowMode::Windowed)
	, m_AspectRatio(1.0f)
	, m_DPIScaleFactor(1.0f)
	, m_IsVisible(false)
	, m_IsFirstTimeVisible(false)
	, m_InitiallyMinimized(false)
	, m_InitiallyMaximized(false)
	, m_HandleManualDPIChanges(false)
{
	memset(&m_PreFullscreenWindowPlacement,      0, sizeof(m_PreFullscreenWindowPlacement));
	memset(&m_PreParentMinimizedWindowPlacement, 0, sizeof(m_PreParentMinimizedWindowPlacement));

	m_PreParentMinimizedWindowPlacement.length = sizeof(WINDOWPLACEMENT);
}

void WindowsWindow::UpdateVisibility()
{

}

HRGN WindowsWindow::MakeWindowRegionObject(bool includeBorderWhenMaximized) const
{
	HRGN region;
	if (m_RegionWidth != -1 && m_RegionHeight != -1)
	{
		const bool isBorderlessGameWindow = !m_Definition->hasOSWindowBorder;
		if (IsMaximized())
		{
			if (isBorderlessGameWindow)
			{
				WINDOWINFO windowInfo = { };
				windowInfo.cbSize = sizeof(WINDOWINFO);
				::GetWindowInfo(m_HWnd, &windowInfo);

				const int32 windowBorderSize = includeBorderWhenMaximized ? windowInfo.cxWindowBorders : 0;
				region = CreateRectRgn(windowBorderSize, windowBorderSize, m_RegionWidth + windowBorderSize, m_RegionHeight + windowBorderSize);
			}
			else
			{
				const int32 windowBorderSize = includeBorderWhenMaximized ? GetWindowBorderSize() : 0;
				region = CreateRectRgn(windowBorderSize, windowBorderSize, m_RegionWidth - windowBorderSize, m_RegionHeight - windowBorderSize);
			}
		}
		else
		{
			const bool useCornerRadius = 
				m_WindowMode == WindowMode::Windowed && 
				!isBorderlessGameWindow &&
				m_Definition->transparencySupport != WindowTransparency::PerPixel &&
				m_Definition->cornerRadius > 0;

			if (useCornerRadius)
			{
				region = CreateRoundRectRgn(0, 0, m_RegionWidth + 1, m_RegionHeight + 1, m_Definition->cornerRadius, m_Definition->cornerRadius);
			}
			else
			{
				region = CreateRectRgn(0, 0, m_RegionWidth, m_RegionHeight);
			}
		}
	}
	else
	{
		RECT rcWnd;
		GetWindowRect(m_HWnd, &rcWnd);
		region = CreateRectRgn(0, 0, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top);
	}

	return region;
}