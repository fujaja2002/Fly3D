﻿#pragma once

#include "Runtime/Platform/Platform.h"

template<typename T>
FORCE_INLINE bool IsPowerOfTwo(T value)
{
	return (value & (value - 1)) == 0;
}

struct FMath
{
	static constexpr int32 TruncToInt(float F)
	{
		return (int32)F;
	}

	static constexpr float TruncToFloat(float F)
	{
		return (float)TruncToInt(F);
	}

	template<class T> 
	static constexpr T Min(const T a, const T b)
	{
		return (a <= b) ? a : b;
	}

	template<class T> 
	static constexpr T Max(const T a, const T b)
	{
		return (a >= b) ? a : b;
	}
};