#pragma once

#include "Runtime/Template/RemoveCV.h"
#include "Runtime/Template/RemoveReference.h"

namespace Fly3DPrivateDecay
{
	template <typename T>
	struct TDecayNonReference
	{
		typedef typename TRemoveCV<T>::Type Type;
	};

	template <typename T>
	struct TDecayNonReference<T[]>
	{
		typedef T* Type;
	};

	template <typename T, uint32 N>
	struct TDecayNonReference<T[N]>
	{
		typedef T* Type;
	};

	template <typename RetType, typename... Params>
	struct TDecayNonReference<RetType(Params...)>
	{
		typedef RetType (*Type)(Params...);
	};
}

template <typename T>
struct TDecay
{
	typedef typename Fly3DPrivateDecay::TDecayNonReference<typename TRemoveReference<T>::Type>::Type Type;
};
