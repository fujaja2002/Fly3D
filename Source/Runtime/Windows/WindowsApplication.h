#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Template/Noncopyable.h"
#include "Runtime/Windows/WindowDefinition.h"

#include <Windows.h>

#include <vector>
#include <memory>

class FWindowsWindow;

typedef std::shared_ptr<FWindowsWindow> WindowPtr;

class FWindowsApplication : public Noncopyable
{
public:

	static void CreateApplication(const HINSTANCE instanceHandle, const HICON iconHandle);

	static void DestroyApplication();

public:

	virtual ~FWindowsApplication();

	WindowPtr MakeWindow(const std::shared_ptr<FWindowDefinition>& definition, const WindowPtr& parent, const bool showImmediately);

	void PumpMessages(const float deltaTime);

protected:

	static LRESULT CALLBACK AppWndProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);

	int32 ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);

	static bool RegisterWindowClass(const HINSTANCE hInstance, const HICON hIcon);

private:

	FWindowsApplication(const HINSTANCE instanceHandle, const HICON iconHandle);

private:

	HINSTANCE				m_InstanceHandle;
	HICON					m_IconHandle;
	std::vector<WindowPtr>	m_Windows;

};

FWindowsApplication* GetApplication();