#pragma once

#include "Runtime/Template/AndOrNot.h"
#include "Runtime/Template/IsEnum.h"
#include "Runtime/Template/IsPointer.h"
#include "Runtime/Template/IsArithmetic.h"
#include "Runtime/Template/TypeTraits.h"
#include "Runtime/Template/IsTriviallyCopyConstructible.h"

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

template<typename T> struct TContainerTraitsBase
{
	enum 
	{ 
		MoveWillEmptyContainer = false 
	};
};

template<typename T> struct TContainerTraits : public TContainerTraitsBase<T> 
{

};

template <typename T, typename Arg>
struct TIsBitwiseConstructible
{
	enum 
	{ 
		Value = false 
	};
};

template <typename T>
struct TIsBitwiseConstructible<T, T>
{
	enum 
	{ 
		Value = TIsTriviallyCopyConstructible<T>::Value 
	};
};

template <typename T, typename U>
struct TIsBitwiseConstructible<const T, U> : TIsBitwiseConstructible<T, U>
{

};

template <typename T>
struct TIsBitwiseConstructible<const T*, T*>
{
	enum 
	{ 
		Value = true 
	};
};

template <> 
struct TIsBitwiseConstructible<uint8, int8>  
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template <> 
struct TIsBitwiseConstructible<int8, uint8>  
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template <> 
struct TIsBitwiseConstructible<uint16, int16> 
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template <> 
struct TIsBitwiseConstructible<int16, uint16> 
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template <> 
struct TIsBitwiseConstructible<uint32, int32> 
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template <> 
struct TIsBitwiseConstructible<int32, uint32> 
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template <> 
struct TIsBitwiseConstructible<uint64, int64> 
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template <> 
struct TIsBitwiseConstructible<int64, uint64> 
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template <typename T, bool TypeIsSmall>
struct TCallTraitsParamTypeHelper
{
	typedef const T& ParamType;
	typedef const T& ConstParamType;
};

template <typename T>
struct TCallTraitsParamTypeHelper<T, true>
{
	typedef const T ParamType;
	typedef const T ConstParamType;
};

template <typename T>
struct TCallTraitsParamTypeHelper<T*, true>
{
	typedef T* ParamType;
	typedef const T* ConstParamType;
};

template <typename T>
struct TCallTraitsBase
{
private:
	enum 
	{ 
		PassByValue = TOr<TAndValue<(sizeof(T) <= sizeof(void*)), TIsPODType<T>>, TIsArithmetic<T>, TIsPointer<T>>::Value 
	};

public:
	typedef T ValueType;
	typedef T& Reference;
	typedef const T& ConstReference;
	typedef typename TCallTraitsParamTypeHelper<T, PassByValue>::ParamType ParamType;
	typedef typename TCallTraitsParamTypeHelper<T, PassByValue>::ConstParamType ConstPointerType;
};

template <typename T>
struct TCallTraits : public TCallTraitsBase<T> 
{

};

template <typename T>
struct TCallTraits<T&>
{
	typedef T& ValueType;
	typedef T& Reference;
	typedef const T& ConstReference;
	typedef T& ParamType;
	typedef T& ConstPointerType;
};

template <typename T, size_t N>
struct TCallTraits<T [N]>
{
private:
	typedef T ArrayType[N];

public:
	typedef const T* ValueType;
	typedef ArrayType& Reference;
	typedef const ArrayType& ConstReference;
	typedef const T* const ParamType;
	typedef const T* const ConstPointerType;
};

template <typename T, size_t N>
struct TCallTraits<const T [N]>
{
private:
	typedef const T ArrayType[N];

public:
	typedef const T* ValueType;
	typedef ArrayType& Reference;
	typedef const ArrayType& ConstReference;
	typedef const T* const ParamType;
	typedef const T* const ConstPointerType;
};

template<typename T>
struct TTypeTraitsBase
{
	typedef typename TCallTraits<T>::ParamType ConstInitType;
	typedef typename TCallTraits<T>::ConstPointerType ConstPointerType;

	enum 
	{ 
		IsBytewiseComparable = TOr<TIsEnum<T>, TIsArithmetic<T>, TIsPointer<T>>::Value 
	};
};

template<typename T> struct TTypeTraits : public TTypeTraitsBase<T> 
{

};
