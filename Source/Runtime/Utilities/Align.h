#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Math/Math.h"

template<typename T, typename V>
inline T AlignUp(T value, V alignment)
{
	Assert(IsPowerOfTwo(alignment));
	return (value + (alignment - 1)) & ~(alignment - 1);
}

template<typename T, typename V>
inline T AlignDown(T value, V alignment)
{
	Assert(IsPowerOfTwo(alignment));
	return value - (value & (alignment - 1));
}