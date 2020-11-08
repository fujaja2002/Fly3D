#pragma once

#include "Runtime/Template/AndOrNot.h"

namespace Fly3DPrivateIsEnumClass
{
	template <typename T>
	struct TIsEnumConvertibleToInt
	{
		static char (&Resolve(int32))[2];

		static char Resolve(...);

		enum 
		{ 
			Value = sizeof(Resolve(T())) - 1 
		};
	};
}

template <typename T>
struct TIsEnumClass
{ 
	enum 
	{ 
		Value = TAndValue<__is_enum(T), TNot<Fly3DPrivateIsEnumClass::TIsEnumConvertibleToInt<T>>>::Value 
	};
};