#pragma once

#include "Runtime/Template/AndOrNot.h"
#include "Runtime/Template/IsPODType.h"

template <typename T>
struct TIsTriviallyCopyConstructible
{
	enum 
	{ 
		Value = TOrValue<__has_trivial_copy(T), TIsPODType<T>>::Value 
	};
};
