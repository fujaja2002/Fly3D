#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Template/Template.h"

template <typename T = void>
struct TGreater
{
	FORCE_INLINE bool operator()(const T& a, const T& b) const
	{
		return b < a;
	}
};

template <>
struct TGreater<void>
{
	template <typename T>
	FORCE_INLINE bool operator()(const T& a, const T& b) const
	{
		return b < a;
	}
};