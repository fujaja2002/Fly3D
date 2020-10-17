#pragma once

#include "Runtime/Platform/Platform.h"

template<typename T>
FORCE_INLINE bool IsPowerOfTwo(T value)
{
	return (value & (value - 1)) == 0;
}
