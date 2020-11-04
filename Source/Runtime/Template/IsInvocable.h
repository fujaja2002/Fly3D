#pragma once

#include "Runtime/Template/Invoke.h"

namespace Fly3DPrivateIsInvocable
{
	template <typename T>
	T&& DeclVal();

	template <typename T>
	struct TVoid
	{
		typedef void Type;
	};

	template <typename, typename CallableType, typename... ArgTypes>
	struct TIsInvocableImpl
	{
		enum 
		{ 
			Value = false 
		};
	};

	template <typename CallableType, typename... ArgTypes>
	struct TIsInvocableImpl<typename TVoid<decltype(Invoke(DeclVal<CallableType>(), DeclVal<ArgTypes>()...))>::Type, CallableType, ArgTypes...>
	{
		enum { Value = true };
	};
}

template <typename CallableType, typename... ArgTypes>
struct TIsInvocable : Fly3DPrivateIsInvocable::TIsInvocableImpl<void, CallableType, ArgTypes...>
{

};

