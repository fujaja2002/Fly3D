#include "Runtime/Windows/WindowsWindow.h"
#include "Runtime/Windows/WindowsMisc.h"
#include "Runtime/Math/Math.h"

static int32 WindowsAeroBorderSize     = 8;
static int32 WindowsStandardBorderSize = 4;

const WIDECHAR FWindowsWindow::AppWindowClass[] = L"Fly3DWindow";

std::shared_ptr<FWindowsWindow> FWindowsWindow::MakeWindow()
{
	return std::shared_ptr<FWindowsWindow>(new FWindowsWindow());
}

FWindowsWindow::~FWindowsWindow()
{

}

HWND FWindowsWindow::GetHWnd() const
{
	return m_HWnd;
}

void FWindowsWindow::Initialize(const std::shared_ptr<FWindowDefinition>& inDefinition, HINSTANCE inHInstance, const std::shared_ptr<FWindowsWindow>& inParent, const bool showImmediately)
{
	m_Definition   = inDefinition;
	m_RegionX      = -1;
	m_RegionY      = -1;
	m_RegionWidth  = -1;
	m_RegionHeight = -1;

	const int32 xInitialRect  = inDefinition->xDesiredPositionOnScreen;
	const int32 yInitialRect  = inDefinition->yDesiredPositionOnScreen;
	const int32 widthInitial  = inDefinition->widthDesiredOnScreen;
	const int32 heightInitial = inDefinition->heightDesiredOnScreen;

	m_DPIScaleFactor = FWindowsMisc::GetDPIScaleFactorAtPoint(xInitialRect, yInitialRect);

	int32 clientX = xInitialRect;
	int32 clientY = yInitialRect;
	int32 clientWidth  = widthInitial;
	int32 clientHeight = heightInitial;
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
		if (inDefinition->transparencySupport == EWindowTransparency::PerWindow)
		{
			windowExStyle |= WS_EX_LAYERED;
		}
		else if (inDefinition->transparencySupport == EWindowTransparency::PerPixel)
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

	if (inDefinition->transparencySupport == EWindowTransparency::PerWindow)
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

		if (inDefinition->activationPolicy == EWindowActivationPolicy::Never)
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

bool FWindowsWindow::IsRegularWindow() const
{
	return m_Definition->isRegularWindow;
}

void FWindowsWindow::AdjustWindowRegion(int32 width, int32 height)
{
	m_RegionWidth  = width;
	m_RegionHeight = height;

	HRGN region = MakeWindowRegionObject(true);
	SetWindowRgn(m_HWnd, region, false);
}

void FWindowsWindow::OnParentWindowMinimized()
{
	::GetWindowPlacement(m_HWnd, &m_PreParentMinimizedWindowPlacement);
}

void FWindowsWindow::OnParentWindowRestored()
{
	::SetWindowPlacement(m_HWnd, &m_PreParentMinimizedWindowPlacement);
}

bool FWindowsWindow::IsEnabled()
{
	return !!::IsWindowEnabled(m_HWnd);
}

bool FWindowsWindow::IsManualManageDPIChanges() const
{
	return m_HandleManualDPIChanges;
}

void FWindowsWindow::SetManualManageDPIChanges(const bool manualDPIChanges)
{
	m_HandleManualDPIChanges = manualDPIChanges;
}

void FWindowsWindow::ReshapeWindow(int32 newX, int32 newY, int32 newWidth, int32 newHeight)
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

	::SetWindowPos(m_HWnd, nullptr, windowX, windowY, newWidth, newHeight, SWP_NOZORDER | SWP_NOACTIVATE | ((m_WindowMode == EWindowMode::Fullscreen) ? SWP_NOSENDCHANGING : 0));

	m_RegionX = newX;
	m_RegionY = newY;
	m_RegionWidth  = newWidth;
	m_RegionHeight = newHeight;
}

bool FWindowsWindow::GetFullScreenInfo(int32& x, int32& y, int32& width, int32& height) const
{
	bool fullscreen  = m_WindowMode == EWindowMode::Fullscreen;
	HMONITOR monitor = MonitorFromWindow(m_HWnd, fullscreen ? MONITOR_DEFAULTTOPRIMARY : MONITOR_DEFAULTTONEAREST);

	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &monitorInfo);

	x = monitorInfo.rcMonitor.left;
	y = monitorInfo.rcMonitor.top;
	width  = monitorInfo.rcMonitor.right  - x;
	height = monitorInfo.rcMonitor.bottom - y;

	return true;
}

void FWindowsWindow::MoveWindowTo(int32 x, int32 y)
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

void FWindowsWindow::BringToFront(bool force)
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

void FWindowsWindow::ForceToFront()
{
	::SetForegroundWindow(m_HWnd);
}

void FWindowsWindow::Destroy()
{
	::DestroyWindow(m_HWnd);
}

void FWindowsWindow::Minimize()
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

void FWindowsWindow::Maximize()
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

void FWindowsWindow::Restore()
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

