#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Template/EnableIf.h"
#include "Runtime/Template/IsTriviallyDestructible.h"
#include "Runtime/Template/TypeTraits.h"

#include <memory>

template <typename ElementType>
FORCE_INLINE typename TEnableIf<!TIsTriviallyDestructible<ElementType>::Value>::Type DestructItem(ElementType* element)
{
	typedef ElementType DestructItemsElementTypeTypedef;
	element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
}

template <typename ElementType>
FORCE_INLINE typename TEnableIf<TIsTriviallyDestructible<ElementType>::Value>::Type DestructItem(ElementType* element)
{

}

template <typename ElementType, typename SizeType>
FORCE_INLINE typename TEnableIf<!TIsTriviallyDestructible<ElementType>::Value>::Type DestructItems(ElementType* element, SizeType count)
{
	while (count)
	{
		typedef ElementType DestructItemsElementTypeTypedef;

		element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
		++element;
		--count;
	}
}

template <typename ElementType, typename SizeType>
FORCE_INLINE typename TEnableIf<TIsTriviallyDestructible<ElementType>::Value>::Type DestructItems(ElementType* elements, SizeType count)
{

}

template <typename ElementType, typename SizeType>
FORCE_INLINE typename TEnableIf<TTypeTraits<ElementType>::IsBytewiseComparable, bool>::Type CompareItems(const ElementType* a, const ElementType* b, SizeType count)
{
	return !memcmp(a, b, sizeof(ElementType) * count);
}

template <typename ElementType, typename SizeType>
FORCE_INLINE typename TEnableIf<!TTypeTraits<ElementType>::IsBytewiseComparable, bool>::Type CompareItems(const ElementType* a, const ElementType* b, SizeType count)
{
	while (count)
	{
		if (!(*a == *b))
		{
			return false;
		}

		++a;
		++b;
		--count;
	}

	return true;
}