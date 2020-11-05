#pragma once

#include "Runtime/Template/IsPointer.h"
#include "Runtime/Template/AndOrNot.h"

template <typename T>
struct TIsPODType 
{ 
	enum 
	{ 
		Value = TOrValue<__is_pod(T) || __is_enum(T), TIsArithmetic<T>, TIsPointer<T>>::Value 
	};
};
