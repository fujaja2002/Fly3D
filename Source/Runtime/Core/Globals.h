#pragma once

#include "Runtime/Platform/Platform.h"

#include <Windows.h>

struct Globals
{
	static bool IsRequestingExit;

	// window
	static HINSTANCE HInstance;
	
	static bool Minimized;
};