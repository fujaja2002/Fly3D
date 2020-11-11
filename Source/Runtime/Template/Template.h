#pragma once

#include "Runtime/Platform/Platform.h"

#include "Runtime/Template/RemoveReference.h"
#include "Runtime/Template/IsArithmetic.h"
#include "Runtime/Template/IsPointer.h"
#include "Runtime/Template/AndOrNot.h"

#include <memory>

template <typename T>
struct TUseBitwiseSwap
{
	enum 
	{ 
		Value = !TOrValue<__is_enum(T), TIsPointer<T>, TIsArithmetic<T>>::Value 
	};
};

FORCE_INLINE bool XOR(bool a, bool b)
{
	return a != b;
}

template <typename T>
FORCE_INLINE typename TRemoveReference<T>::Type&& MoveTemp(T&& obj)
{
	typedef typename TRemoveReference<T>::Type CastType;
	return (CastType&&)obj;
}

template <typename T>
FORCE_INLINE typename TRemoveReference<T>::Type&& MoveTempIfPossible(T&& obj)
{
	typedef typename TRemoveReference<T>::Type CastType;
	return (CastType&&)obj;
}

template <typename T>
FORCE_INLINE T CopyTemp(T& val)
{
	return const_cast<const T&>(val);
}

template <typename T>
FORCE_INLINE T CopyTemp(const T& val)
{
	return val;
}

template <typename T>
FORCE_INLINE T&& CopyTemp(T&& val)
{
	return MoveTemp(val);
}

template <typename T>
FORCE_INLINE T&& Forward(typename TRemoveReference<T>::Type& obj)
{
	return (T&&)obj;
}

template <typename T>
FORCE_INLINE T&& Forward(typename TRemoveReference<T>::Type&& obj)
{
	return (T&&)obj;
}

template <typename T>
T&& DeclVal();

template <typename T>
FORCE_INLINE typename TEnableIf<TUseBitwiseSwap<T>::Value>::Type Swap(T& a, T& b)
{
	if (&a != &b)
	{
		TTypeCompatibleBytes<T> temp;
		memcpy(&temp, &a,     sizeof(T));
		memcpy(&a,    &b,     sizeof(T));
		memcpy(&b,    &temp,  sizeof(T));
	}
}

template <typename T>
FORCE_INLINE typename TEnableIf<!TUseBitwiseSwap<T>::Value>::Type Swap(T& a, T& b)
{
	T temp = MoveTemp(a);
	a = MoveTemp(b);
	b = MoveTemp(temp);
}

template <typename T>
FORCE_INLINE void Exchange(T& a, T& b)
{
	Swap(a, b);
}

template <typename T>
struct TIdentity
{
	typedef T Type;
};

template <typename T>
FORCE_INLINE T ImplicitConv(typename TIdentity<T>::Type obj)
{
	return obj;
}