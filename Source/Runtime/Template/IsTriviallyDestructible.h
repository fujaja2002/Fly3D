#pragma once

namespace Fly3DPrivateIsTriviallyDestructible
{
	template <typename T, bool bIsTriviallyTriviallyDestructible = __is_enum(T)>
	struct TImpl
	{
		enum 
		{ 
			Value = true 
		};
	};

	template <typename T>
	struct TImpl<T, false>
	{
		enum 
		{ 
			Value = __has_trivial_destructor(T) 
		};
	};
}

template <typename T>
struct TIsTriviallyDestructible
{
	enum 
	{ 
		Value = Fly3DPrivateIsTriviallyDestructible::TImpl<T>::Value 
	};
};