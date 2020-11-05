#pragma once

#include "Runtime/Template/AreTypesEqual.h"

template <typename From, typename To> 
struct TCopyQualifiersFromTo                          
{ 
	typedef To Type; 
};

template <typename From, typename To> 
struct TCopyQualifiersFromTo<const From, To> 
{ 
	typedef const To Type; 
};

template <typename From, typename To> 
struct TCopyQualifiersFromTo<volatile From, To> 
{ 
	typedef volatile To Type; 
};

template <typename From, typename To> 
struct TCopyQualifiersFromTo<const volatile From, To> 
{ 
	typedef const volatile To Type;
};

template <typename From, typename To>
struct TLosesQualifiersFromTo
{
	enum 
	{ 
		Value = !TAreTypesEqual<typename TCopyQualifiersFromTo<From, To>::Type, To>::Value 
	};
};