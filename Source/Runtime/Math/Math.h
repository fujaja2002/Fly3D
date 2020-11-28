#pragma once

#include "Runtime/Platform/Platform.h"

#include <intrin0.h>

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

	static int32 FloorToInt(float F)
	{
		return TruncToInt(floorf(F));
	}

	static int32 RoundToInt(float F)
	{
		return FloorToInt(F + 0.5f);
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

	static uint32 CountLeadingZeros(uint32 value)
	{
		unsigned long log2;

		if (_BitScanReverse(&log2, value) != 0)
		{
			return 31 - log2;
		}

		return 32;
	}

	static uint32 CountTrailingZeros(uint32 value)
	{
		if (value == 0)
		{
			return 32;
		}

		unsigned long bitIndex;
		_BitScanForward(&bitIndex, value);

		return bitIndex;
	}

	static uint32 CeilLogTwo(uint32 arg)
	{
		int32 bitmask = ((int32)(CountLeadingZeros(arg) << 26)) >> 31;
		return (32 - CountLeadingZeros(arg - 1)) & (~bitmask);
	}

	static uint32 RoundUpToPowerOfTwo(uint32 arg)
	{
		return 1 << CeilLogTwo(arg);
	}

};