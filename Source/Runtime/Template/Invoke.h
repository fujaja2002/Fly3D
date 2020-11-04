#pragma once

#include "Runtime/Platform/Platform.h"

#include "Runtime/Template/Decay.h"
#include "Runtime/Template/Template.h"
#include "Runtime/Template/PointerIsConvertibleFromTo.h"

namespace Fly3DPrivateInvoke
{
	template <typename BaseType, typename CallableType>
	FORCE_INLINE auto DereferenceIfNecessary(CallableType&& callable) 
		-> typename TEnableIf<TPointerIsConvertibleFromTo<typename TDecay<CallableType>::Type, typename TDecay<BaseType>::Type>::Value, decltype((CallableType&&)callable)>::Type
	{
		return (CallableType&&)callable;
	}

	template <typename BaseType, typename CallableType>
	FORCE_INLINE auto DereferenceIfNecessary(CallableType&& callable) 
		-> typename TEnableIf<!TPointerIsConvertibleFromTo<typename TDecay<CallableType>::Type, typename TDecay<BaseType>::Type>::Value, decltype(*(CallableType&&)callable)>::Type
	{
		return *(CallableType&&)callable;
	}
}

template <typename FuncType, typename... ArgTypes>
FORCE_INLINE auto Invoke(FuncType&& func, ArgTypes&&... args) 
	-> decltype(Forward<FuncType>(func)(Forward<ArgTypes>(args)...))
{
	return Forward<FuncType>(func)(Forward<ArgTypes>(args)...);
}

template <typename ReturnType, typename ObjType, typename CallableType>
FORCE_INLINE auto Invoke(ReturnType ObjType::*pdm, CallableType&& callable) 
	-> decltype(Fly3DPrivateInvoke::DereferenceIfNecessary<ObjType>(Forward<CallableType>(callable)).*pdm)
{
	return Fly3DPrivateInvoke::DereferenceIfNecessary<ObjType>(Forward<CallableType>(callable)).*pdm;
}

template <typename ReturnType, typename ObjType, typename... PMFArgTypes, typename CallableType, typename... ArgTypes>
FORCE_INLINE auto Invoke(ReturnType (ObjType::*ptrMemFun)(PMFArgTypes...), CallableType&& callable, ArgTypes&&... args) 
	-> decltype((Fly3DPrivateInvoke::DereferenceIfNecessary<ObjType>(Forward<CallableType>(callable)).*ptrMemFun)(Forward<ArgTypes>(args)...))
{
	return (Fly3DPrivateInvoke::DereferenceIfNecessary<ObjType>(Forward<CallableType>(callable)).*ptrMemFun)(Forward<ArgTypes>(args)...);
}

template <typename ReturnType, typename ObjType, typename... PMFArgTypes, typename CallableType, typename... ArgTypes>
FORCE_INLINE auto Invoke(ReturnType (ObjType::*ptrMemFun)(PMFArgTypes...) const, CallableType&& callable, ArgTypes&&... args) 
	-> decltype((Fly3DPrivateInvoke::DereferenceIfNecessary<ObjType>(Forward<CallableType>(callable)).*ptrMemFun)(Forward<ArgTypes>(args)...))
{
	return (Fly3DPrivateInvoke::DereferenceIfNecessary<ObjType>(Forward<CallableType>(callable)).*ptrMemFun)(Forward<ArgTypes>(args)...);
}

#define PROJECTION(FuncName) \
	[](auto&&... args) \
	{ \
		return FuncName(Forward<decltype(args)>(args)...); \
	}

#define PROJECTION_MEMBER(Type, FuncName) \
	[](auto&& obj, auto&&... args) \
	{ \
		return Fly3DPrivateInvoke::DereferenceIfNecessary<Type>(Forward<decltype(obj)>(obj)).FuncName(Forward<decltype(args)>(args)...); \
	}