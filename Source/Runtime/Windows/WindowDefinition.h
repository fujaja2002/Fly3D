#pragma once

#include "Runtime/Platform/Platform.h"

#include <string>

enum class EWindowMode
{
	Fullscreen,
	WindowedFullscreen,
	Windowed,
	NumWindowModes
};

enum class EWindowType
{
	/** general-purpose window */
	Normal,
	/** game window */
	GameWindow,
	/** popup menu */
	Menu,
	/** tooltip */
	ToolTip,
	/** toast */
	Notification,
	/** cursor decorator */
	CursorDecorator
};

enum class EWindowTransparency
{
	/** not support transparency */
	None,

	/** supports transparency (one opacity applies to the entire window) */
	PerWindow,

	/** supports per-pixel alpha blended transparency */
	PerPixel
};

enum class EWindowActivationPolicy
{
	/** never activates when it is shown */
	Never,

	/** always activates when it is shown */
	Always,

	/** only activates when it is first shown */
	FirstShown
};

struct FWindowSizeLimits
{

public:

	FWindowSizeLimits& SetMinWidth(int32 width)
	{
		m_MinWidth = width;
		return *this;
	}

	int32 GetMinWidth() const 
	{ 
		return m_MinWidth;
	}

	FWindowSizeLimits& SetMinHeight(int32 height)
	{ 
		m_MinHeight = height; 
		return *this; 
	}

	int32 GetMinHeight() const 
	{ 
		return m_MinHeight; 
	}

	FWindowSizeLimits& SetMaxWidth(int32 width)
	{ 
		m_MaxWidth = width; 
		return *this; 
	}

	int32 GetMaxWidth() const 
	{ 
		return m_MaxWidth; 
	}

	FWindowSizeLimits& SetMaxHeight(int32 height)
	{ 
		m_MaxHeight = height; 
		return *this; 
	}

	const int32 GetMaxHeight() const 
	{ 
		return m_MaxHeight; 
	}

private:
	int32 m_MinWidth;
	int32 m_MinHeight;
	int32 m_MaxWidth;
	int32 m_MaxHeight;
};

struct FWindowDefinition
{
	EWindowType				type = EWindowType::Normal;
	EWindowTransparency		transparencySupport = EWindowTransparency::None;
	EWindowActivationPolicy	activationPolicy = EWindowActivationPolicy::Always;

	/** desired horizontal screen position */
	int32					xDesiredPositionOnScreen = 0;
	/** desired vertical screen position */
	int32					yDesiredPositionOnScreen = 0;

	/** desired width */
	int32					widthDesiredOnScreen = 1024;
	/** desired height */
	int32					heightDesiredOnScreen = 768;

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