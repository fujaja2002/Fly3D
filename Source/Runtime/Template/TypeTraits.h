#pragma once

#include "Runtime/Template/AndOrNot.h"
#include "Runtime/Template/IsEnum.h"
#include "Runtime/Template/IsPointer.h"
#include "Runtime/Template/IsArithmetic.h"

template <typename T>
struct TIsFunction
{
	enum 
	{ 
		Value = false 
	};
};

template <typename RetType, typename... Params>
struct TIsFunction<RetType(Params...)>
{
	enum 
	{ 
		Value = true 
	};
};

template<typename T> 
struct TIsZeroConstructType 
{ 
	enum 
	{ 
		Value = TOr<TIsEnum<T>, TIsArithmetic<T>, TIsPointer<T>>::Value 
	};
};

template<typename T> 
struct TIsWeakPointerType
{ 
	enum 
	{ 
		Value = false 
	};
};