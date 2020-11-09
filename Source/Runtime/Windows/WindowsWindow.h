#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Template/Noncopyable.h"
#include "Runtime/Windows/WindowDefinition.h"

#include <Windows.h>
#include <memory>

class FWindowsApplication;

typedef std::shared_ptr<FWindowDefinition> WindowDefinitionPtr;

class FWindowsWindow : public Noncopyable, public IDropTarget
{
	friend class FWindowsApplication;

public:

	static const WIDECHAR AppWindowClass[];

	static std::shared_ptr<FWindowsWindow> MakeWindow();

public:

	virtual ~FWindowsWindow();

	HWND GetHWnd() const;

	void Initialize(const std::shared_ptr<FWindowDefinition>& inDefinition, HINSTANCE inHInstance, const std::shared_ptr<FWindowsWindow>& inParent, const bool showImmediately);

	bool IsRegularWindow() const;

	void AdjustWindowRegion(int32 Width, int32 Height);

	void OnParentWindowMinimized();

	void OnParentWindowRestored();

	bool IsEnabled();

	virtual bool IsManualManageDPIChanges() const;

	virtual void SetManualManageDPIChanges(const bool manualDPIChanges);

	virtual void ReshapeWindow(int32 x, int32 y, int32 width, int32 height);

	virtual bool GetFullScreenInfo(int32& x, int32& y, int32& width, int32& height) const;

	virtual void MoveWindowTo(int32 x, int32 y);

	virtual void BringToFront(bool force = false);

	virtual void ForceToFront();

	virtual void Destroy();

	virtual void Minimize();

	virtual void Maximize();

	virtual void Restore();

	virtual void Show();

	virtual void Hide();

	virtual bool IsMaximized() const;

	virtual bool IsMinimized() const;

	virtual bool IsVisible() const;

	virtual bool GetRestoredDimensions(int32& x, int32& y, int32& width, int32& height);

	virtual void SetWindowFocus();

	virtual void SetOpacity(const float inOpacity);

	virtual void Enable(bool enable);

	virtual bool IsPointInWindow(int32 x, int32 y) const;

	virtual int32 GetWindowBorderSize() const;

	virtual int32 GetWindowTitleBarSize() const;

	virtual bool IsForegroundWindow() const;

	virtual void SetText(const WIDECHAR* const Text);

	virtual void SetWindowMode(EWindowMode newWindowMode);

	virtual EWindowMode GetWindowMode() const 
	{ 
		return m_WindowMode; 
	} 

	virtual float GetAspectRatio() const 
	{ 
		return m_AspectRatio; 
	}

	virtual float GetDPIScaleFactor() const
	{
		return m_DPIScaleFactor;
	}

	virtual void SetDPIScaleFactor(float Value)
	{
		m_DPIScaleFactor = Value;
	}

	virtual const WindowDefinitionPtr& GetDefinition() const
	{
		return m_Definition;
	}

protected:

	virtual void OnWindowMode(int32 x, int32 y)
	{
		m_RegionX = x;
		m_RegionY = y;
	}

public:

	virtual HRESULT __stdcall QueryInterface(REFIID iid, void ** ppvObject) override;

	virtual ULONG __stdcall AddRef(void) override;

	virtual ULONG __stdcall Release(void) override;

	virtual HRESULT __stdcall DragEnter(__RPC__in_opt IDataObject *dataObjectPointer, ::DWORD keyState, POINTL cursorPosition, __RPC__inout ::DWORD *cursorEffect);

	virtual HRESULT __stdcall DragOver(::DWORD keyState, POINTL cursorPosition, __RPC__inout ::DWORD *cursorEffect);

	virtual HRESULT __stdcall DragLeave(void);

	virtual HRESULT __stdcall Drop(__RPC__in_opt IDataObject *dataObjectPointer, ::DWORD keyState, POINTL cursorPosition, __RPC__inout ::DWORD *cursorEffect);

private:

	FWindowsWindow();

	void UpdateVisibility();

	HRGN MakeWindowRegionObject(bool includeBorderWhenMaximized) const;

private:

	WindowDefinitionPtr m_Definition;

	HWND				m_HWnd;

	int32				m_RegionX;
	int32				m_RegionY;
	int32				m_RegionWidth;
	int32				m_RegionHeight;

	EWindowMode			m_WindowMode;

	WINDOWPLACEMENT		m_PreFullscreenWindowPlacement;
	WINDOWPLACEMENT		m_PreParentMinimizedWindowPlacement;

	float				m_AspectRatio;
	float				m_DPIScaleFactor;

	bool				m_IsVisible;
	bool				m_IsFirstTimeVisible;

	bool				m_InitiallyMinimized;
	bool				m_InitiallyMaximized;

	bool				m_HandleManualDPIChanges;
};