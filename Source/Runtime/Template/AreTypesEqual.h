﻿#pragma once

template<typename A, typename B>
struct TAreTypesEqual;

template<typename, typename>
struct TAreTypesEqual
{
	enum 
	{ 
		Value = false 
	};
};

template<typename A>
struct TAreTypesEqual<A, A>
{
	enum 
	{ 
		Value = true 
	};
};

#define ARE_TYPES_EQUAL(A, B) TAreTypesEqual<A, B>::Value

template<typename A, typename B>	
struct TIsSame			
{ 
	enum 
	{ 
		Value = false	
	}; 
};

template<typename T>				
struct TIsSame<T, T>	
{ 
	enum 
	{ 
		Value = true	
	};
};