void FWindowsWindow::Show()
{
	if (!m_IsVisible)
	{
		m_IsVisible = true;

		bool shouldActivate = false;
		if (m_Definition->acceptsInput)
		{
			shouldActivate = m_Definition->activationPolicy == EWindowActivationPolicy::Always;
			if (m_IsFirstTimeVisible && m_Definition->activationPolicy == EWindowActivationPolicy::FirstShown)
			{
				shouldActivate = true;
			}
		}

		int32 showWindowCommand = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
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

void FWindowsWindow::Hide()
{
	if (m_IsVisible)
	{
		m_IsVisible = false;
		::ShowWindow(m_HWnd, SW_HIDE);
	}
}

bool FWindowsWindow::IsMaximized() const
{
	return !!::IsZoomed(m_HWnd);
}

bool FWindowsWindow::IsMinimized() const
{
	return !!::IsIconic(m_HWnd);
}

bool FWindowsWindow::IsVisible() const
{
	return m_IsVisible;
}

bool FWindowsWindow::GetRestoredDimensions(int32& x, int32& y, int32& width, int32& height)
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
			const bool trueFullscreen = (m_WindowMode == EWindowMode::Fullscreen);
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

void FWindowsWindow::SetWindowFocus()
{
	if (GetFocus() != m_HWnd)
	{
		::SetFocus(m_HWnd);
	}
}

void FWindowsWindow::SetOpacity(const float inOpacity)
{
	SetLayeredWindowAttributes(m_HWnd, 0, FMath::TruncToInt(inOpacity * 255.0f), LWA_ALPHA);
}

void FWindowsWindow::Enable(bool enable)
{
	::EnableWindow(m_HWnd, enable);
}

bool FWindowsWindow::IsPointInWindow(int32 x, int32 y) const
{
	HRGN region = MakeWindowRegionObject(false);
	bool result = !!PtInRegion(region, x, y);

	DeleteObject(region);

	return result;
}

int32 FWindowsWindow::GetWindowBorderSize() const
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

int32 FWindowsWindow::GetWindowTitleBarSize() const
{
	return GetSystemMetrics(SM_CYCAPTION);
}

bool FWindowsWindow::IsForegroundWindow() const
{
	return ::GetForegroundWindow() == m_HWnd;
}

void FWindowsWindow::SetText(const WIDECHAR* const text)
{
	SetWindowText(m_HWnd, text);
}

void FWindowsWindow::SetWindowMode(EWindowMode newWindowMode)
{
	if (newWindowMode != m_WindowMode)
	{
		EWindowMode previousWindowMode = m_WindowMode;
		m_WindowMode = newWindowMode;

		const bool trueFullscreen = newWindowMode == EWindowMode::Fullscreen;
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

		if (newWindowMode == EWindowMode::WindowedFullscreen || newWindowMode == EWindowMode::Fullscreen)
		{
			if (previousWindowMode == EWindowMode::Windowed)
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
			LONG targetClientWidth = trueFullscreen ? FMath::Min(monitorWidth, clientRect.right - clientRect.left) : monitorWidth;
			LONG monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
			LONG targetClientHeight = trueFullscreen ? FMath::Min(monitorHeight, clientRect.bottom - clientRect.top) : monitorHeight;

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

HRESULT __stdcall FWindowsWindow::DragEnter(__RPC__in_opt IDataObject *dataObjectPointer, ::DWORD keyState, POINTL cursorPosition, __RPC__inout::DWORD *cursorEffect)
{
	printf("DragEnter\n");
	return 0;
}

HRESULT __stdcall FWindowsWindow::DragOver(::DWORD keyState, POINTL cursorPosition, __RPC__inout ::DWORD *cursorEffect)
{
	printf("DragOver\n");
	return 0;
}

HRESULT __stdcall FWindowsWindow::DragLeave(void)
{
	printf("DragLeave\n");
	return 0;
}

HRESULT __stdcall FWindowsWindow::Drop(__RPC__in_opt IDataObject *dataObjectPointer, ::DWORD keyState, POINTL cursorPosition, __RPC__inout::DWORD *cursorEffect)
{
	printf("Drop\n");
	return 0;
}

HRESULT __stdcall FWindowsWindow::QueryInterface(REFIID iid, void ** ppvObject)
{
	printf("QueryInterface\n");
	return 0;
}

ULONG __stdcall FWindowsWindow::AddRef(void)
{
	printf("AddRef\n");
	return 0;
}

ULONG __stdcall FWindowsWindow::Release(void)
{
	printf("Release\n");
	return 0;
}

FWindowsWindow::FWindowsWindow()
	: m_Definition(nullptr)
	, m_HWnd(NULL)
	, m_RegionX(-1)
	, m_RegionY(-1)
	, m_RegionWidth(-1)
	, m_RegionHeight(-1)
	, m_WindowMode(EWindowMode::Windowed)
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

void FWindowsWindow::UpdateVisibility()
{

}

HRGN FWindowsWindow::MakeWindowRegionObject(bool includeBorderWhenMaximized) const
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
				m_WindowMode == EWindowMode::Windowed && 
				!isBorderlessGameWindow &&
				m_Definition->transparencySupport != EWindowTransparency::PerPixel &&
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