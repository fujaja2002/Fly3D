#pragma once

#include "Runtime/Platform/Platform.h"

#include <string>

enum class WindowMode
{
	Fullscreen,
	WindowedFullscreen,
	Windowed,
	NumWindowModes
};

enum class WindowType
{
	/** general-purpose window */
	Normal,
	/** popup menu */
	Menu,
	/** tooltip */
	ToolTip,
	/** toast */
	Notification,
	/** cursor decorator */
	CursorDecorator
};

enum class WindowTransparency
{
	/** not support transparency */
	None,

	/** supports transparency (one opacity applies to the entire window) */
	PerWindow,

	/** supports per-pixel alpha blended transparency */
	PerPixel
};

enum class WindowActivationPolicy
{
	/** never activates when it is shown */
	Never,

	/** always activates when it is shown */
	Always,

	/** only activates when it is first shown */
	FirstShown
};

struct WindowSizeLimits
{

public:

	WindowSizeLimits& SetMinWidth(float width)
	{
		m_MinWidth = width;
		return *this;
	}

	float GetMinWidth() const 
	{ 
		return m_MinWidth;
	}

	WindowSizeLimits& SetMinHeight(float height)
	{ 
		m_MinHeight = height; 
		return *this; 
	}

	float GetMinHeight() const 
	{ 
		return m_MinHeight; 
	}

	WindowSizeLimits& SetMaxWidth(float width)
	{ 
		m_MaxWidth = width; 
		return *this; 
	}

	float GetMaxWidth() const 
	{ 
		return m_MaxWidth; 
	}

	WindowSizeLimits& SetMaxHeight(float height)
	{ 
		m_MaxHeight = height; 
		return *this; 
	}

	const float GetMaxHeight() const 
	{ 
		return m_MaxHeight; 
	}

private:
	float m_MinWidth;
	float m_MinHeight;
	float m_MaxWidth;
	float m_MaxHeight;
};

struct WindowDefinition
{
	WindowType				type = WindowType::Normal;
	WindowTransparency		transparencySupport = WindowTransparency::None;
	WindowActivationPolicy	activationPolicy = WindowActivationPolicy::Always;

	/** desired horizontal screen position */
	float					xDesiredPositionOnScreen = 0.0f;
	/** desired vertical screen position */
	float					yDesiredPositionOnScreen = 0.0f;

	/** desired width */
	float					widthDesiredOnScreen = 1024.0f;
	/** desired height */
	float					heightDesiredOnScreen = 768.0f;

	/** expected maximum width */
	int32					expectedMaxWidth = 4096;
	/** expected maximum height */
	int32					expectedMaxHeight = 4096;

	/** has os window border */
	bool					hasOSWindowBorder = true;
	/** show up in the taskbar */
	bool					appearsInTaskbar = true;
	/** on top of all other windows; */
	bool					isTopmostWindow = true;
	/** accepts input */
	bool					acceptsInput = true;
	
	/** will be focused when it is first shown */
	bool					focusWhenFirstShown = true;
	/** displays an close button */
	bool					hasCloseButton = true;
	/** displays an minimize button */
	bool					supportsMinimize = true;
	/** displays an maximize button */
	bool					supportsMaximize = true;

	/** modal window */
	bool					isModalWindow = false;
	/** vanilla window */
	bool					isRegularWindow = true;
	/** thick edge */
	bool					hasSizingFrame = true;

	/** window should preserve its aspect ratio when resized */
	bool					shouldPreserveAspectRatio = true;
	
	/** the title of the window */
	std::wstring			title = L"Wiindow";
	/** opacity of the window (0-1) */
	float					opacity = 1.0f;
	/** radius of the corner rounding */
	int32					cornerRadius = 10;

	/** false if the window should respond to system DPI changes */
	bool					manualDPI = false;
};